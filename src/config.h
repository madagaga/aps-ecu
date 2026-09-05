#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <LittleFS.h>
#include <panel_data.h>
#include <logger.h>
#include <utils.h>


#define MAX_CONFIG_LINE_SIZE 255
#define CONFIG_PATH "/config.txt"
#define INVERTER_PATH "/inverter_config.txt"
#define MAX_INVERTER_COUNT 3


typedef struct 
{
    char wifi_ssid[MAX_CONFIG_LINE_SIZE];
    char wifi_password[MAX_CONFIG_LINE_SIZE];
    char mqtt_url[MAX_CONFIG_LINE_SIZE];
    int mqtt_port;
    char mqtt_username[MAX_CONFIG_LINE_SIZE];
    char mqtt_password[MAX_CONFIG_LINE_SIZE];
    char mqtt_publish_topic[MAX_CONFIG_LINE_SIZE];

} Config;


void loadConfig(Config *config);
// Returns the number of inverters actually configured (0 when the file is
// missing or holds no usable entry).
uint8_t loadInverterConfig(Inverter inverter[MAX_INVERTER_COUNT]);

#endif