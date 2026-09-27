#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <MQTTClient.h>
#include <cjson/cJSON.h>

/* ==================== OneNET 设备配置 ==================== */
#define PRODUCT_ID   "57OIJV9Jlj"
#define DEVICE_NAME  "device01"

/* 产品级 Token（用于身份鉴权） */
#define TOKEN "version=2018-10-31&res=products%2F57OIJV9Jlj&et=1804125908&method=md5&sign=QBqx42o9kfNg%2FAqVZGAP8Q%3D%3D"

/* MQTT Broker 连接配置 */
#define HOSTNAME     "mqtts.heclouds.com"
#define PORT         1883
#define KEEPALIVE    60
#define QOS          1
#define TIMEOUT      10000L   /* 发布超时 10 秒 */

/* 属性上报 Topic（OneNET 物模型标准格式） */
/* 格式: $sys/{pid}/{device-name}/thing/property/post */
static char pub_topic[256];

/* ==================== 辅助函数 ==================== */

/* 获取当前毫秒级时间戳，用作消息 ID */
static long long current_millis(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

/* 延迟指定毫秒（替代 Python 的 time.sleep） */
static void sleep_ms(int milliseconds)
{
    sleep(milliseconds * 1000);
}

/* ==================== 连接丢失回调 ==================== */
static void connlost(void *context, char *cause)
{
    (void)context;
    printf("⚠️  连接丢失: %s\n", cause ? cause : "未知原因");
}

/* ==================== 主业务逻辑 ==================== */
int main(void)
{
    /* 构造上报 Topic */
    snprintf(pub_topic, sizeof(pub_topic),
             "$sys/%s/%s/thing/property/post",
             PRODUCT_ID, DEVICE_NAME);

    /* 初始化 MQTT 客户端 */
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    /* 创建客户端，Client ID 使用设备名称 */
    rc = MQTTClient_create(&client, "tcp://" HOSTNAME ":" "1883",
                           DEVICE_NAME,
                           MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("❌ 创建 MQTT 客户端失败: %d\n", rc);
        return EXIT_FAILURE;
    }

    /* 设置连接丢失回调 */
    MQTTClient_setCallbacks(client, NULL, connlost, NULL, NULL);

    /* 配置连接参数：用户名 = product_id，密码 = token */
    conn_opts.keepAliveInterval = KEEPALIVE;
    conn_opts.cleansession = 1;
    conn_opts.username = PRODUCT_ID;
    conn_opts.password = TOKEN;

    /* 建立连接 */
    rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("❌ OneNET 连接失败: %d\n", rc);
        MQTTClient_destroy(&client);
        return EXIT_FAILURE;
    }
    printf("✅ OneNET 连接成功\n");

    /* 等待 1 秒确保连接建立完成 */
    sleep_ms(1000);

    /* 循环上报 10 组模拟传感器数据 */
    for (int i = 0; i < 10; i++) {
        /* 构造模拟数据值 */
        double temp       = 20.5 + i;   /* 温度: 20.5 ~ 29.5 */
        int    humidity   = 40 + i;     /* 湿度: 40 ~ 49   */
        int    brightness = 50 + i;     /* 光照度: 50 ~ 59 */

        /* 用 cJSON 构造 OneNET 物模型属性上报 JSON 载荷 */
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "id",
                                (char[]){0}[0] ? "" : ""); /* 占位，下面重新赋值 */

        /* id 用毫秒级时间戳字符串 */
        char id_str[32];
        snprintf(id_str, sizeof(id_str), "%lld", current_millis());
        cJSON_ReplaceItemInObject(root, "id", cJSON_CreateString(id_str));

        cJSON_AddStringToObject(root, "version", "1.0");

        cJSON *params = cJSON_CreateObject();

        cJSON *temp_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(temp_obj, "value", temp);
        cJSON_AddItemToObject(params, "Temperature", temp_obj);

        cJSON *humi_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(humi_obj, "value", humidity);
        cJSON_AddItemToObject(params, "Humidity", humi_obj);

        cJSON *bri_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(bri_obj, "value", brightness);
        cJSON_AddItemToObject(params, "Brightness", bri_obj);

        cJSON_AddItemToObject(root, "params", params);

        /* 序列化为字符串 */
        char *payload = cJSON_PrintUnformatted(root);

        /* 填充发布消息结构体 */
        pubmsg.payload    = payload;
        pubmsg.payloadlen = (int)strlen(payload);
        pubmsg.qos        = QOS;
        pubmsg.retained   = 0;

        /* 发布消息，QoS=1 */
        rc = MQTTClient_publishMessage(client, pub_topic, &pubmsg, &token);
        if (rc != MQTTCLIENT_SUCCESS) {
            printf("📤 上报失败: %d\n", rc);
        } else {
            /* 等待 QoS=1 的 PUBACK 确认 */
            rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
            if (rc == MQTTCLIENT_SUCCESS) {
                printf("📤 上报成功: %s\n", payload);
            } else {
                printf("📤 上报超时或失败: %d\n", rc);
            }
        }

        /* 释放内存 */
        cJSON_free(payload);
        cJSON_Delete(root);

        /* 每 2 秒上报一次，避免触发平台限流 */
        sleep_ms(2000);
    }

    /* 优雅断开连接并释放资源 */
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    printf("🔌 已断开连接并释放资源\n");

    return EXIT_SUCCESS;
}