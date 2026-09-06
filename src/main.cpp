#include <Arduino.h>
#include <panel_data.h>
#include <ecu_handler.h>
#include <ESP8266WebServer.h> // Include the WebServer library
#include <LittleFS.h>         // Include the SPIFFS library
#include <config.h>
#include <wifi.h>
#include <webserver.h>
#include <mqtt.h>

#define POLL_INTERVAL_MS 10000
Config config;

Inverter inverters[MAX_INVERTER_COUNT];
bool all_Paired = true;
uint8_t inverterCount = 0;
uint32_t lastPoll = 0;

// Serviced between inverters rather than during a transfer: doing it while a
// frame is in flight costs received bytes.
void serviceNetwork()
{
  webserver_loop();
  mqtt_loop();
}

void setup()
{

  Serial.begin(115200);
  LittleFS.begin(); // Start the SPI Flash Files System

  loadConfig(&config);

  bool connected = false;
  // if wifi credentials are detected
  if (config.wifi_ssid[0] != 0)
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
    inverterCount = loadInverterConfig(inverters);

    if (inverterCount == 0)
    {
      // nothing to talk to: leave the zigbee coordinator down and let the user
      // fill in the inverter config through the web UI
      log_line(F("no inverter configured - zigbee, pairing and polling disabled"));
    }
    else
    {
      ecu_begin();
#ifdef DEBUG
      log_line(F("Setup OK - initializing"));
#endif
      ecu_initialize();

      for (uint8_t i = 0; i < inverterCount; i++)
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
  }

  webserver_begin(); // Actually start the server
#ifdef DEBUG
  log_line("HTTP server started");
#endif
}

void loop()
{
  serviceNetwork();

  // nothing configured: only keep the web UI alive so inverters can be added
  if (inverterCount == 0)
  {
    yield();
    return;
  }

  if (!all_Paired)
  {
    all_Paired = true;
    for (uint8_t i = 0; i < inverterCount; i++)
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
    return;
  }

  // millis() arithmetic on uint32_t wraps correctly, no rollover special case
  if (millis() - lastPoll < POLL_INTERVAL_MS)
  {
    yield();
    return;
  }
  lastPoll = millis();

  ecu_heart_beat();

  for (uint8_t i = 0; i < inverterCount; i++)
  {
    serviceNetwork();
    ecu_poll(&inverters[i]);

    if (inverters[i].paired && inverters[i].polled)
    {
      mqtt_publish(config.mqtt_publish_topic, &inverters[i]);
    }
  }
  log_total(inverters, inverterCount);

#ifdef DEBUG
  logf_P(PSTR("[Server Connected] : %s\n"), getIP());
#endif
}
