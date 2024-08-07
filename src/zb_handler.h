#ifndef zb_handler_h_
#define zb_handler_h_

#include <SoftwareSerial.h>
#include <logger.h>

/* SERIAL */
#if defined(__AVR_ATmega328P__)
#define RXD2 2
#define TXD2 3
#define ZB_RESET 5

//inline void printf(char* a, uint8_t b) { char buf[1]; sprintf(buf, "%02d",b); Serial.print(buf);}
#else
#define RXD2 D7
#define TXD2 D8
#define ZB_RESET D5
#endif

#define DEBUG

void zigbee_begin();
void zigbee_reset();
void zigbee_flush();

void zigbee_send(const uint8_t *buffer, uint8_t buffer_len);
uint8_t zigbee_recv(uint8_t *buffer);

#endif