#ifndef aps_protocol_h_
#define aps_protocol_h_

#include <stdint.h>
#include <panel_data.h>

/*
 * APsystems application layer ("L2"), carried inside the zigbee payload
 * right after the 6-byte inverter serial:
 *
 *   FB FB | LEN | CMD | BODY[LEN-1] | SUM_HI SUM_LO | FE FE
 *
 * LEN counts CMD + BODY. SUM is the 16-bit sum of LEN, CMD and BODY.
 * Layout, opcodes and scales cross-checked with openaps (its codec package), which
 * reverse-engineered them from the stock ECU firmware.
 */
#define APS_L2_SOF 0xFB
#define APS_L2_EOF 0xFE
#define APS_L2_OVERHEAD 7 // SOF(2) LEN(1) SUM(2) EOF(2)

// Every query sent by the ECU has a 5-byte all-zero body.
#define APS_QUERY_BODY_LEN 5
#define APS_QUERY_LEN (APS_L2_OVERHEAD + 1 + APS_QUERY_BODY_LEN)

#define APS_CMD_TELEMETRY  0xBB // query; also the DS3-class reply
#define APS_CMD_REPLY_QS1A 0xB1
#define APS_CMD_INFO       0xDC // model + firmware version
#define APS_CMD_INFO_EXT   0xDD // newer firmwares wrap the 0xDC reply in it

// First byte after the serial: FB on a cleartext frame, first byte of the
// random AES nonce otherwise. The stock firmware uses the same test.
#define APS_GATE_CLEARTEXT_MIN 0xF0

// Model codes returned by the 0xDC query
#define APS_MODEL_YC600_OLD 0x05
#define APS_MODEL_YC1000    0x06
#define APS_MODEL_YC600     0x07
#define APS_MODEL_QS1       0x08
#define APS_MODEL_YC600B    0x17
#define APS_MODEL_QS1A      0x18
#define APS_MODEL_DS3       0x20
#define APS_MODEL_DS3H      0x21
#define APS_MODEL_DS3L      0x22
#define APS_MODEL_QT2       0x32
#define APS_MODEL_QS2       0x36 // DS3-class reply, but its own field layout

typedef enum
{
    APS_FAMILY_UNKNOWN = 0,
    APS_FAMILY_DS3,
    APS_FAMILY_QS1,
    APS_FAMILY_YC600,
    APS_FAMILY_OTHER, // known to exist, no decoder here
} ApsFamily;

typedef enum
{
    APS_L2_OK = 0,
    APS_L2_SHORT,
    APS_L2_BAD_SOF,
    APS_L2_BAD_CHECKSUM,
    APS_L2_BAD_EOF,
} ApsL2Status;

// Accessors, valid once aps_check_l2() returned APS_L2_OK
#define APS_L2_CMD(f)      ((f)[3])
#define APS_L2_BODY(f)     ((f) + 4)
#define APS_L2_BODY_LEN(f) ((uint8_t)((f)[2] - 1))

// Writes a query frame with an all-zero body into `out` (APS_QUERY_LEN bytes)
// and returns its length.
uint8_t aps_build_query(uint8_t *out, uint8_t cmd);

ApsL2Status aps_check_l2(const uint8_t *frame, uint16_t len);

inline bool aps_is_encrypted(uint8_t gate) { return gate < APS_GATE_CLEARTEXT_MIN; }

ApsFamily aps_family(uint8_t model);

// Model code from a validated 0xDC reply, 0 when the shape is not recognised.
uint8_t aps_decode_info(const uint8_t *frame);

// Decodes a validated telemetry reply. `model` selects the layout; 0 (not yet
// identified) falls back to DS3 for a 0xBB reply. Returns false when there is
// no decoder for this model / reply.
bool aps_decode_telemetry(uint8_t model, const uint8_t *frame, Reading *out);

#endif /* aps_protocol_h_ */
