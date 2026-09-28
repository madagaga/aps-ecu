#ifndef panel_data_h_
#define panel_data_h_

#include <stdint.h>

#define APS_MAX_PANELS 4

// Inverter::flags
#define INV_PAIRED    0x01 // short address known
#define INV_ONLINE    0x02 // answered recently (cleared after ECU_OFFLINE_AFTER misses)
#define INV_ENCRYPTED 0x04 // replied with an AES-wrapped frame (not decodable)
#define INV_ID_TRIED  0x08 // model query already sent during this online period

/*
 * Persistent per-inverter state. This is the only structure that grows with
 * the fleet, so it holds nothing that is only needed while a reply is being
 * decoded: measurements go to a single shared Reading instead.
 */
typedef struct
{
    uint8_t serial[6]; // BCD, as on the wire: 70 30 00 08 08 35
    uint8_t addr[2];   // zigbee short address, wire order; 00 00 = unknown
    uint8_t model;     // model code from the 0xDC info reply, 0 = unknown
    uint8_t flags;     // INV_*
    uint8_t missed;    // consecutive failed polls, saturates at 255
    uint8_t lqi;       // link quality of the last reply, 0-255
} Inverter;

static_assert(sizeof(Inverter) == 12, "Inverter must stay 12 bytes");

// Reading::faults, family independent summary of the raw status bytes
#define FAULT_AC_OVER_VOLTAGE   0x0001
#define FAULT_AC_UNDER_VOLTAGE  0x0002
#define FAULT_OVER_FREQUENCY    0x0004
#define FAULT_UNDER_FREQUENCY   0x0008
#define FAULT_GRID_RELAY        0x0010
#define FAULT_DC_BUS            0x0020
#define FAULT_DC_CONTACTOR      0x0040
#define FAULT_DC_GROUND         0x0080
#define FAULT_ISOLATION         0x0100

typedef struct
{
    uint16_t voltage_cV; // 0.01 V
    uint16_t current_mA;
    uint32_t energy_Wh;  // inverter counter, resets on its own (daily / wrap)
} PanelReading;

/*
 * One decoded telemetry reply, in fixed units whatever the inverter family.
 * A single instance is reused for every inverter: decode, publish, overwrite.
 */
typedef struct
{
    uint16_t acPower_W;
    int16_t reactive_VAR;
    uint16_t acVoltage_dV;  // 0.1 V
    uint16_t frequency_cHz; // 0.01 Hz
    int16_t temperature_dC; // 0.1 degC
    uint16_t counter_s;     // inverter uptime counter
    uint16_t faults;        // FAULT_*
    uint8_t status[5];      // raw status bytes, family specific
    uint8_t panelCount;
    PanelReading panels[APS_MAX_PANELS];
} Reading;

#endif /* panel_data_h_ */
