#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <logger.h>

void wifi_startAP();
void wifi_setup(const char *ssid, const char *password);
bool wifi_connect(uint8_t max_tries);

char * getIP();

char * getMAC();

#endif