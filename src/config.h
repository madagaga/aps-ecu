#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <LittleFS.h>
#include <panel_data.h>
#include <logger.h>
#include <utils.h>


#define MAX_CONFIG_LINE_SIZE 255
#define CONFIG_PATH "/config.txt"
#define INVERTER_PATH "/inverters_config.txt"


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
void loadInverterConfig(Inverter inverter[3]);

#endif