#ifndef ecu_handler_h_
#define ecu_handler_h_

#include <commands.h>
#include <panel_data.h>
#include <logger.h>
#include <zb_handler.h>

#include <utils.h>

#define MAX_SERIAL_BUFFER_SIZE 512

void ecu_begin();
void ecu_reset();
void ecu_initialize();

void ecu_ping();
bool ecu_heart_beat();

void ecu_noop();

void ecu_pair(Inverter *inverter);

void ecu_poll(Inverter *inverter);
void ecu_decode_poll_answer(Inverter *inverter);
#endif