#include <aps_protocol.h>
#include <string.h>

static uint16_t be16(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) | p[1];
}

static uint32_t be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static uint16_t sat16(uint32_t v)
{
    return v > 0xFFFF ? 0xFFFF : (uint16_t)v;
}

uint8_t aps_build_query(uint8_t *out, uint8_t cmd)
{
    const uint8_t len = 1 + APS_QUERY_BODY_LEN;
    // the body is all zero, so the sum is just LEN + CMD
    const uint16_t sum = len + cmd;

    out[0] = APS_L2_SOF;
    out[1] = APS_L2_SOF;
    out[2] = len;
    out[3] = cmd;
    memset(out + 4, 0, APS_QUERY_BODY_LEN);
    out[4 + APS_QUERY_BODY_LEN] = sum >> 8;
    out[5 + APS_QUERY_BODY_LEN] = sum & 0xFF;
    out[6 + APS_QUERY_BODY_LEN] = APS_L2_EOF;
    out[7 + APS_QUERY_BODY_LEN] = APS_L2_EOF;
    return APS_QUERY_LEN;
}

ApsL2Status aps_check_l2(const uint8_t *f, uint16_t len)
{
    if (len < APS_L2_OVERHEAD + 1)
    {
        return APS_L2_SHORT;
    }
    if (f[0] != APS_L2_SOF || f[1] != APS_L2_SOF)
    {
        return APS_L2_BAD_SOF;
    }

    const uint8_t inner = f[2]; // CMD + BODY
    if (inner == 0 || inner + APS_L2_OVERHEAD > len)
    {
        return APS_L2_SHORT;
    }

    uint16_t sum = 0;
    for (uint16_t i = 2; i <= 2u + inner; i++)
    {
        sum += f[i];
    }

    const uint8_t *tail = f + 3 + inner;
    if (tail[0] != (sum >> 8) || tail[1] != (sum & 0xFF))
    {
        return APS_L2_BAD_CHECKSUM;
    }
    if (tail[2] != APS_L2_EOF || tail[3] != APS_L2_EOF)
    {
        return APS_L2_BAD_EOF;
    }
    return APS_L2_OK;
}

ApsFamily aps_family(uint8_t model)
{
    switch (model)
    {
    case APS_MODEL_DS3:
    case APS_MODEL_DS3H:
    case APS_MODEL_DS3L:
        return APS_FAMILY_DS3;
    case APS_MODEL_QS1:
    case APS_MODEL_QS1A:
        return APS_FAMILY_QS1;
    case APS_MODEL_YC600_OLD:
    case APS_MODEL_YC600:
    case APS_MODEL_YC600B:
        return APS_FAMILY_YC600;
    case APS_MODEL_YC1000:
    case APS_MODEL_QT2:
    case APS_MODEL_QS2:
        return APS_FAMILY_OTHER;
    default:
        return APS_FAMILY_UNKNOWN;
    }
}

/*
 * Three 0xDC reply shapes exist, told apart by LEN:
 *   09  FB FB 09 DC <model> ...        older firmware
 *   0A  FB FB 0A DD DC <model> ...     two-part version
 *   0C  FB FB 0C DD DC <model> ...     three-part version
 */
uint8_t aps_decode_info(const uint8_t *f)
{
    switch (f[2])
    {
    case 0x09:
        return f[3] == APS_CMD_INFO ? f[4] : 0;
    case 0x0A:
    case 0x0C:
        return (f[3] == APS_CMD_INFO_EXT && f[4] == APS_CMD_INFO) ? f[5] : 0;
    default:
        return 0;
    }
}

/*
 * DS3 reply body, offset 0 = first byte after CMD (0xBB). Add 10 to get the
 * offset in the zigbee payload (serial + FB FB LEN CMD).
 *
 *   0x0B-0x0F  status / fault bits
 *   0x10 0x12  DC voltage A B     raw * 0.020 V
 *   0x14 0x16  DC current A B     raw * 0.01172 A
 *   0x18       AC voltage         raw * 0.268 V
 *   0x1A       frequency          raw * 0.01 Hz
 *   0x1C       uptime counter     s
 *   0x1E       AC power           W, as measured by the inverter
 *   0x20       reactive power     VAR, signed
 *   0x26       temperature        raw * 0.0198 - 23.84 degC (empirical, not in openaps)
 *   0x28 0x2C  energy A B         raw * 1.674e-5 Wh, 32 bits
 */
#define DS3_BODY_MIN 0x30

static uint16_t ds3_faults(const uint8_t *s)
{
    const uint8_t bC = s[1], bD = s[2], bE = s[3], bF = s[4];
    uint16_t faults = 0;

    if (bE & 0x05) faults |= FAULT_AC_OVER_VOLTAGE;  // stage 1, 2
    if (bE & 0x0A) faults |= FAULT_AC_UNDER_VOLTAGE; // stage 1, 2
    if (bE & 0x50) faults |= FAULT_ISOLATION;        // A, B
    if (bF & 0x55) faults |= FAULT_OVER_FREQUENCY;   // stage 1, 2, aux, extra
    if (bF & 0xAA) faults |= FAULT_UNDER_FREQUENCY;
    if (bC & 0x02) faults |= FAULT_GRID_RELAY;
    if (bD & 0x80) faults |= FAULT_DC_CONTACTOR;
    if (bD & 0x10) faults |= FAULT_DC_BUS;
    if (bD & 0x08) faults |= FAULT_DC_GROUND;
    return faults;
}

