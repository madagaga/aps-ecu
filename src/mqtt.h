#ifndef mqtt_h
#define mqtt_h  

#include <Arduino.h>
#include <panel_data.h>

#define MQTT_CLIENT_ID "aps_ecu"
// JSON document; 4 panels is about 450 bytes
#define MQTT_PAYLOAD_SIZE 512
// PubSubClient's packet buffer also holds the fixed header (5), the topic
// length (2) and the topic itself (Config::mqtt_publish_topic, 64 max)
#define MQTT_BUFFER_SIZE (MQTT_PAYLOAD_SIZE + 5 + 2 + 64)

void mqtt_begin(const char *mqtt_url, int mqtt_port);
void mqtt_publish(const char *topic, const Inverter *inverter, const Reading *reading);
// sent once when an inverter stops answering (see ECU_OFFLINE_AFTER)
void mqtt_publish_offline(const char *topic, const Inverter *inverter);
void mqtt_loop();

#endif