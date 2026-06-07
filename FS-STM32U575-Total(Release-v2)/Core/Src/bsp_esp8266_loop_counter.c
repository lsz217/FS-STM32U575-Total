// 备用方案：纯循环计数器版本（如果 HAL_GetTick 有问题用这个）
// 把这个添加到 bsp_esp8266.c，注释掉原来的 OneNet_Report_Task

static uint32_t loop_counter = 0;

void OneNet_Report_Task(void)
{
    loop_counter++;
    char payload[1024];

    // S0: AT测试 (循环1)
    if(gOneNet_InitStage == 0 && loop_counter == 1)
    {
        printf("[ON] S0: AT> loop=%lu\r\n", loop_counter);
        ESP8266_Send_AT_Cmd("AT", "OK", NULL, 100);
        gOneNet_InitStage = 1;
        return;
    }

    // S1: 设置模式 (循环500 ≈ 1秒)
    if(gOneNet_InitStage == 1 && loop_counter >= 500)
    {
        printf("[ON] S1: MODE> loop=%lu\r\n", loop_counter);
        ESP8266_Send_AT_Cmd("AT+CWMODE=1", "OK", "no change", 500);
        gOneNet_InitStage = 2;
        return;
    }

    // S2: 连接WiFi (循环1000)
    if(gOneNet_InitStage == 2 && loop_counter >= 1000)
    {
        printf("[ON] S2: WiFi> loop=%lu\r\n", loop_counter);
        ESP8266_JoinAP(User_ESP8266_SSID, User_ESP8266_PWD);
        gOneNet_InitStage = 3;
        return;
    }

    // S3: 清理MQTT (循环8000 ≈ WiFi连接完成后)
    if(gOneNet_InitStage == 3 && loop_counter >= 8000)
    {
        printf("[ON] S3: CLEAN> loop=%lu\r\n", loop_counter);
        OneNet_MQTT_Clear();
        gOneNet_InitStage = 4;
        return;
    }

    // S4: 配置MQTT (循环8500)
    if(gOneNet_InitStage == 4 && loop_counter >= 8500)
    {
        printf("[ON] S4: CFG> loop=%lu\r\n", loop_counter);
        OneNet_MQTT_Config(ONENET_CLIENT_ID, ONENET_PRODUCT_ID, ONENET_TOKEN);
        gOneNet_InitStage = 5;
        return;
    }

    // S5: 连接OneNet (循环10000)
    if(gOneNet_InitStage == 5 && loop_counter >= 10000)
    {
        printf("[ON] S5: CONN> loop=%lu\r\n", loop_counter);
        if(OneNet_MQTT_Connect(ONENET_SERVER_IP, ONENET_SERVER_PORT))
        {
            printf("[ON] === CONNECTED! ===\r\n");
            gOneNet_Connected = 1;
            gOneNet_InitStage = 100;
        }
        else
        {
            printf("[ON] Connect failed, retry from S3...\r\n");
            gOneNet_InitStage = 3;  // 重试
            loop_counter = 8000;    // 回到S3时间点
        }
        return;
    }

    // S100: 已连接，每5000循环(约10秒)上报一次
    if(gOneNet_InitStage == 100 && gOneNet_Connected)
    {
        if(loop_counter % 5000 == 0)
        {
            simulate_sensor_data();
            build_onenet_payload(payload, sizeof(payload));

            int payload_len = strlen(payload);
            printf("[ON] REP> len=%d: %s\r\n", payload_len, payload);

            if(OneNet_Publish_Property(ONENET_TOPIC_PROP_POST, payload, payload_len))
            {
                printf("[ON] === REP OK! ===\r\n");
            }
            else
            {
                printf("[ON] REP FAIL!\r\n");
            }
        }
    }
}
