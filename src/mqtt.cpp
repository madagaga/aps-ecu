#include <mqtt.h>

void mqtt_connect()
{
    if (!mqttClient.connected())
    {
        if (mqttClient.connect(MQTT_CLIENT_ID))
        {
            Serial.println(F("mqtt connected"));
        }
        else
        {
            Serial.printf_P(PSTR("mqtt connection failed, rc=%i\n"), mqttClient.state());
        }
    }
}

void mqtt_begin(const char *mqtt_url, int mqtt_port)
{
    mqttClient.setServer(mqtt_url, mqtt_port);
    mqtt_connect();
}

void mqtt_publish(const char *topic, Inverter *Inverter)
{
    if (!mqttClient.connected())
    {
        Serial.println(F("mqtt not connected"));

        return;
    }

    char text[768];
    sprintf(text, "{"
                  "\"type\":\"inverter\","
                  "\"serial\":\"%02X-%02X-%02X-%02X-%02X-%02X\","
                  "\"id\":\"%02X-%02X\","
                  "\"invType\":%d,"
                  "\"index\":%i,"
                  "\"polled\":%i,"
                  "\"power\":%i,"
                  "\"frequency\":%.2f,"
                  "\"temperature\":%.2f,"
                  "\"voltage\":%.2f,"
                  "\"energy\":%.2f,"
                  "\"mac\":\"%s\""
                  "}",
            Inverter->serial[0], Inverter->serial[1], Inverter->serial[2], Inverter->serial[3], Inverter->serial[4], Inverter->serial[5],
            Inverter->iD[0], Inverter->iD[1],
            Inverter->invType,
            Inverter->idx,
            Inverter->polled,
            Inverter->acPower,
            Inverter->frequency,
            Inverter->temperature,
            Inverter->acVoltage,
            Inverter->energy,
            getMAC());

    // include panels data: format [ array of panels]
    // strcat(text, ",\"panels\":[");
    // for (int i = 0; i < 4; i++)
    // {
    //     // skip panel if not present
    //     if(Inverter->panels[i].present == false)
    //     {
    //         continue;
    //     }
    //     if (i > 0)
    //     {
    //         strcat(text, ",");
    //     }
    //     char panelText[128];
    //     sprintf(panelText, "{\"dcVoltage\":%.2f,\"dcCurrent\":%.2f,\"energy\":%.2f}",
    //             Inverter->panels[i].dcVoltage,
    //             Inverter->panels[i].dcCurrent,
    //             Inverter->panels[i].energy);
    //     strcat(text, panelText);

    // }
    // strcat(text, "]}");

    Serial.println(text);
#ifdef DEBUG
    bool ret = mqttClient.publish(topic, text);
    Serial.printf_P(PSTR("Publishin to '%s' : %i\n"),topic, ret);
#else
    mqttClient.publish(topic, text);
#endif
}

void mqtt_loop()
{
    mqtt_connect();
    
    mqttClient.loop();
}
