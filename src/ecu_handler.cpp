#include <ecu_handler.h>

static uint8_t zb_buffer[ZB_BUFFER_SIZE];

void ecu_begin()
{
    zigbee_begin();
}

void ecu_reset()
{
    zigbee_reset();
    delay(2000); // wait for the cc2530 to reboot
    zigbee_recv(zb_buffer);
}

void ecu_initialize()
{

    /*
     * init the coordinator takes the following procedure
     * 1st we send a resetcommand 4 times Sent=FE0141000040
     * then we send the following commands
     *  0 Sent=FE 03 26 05 03 01 03 21
     *  Received=FE0166050062
     *  1 Sent=FE01 41 00 00 40
     *  Received=FE064180020202020702C2
     *  2 Sent=FE0A 26 05 01 08 FF FF 80 97 1B 01 A3 D8 56
     *  Received=FE01 66 05 00 62
     *  3 Sent=FE03 26 05 87 01 00 A6
     *  Received=FE0166050062 FE-01-66-05-00-62
     *  4 Sent=FE04 26 05 83 02 D8A3 DD  should be ecu_id the fst 2 bytes
     *  Received=FE0166050062
     *  5 Sent=FE06 26 05 84 04 00 00 01 00 A4
     *  Received=FE0166050062
     *  6 Sent=FE0D240014050F0001010002000015000020
     *  Received=FE01 64 00 00 65
     *  7 Sent=FE00 26 00 26
     *
     *  8 Sent=FE00 67 00 67
     *  Received=FE0145C0098D
     *  received FE00660066 FE0145C0088C FE0145C0098D F0F8FE0E670000FFFF80971B01A3D8000007090011
     *  now we can pair if we want to or else an extra command for retrieving data (normal operation)
     *  9 for normal operation we send cmd 9
     *  Finished. Heap=26712
     *
     */
    #ifdef DEBUG
    log_line(F("Reset ZB coordinator"));
    #endif
    // we start with a hard reset of the zb module
    zigbee_reset();
    uint16_t index = 0;
#ifdef DEBUG
    log_line(F("****** Init ******"));
#endif
    uint8_t commandIndex = 0;

    while (commandIndex < 9)
    {
        switch (commandIndex)
        {
        case 0:
            zigbee_send(INIT_0_COMMAND, 5);
            break;
        case 1:
            zigbee_send(INIT_1_COMMAND, 3);
            break;
        case 2:
            zigbee_send(INIT_2_COMMAND, 12);
            break;
        case 3:
            zigbee_send(INIT_3_COMMAND, 5);
            break;
        case 4:
            zigbee_send(INIT_4_COMMAND, 6);
            break;
        case 5:
            zigbee_send(INIT_5_COMMAND, 8);
            break;
        case 6:
            zigbee_send(INIT_6_COMMAND, 15);
            break;
        case 7:
            zigbee_send(INIT_7_COMMAND, 2);
            delay(500);
            break;
        case 8:
            zigbee_send(INIT_8_COMMAND, 2);
            break;
        }
        commandIndex++;
        index = zigbee_recv(zb_buffer);
        while (index != 0)
        {
            memset(zb_buffer, 0, sizeof(zb_buffer));
            index = zigbee_recv(zb_buffer);
        }
    }
}

/*
 * Send : FE 00 21 01 20
 * Receive : FE 02 61 01 79 07 1C
 */
void ecu_ping()
{
    #ifdef DEBUG
    log_line(F("Ping"));
    #endif

    zigbee_send(PING_COMMAND, sizeof(PING_COMMAND));
    zigbee_recv(zb_buffer);
}

/*
 * Send : FE00270027
 * Receive :
 * check if received contains ECU_ID_REVERSE
 * check if received contains "0709" after ECU_ID_REVERSE
 */

// the response = 67 00, status 1 bt, IEEEAddr 8bt, ShortAddr 2bt, DeviceType 1bt, Device State 1bt
//  FE0E 67 00 00 FFFF 80971B01A3D8 0000 0709001
// status = 00 means succes, IEEEAddress= FFFF80971B01A3D8, ShortAdr = 0000, devicetype=07 bits 0 to 2

