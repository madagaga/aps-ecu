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
// An inverter that does not pair (wrong serial, not installed yet) must not
// stop the others from being polled: pairing is retried at this pace.
#define PAIR_RETRY_MS 300000UL
// Soft-AP started because the configured network was not there (box still
// booting after a power cut...): reboot to try it again, unless someone is
// connected to the AP to change the settings.
#define AP_FALLBACK_RETRY_MS 300000UL
Config config;

Inverter inverters[MAX_INVERTER_COUNT];
// one decoded reply at a time: filled by ecu_poll, published, overwritten
static Reading reading;
bool all_Paired = true;
uint8_t inverterCount = 0;
uint32_t lastPoll = 0;
static bool pairingTried = false;
static uint32_t lastPairing = 0;
static bool apFallback = false;

// Serviced between inverters rather than during a transfer: doing it while a
// frame is in flight costs received bytes.
void serviceNetwork()
{
  webserver_loop();
  mqtt_loop();
}

static bool allPaired()
{
  for (uint8_t i = 0; i < inverterCount; i++)
  {
    if (!(inverters[i].flags & INV_PAIRED))
      return false;
  }
  return true;
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
    apFallback = config.wifi_ssid[0] != 0;
  }
  else
  {
    mqtt_begin(config.mqtt_url, config.mqtt_port);
    inverterCount = loadInverterConfig(inverters, MAX_INVERTER_COUNT);

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

      all_Paired = allPaired();
      if (all_Paired)
      {
        ecu_noop();
      }
    }
  }

  webserver_begin(); // Actually start the server
#ifdef DEBUG
  log_line(F("HTTP server started"));
#endif
}

// Polls every inverter once and publishes what came back.
static void pollRound()
{
  uint32_t totalPower = 0;
  uint8_t answered = 0;

  for (uint8_t i = 0; i < inverterCount; i++)
  {
    Inverter *inverter = &inverters[i];
    if (!(inverter->flags & INV_PAIRED))
    {
      continue;
    }
    serviceNetwork();

    const EcuPollResult result = ecu_poll(inverter, &reading);
    switch (result)
    {
    case ECU_POLL_OK:
      answered++;
      totalPower += reading.acPower_W;
#ifdef DEBUG
      log_reading(inverter, &reading);
#endif
      mqtt_publish(config.mqtt_publish_topic, inverter, &reading);
      break;

    case ECU_POLL_WENT_OFFLINE:
      mqtt_publish_offline(config.mqtt_publish_topic, inverter);
      break;

    case ECU_POLL_UNSUPPORTED:
      answered++;
      break;

    case ECU_POLL_FAILED:
      break;
    }

    // once per online period, right after a reply proved it reachable.
    // Until then the telemetry is decoded as DS3.
    const bool replied = result == ECU_POLL_OK || result == ECU_POLL_UNSUPPORTED;
    if (replied && inverter->model == 0 && !(inverter->flags & INV_ID_TRIED))
    {
      serviceNetwork();
      ecu_identify(inverter);
    }
  }

  logf_P(PSTR("round: %u/%u inverters answered, %luW\n"), answered, inverterCount, (unsigned long)totalPower);
}

void loop()
{
  serviceNetwork();

  if (apFallback && millis() >= AP_FALLBACK_RETRY_MS && wifi_ap_clients() == 0)
  {
    log_line(F("configured wifi still unreachable - rebooting to retry"));
    ESP.restart();
  }

  // nothing configured: only keep the web UI alive so inverters can be added
  if (inverterCount == 0)
  {
    yield();
    return;
  }

  if (!all_Paired && (!pairingTried || millis() - lastPairing >= PAIR_RETRY_MS))
  {
    bool newlyPaired = false;
    for (uint8_t i = 0; i < inverterCount; i++)
    {
      if (!(inverters[i].flags & INV_PAIRED) && ecu_pair(&inverters[i]))
      {
        newlyPaired = true;
      }
    }

    // keep the addresses: no pairing on the next boot
    if (newlyPaired)
    {
      saveInverterConfig(inverters, inverterCount);
    }

    all_Paired = allPaired();
    ecu_noop();
    pairingTried = true;
    lastPairing = millis();
    return;
  }

  // millis() arithmetic on uint32_t wraps correctly, no rollover special case
  if (millis() - lastPoll < POLL_INTERVAL_MS)
  {
    yield();
    return;
  }
  lastPoll = millis();

  // module wedged: skip the round, the watchdog reinitialises it
  if (!ecu_check_alive())
  {
    return;
  }

  pollRound();

#ifdef DEBUG
  logf_P(PSTR("[Server Connected] : %s\n"), getIP());
#endif
}
