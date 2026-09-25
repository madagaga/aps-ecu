#include <mqtt.h>
#include <PubSubClient.h>
#include <wifi.h>
#include <logger.h>
#include <utils.h>
#include <stdarg.h>

// Single instance, owned by this translation unit. Defining these in the
// header gave every includer its own unused copy.
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);

void mqtt_connect()
{
    if (!mqttClient.connected())
    {
        if (mqttClient.connect(MQTT_CLIENT_ID))
        {
            log_line(F("mqtt connected"));
        }
        else
        {
            logf_P(PSTR("mqtt connection failed, rc=%i\n"), mqttClient.state());
        }
    }
}

void mqtt_begin(const char *mqtt_url, int mqtt_port)
{
    mqttClient.setServer(mqtt_url, mqtt_port);
    // default is 256 bytes, too small for the payload
    if (!mqttClient.setBufferSize(MQTT_BUFFER_SIZE))
    {
        log_line(F("mqtt buffer allocation failed"));
    }
    mqtt_connect();
}

// static: keeps the scratch buffer off the 4KB stack, since publishing is
// called from loop()
static char text[MQTT_PAYLOAD_SIZE];

// Appends to `text` at `*pos`. Returns false once the buffer is full: a
// truncated JSON document must not be published.
static bool append(size_t *pos, const char *format, ...)
{
    if (*pos >= sizeof(text))
    {
        return false;
    }
    va_list args;
    va_start(args, format);
    const int written = vsnprintf_P(text + *pos, sizeof(text) - *pos, format, args);
    va_end(args);
    if (written < 0 || (size_t)written >= sizeof(text) - *pos)
    {
        *pos = sizeof(text);
        return false;
    }
    *pos += written;
    return true;
}

static void send(const char *topic, bool complete)
{
    if (!complete)
    {
        log_line(F("mqtt payload truncated - not published"));
        return;
    }

    log_line(text);
#ifdef DEBUG
    bool ret = mqttClient.publish(topic, text);
    logf_P(PSTR("Publishin to '%s' : %i\n"),topic, ret);
#else
    mqttClient.publish(topic, text);
#endif
}

void mqtt_publish(const char *topic, const Inverter *inverter, const Reading *reading)
{
    if (!mqttClient.connected())
    {
        log_line(F("mqtt not connected"));
        return;
    }

    char serial[13];
    char addr[5];
    char status[11];
    toHex(inverter->serial, 6, serial);
    toHex(inverter->addr, 2, addr);
    toHex(reading->status, sizeof(reading->status), status);

    uint32_t energy = 0;
    for (uint8_t i = 0; i < reading->panelCount; i++)
    {
        energy += reading->panels[i].energy_Wh;
    }

    size_t pos = 0;
    bool ok = append(&pos, PSTR("{"
                                "\"type\":\"inverter\","
                                "\"serial\":\"%s\","
                                "\"addr\":\"%s\","
                                "\"model\":%u,"
                                "\"online\":true,"
                                "\"lqi\":%u,"
                                "\"power\":%u,"
                                "\"reactive\":%d,"
                                "\"voltage\":%.1f,"
                                "\"frequency\":%.2f,"
                                "\"temperature\":%.1f,"
                                "\"counter\":%u,"
                                "\"status\":\"%s\","
                                "\"faults\":%u,"
                                "\"energy\":%lu,"
                                "\"deviceID\":\"%s\","
                                "\"panels\":["),
                     serial, addr, inverter->model, inverter->lqi,
                     reading->acPower_W, reading->reactive_VAR,
                     reading->acVoltage_dV / 10.0f, reading->frequency_cHz / 100.0f,
                     reading->temperature_dC / 10.0f, reading->counter_s,
                     status, reading->faults, (unsigned long)energy, getMAC());

    for (uint8_t i = 0; ok && i < reading->panelCount; i++)
    {
        const PanelReading *p = &reading->panels[i];
        ok = append(&pos, PSTR("%s{\"voltage\":%.2f,\"current\":%.3f,\"energy\":%lu}"),
                    i > 0 ? "," : "",
                    p->voltage_cV / 100.0f, p->current_mA / 1000.0f, (unsigned long)p->energy_Wh);
    }
    ok = ok && append(&pos, PSTR("]}"));

    send(topic, ok);
}

void mqtt_publish_offline(const char *topic, const Inverter *inverter)
{
    if (!mqttClient.connected())
    {
        log_line(F("mqtt not connected"));
        return;
    }

    char serial[13];
    toHex(inverter->serial, 6, serial);

    size_t pos = 0;
    const bool ok = append(&pos, PSTR("{\"type\":\"inverter\",\"serial\":\"%s\",\"online\":false,\"deviceID\":\"%s\"}"),
                           serial, getMAC());
    send(topic, ok);
}

void mqtt_loop()
{
    mqtt_connect();
    
    mqttClient.loop();
}
