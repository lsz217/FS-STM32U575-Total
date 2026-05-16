#include "uart_receiver.h"
#include <string.h>
#include <stdio.h>

static char nfc_tx_temp[128]; // 静态区，防止DMA发送过程中局部变量销毁

// 逻辑解析缓冲区：大小应至少为最大预期回复长度的两倍（建议 256 字节）
#define LOGIC_BUF_SIZE 256
static char logic_buf[LOGIC_BUF_SIZE];
static uint16_t logic_data_len = 0;

void NFC_Init(NFC_Handler_t *handler, UART_HandleTypeDef *huart_nfc, UART_HandleTypeDef *huart_log, uint8_t *buf, uint16_t size) {
    handler->huart_nfc = huart_nfc;
    handler->huart_log = huart_log;
    handler->rx_buf = buf;
    handler->buf_size = size;
    handler->read_idx = 0;
    
		// 清除逻辑缓冲区状态
    memset(logic_buf, 0, LOGIC_BUF_SIZE);
    logic_data_len = 0;
	
    HAL_UARTEx_ReceiveToIdle_DMA(huart_nfc, buf, size);
}

void NFC_Send_SetKey(NFC_Handler_t *handler, const char *key) {
    snprintf(nfc_tx_temp, sizeof(nfc_tx_temp), "AT+KEYA=1,%s\r\n", key);
    HAL_UART_Transmit_DMA(handler->huart_nfc, (uint8_t *)nfc_tx_temp, strlen(nfc_tx_temp));
}

void NFC_Send_ReadBlock(NFC_Handler_t *handler, uint8_t block) {
    snprintf(nfc_tx_temp, sizeof(nfc_tx_temp), "AT+READ=1,%d\r\n", block);
    HAL_UART_Transmit_DMA(handler->huart_nfc, (uint8_t *)nfc_tx_temp, strlen(nfc_tx_temp));
}

NFC_Result_t NFC_Analyze_And_Forward(NFC_Handler_t *handler, uint16_t Size) {
    // 1. 计算并追加数据到 logic_buf (保持原有 DMA 环形处理逻辑)
    uint16_t start = handler->read_idx;
    uint16_t new_len = (Size >= start) ? (Size - start) : (handler->buf_size - start + Size);
    if (new_len == 0) return NFC_RES_NONE;

    if (logic_data_len + new_len < LOGIC_BUF_SIZE - 1) {
        if (Size >= start) {
            memcpy(&logic_buf[logic_data_len], &handler->rx_buf[start], new_len);
        } else {
            uint16_t first_part = handler->buf_size - start;
            memcpy(&logic_buf[logic_data_len], &handler->rx_buf[start], first_part);
            memcpy(&logic_buf[logic_data_len + first_part], &handler->rx_buf[0], Size);
        }
        logic_data_len += new_len;
    } else {
        logic_data_len = 0; 
        memset(logic_buf, 0, LOGIC_BUF_SIZE);
    }
    handler->read_idx = Size;

    // 2. 严谨的逻辑匹配
    NFC_Result_t result = NFC_RES_NONE;
    char *match_ptr = NULL;
    uint16_t consumed_len = 0;

    // A. 匹配读块回复 (协议头 "+READ=" + 16字节数据 + \r\n，总长 24 字节)
    if ((match_ptr = strstr(logic_buf, "+READ=")) != NULL) {
        uint16_t head_offset = (uint16_t)(match_ptr - logic_buf);
        // 检查缓冲区是否有足够长度：6(头) + 16(数据) + 2(\r\n) = 24
        if (logic_data_len >= head_offset + 24) {
            // 获取数据区指针 (match_ptr + 6)
            uint8_t *data_ptr = (uint8_t *)(match_ptr + 6);
            
            // 校验特征值 (根据你提供的 HEX: 53 6D 8C 1A 10 32 ...)
            if (data_ptr[0] == 0x53 && data_ptr[1] == 0x6D && data_ptr[2] == 0x8C &&
                data_ptr[3] == 0x1A && data_ptr[4] == 0x10 && data_ptr[5] == 0x32) {
                result = NFC_RES_READ_SUCCESS;
            } else {
                result = NFC_RES_UNKNOWN; // 数据内容不匹配
            }
            consumed_len = head_offset + 24;
        }
    } 
    // B. 匹配 OK (用于 SetKey 确认)
    else if ((match_ptr = strstr(logic_buf, "OK")) != NULL) {
        result = NFC_RES_OK;
        consumed_len = (uint16_t)(match_ptr - logic_buf) + 2;
    } 
    // C. 匹配错误
    else if ((match_ptr = strstr(logic_buf, "+ERROR")) != NULL) {
        result = NFC_RES_ERROR;
        consumed_len = (uint16_t)(match_ptr - logic_buf) + 6;
    }
    // D. 判定超时或无效碎片 (针对 Read 状态特殊处理，避免二进制 0x0A 误触发)
    else if (logic_data_len > (LOGIC_BUF_SIZE * 3 / 4)) {
        result = NFC_RES_UNKNOWN;
        consumed_len = logic_data_len;
    }

    // 3. 清理缓冲区
    if (result != NFC_RES_NONE) {
        if (consumed_len > 0 && consumed_len <= logic_data_len) {
            uint16_t remaining_len = logic_data_len - consumed_len;
            if (remaining_len > 0) {
                memmove(logic_buf, &logic_buf[consumed_len], remaining_len);
                logic_data_len = remaining_len;
            } else {
                logic_data_len = 0;
            }
        } else {
            logic_data_len = 0;
        }
        memset(&logic_buf[logic_data_len], 0, LOGIC_BUF_SIZE - logic_data_len);
    }
    
    return result;
}
