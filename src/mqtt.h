#ifndef mqtt_h
#define mqtt_h  

#include <Arduino.h>
#include <PubSubClient.h>
#include <panel_data.h>
#include <wifi.h>

#define MQTT_CLIENT_ID "aps_ecu_inverter"

static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

void mqtt_begin(const char *mqtt_url, int mqtt_port);
void mqtt_publish(const char *topic, Inverter *Inverter);
void mqtt_loop();

#endif