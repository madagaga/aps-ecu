#ifndef mqtt_h
#define mqtt_h  

#include <Arduino.h>
#include <panel_data.h>

#define MQTT_CLIENT_ID "aps_ecu"
#define MQTT_BUFFER_SIZE 512

void mqtt_begin(const char *mqtt_url, int mqtt_port);
void mqtt_publish(const char *topic, Inverter *Inverter);
void mqtt_loop();

#endif