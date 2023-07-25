/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-5-10      ShiHao       first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include<stdio.h>
#include<aht10.h>

#include <stdint.h>

#include <wlan_mgnt.h>
#include <wlan_prot.h>
#include <wlan_cfg.h>
#include <stdio.h>
#include <stdlib.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG

#include <rttlogo.h>

#define DBG_TAG "main"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>
#include<ntp.h>

#include <drv_matrix_led.h>

#define WLAN_SSID "ycx"
#define WLAN_PASSWORD "wsycxhh1"
#define NET_READY_TIME_OUT (rt_tick_from_millisecond(15 * 1000))

static void print_wlan_information(struct rt_wlan_info *info, int index);
static int wifi_autoconnect(void);

static struct rt_semaphore net_ready;
static struct rt_semaphore scan_done;

float humidity, temperature;

aht10_device_t dev; //定义器件变量

/* define LED  */
enum
{
    EXTERN_LED_0,
    EXTERN_LED_1,
    EXTERN_LED_2,
    EXTERN_LED_3,
    EXTERN_LED_4,
    EXTERN_LED_5,
    EXTERN_LED_6,
    EXTERN_LED_7,
    EXTERN_LED_8,
    EXTERN_LED_9,
    EXTERN_LED_10,
    EXTERN_LED_11,
    EXTERN_LED_12,
    EXTERN_LED_13,
    EXTERN_LED_14,
    EXTERN_LED_15,
    EXTERN_LED_16,
    EXTERN_LED_17,
    EXTERN_LED_18,
};

rt_thread_t led_matrix_thread;

static void led_matrix_example_entry()
{
    int h;
    time_t cur_time2;
    int count = 0;
    for (int i = 0; i <= 18; i++)
    {
        Set_LEDColor(i, DARK);
    }
    RGB_Reflash();

    while (1)
    {
        cur_time2 = ntp_get_time(RT_NULL); //定义时间变量
        char *tim2 = ctime((const time_t*) &cur_time2);
        char*hour[2] = { 0 };
        strncpy(hour, tim2 + 11, 2);

        if (atoi(hour) >= 12)
        {

            h = atoi(hour) - 12;

        }
        else
        {
            h = atoi(hour);
        }

        for (int i = EXTERN_LED_0; i <= h; i++)
        {
            switch (count)
            {
            case 0:
                Set_LEDColor(i, RED);
                Set_LEDColor(18, RED);
                Set_LEDColor(12, BLUE);
                Set_LEDColor(13, BLUE);
                Set_LEDColor(14, BLUE);
                Set_LEDColor(15, BLUE);
                Set_LEDColor(16, BLUE);
                Set_LEDColor(17, BLUE);
                break;
            case 1:
                Set_LEDColor(i, BLUE);
                Set_LEDColor(18, BLUE);
                Set_LEDColor(12, RED);
                Set_LEDColor(13, RED);
                Set_LEDColor(14, RED);
                Set_LEDColor(15, RED);
                Set_LEDColor(16, RED);
                Set_LEDColor(17, RED);
                break;
            default:
                return;
                break;
            }
            RGB_Reflash();
            rt_thread_delay(1000);
        }
        count = (count + 1) % 2;

    }
}

int led_matrix_cycle(void)
{
    led_matrix_thread = rt_thread_create("led matrix demo", led_matrix_example_entry, RT_NULL, 2 * 1024,
    RT_THREAD_PRIORITY_MAX / 4 - 1, 5);
    if (led_matrix_thread == RT_NULL)
    {
        rt_kprintf("led matrix demo thread creat failed!\n");
        return 0;
    }
    rt_thread_startup(led_matrix_thread);

    return 0;
}

static void onenet_upload_entry(void)
{

    while (1)
    {

        if (onenet_mqtt_upload_digit("temperature", temperature) < 0)
        {
            LOG_E("upload has an error, stop uploading");
            break;
        }
        else
        {
            LOG_D("buffer : {\"temperature\":%d}", (int )temperature);
        }

        rt_thread_delay(rt_tick_from_millisecond(5000));

        rt_thread_mdelay(1000);

        if (onenet_mqtt_upload_digit("humidity", humidity) < 0)
        {
            LOG_E("upload has an error, stop uploading");
            break;
        }
        else
        {
            LOG_D("buffer : {\"humidity\":%d}", (int )humidity);
        }

        rt_thread_delay(rt_tick_from_millisecond(5000));
    }
}

int onenet_upload_cycle(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("onenet_send", onenet_upload_entry,
    RT_NULL, 2 * 1024,
    RT_THREAD_PRIORITY_MAX / 4 - 1, 5);
    if (tid)
    {
        rt_thread_startup(tid);
    }

    return 0;
}

