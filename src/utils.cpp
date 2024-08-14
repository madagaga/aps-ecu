#include <utils.h>

int16_t indexOf(const char *data, const char pattern, int startIndex)
{

    for (int i = startIndex; i < MAX_CONFIG_LINE_SIZE; i++)
    {
        if (data[i] == pattern)
        {
            return i;
        }
    }
    return -1;
}

int16_t indexOf(const uint8_t *data, uint16_t data_len, const uint8_t *pattern, uint8_t pattern_len)
{
    if (data_len < pattern_len)
    {        
        return -2; // No match possible if data is shorter than the pattern
    }

    for (uint8_t i = 0; i <= data_len - pattern_len; ++i)
    {
        if (memcmp(&data[i], pattern, pattern_len) == 0)
        {
            return i; // Return index
        }
    }

    return -1;
}



float toFloat(const uint8_t *buffer, uint8_t index, uint8_t length)
{
    return (float)toInt(buffer, index, length);
}

int toInt(const uint8_t *buffer, uint8_t index, uint8_t length)
{
    int result = 0;
    for (uint8_t i = index; i < index + length; i++)
    {
        result = (result << 8) | buffer[i];
    }
    return result;
}

void convertToByteArray(const char *hexString, uint8_t *destination)
{
    // Iterate over each pair of characters in the hexadecimal string
    for (int i = 0; hexString[i] && hexString[i + 1]; i += 2)
    {
        // Convert the pair of hexadecimal characters to a byte value
        int value = 0;
        sscanf(hexString + i, "%02x", &value); // Read two characters and interpret as hexadecimal
        // Store the byte value in the destination array
        *destination++ = value;
    }
}