bool ecu_heart_beat()
{
    #ifdef DEBUG
    log_line(F("Heart beat"));
    #endif
    const uint8_t check[2] = {0x07, 0x09};
    zigbee_send(HEART_BEAT_COMMAND, sizeof(HEART_BEAT_COMMAND));
    memset(&zb_buffer, 0, sizeof(zb_buffer));
    const uint16_t received = zigbee_recv(zb_buffer);

    if (received == 0 || received == ZB_RECV_INVALID)
    {
        return false;
    }

    // searching the whole buffer could match leftovers from an earlier frame
    const int16_t index = indexOf(zb_buffer, received, ECU_ID_REVERSE, 6);

    if (index > -1 && index + 9 < (int16_t)received)
    {
        if (zb_buffer[index + 8] == check[0] && zb_buffer[index + 9] == check[1])
        {
            #ifdef DEBUG
            log_line(F("Alive"));
            #endif
            return true;
        }
    }

    return false;
}

/*
 * The module answers the heartbeat even when every inverter sleeps, so a
 * missing answer means the module (or the UART) is wedged, not that it is
 * night. One lost frame is not enough to reset it, and a reset that does not
 * help is not retried in a loop: each one tears the network down.
 */
bool ecu_check_alive()
{
    static uint8_t failures = 0;
    static bool reinitialised = false;
    static uint32_t lastReinit = 0;
    static uint32_t backoff = ECU_WATCHDOG_BACKOFF_MIN_MS;

    if (ecu_heart_beat())
    {
        failures = 0;
        backoff = ECU_WATCHDOG_BACKOFF_MIN_MS;
        return true;
    }

    if (failures < 255)
    {
        failures++;
    }
    logf_P(PSTR("heartbeat failed (%u in a row)\n"), failures);

    if (failures < ECU_WATCHDOG_FAILURES)
    {
        return true;
    }
    if (reinitialised && millis() - lastReinit < backoff)
    {
        return false;
    }

    logf_P(PSTR("zigbee module not answering - reinitializing (next attempt in %lus at the earliest)\n"),
           backoff / 1000);
    ecu_initialize();
    ecu_noop();
    reinitialised = true;
    lastReinit = millis();
    backoff = backoff >= ECU_WATCHDOG_BACKOFF_MAX_MS / 2 ? ECU_WATCHDOG_BACKOFF_MAX_MS : backoff * 2;
    return false;
}

/*
                                                      inverter sn             ecu reverse sn
 Send : 24020FFFFFFFFFFFFFFFFF14FFFF14 0D0200000F1100 408000158215 FFFF10FFFF 80971B01A3D8
 Receive :

                                                      inverter sn
 Send : 24020FFFFFFFFFFFFFFFFF14FFFF14 0C0201000F0600 408000158215
 Receive :

                                                      inverter sn  ecu         ecu reverse sn
 Send : 24020FFFFFFFFFFFFFFFFF14FFFF14 0F0102000F1100 408000158215 A3D8 10FFFF 80971B01A3D8
 Receive :

 Send : 4020FFFFFFFFFFFFFFFFF14FFFF14 010103000F0600 80971B01A3D8
 Receive :

*/

bool ecu_pair(Inverter *inverter)
{
    #ifdef DEBUG
    log_line(F("****** Pairing ******"));
    #endif

    uint8_t command[39];

    for (uint8_t step = 1; step <= 4; step++)
    {
        switch (step)
        {
        case 1:
            memcpy(command, PAIR_1_COMMAND, 39);
            memcpy(command + 22, inverter->serial, 6);
            zigbee_send(command, 39);
            break;

        case 2:
            memcpy(command, PAIR_2_COMMAND, 22);
            memcpy(command + 22, inverter->serial, 6);
            zigbee_send(command, 28);
            break;

        case 3:
            memcpy(command, PAIR_3_COMMAND, 39);
            memcpy(command + 22, inverter->serial, 6);
            zigbee_send(command, 39);
            break;

        case 4:
            zigbee_send(PAIR_4_COMMAND, 28);
            break;
        }

        // every frame of the answer is checked, the first one included
        uint16_t size;
        while ((size = zigbee_recv(zb_buffer)) != 0)
        {
            if (size == ZB_RECV_INVALID || (inverter->flags & INV_PAIRED))
            {
                continue;
            }

            // the short address follows the inverter serial
            const int16_t index = indexOf(zb_buffer, size, inverter->serial, 6);
            if (index < 0 || index + 7 >= (int16_t)size)
            {
                continue;
            }
            if (zb_buffer[index + 6] != 0xFF && zb_buffer[index + 7] != 0xFF)
            {
                inverter->addr[0] = zb_buffer[index + 6];
                inverter->addr[1] = zb_buffer[index + 7];
                inverter->flags |= INV_PAIRED;
                logf_P(PSTR("paired, short address %02X%02X\n"), inverter->addr[0], inverter->addr[1]);
            }
        }
    }

    return inverter->flags & INV_PAIRED;
}

