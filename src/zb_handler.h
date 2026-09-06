#ifndef zb_handler_h_
#define zb_handler_h_

#include <SoftwareSerial.h>
#include <logger.h>
#include <commands.h>

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



/*
 * ZNP frame layout (TI Z-Stack):
 *   SOF(1) | LEN(1) | CMD0(1) | CMD1(1) | DATA[LEN] | FCS(1)
 * zigbee_recv() only returns frames whose LEN and FCS check out, so these
 * accessors are safe to use on whatever it hands back.
 */
// zigbee_recv() return value for a frame that failed validation. Distinct
// from 0, which means "nothing arrived": a caller draining the port must
// keep reading after a bad frame, not stop.
#define ZB_RECV_INVALID 0xFFFF

// A ZNP frame is at most LEN(255) + 5. Reading beyond that is pointless and
// would run past the caller's buffer.
#define ZB_MAX_FRAME 260

// How long to wait for a frame before giving up.
#define ZB_RECV_TIMEOUT_MS 1500


#define ZNP_LEN(f)  ((f)[1])
#define ZNP_CMD(f)  ((uint16_t)(((f)[2] << 8) | (f)[3]))
#define ZNP_DATA(f) ((f) + 4)

#define ZNP_AF_DATA_CONFIRM          0x4480
#define ZNP_AF_INCOMING_MSG          0x4481
#define ZNP_AF_DATA_REQUEST_SRSP     0x6401
#define ZNP_AF_DATA_REQUEST_EXT_SRSP 0x6402

// ZDO indications the coordinator emits on its own; nothing here acts on them
#define ZNP_ZDO_STATE_CHANGE_IND     0x45C0
#define ZNP_ZDO_SRC_RTG_IND          0x45C4

#define AF_STATUS_SUCCESS  0x00
#define AF_STATUS_NO_ROUTE 0xCD

/*
 * AF_INCOMING_MSG data field, relative to ZNP_DATA:
 *   GroupID(2) ClusterID(2) SrcAddr(2) SrcEndpoint(1) DstEndpoint(1)
 *   WasBroadcast(1) LinkQuality(1) SecurityUse(1) TimeStamp(4)
 *   TransSeqNumber(1) Len(1) Data[Len]
 * 4 + 17 = 21, which is where the APsystems payload starts.
 */
#define AF_HEADER_SIZE     17
#define AF_SRC_ADDR(d)     ((d) + 4)
#define AF_LINK_QUALITY(d) ((d)[9])
#define AF_PAYLOAD_LEN(d)  ((d)[16])
#define AF_PAYLOAD(d)      ((d) + AF_HEADER_SIZE)


void zigbee_begin();
void zigbee_reset();
void zigbee_flush();

void zigbee_send(const uint8_t *buffer, uint8_t buffer_len);
uint16_t zigbee_recv(uint8_t *buffer, uint16_t timeout_ms = ZB_RECV_TIMEOUT_MS);

#endif