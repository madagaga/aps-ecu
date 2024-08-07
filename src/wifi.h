#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <logger.h>

static IPAddress local_IP(192, 168, 4, 1);
static IPAddress gateway(192, 168, 4, 254);
static IPAddress subnet(255, 255, 255, 0);

void wifi_startAP();
void wifi_setup(const char *ssid, const char *password);
bool wifi_connect(uint8_t max_tries);

//255.255.255.255
static char wifi_IP[16];
char * getIP();

//AA:BB:CC:DD:EE:FF
static char wifi_MAC[18];
char * getMAC();

#endif