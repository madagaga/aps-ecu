#ifndef LOGGER_H
#define LOGGER_H

#include <panel_data.h>
#include <Arduino.h>
#define DEBUG
// Per-byte hex dump of every zigbee frame. It runs interleaved with the
// bit-banged SoftwareSerial transfer, so leaving it on costs received bytes.
// #define DEBUG_FRAMES

#define log_line(a) Serial.println(a)
#define log(a) Serial.print(a)
#define logf(format, ...) Serial.printf(format, __VA_ARGS__)
#define logf_P(format, ...) Serial.printf_P(format, __VA_ARGS__)

void log_inverter(Inverter *inverter);
void log_array(uint8_t *array, uint8_t len);
void log_total(Inverter *inverter, uint8_t len);


#endif /* LOGGER_H */
