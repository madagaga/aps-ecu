#include <Arduino.h>
#include <panel_data.h>
#include <ecu_handler.h>
#include <ESP8266WebServer.h> // Include the WebServer library
#include <LittleFS.h>         // Include the SPIFFS library
#include <config.h>
#include <wifi.h>
#include <webserver.h>
#include <mqtt.h>

#define MAX_INVERTER_COUNT 3
Config config;

Inverter inverters[MAX_INVERTER_COUNT];
bool all_Paired = true;

void setup()
{

  Serial.begin(115200);
  LittleFS.begin(); // Start the SPI Flash Files System

  loadConfig(&config);

  bool connected = false;
  // if wifi credentials are detected
  if (config.wifi_ssid > 0)
  {
    wifi_setup(config.wifi_ssid, config.wifi_password);
    connected = wifi_connect(5);
  }

  if (!connected) // Here when STA connect fails
  {
    wifi_startAP();
  }
  else
  {
    mqtt_begin(config.mqtt_url, config.mqtt_port);
    loadInverterConfig(inverters);
    ecu_begin();
#ifdef DEBUG
    Serial.println(F("Setup OK - initializing"));
#endif
    ecu_initialize();

    for (uint8_t i = 0; i < MAX_INVERTER_COUNT; i++)
    {
      if (inverters[i].iD[0] != 0 && inverters[i].iD[1] != 0)
      {
        inverters[i].idx = i;
        inverters[i].paired = true;
      }
      else
        all_Paired = false;
    }

    if (all_Paired)
    {
      ecu_noop();
    }
  }

  webserver_begin(); // Actually start the server
#ifdef DEBUG
  log_line("HTTP server started");
#endif
}

void loop()
{
  if (!all_Paired)
  {
    all_Paired = true;
    for (uint8_t i = 0; i < MAX_INVERTER_COUNT; i++)
    {
      if (!inverters[i].paired)
      {
        ecu_pair(&inverters[i]);
      }

      if (!inverters[i].paired)
        all_Paired = false;
    }

    // if (all_Paired)
    //  save config

    ecu_noop();
  }
  else
  {

    for (uint8_t i = 0; i < MAX_INVERTER_COUNT; i++)
    {
      ecu_poll(&inverters[i]);

      if (inverters[i].paired && inverters[i].polled)
      {
        mqtt_publish(config.mqtt_publish_topic, &inverters[i]);
      }
    }
    log_total(inverters, MAX_INVERTER_COUNT);
  }

//   ecu.ping();
#ifdef DEBUG
  printf("[Server Connected] : %s\n", getIP());
#endif
  webserver_loop();

  mqtt_loop();


  // ecu.heart_beat();
  delay(1000);
}
