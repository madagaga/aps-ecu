#ifndef LOGGER_H
#define LOGGER_H

#include <panel_data.h>
#include <Arduino.h>
#define DEBUG

#define log_line(a) Serial.println(a)
#define log(a) Serial.print(a)
#define logf(format, ...) Serial.printf(format, __VA_ARGS__)
#define logf_P(format, ...) Serial.printf_P(format, __VA_ARGS__)

void log_inverter(Inverter *inverter);
void log_array(uint8_t *array, uint8_t len);
void log_total(Inverter *inverter, uint8_t len);


#endif /* LOGGER_H */
