#include <config.h>

void loadConfig(Config *config)
{

    File file = LittleFS.open(CONFIG_PATH, "r");
    char buffer[1024];
    file.readBytes(buffer, sizeof(buffer));
    file.close();

    int start = 0;
    int end = 0;
    for (int i = 0; i < 7; i++)
    {
        start = indexOf(buffer, '=', end) + 1;
        end = indexOf(buffer, ';', start);
        switch (i)
        {
        case 0:
            memccpy((void *)config->wifi_ssid, buffer + start, '\0', end - start);
            #ifdef DEBUG
            Serial.printf("wifi_ssid: %s\n", config->wifi_ssid);
            #endif
            break;
        case 1:
            memccpy((void *)config->wifi_password, buffer + start, '\0', end - start);
            #ifdef DEBUG
            Serial.printf("wifi_password: %s\n", config->wifi_password);
            #endif
            break;
        case 2:
            memccpy((void *)config->mqtt_url, buffer + start, '\0', end - start);
            #ifdef DEBUG
            Serial.printf("mqtt_url: %s\n", config->mqtt_url);
            #endif
            break;
        case 3:
            config->mqtt_port = atoi(buffer + start);
            #ifdef DEBUG
            Serial.printf("mqtt_port: %i\n", config->mqtt_port);
            #endif
            break;
        case 4:
            memccpy((void *)config->mqtt_username, buffer + start, '\0', end - start);
            #ifdef DEBUG
            Serial.printf("mqtt_username: %s\n", config->mqtt_username);
            #endif
            break;
        case 5:
            memccpy((void *)config->mqtt_password, buffer + start, '\0', end - start);
            #ifdef DEBUG
            Serial.printf("mqtt_password: %s\n", config->mqtt_password);
            #endif
            break;
        case 6:
            memccpy((void *)config->mqtt_publish_topic, buffer + start, '\0', end - start);
            #ifdef DEBUG
            Serial.printf("mqtt_publish_topic: %s\n", config->mqtt_publish_topic);
            #endif
            break;
        }
    }
}

void loadInverterConfig(Inverter inverters[3])
{
    File file = LittleFS.open(INVERTER_PATH, "r");
    char buffer[1024];
    file.readBytes(buffer, sizeof(buffer));
    file.close();

    int line = 0;
    int index = 0;
    int start = 0;
    int end = 0;
    char serial[13] = {0};
    char id[5] = {0};
    
    
    while (line != -1)
    {        
        start = indexOf(buffer, '=',  line) + 1;
        end = indexOf(buffer, ';', start);
        memcpy(serial, buffer + start, end - start);
        convertToByteArray(serial, inverters[index].serial);
        log_array(inverters[index].serial, 6);
        start = indexOf(buffer, '=', end) + 1;
        end = indexOf(buffer, ';', start);
        memcpy(id, buffer + start, end - start);
        convertToByteArray(id, inverters[index].iD);
        index++;
        line = indexOf(buffer, '\n', start);
     }
}