// Frames left over from the previous exchange (late route indications, a
// reply that came after its deadline) must not be taken for this one's.
static void ecu_drain()
{
    for (uint8_t i = 0; i < 8 && zigbee_recv(zb_buffer, 20) != 0; i++)
    {
    }
}

static void ecu_send_query(const Inverter *inverter, uint8_t cmd)
{
    uint8_t frame[sizeof(AF_UNICAST_HEADER) + sizeof(ECU_ID_REVERSE) + APS_QUERY_LEN];

    memcpy(frame, AF_UNICAST_HEADER, sizeof(AF_UNICAST_HEADER));
    frame[AF_UNICAST_ADDR_OFFSET] = inverter->addr[0];
    frame[AF_UNICAST_ADDR_OFFSET + 1] = inverter->addr[1];
    frame[AF_UNICAST_LEN_OFFSET] = sizeof(ECU_ID_REVERSE) + APS_QUERY_LEN;
    memcpy(frame + sizeof(AF_UNICAST_HEADER), ECU_ID_REVERSE, sizeof(ECU_ID_REVERSE));
    aps_build_query(frame + sizeof(AF_UNICAST_HEADER) + sizeof(ECU_ID_REVERSE), cmd);

    zigbee_send(frame, sizeof(frame));
}

typedef enum
{
    ECU_REPLY_OK,
    ECU_REPLY_TIMEOUT,
    ECU_REPLY_NO_ROUTE,
    ECU_REPLY_REJECTED,
    ECU_REPLY_ENCRYPTED,
} EcuReply;

/*
 * Sends one query and waits for the matching reply. On ECU_REPLY_OK `*l2`
 * points into zb_buffer at a validated APsystems frame (FB FB ... FE FE),
 * valid until the next zigbee_recv().
 *
 * Returns as soon as the reply is in instead of waiting for the line to go
 * quiet: with a large fleet the idle wait was most of the round.
 */
