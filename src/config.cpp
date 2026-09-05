#include <config.h>

// Copy a field of `len` bytes and always null-terminate. Truncates instead of
// overflowing when the field is longer than the destination.
static void copyField(char *dest, size_t destSize, const char *src, int len)
{
    if (len < 0)
    {
        len = 0;
    }
    if ((size_t)len >= destSize)
    {
        len = destSize - 1;
    }
    memcpy(dest, src, len);
    dest[len] = '\0';
}

static bool isZero(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (data[i] != 0)
        {
            return false;
        }
    }
    return true;
}

// Read a whole config file into `buffer` and null-terminate it.
// Returns the number of bytes read, or -1 when the file cannot be opened.
static int readConfigFile(const char *path, char *buffer, size_t bufferSize)
{
    File file = LittleFS.open(path, "r");
    if (!file)
    {
        logf("config file not found: %s\n", path);
        return -1;
    }

    size_t len = file.readBytes(buffer, bufferSize - 1);
    file.close();
    buffer[len] = '\0';
    return (int)len;
}

void loadConfig(Config *config)
{
    memset(config, 0, sizeof(Config));

    char buffer[1024];
    int len = readConfigFile(CONFIG_PATH, buffer, sizeof(buffer));
    if (len <= 0)
    {
        return;
    }

    int end = 0;
    for (int i = 0; i < 7; i++)
    {
        int start = indexOf(buffer, len, '=', end) + 1;
        if (start <= 0)
        {
            break;
        }
        end = indexOf(buffer, len, ';', start);
        if (end < 0)
        {
            break;
        }

        switch (i)
        {
        case 0:
            copyField(config->wifi_ssid, sizeof(config->wifi_ssid), buffer + start, end - start);
            #ifdef DEBUG
            logf("wifi_ssid: %s\n", config->wifi_ssid);
            #endif
            break;
        case 1:
            copyField(config->wifi_password, sizeof(config->wifi_password), buffer + start, end - start);
            #ifdef DEBUG
            logf("wifi_password: %s\n", config->wifi_password);
            #endif
            break;
        case 2:
            copyField(config->mqtt_url, sizeof(config->mqtt_url), buffer + start, end - start);
            #ifdef DEBUG
            logf("mqtt_url: %s\n", config->mqtt_url);
            #endif
            break;
        case 3:
            config->mqtt_port = atoi(buffer + start);
            #ifdef DEBUG
            logf("mqtt_port: %i\n", config->mqtt_port);
            #endif
            break;
        case 4:
            copyField(config->mqtt_username, sizeof(config->mqtt_username), buffer + start, end - start);
            #ifdef DEBUG
            logf("mqtt_username: %s\n", config->mqtt_username);
            #endif
            break;
        case 5:
            copyField(config->mqtt_password, sizeof(config->mqtt_password), buffer + start, end - start);
            #ifdef DEBUG
            logf("mqtt_password: %s\n", config->mqtt_password);
            #endif
            break;
        case 6:
            copyField(config->mqtt_publish_topic, sizeof(config->mqtt_publish_topic), buffer + start, end - start);
            #ifdef DEBUG
            logf("mqtt_publish_topic: %s\n", config->mqtt_publish_topic);
            #endif
            break;
        }
    }
}

uint8_t loadInverterConfig(Inverter inverters[MAX_INVERTER_COUNT])
{
    char buffer[1024];
    int len = readConfigFile(INVERTER_PATH, buffer, sizeof(buffer));
    if (len <= 0)
    {
        return 0;
    }

    char serial[13] = {0};
    char id[5] = {0};
    int pos = 0;
    uint8_t count = 0;

    // one entry per "serial=<12 hex>;id=<4 hex>;" pair, at most MAX_INVERTER_COUNT
    while (count < MAX_INVERTER_COUNT)
    {
        int start = indexOf(buffer, len, '=', pos) + 1;
        if (start <= 0)
        {
            break;
        }
        int end = indexOf(buffer, len, ';', start);
        if (end < 0)
        {
            break;
        }
        copyField(serial, sizeof(serial), buffer + start, end - start);

        start = indexOf(buffer, len, '=', end) + 1;
        if (start <= 0)
        {
            break;
        }
        end = indexOf(buffer, len, ';', start);
        if (end < 0)
        {
            break;
        }
        copyField(id, sizeof(id), buffer + start, end - start);
        pos = end + 1;

        convertToByteArray(serial, inverters[count].serial);
        convertToByteArray(id, inverters[count].iD);

        // an entry without a serial cannot be paired, drop it
        if (isZero(inverters[count].serial, sizeof(inverters[count].serial)))
        {
            memset(inverters[count].serial, 0, sizeof(inverters[count].serial));
            memset(inverters[count].iD, 0, sizeof(inverters[count].iD));
            log_line(F("skipping inverter entry without serial"));
            continue;
        }

        log_array(inverters[count].serial, 6);
        count++;
    }

    logf("inverters configured: %u", count);
    log_line("");
    return count;
}
