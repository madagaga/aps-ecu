#include <utils.h>

int16_t indexOf(const uint8_t *data, uint16_t data_len, const uint8_t *pattern, uint8_t pattern_len)
{
    if (data_len < pattern_len)
    {
        return -2; // No match possible if data is shorter than the pattern
    }

    for (uint16_t i = 0; i <= (uint16_t)(data_len - pattern_len); ++i)
    {
        if (memcmp(&data[i], pattern, pattern_len) == 0)
        {
            return i; // Return index
        }
    }

    return -1;
}

static int8_t hexDigit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

bool parseHex(const char *hex, uint8_t *out, uint8_t len)
{
    if (strlen(hex) != (size_t)len * 2)
    {
        return false;
    }

    uint8_t tmp[8];
    if (len > sizeof(tmp))
    {
        return false;
    }
    for (uint8_t i = 0; i < len; i++)
    {
        const int8_t hi = hexDigit(hex[2 * i]);
        const int8_t lo = hexDigit(hex[2 * i + 1]);
        if (hi < 0 || lo < 0)
        {
            return false;
        }
        tmp[i] = (hi << 4) | lo;
    }
    memcpy(out, tmp, len);
    return true;
}

void toHex(const uint8_t *data, uint8_t len, char *out)
{
    static const char digits[] = "0123456789ABCDEF";
    for (uint8_t i = 0; i < len; i++)
    {
        out[2 * i] = digits[data[i] >> 4];
        out[2 * i + 1] = digits[data[i] & 0x0F];
    }
    out[2 * len] = '\0';
}
