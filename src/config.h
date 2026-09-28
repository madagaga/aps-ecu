#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <LittleFS.h>
#include <panel_data.h>
#include <logger.h>
#include <utils.h>

#define CONFIG_PATH "/config.txt"
#define INVERTER_PATH "/inverter_config.txt"

// 12 bytes of RAM each (see Inverter). The practical ceiling is more likely
// the coordinator's routing tables and the time one poll round takes.
#define MAX_INVERTER_COUNT 64

// Longest "key=value" token in a config file.
#define MAX_CONFIG_TOKEN 96

typedef struct
{
    char wifi_ssid[33];     // 802.11: 32 bytes max
    char wifi_password[65]; // WPA2: 63 characters, or 64 hex digits
    char mqtt_url[65];
    int mqtt_port;
    char mqtt_username[65];
    char mqtt_password[65];
    char mqtt_publish_topic[65];

} Config;


void loadConfig(Config *config);
// Returns the number of inverters actually configured (0 when the file is
// missing or holds no usable entry).
uint8_t loadInverterConfig(Inverter *inverters, uint8_t max);
// Rewrites the inverter file, short addresses included, so that pairing does
// not have to run again on the next boot.
bool saveInverterConfig(const Inverter *inverters, uint8_t count);

#endif