static void decode_ds3(const uint8_t *b, Reading *r)
{
    r->panelCount = 2;
    for (uint8_t i = 0; i < 2; i++)
    {
        PanelReading *p = &r->panels[i];
        p->voltage_cV = sat16((uint32_t)be16(b + 0x10 + 2 * i) * 2);
        p->current_mA = sat16(((uint32_t)be16(b + 0x14 + 2 * i) * 1172 + 50) / 100);
        p->energy_Wh = (uint32_t)(((uint64_t)be32(b + 0x28 + 4 * i) * 1674) / 100000000);
    }

    r->acVoltage_dV = sat16(((uint32_t)be16(b + 0x18) * 268 + 50) / 100);
    r->frequency_cHz = be16(b + 0x1A);
    r->counter_s = be16(b + 0x1C);
    r->acPower_W = be16(b + 0x1E);
    r->reactive_VAR = (int16_t)be16(b + 0x20);
    r->temperature_dC = (int16_t)(((int32_t)be16(b + 0x26) * 198 - 238400) / 1000);

    memcpy(r->status, b + 0x0B, sizeof(r->status));
    r->faults = ds3_faults(r->status);
}

bool aps_decode_telemetry(uint8_t model, const uint8_t *f, Reading *out)
{
    const uint8_t cmd = APS_L2_CMD(f);
    const ApsFamily family = aps_family(model);

    memset(out, 0, sizeof(*out));

    if (cmd == APS_CMD_TELEMETRY && (family == APS_FAMILY_DS3 || model == 0))
    {
        if (APS_L2_BODY_LEN(f) < DS3_BODY_MIN)
        {
            return false;
        }
        decode_ds3(APS_L2_BODY(f), out);
        return true;
    }

    // QS1 (0xB1), YC600, QS2...: layouts known from openaps / the notes below,
    // no decoder until one can be tested on real hardware
    return false;
}

/* Notes kept for a future YC600 decoder (from ApsYc600-Pythonlib), offsets in
   the zigbee payload, i.e. body offset + 10:
# 000-005: len 06: 0x70 0x20 0x00 0xaa 0xbb 0xcc : inverter serial number
# 006-007: len 02: 0xfb 0xfb           : tag_start
# 008-008: len 01: 0x5c                : datalen (without sum data)
# 009-010: len 02: 0xbb 0xbb           : static09
# 011-011: len 01: 0x20                : version_patch?
# 012-012: len 01: 0x00                : static12
# 013-013: len 01: 0x02                : version_minor?
# 014-014: len 01: 0x01                : version_major?
# 015-021: len 07: 0x0f 0xff 0xff 0x00 0x00 0x00 0x00 : static15
# 022-022: len 01: 0x00                : unk22       : 0x00 (default), 0x01 (overload?), 0x80 (startup?)
# 023-023: len 01: 0x00                : unk23       : 0x00 (default), 0x,02 (???), 0x40 (short after startup)
# 024-024: len 01: 0x00                : unk24       : 0x00 (default), 0x0a (startup/shutdown?), 0x20 (DC1 missing), 0x80 (DC2 missing), 0xaa (no power?)
# 025-025: len 01: 0x00                : unk25       : 0x00 (default), 0xaa (startup/shutdown?)
# 026-027: len 02: 0x07 0xcf           : DC1 voltage : V (raw / 48) confirm by measurement
# 028-029: len 02: 0x01 0x96           : DC2 voltage : V (raw / 48) confirm by measurement
# 030-031: len 02: 0x00 0xb3           : DC1 current : A (raw / 80) confirm by measurement
# 032-033: len 02: 0x00 0x01           : DC2 current : A (raw / 80) confirm by measurement
# 034-035: len 02: 0x03 0x75           : AC Voltage  : V (raw / 3.75?)
# 036-037: len 02: 0x13 0x87           : AC Freq     : Hz (raw / 100)
# 038-039: len 02: 0x4f 0xc8           : uptime      : seconds (counter reset after 43200 will reset DC* energy also)
# 040-041: len 02: 0x00 0x4b           : AC power    : W (max seen 651 W)
# 042-043: len 02: 0x00 0x30           : DC mppt?    : V (24 until 66)
# 044-045: len 02: 0xff 0xff           : static44
# 046-047: len 02: 0x05 0x36           : unk46       : 0x0021 (at starting), 0x055a[1370] (max seen), 0x02fc (ending)
# 048-049: len 02: 0x08 0xd5           : temperature : degC  (raw / 100?)
# 050-053: len 04: 0x01 0x27 0x67 0xae : DC1 energy  : Wh (raw / 65535)
# 054-057: len 04: 0x00 0x00 0x58 0x1b : DC2 energy  : Wh (raw / 65535)
# 058-058: len 01: 0x00                : unk58       : 0x00 (both panel power generate), 0x02 (under/overload?), 0x03 (no power generate), 0x04 (???), 0x05 (only DC2 power generate), 0x06 (only DC1 power generate), 0x0b (boot up?), 0x0c (boot up?)
# 059-100: len 42: 0xff ... : static59
# 101-102: len 02: 0x38 0x3c           : sum (008-100)
# 103-104: len 02: 0xfe 0xfe           : tag_end
*/
