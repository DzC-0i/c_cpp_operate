/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-04-18 12:37:34
 * @LastEditTime: 2020-06-08 20:32:33
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include "mqttclient.h"

extern const char *test_ca_get();

static void interceptor_handler(void *client, message_data_t *msg)
{
    (void)client;
    MQTT_LOG_I("-----------------------------------------------------------------------------------");
    MQTT_LOG_I("%s:%d %s()...\ntopic: %s\nmessage:%s", __FILE__, __LINE__, __FUNCTION__, msg->topic_name, (char *)msg->message->payload);
    MQTT_LOG_I("-----------------------------------------------------------------------------------");
}

void *mqtt_publish_thread(void *arg)
{
    mqtt_client_t *client = (mqtt_client_t *)arg;

    char buf[100] = {0};
    mqtt_message_t msg;
    memset(&msg, 0, sizeof(msg));
    sprintf(buf, "welcome to mqttclient, this is a publish test...");

    msg.qos = 0;
    msg.payload = (void *)buf;
    while (1)
    {
        sprintf(buf, "{\"id\":%d,\"dp\": {\"temperatrue\": [{\"v\": %f}],\"power\": [{\"v\": %f}]}}", 123, 12.5, 16.0);
        mqtt_publish(client, "$sys/A41y1rte9t/1_test_stream/dp/post/json", &msg);
        sleep(4);
    }
}

int main(void)
{
    int res;
    pthread_t thread1;
    mqtt_client_t *client = NULL;

    printf("\nwelcome to mqttclient test...\n");

    mqtt_log_init();

    client = mqtt_lease();

    mqtt_set_port(client, "1883");
    mqtt_set_host(client, "mqtts.heclouds.com");
    mqtt_set_client_id(client, "1_test_stream");
    mqtt_set_user_name(client, "A41y1rte9t");
    mqtt_set_password(client, "version=2018-10-31&res=products%2FA41y1rte9t%2Fdevices%2F1_test_stream&et=1772867940&method=md5&sign=dddRjT7feylAxQsYV6y%2Ffg%3D%3D");
    mqtt_set_clean_session(client, 1);

    mqtt_connect(client);

    mqtt_subscribe(client, "$sys/A41y1rte9t/1_test_stream/dp/post/json/+", QOS0, NULL);
    mqtt_subscribe(client, "$sys/A41y1rte9t/1_test_stream/cmd/#", QOS0, NULL);

    mqtt_set_interceptor_handler(client, interceptor_handler); // set interceptor handler

    res = pthread_create(&thread1, NULL, mqtt_publish_thread, client);
    if (res != 0)
    {
        MQTT_LOG_E("create mqtt publish thread fail");
        exit(res);
    }

    while (1)
    {
        sleep(100);
    }
}
