#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <logger.h>

#define MAX_CONFIG_LINE_SIZE 255
int16_t indexOf(const char *data, const char pattern,int startIndex);

int16_t indexOf(const uint8_t *data, uint8_t data_len, const uint8_t *pattern, uint8_t pattern_len);
void convertToByteArray(const char *hexString, uint8_t *destination);

int toInt(const uint8_t *buffer, uint8_t index, uint8_t length);
float toFloat(const uint8_t *buffer, uint8_t index, uint8_t length);

#endif /* UTILS_H */