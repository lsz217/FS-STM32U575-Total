// bsp_esp8266.c - OneNet 非阻塞任务（终极修复版）
// 不再使用 HAL_GetTick()，改用循环计数器

// OneNet 非阻塞任务（在主循环中每循环调用一次）
void OneNet_Report_Task(void)
{
    static uint32_t loop_count = 0;  // 主循环计数器
    loop_count++;

    char payload[512];
    static uint8_t last_stage = 255;

    // 打印阶段变化
    if(gOneNet_InitStage != last_stage)
    {
        printf("[OneNet] ===> Enter Stage %d, loop=%lu\r\n", gOneNet_InitStage, loop_count);
        last_stage = gOneNet_InitStage;
    }

    // 阶段0: 测试AT (第1次调用)
    if(gOneNet_InitStage == 0 && loop_count >= 1)
    {
        printf("[OneNet] S0: AT> loop=%lu\r\n", loop_count);
        ESP8266_USART("AT\r\n");
        gOneNet_InitStage = 1;
        return;
    }

    // 阶段1: 设置STA模式 (等待约1000个循环 ≈ 1秒)
    if(gOneNet_InitStage == 1 && loop_count >= 1000)
    {
        printf("[OneNet] S1: CWMODE> loop=%lu\r\n", loop_count);
        ESP8266_USART("AT+CWMODE=1\r\n");
        gOneNet_InitStage = 2;
        return;
    }

    // 阶段2: 连接WiFi (等待约2500个循环)
    if(gOneNet_InitStage == 2 && loop_count >= 2500)
    {
        printf("[OneNet] S2: WiFi> [%s] loop=%lu\r\n", User_ESP8266_SSID, loop_count);
        char cmd[128];
        sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", User_ESP8266_SSID, User_ESP8266_PWD);
        ESP8266_USART("%s", cmd);
        gOneNet_InitStage = 3;
        return;
    }

    // 阶段3: 配置MQTT - 清理 (等待约15000个循环 ≈ 10秒WiFi连接时间)
    if(gOneNet_InitStage == 3 && loop_count >= 15000)
    {
        printf("[OneNet] S3: MQTTCLEAN> loop=%lu\r\n", loop_count);
        ESP8266_USART("AT+MQTTCLEAN=0\r\n");
        gOneNet_InitStage = 30;  // 子阶段：等待100ms
        return;
    }

    // 阶段30: 等待后发送 USERCFG (等待约100个循环)
    if(gOneNet_InitStage == 30 && loop_count >= 15100)
    {
        char cmd[512];
        sprintf(cmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
                ONENET_CLIENT_ID, ONENET_PRODUCT_ID, ONENET_TOKEN);
        ESP8266_USART("%s", cmd);
        printf("[OneNet] S3: USERCFG> loop=%lu\r\n", loop_count);
        gOneNet_InitStage = 4;
        return;
    }

    // 阶段4: 连接OneNet (等待约2000个循环)
    if(gOneNet_InitStage == 4 && loop_count >= 17100)
    {
        printf("[OneNet] S4: CONNECT> loop=%lu\r\n", loop_count);
        char cmd[128];
        sprintf(cmd, "AT+MQTTCONN=0,\"%s\",%d,1\r\n", ONENET_SERVER_IP, ONENET_SERVER_PORT);
        ESP8266_USART("%s", cmd);
        gOneNet_InitStage = 5;
        gOneNet_Connected = 1;
        printf("[OneNet] S4: === CONNECTED! ===\r\n");
        return;
    }

    // 阶段5: 已连接，定期上报 (每5000个循环上报一次)
    if(gOneNet_InitStage == 5 && gOneNet_Connected)
    {
        if(loop_count % 5000 == 0)  // 每5000个循环
        {
            simulate_sensor_data();
            build_onenet_payload(payload, sizeof(payload));
            char cmd[256];
            int len = strlen(payload);
            sprintf(cmd, "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", ONENET_TOPIC_PROP_POST, len);
            ESP8266_USART("%s", cmd);
            printf("[OneNet] S5: PUBRAW> loop=%lu\r\n", loop_count);
            gOneNet_InitStage = 50;  // 子阶段：等待后发送payload
            return;
        }
    }

    // 阶段50: 发送payload (下一个循环)
    if(gOneNet_InitStage == 50)
    {
        build_onenet_payload(payload, sizeof(payload));
        ESP8266_USART("%s", payload);
        printf("[OneNet] S5: Reported T=%.1f,H=%d%% loop=%lu\r\n",
               car_temp/10.0f, car_humidity, loop_count);
        gOneNet_InitStage = 5;  // 回到阶段5继续等待
        return;
    }
}
