/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <quackmore-ff@yahoo.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return. Quackmore
 * ----------------------------------------------------------------------------
 */

// SDK includes
extern "C"
{
#include "mem.h"
#include "user_interface.h"
#include "app_event_codes.h"
#include "drivers_dio_task.h"
#include "esp8266_io.h"
}

#include "app.hpp"
#include "espbot.hpp"
#include "espbot_cron.hpp"
#include "espbot_diagnostic.hpp"
#include "espbot_timedate.hpp"
#include "espbot_utils.hpp"
#include "drivers.hpp"
#include "drivers_dht.hpp"

/*
 *  APP_RELEASE is coming from git
 *  'git --no-pager describe --tags --always --dirty'
 *  and is generated by the Makefile
 */

#ifndef APP_RELEASE
#define APP_RELEASE "Unavailable"
#endif

// required by espbot (see espbot.cpp)
char *app_name = "ESPBOT";
char *app_release = APP_RELEASE;

// Dht *dht22;

static void heartbeat_cb(void *param)
{
    DEBUG("ESPBOT HEARTBEAT: ---------------------------------------------------");
    uint32 current_timestamp = timedate_get_timestamp();
    signed char tz = timedate_get_timezone();
    DEBUG("ESPBOT HEARTBEAT: [%d] [UTC%c%d] %s",
          current_timestamp,
          (tz >= 0 ? '+' : ' '),
          tz,
          timedate_get_timestr(current_timestamp));
    DEBUG("ESPBOT HEARTBEAT: Available heap size: %d", system_get_free_heap_size());
}

void app_init_before_wifi(void)
{
    init_dio_task();
    // dht22 = new Dht(ESPBOT_D2, DHT22, 1000, 2000, 0, 10);
    // cron_add_job(CRON_STAR, CRON_STAR, CRON_STAR, CRON_STAR, CRON_STAR, heartbeat_cb, NULL);
    cron_add_job(0, CRON_STAR, CRON_STAR, CRON_STAR, CRON_STAR, heartbeat_cb, NULL);
    cron_add_job(10, CRON_STAR, CRON_STAR, CRON_STAR, CRON_STAR, heartbeat_cb, NULL);
    cron_add_job(20, CRON_STAR, CRON_STAR, CRON_STAR, CRON_STAR, heartbeat_cb, NULL);
    cron_add_job(30, CRON_STAR, CRON_STAR, CRON_STAR, CRON_STAR, heartbeat_cb, NULL);
    cron_add_job(40, CRON_STAR, CRON_STAR, CRON_STAR, CRON_STAR, heartbeat_cb, NULL);
    cron_add_job(50, CRON_STAR, CRON_STAR, CRON_STAR, CRON_STAR, heartbeat_cb, NULL);
    cron_sync();
}

void app_init_after_wifi(void)
{
    static bool first_time = true;
    if (first_time)
    {
        first_time = false;
        // test if sntp get_timestamp works fine
        uint32 timestamp = timedate_get_timestamp();
        DEBUG("INIT AFTER WIFI");
        fs_printf("=======> current timestamp %s\n", timedate_get_timestr(timestamp));
    }
}

void app_deinit_on_wifi_disconnect()
{
}

char *app_info_json_stringify(char *dest, int len)
{
   // {"device_name":"","chip_id":"","app_name":"","app_version":"","espbot_version":"","api_version":"","drivers_version":"","sdk_version":"","boot_version":""}
    int msg_len = 155 +
                  os_strlen(espbot_get_name()) +
                  10 +
                  os_strlen(app_name) +
                  os_strlen(app_release) +
                  os_strlen(espbot_get_version()) +
                  os_strlen(f_str(API_RELEASE)) +
                  os_strlen(drivers_release) +
                  os_strlen(system_get_sdk_version()) +
                  10 +
                  1;
    char *msg;
    if (dest == NULL)
    {
        msg = new char[msg_len];
        if (msg == NULL)
        {
            dia_error_evnt(APP_INFO_STRINGIFY_HEAP_EXHAUSTED, msg_len);
            ERROR("app_info_json_stringify heap exhausted [%d]", msg_len);
            return NULL;
        }
    }
    else
    {
        msg = dest;
        if (len < msg_len)
        {
            *msg = 0;
            return msg;
        }
    }
    fs_sprintf(msg,
               "{\"device_name\":\"%s\",\"chip_id\":\"%d\",\"app_name\":\"%s\",",
               espbot_get_name(),
               system_get_chip_id(),
               app_name);
    fs_sprintf((msg + os_strlen(msg)),
               "\"app_version\":\"%s\",\"espbot_version\":\"%s\",",
               app_release,
               espbot_get_version());
    fs_sprintf(msg + os_strlen(msg),
               "\"api_version\":\"%s\",\"drivers_version\":\"%s\",",
               f_str(API_RELEASE),
               drivers_release);
    fs_sprintf(msg + os_strlen(msg),
               "\"sdk_version\":\"%s\",\"boot_version\":\"%d\"}",
               system_get_sdk_version(),
               system_get_boot_version());
     return msg;
}