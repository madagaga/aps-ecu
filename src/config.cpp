#include <config.h>

// Copy a value and always null-terminate. Truncates instead of overflowing
// when the value is longer than the destination.
static void copyField(char *dest, size_t destSize, const char *src)
{
    if (strlcpy(dest, src, destSize) >= destSize)
    {
        logf_P(PSTR("config value truncated to %u characters: %s\n"), destSize - 1, dest);
    }
}

typedef void (*KeyValueHandler)(const char *key, const char *value, void *ctx);

/*
 * Calls `handler` for every "key=value" token of the file. ';' and line ends
 * both close a token. The file is read byte by byte: no whole-file buffer on
 * the 4KB stack, and the file size does not cap the number of inverters.
 */
static bool parseKeyValueFile(const char *path, KeyValueHandler handler, void *ctx)
{
    File file = LittleFS.open(path, "r");
    if (!file)
    {
        logf_P(PSTR("config file not found: %s\n"), path);
        return false;
    }

    char token[MAX_CONFIG_TOKEN];
    uint8_t len = 0;
    bool overflow = false;

    while (true)
    {
        const int c = file.read();
        if (c < 0 || c == ';' || c == '\n' || c == '\r')
        {
            if (overflow)
            {
                log_line(F("config token too long - ignored"));
            }
            else if (len > 0)
            {
                token[len] = '\0';
                char *eq = strchr(token, '=');
                if (eq != NULL)
                {
                    *eq = '\0';
                    handler(token, eq + 1, ctx);
                }
            }
            len = 0;
            overflow = false;

            if (c < 0)
            {
                break;
            }
            continue;
        }

        if (len < sizeof(token) - 1)
        {
            token[len++] = (char)c;
        }
        else
        {
            overflow = true;
        }
    }

    file.close();
    return true;
}

static void onConfigValue(const char *key, const char *value, void *ctx)
{
    Config *config = (Config *)ctx;

    if (strcmp(key, "wifi_ssid") == 0)
        copyField(config->wifi_ssid, sizeof(config->wifi_ssid), value);
    else if (strcmp(key, "wifi_password") == 0)
        copyField(config->wifi_password, sizeof(config->wifi_password), value);
    else if (strcmp(key, "mqtt_url") == 0)
        copyField(config->mqtt_url, sizeof(config->mqtt_url), value);
    else if (strcmp(key, "mqtt_port") == 0)
        config->mqtt_port = atoi(value);
    else if (strcmp(key, "mqtt_username") == 0)
        copyField(config->mqtt_username, sizeof(config->mqtt_username), value);
    else if (strcmp(key, "mqtt_password") == 0)
        copyField(config->mqtt_password, sizeof(config->mqtt_password), value);
    else if (strcmp(key, "mqtt_publish_topic") == 0)
        copyField(config->mqtt_publish_topic, sizeof(config->mqtt_publish_topic), value);
    else
        return;

#ifdef DEBUG
    logf_P(PSTR("%s: %s\n"), key, value);
#endif
}

void loadConfig(Config *config)
{
    memset(config, 0, sizeof(Config));
    parseKeyValueFile(CONFIG_PATH, onConfigValue, config);
}

typedef struct
{
    Inverter *inverters;
    uint8_t max;
    uint8_t count;
    bool current; // the last "serial" was accepted, so "id" belongs to it
} InverterLoad;

// one entry per "serial=<12 hex>;id=<4 hex, may be empty>;"
static void onInverterValue(const char *key, const char *value, void *ctx)
{
    InverterLoad *load = (InverterLoad *)ctx;

    if (strcmp(key, "serial") == 0)
    {
        load->current = false;
        if (load->count >= load->max)
        {
            log_line(F("too many inverters configured - extra entries ignored"));
            return;
        }

        Inverter *inverter = &load->inverters[load->count];
        memset(inverter, 0, sizeof(Inverter));
        static const uint8_t zero[6] = {0};
        // an entry without a serial cannot be paired, drop it
        if (!parseHex(value, inverter->serial, 6) || memcmp(inverter->serial, zero, 6) == 0)
        {
            logf_P(PSTR("skipping inverter entry with invalid serial '%s'\n"), value);
            return;
        }
        load->count++;
        load->current = true;
    }
    else if (strcmp(key, "id") == 0 && load->current)
    {
        Inverter *inverter = &load->inverters[load->count - 1];
        if (value[0] == '\0')
        {
            return; // not paired yet
        }
        if (!parseHex(value, inverter->addr, 2))
        {
            logf_P(PSTR("ignoring invalid inverter id '%s'\n"), value);
            return;
        }
        const bool unset = (inverter->addr[0] == 0x00 && inverter->addr[1] == 0x00) ||
                           (inverter->addr[0] == 0xFF && inverter->addr[1] == 0xFF);
        if (!unset)
        {
            inverter->flags |= INV_PAIRED;
        }
    }
}

uint8_t loadInverterConfig(Inverter *inverters, uint8_t max)
{
    InverterLoad load = {inverters, max, 0, false};
    parseKeyValueFile(INVERTER_PATH, onInverterValue, &load);

    logf_P(PSTR("inverters configured: %u\n"), load.count);
    return load.count;
}

bool saveInverterConfig(const Inverter *inverters, uint8_t count)
{
    File file = LittleFS.open(INVERTER_PATH, "w");
    if (!file)
    {
        log_line(F("cannot write the inverter config"));
        return false;
    }

    char serial[13];
    char id[5];
    for (uint8_t i = 0; i < count; i++)
    {
        toHex(inverters[i].serial, 6, serial);
        if (inverters[i].flags & INV_PAIRED)
            toHex(inverters[i].addr, 2, id);
        else
            id[0] = '\0';

        // same layout as the web UI writes; no trailing newline, the UI
        // would show an empty entry for it
        file.printf_P(PSTR("%sserial=%s;id=%s;"), i > 0 ? "\n" : "", serial, id);
    }
    file.close();

    log_line(F("inverter config saved"));
    return true;
}