void init_sensor(void)
{

    //初始化温度传感器
    /* 总线名称 */
    const char *i2c_bus_name = "i2c3";
    int count = 0;

    /* 等待传感器正常工作 */
    rt_thread_mdelay(2000);

    /* 初始化 aht10 */
    dev = aht10_init(i2c_bus_name);
    if (dev == RT_NULL)
    {
        LOG_E(" The sensor initializes failure");
        return 0;
    }
}

static void get_temp_humi_entry(void)
{
    while (1)
    {/* 读取湿度 */
        humidity = aht10_read_humidity(dev);
        //LOG_D("humidity   : %d.%d %%", (int )humidity, (int)(humidity * 10) % 10);
        rt_thread_mdelay(1000);
        /* 读取温度 */
        temperature = aht10_read_temperature(dev);
        //LOG_D("temperature: %d.%d", (int )temperature, (int)(temperature * 10) % 10);
    }
}

int get_temp_humi_cycle(void)
{

    rt_thread_t tid2;

    tid2 = rt_thread_create(" get_temp_humi", get_temp_humi_entry,
    RT_NULL, 2 * 1024,
    RT_THREAD_PRIORITY_MAX / 4 - 1, 5);
    if (tid2)
    {
        rt_thread_startup(tid2);
    }

    return 0;
}

static void LCD_FLASH_entry(void)
{

    while (1)
    {
        //屏幕显示数据
        time_t cur_time = ntp_get_time(RT_NULL); //定义时间变量
        char *tim = ctime((const time_t*) &cur_time);
        char *day[3] = { 0 };
        char *month[3] = { 0 };
        char*date[2] = { 0 };
        char*time[8] = { 0 };
        char*year[4] = { 0 };

        strncpy(day, tim, 3);
        strncpy(month, tim + 4, 3);
        strncpy(date, tim + 8, 2);
        strncpy(time, tim + 11, 8);
        strncpy(year, tim + 20, 4);

        lcd_show_string(15, 20 + 16 + 24, 24, year);
        lcd_show_string(90, 20 + 16 + 24, 24, month);
        lcd_show_string(165, 20 + 16 + 24, 24, date);

        lcd_show_string(30, 20 + 16 + 24 + 24, 24, day);
        lcd_show_string(90, 20 + 16 + 24 + 24, 24, time);

        lcd_show_string(0, 20 + 16 + 42 + 24 + 24, 24, "TEMPERATUR:");
        lcd_show_num(0, 20 + 16 + 24 + 24 + 24 + 24 + 24, (int) temperature, sizeof(temperature), 24);
        lcd_show_string(0, 20 + 16 + 24 + 24 + 24 + 24 + 24 + 24, 24, " HUMIDITY:");
        lcd_show_num(0, 20 + 16 + 24 + 24 + 24 + 24 + 24 + 24 + 24, (int) humidity, sizeof(humidity), 24);
        rt_thread_mdelay(1000);
    }

}

int LCD_FLASH_cycle(void)
{

    rt_thread_t tid3;

    tid3 = rt_thread_create(" LCD_FLASH", LCD_FLASH_entry,
    RT_NULL, 2 * 1024,
    RT_THREAD_PRIORITY_MAX / 4 - 1, 5);
    if (tid3)
    {
        rt_thread_startup(tid3);
    }

    return 0;
}

void wlan_scan_report_hander(int event, struct rt_wlan_buff *buff, void *parameter)
{
    struct rt_wlan_info *info = RT_NULL;
    int index = 0;
    RT_ASSERT(event == RT_WLAN_EVT_SCAN_REPORT);
    RT_ASSERT(buff != RT_NULL);
    RT_ASSERT(parameter != RT_NULL);

    info = (struct rt_wlan_info *) buff->data;
    index = *((int *) (parameter));
    print_wlan_information(info, index);
    ++*((int *) (parameter));
}

void wlan_scan_done_hander(int event, struct rt_wlan_buff *buff, void *parameter)
{
    RT_ASSERT(event == RT_WLAN_EVT_SCAN_DONE);
    rt_sem_release(&scan_done);
}

void wlan_ready_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_sem_release(&net_ready);
}

/* 断开连接回调函数 */
void wlan_station_disconnect_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    LOG_I("disconnect from the network!");
}

static void wlan_connect_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("%s\n", __FUNCTION__);
    if ((buff != RT_NULL) && (buff->len == sizeof(struct rt_wlan_info)))
    {
        rt_kprintf("ssid : %s \n", ((struct rt_wlan_info *) buff->data)->ssid.val);
    }
}

static void wlan_connect_fail_handler(int event, struct rt_wlan_buff *buff, void *parameter)
{
    rt_kprintf("%s\n", __FUNCTION__);
    if ((buff != RT_NULL) && (buff->len == sizeof(struct rt_wlan_info)))
    {
        rt_kprintf("ssid : %s \n", ((struct rt_wlan_info *) buff->data)->ssid.val);
    }
}