static EcuReply ecu_transact(Inverter *inverter, uint8_t cmd, const uint8_t **l2)
{
    ecu_drain();
    ecu_send_query(inverter, cmd);

    const uint32_t deadline = millis() + ECU_REPLY_BUDGET_MS;
    while ((int32_t)(millis() - deadline) < 0)
    {
        const uint16_t received = zigbee_recv(zb_buffer);
        if (received == 0)
        {
            return ECU_REPLY_TIMEOUT;
        }
        if (received == ZB_RECV_INVALID)
        {
            continue;
        }

        const uint16_t znp = ZNP_CMD(zb_buffer);
        const uint8_t znpLen = ZNP_LEN(zb_buffer);
        const uint8_t *data = ZNP_DATA(zb_buffer);
        const uint8_t status = znpLen > 0 ? data[0] : 0xFF;

        switch (znp)
        {
        case ZNP_AF_DATA_REQUEST_SRSP:
            if (status != AF_STATUS_SUCCESS)
            {
                logf_P(PSTR("data request refused by the module: %02X\n"), status);
                return ECU_REPLY_REJECTED;
            }
            break;

        // the original code read this response as "unreachable"; kept as-is
        case ZNP_AF_DATA_REQUEST_EXT_SRSP:
            #ifdef DEBUG
            log_line(F("Pair unreachable"));
            #endif
            return ECU_REPLY_REJECTED;

        case ZNP_AF_DATA_CONFIRM:
            if (status == AF_STATUS_NO_ROUTE)
            {
                #ifdef DEBUG
                log_line(F("No route"));
                #endif
                return ECU_REPLY_NO_ROUTE;
            }
            break;

        case ZNP_AF_INCOMING_MSG:
        {
            if (znpLen < AF_HEADER_SIZE)
            {
                log_line(F("truncated AF header - ignored"));
                break;
            }
            // a reply from another inverter would otherwise be decoded into
            // this one's measurements
            if (AF_SRC_ADDR(data)[0] != inverter->addr[0] ||
                AF_SRC_ADDR(data)[1] != inverter->addr[1])
            {
                logf_P(PSTR("reply from %02X%02X while polling %02X%02X - ignored\n"),
                       AF_SRC_ADDR(data)[0], AF_SRC_ADDR(data)[1],
                       inverter->addr[0], inverter->addr[1]);
                break;
            }
            const uint8_t payloadLen = AF_PAYLOAD_LEN(data);
            if (AF_HEADER_SIZE + payloadLen > znpLen)
            {
                log_line(F("AF payload longer than frame - ignored"));
                break;
            }

            // payload = inverter serial (6) + APsystems frame
            const uint8_t *payload = AF_PAYLOAD(data);
            if (payloadLen < 7 || memcmp(payload, inverter->serial, 6) != 0)
            {
                log_line(F("reply without this inverter's serial - ignored"));
                break;
            }

            inverter->lqi = AF_LINK_QUALITY(data);

            if (aps_is_encrypted(payload[6]))
            {
                if (!(inverter->flags & INV_ENCRYPTED))
                {
                    log_line(F("inverter replies with AES encrypted frames - not supported"));
                }
                inverter->flags |= INV_ENCRYPTED;
                return ECU_REPLY_ENCRYPTED;
            }
            inverter->flags &= ~INV_ENCRYPTED;

            const ApsL2Status l2Status = aps_check_l2(payload + 6, payloadLen - 6);
            if (l2Status != APS_L2_OK)
            {
                logf_P(PSTR("bad APsystems frame (error %u) - dropped\n"), l2Status);
                break;
            }

            *l2 = payload + 6;
            return ECU_REPLY_OK;
        }

        // unsolicited, and the CC2530 sends ZDO_SRC_RTG_IND twice per poll
        case ZNP_ZDO_STATE_CHANGE_IND:
        case ZNP_ZDO_SRC_RTG_IND:
            break;

        default:
            #ifdef DEBUG
            logf_P(PSTR("unhandled ZNP command %04X\n"), znp);
            #endif
            break;
        }
    }

    return ECU_REPLY_TIMEOUT;
}

bool ecu_identify(Inverter *inverter)
{
    inverter->flags |= INV_ID_TRIED;

    const uint8_t *l2 = NULL;
    if (ecu_transact(inverter, APS_CMD_INFO, &l2) != ECU_REPLY_OK)
    {
        return false;
    }

    const uint8_t model = aps_decode_info(l2);
    if (model == 0)
    {
        logf_P(PSTR("unrecognised info reply: len %02X cmd %02X\n"), l2[2], APS_L2_CMD(l2));
        return false;
    }

    inverter->model = model;
    logf_P(PSTR("inverter %02X%02X: model %02X%s\n"), inverter->addr[0], inverter->addr[1], model,
           aps_family(model) == APS_FAMILY_UNKNOWN ? " (unknown)" : "");
    return true;
}

EcuPollResult ecu_poll(Inverter *inverter, Reading *reading)
{
    #ifdef DEBUG
    log_line(F("****** Polling ******"));
    #endif

    const uint8_t *l2 = NULL;
    if (ecu_transact(inverter, APS_CMD_TELEMETRY, &l2) == ECU_REPLY_OK)
    {
        // it answered: online, whether or not the reply can be decoded
        inverter->missed = 0;
        inverter->flags |= INV_ONLINE;

        if (aps_decode_telemetry(inverter->model, l2, reading))
        {
            return ECU_POLL_OK;
        }
        logf_P(PSTR("no decoder for model %02X, reply %02X\n"), inverter->model, APS_L2_CMD(l2));
        return ECU_POLL_UNSUPPORTED;
    }

    if (inverter->missed < 255)
    {
        inverter->missed++;
    }
    if (inverter->missed == ECU_OFFLINE_AFTER && (inverter->flags & INV_ONLINE))
    {
        // identify again when it comes back: it may have been replaced
        inverter->flags &= ~(INV_ONLINE | INV_ID_TRIED);
        return ECU_POLL_WENT_OFFLINE;
    }
    return ECU_POLL_FAILED;
}

void ecu_noop()
{
    #ifdef DEBUG
    log_line(F("****** NOOP ******"));
    #endif

    zigbee_send(NOOP_COMMAND, 42);

    zigbee_recv(zb_buffer);
    zigbee_recv(zb_buffer);
}
