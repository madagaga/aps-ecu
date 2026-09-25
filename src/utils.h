#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

int16_t indexOf(const uint8_t *data, uint16_t data_len, const uint8_t *pattern, uint8_t pattern_len);

// Parses exactly 2 * len hex digits into `out`. Returns false (and leaves
// `out` untouched) on any other length or on a non-hex character.
bool parseHex(const char *hex, uint8_t *out, uint8_t len);

// Writes 2 * len uppercase hex digits and a terminating zero into `out`.
void toHex(const uint8_t *data, uint8_t len, char *out);

#endif /* UTILS_H */