void wifi_connect(void)
{
    static int i = 0;
    int result = RT_EOK;
    struct rt_wlan_info info;

    /* 等待 500 ms 以便 wifi 完成初始化 */
    rt_thread_mdelay(500);
    /* 热点连接 */
    LOG_D("start to connect ap ...");
    rt_sem_init(&net_ready, "net_ready", 0, RT_IPC_FLAG_FIFO);

    /* 注册 wlan ready 回调函数 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, wlan_ready_handler, RT_NULL);
    /* 注册 wlan 断开回调函数 */
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_DISCONNECTED, wlan_station_disconnect_handler, RT_NULL);
    /* 同步连接热点 */
    result = rt_wlan_connect(WLAN_SSID, WLAN_PASSWORD);
    if (result == RT_EOK)
    {
        rt_memset(&info, 0, sizeof(struct rt_wlan_info));
        /* 获取当前连接热点信息 */
        rt_wlan_get_info(&info);
        LOG_D("station information:");
        print_wlan_information(&info, 0);
        /* 等待成功获取 IP */
        result = rt_sem_take(&net_ready, NET_READY_TIME_OUT);
        if (result == RT_EOK)
        {
            LOG_D("networking ready!");
            msh_exec("ifconfig", rt_strlen("ifconfig"));
        }
        else
        {
            LOG_D("wait ip got timeout!");
        }
        /* 回收资源 */
        rt_wlan_unregister_event_handler(RT_WLAN_EVT_READY);
        rt_sem_detach(&net_ready);
    }
    else
    {
        LOG_E("The AP(%s) is connect failed!", WLAN_SSID);
    }

    rt_thread_mdelay(5000);

    /* 自动连接 */
    LOG_D("start to autoconnect ...");
    wifi_autoconnect();
}

int main(void)
{
    wifi_connect(); //自动连接wifi

    onenet_mqtt_init(); //初始化mqtt协议

    init_sensor(); //初始化温湿度传感器
    get_temp_humi_cycle(); //获取环境温湿度
    weather();
    weatherdraw();
    LCD_FLASH_cycle(); //刷新LCD屏幕

    led_matrix_cycle();

    onenet_upload_cycle(); //上传onenet

    return 0;
}

static void print_wlan_information(struct rt_wlan_info *info, int index)
{
    char *security;

    if (index == 0)
    {
        rt_kprintf("             SSID                      MAC            security    rssi chn Mbps\n");
        rt_kprintf("------------------------------- -----------------  -------------- ---- --- ----\n");
    }

    {
        rt_kprintf("%-32.32s", &(info->ssid.val[0]));
        rt_kprintf("%02x:%02x:%02x:%02x:%02x:%02x  ", info->bssid[0], info->bssid[1], info->bssid[2], info->bssid[3],
                info->bssid[4], info->bssid[5]);
        switch (info->security)
        {
        case SECURITY_OPEN:
            security = "OPEN";
            break;
        case SECURITY_WEP_PSK:
            security = "WEP_PSK";
            break;
        case SECURITY_WEP_SHARED:
            security = "WEP_SHARED";
            break;
        case SECURITY_WPA_TKIP_PSK:
            security = "WPA_TKIP_PSK";
            break;
        case SECURITY_WPA_AES_PSK:
            security = "WPA_AES_PSK";
            break;
        case SECURITY_WPA2_AES_PSK:
            security = "WPA2_AES_PSK";
            break;
        case SECURITY_WPA2_TKIP_PSK:
            security = "WPA2_TKIP_PSK";
            break;
        case SECURITY_WPA2_MIXED_PSK:
            security = "WPA2_MIXED_PSK";
            break;
        case SECURITY_WPS_OPEN:
            security = "WPS_OPEN";
            break;
        case SECURITY_WPS_SECURE:
            security = "WPS_SECURE";
            break;
        default:
            security = "UNKNOWN";
            break;
        }
        rt_kprintf("%-14.14s ", security);
        rt_kprintf("%-4d ", info->rssi);
        rt_kprintf("%3d ", info->channel);
        rt_kprintf("%4d\n", info->datarate / 1000000);
    }
}

static int wifi_autoconnect(void)
{
    /* Configuring WLAN device working mode */
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);
    /* Start automatic connection */
    rt_wlan_config_autoreconnect(RT_TRUE);
    /* register event */
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_CONNECTED, wlan_connect_handler, RT_NULL);
    rt_wlan_register_event_handler(RT_WLAN_EVT_STA_CONNECTED_FAIL, wlan_connect_fail_handler, RT_NULL);
    return 0;
}

