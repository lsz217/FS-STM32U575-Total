#ifndef __UART_RECEIVER_H
#define __UART_RECEIVER_H

#include "main.h"

// 接收器结构体：统一管理硬件句柄与缓冲区状态
typedef struct {
    UART_HandleTypeDef *huart_nfc;   // NFC串口 (UART4)
    UART_HandleTypeDef *huart_log;   // 调试串口 (USART1)
    uint8_t  *rx_buf;               // 环形缓冲区指针
    uint16_t  buf_size;             // 缓冲区大小
    uint16_t  read_idx;             // 处理进度指针
} NFC_Handler_t;

// 基础管理函数
void NFC_Init(NFC_Handler_t *handler, UART_HandleTypeDef *huart_nfc, UART_HandleTypeDef *huart_log, uint8_t *buf, uint16_t size);
//void NFC_Reset_Buffer(NFC_Handler_t *handler);

// 指令发送函数
void NFC_Send_SetKey(NFC_Handler_t *handler, const char *key);
void NFC_Send_ReadBlock(NFC_Handler_t *handler, uint8_t block);

// 解析函数：返回解析到的结果类型
typedef enum {
    NFC_RES_NONE = 0,
    NFC_RES_OK,
    NFC_RES_READ_SUCCESS,
    NFC_RES_ERROR,
		NFC_RES_UNKNOWN  // 新增：收到数据但无法识别
} NFC_Result_t;

NFC_Result_t NFC_Analyze_And_Forward(NFC_Handler_t *handler, uint16_t Size);

#endif
