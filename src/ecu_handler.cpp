#include <ecu_handler.h>

static uint8_t zb_buffer[MAX_SERIAL_BUFFER_SIZE];

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
    uint8_t index = 0;
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
    zigbee_recv(zb_buffer);

    uint8_t index = indexOf(zb_buffer, MAX_SERIAL_BUFFER_SIZE, ECU_ID_REVERSE, 6);

    if (index != -1)
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

void ecu_pair(Inverter *inverter)
{
    #ifdef DEBUG
    log_line(F("****** Pairing ******"));
    #endif    

    uint8_t command[40] = {0};
    uint8_t commandIndex = 1;
    int8_t index = -1;
    uint8_t response_size = 0;

    while (commandIndex < 5)
    {

        switch (commandIndex)
        {
        case 1:
            memcpy(command, PAIR_1_COMMAND, 39);
            memcpy(command + 22, inverter->serial, 6);
            zigbee_send(command, 39);
            break;

        case 2:
            // command 2
            memcpy(command, PAIR_2_COMMAND, 22);
            memcpy(command + 22, inverter->serial, 6);
            zigbee_send(command, 28);
            break;

        case 3:
            memcpy(command, PAIR_3_COMMAND,39);
            memcpy(command + 22, inverter->serial, 6);
            zigbee_send(command, 39);
            break;

        case 4:
            zigbee_send(PAIR_4_COMMAND, 28);
            break;
        }
        // read incoming
        response_size = zigbee_recv(zb_buffer);
        while (response_size != 0)
        {
            memset(&zb_buffer, 0, sizeof(zb_buffer));
            response_size = zigbee_recv(zb_buffer);
            if (!inverter->paired)
            {
                // check if response contains inverter serial
                #ifdef DEBUG
                log_line(F("Checking response"));
                #endif
                index = indexOf(zb_buffer, response_size, inverter->serial, 6);

                if (index > -1)
                {
                    if (zb_buffer[index + 6] != 0xFF && zb_buffer[index + 7] != 0xFF)
                    {
                        inverter->iD[0] = zb_buffer[index + 6];
                        inverter->iD[1] = zb_buffer[index + 7];
                        #ifdef DEBUG
                    
                        log_array(inverter->iD, 2);
                        log_line(F(""));
                        #endif
                        inverter->paired = true;
                    }
                }
            }
        }
        memset(&command, 0, 40);
        commandIndex++;
    }

    memset(&command, 0, sizeof(command));
}

void ecu_poll(Inverter *inverter)
{
    #ifdef DEBUG
    log_line(F("****** Polling ******"));
    #endif
    uint8_t index = 0;
    if (inverter->_poll_command[0] == 0)
    {
        #ifdef DEBUG
        log_line(F("copying poll command"));
        #endif
        memcpy(inverter->_poll_command, POLL_1_COMMAND, 31);
        memcpy(inverter->_poll_command + 2, inverter->iD, 2);
    }
    zigbee_send(inverter->_poll_command, 31);
    memset(&zb_buffer, 0, sizeof(zb_buffer));
    #ifdef DEBUG
    log_line(F("wait 100ms"));
    #endif
    delay(100);

    index = zigbee_recv(zb_buffer);
    while (index != 0)
    {
        if (indexOf(zb_buffer, index, PAIR_NO_ROUTE, 8) > -1)
        {
            #ifdef DEBUG
            log_line(F("No route"));
            #endif
            inverter->polled =false;
            break;
        }
        #ifdef DEBUG
        if (indexOf(zb_buffer, index, POLL_AF_DATA_REQUEST, 6) > -1)
        {
            log_line(F("Data request success"));
        }
        if (indexOf(zb_buffer, index, POLL_AF_DATA_CONFIRM, 5) > -1)
        {
            log_line(F("Data confirm"));
        }
        #endif
        if (indexOf(zb_buffer, index, POLL_AF_INCOMING_MSG, 4) >-1)
        {
            #ifdef DEBUG
            log_line(F("incomming messsage"));            
            #endif
            ecu_decode_poll_answer(inverter);
        }
        
        memset(&zb_buffer, 0, sizeof(zb_buffer));
        index = zigbee_recv(zb_buffer);
    }
}
/* from yc600 
# 000-104: len 105: APsystems DS3-S ZigBee af_incoming_msg for af_data_request FBFB06BB000000000000C1FEFE
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
# 048-049: len 02: 0x08 0xd5           : temperature : °C  (raw / 100?)
# 050-053: len 04: 0x01 0x27 0x67 0xae : DC1 energy  : Wh (raw / 65535)
# 054-057: len 04: 0x00 0x00 0x58 0x1b : DC2 energy  : Wh (raw / 65535)
# 058-058: len 01: 0x00                : unk58       : 0x00 (both panel power generate), 0x02 (under/overload?), 0x03 (no power generate), 0x04 (???), 0x05 (only DC2 power generate), 0x06 (only DC1 power generate), 0x0b (boot up?), 0x0c (boot up?)
# 059-100: len 42: 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff : static59
# 101-102: len 02: 0x38 0x3c           : sum (008-100)
# 103-104: len 02: 0xfe 0xfe           : tag_end
*/

/* from DS3 
ignore the first 21
0-13  : len 14 : FE 7D 44 81 00 00 06 01 76 03 14 14 00 : Header
14-19 : len 6  : 73 00 CE 16 01 00                      : Unknown Field 1
20-31 : len 12 : 69 70 30 00 08 08 35 FB FB 5C BB BB    : Serial Number (703000080835)
32-51 : len 20 : 20 00 03 00 0F FF FF 00 00 00 00 00 00 : Unknown Field 2
52-53 : len 2  : 06 B6                                 : DC Voltage Panel 2 (DCV2)
54-55 : len 2  : 06 B4                                 : DC Voltage Panel 1 (DCV1)
56-57 : len 2  : 00 1E                                 : DC Voltage Panel 3 (DCV3)
58-59 : len 2  : 00 29                                 : DC Voltage Panel 4 (DCV4)
60-61 : len 2  : 03 7F                                 : AC Voltage (ACV)
62-63 : len 2  : 13 89                                 : Frequency
64-67 : len 4  : 04 04 00 15                           : Timestamp (4 bytes)
68-71 : len 4  : 00 1D FF FF                           : Unknown Field 3
72-75 : len 4  : 04 F4 0A 2A                           : Energy 1
76-79 : len 4  : 00 03 B1 FD                           : Energy 2
80-83 : len 4  : 00 04 D8 16                           : Unknown Field 4
84-143: len 60 : FF FF FF ... FF                       : Unknown Field (Padding or Metadata)
144-147: len 4 : 37 B9 FE FF                           : Footer
*/
void ecu_decode_poll_answer(Inverter *inverter)
{
    #ifdef DEBUG
    log_line(F("decode poll answer"));
    #endif

    // ignore 21 first element 
    uint8_t offset = 21;

    
    // 006 007 is tag start 
    if(zb_buffer[offset + 6] != 0xFB && zb_buffer[offset + 7] != 0xFB){
        #ifdef DEBUG
        log_line(F("No tag found"));
        #endif
        return;
    }

    


    // TODO : check array size, should be < 223    
    //inverter->signalQuality = (toInt(zb_buffer, offset + 14, 2) * 100) / 255;
    inverter->polled = false;
    if (inverter->invType == DS3) // DS3
    {
        #ifdef DEBUG
        log_line(F("DS3 inverter"));
        #endif

        // dc voltage
        inverter->panels[0].dcVoltage = toFloat(zb_buffer,offset + 26, 2) * DS3_DC_VOLTAGE_FACTOR;
        inverter->panels[1].dcVoltage = toFloat(zb_buffer,offset + 28, 2) * DS3_DC_VOLTAGE_FACTOR;

        // dc current
        inverter->panels[0].dcCurrent = toFloat(zb_buffer,offset + 30, 2) * DS3_DC_CURRENT_FACTOR;
        inverter->panels[1].dcCurrent = toFloat(zb_buffer,offset + 32, 2) * DS3_DC_CURRENT_FACTOR;

        // Save old energy and timestamp
        float oldEnergy = inverter->panels[0].energy + inverter->panels[1].energy;
        int oldTimestamp = inverter->timeStamp;

        // dc energy 
        inverter->panels[0].energy = (toFloat(zb_buffer,offset + 50, 4) / (float)1000 / 100) * 1.66;
        inverter->panels[1].energy = (toFloat(zb_buffer,offset + 54, 4) / (float)1000 / 100) * 1.66;

        #ifdef DEBUG
        Serial.printf_P(PSTR("DC STATE : %02X\n"), zb_buffer[offset + 24]);
        #endif

        // ac voltage
        inverter->acVoltage = toFloat(zb_buffer,offset + 34, 2) / DS3_AC_VOLTAGE_FACTOR;
        

        // Calculate DC power (V×I method)
        float dcPower = inverter->panels[0].dcVoltage * inverter->panels[0].dcCurrent;
        dcPower += inverter->panels[1].dcVoltage * inverter->panels[1].dcCurrent;

        // Calculate energy-based power if we have valid previous readings
        inverter->timeStamp = toInt(zb_buffer,offset + 38, 2);
        float newEnergy = inverter->panels[0].energy + inverter->panels[1].energy;
        int timeDiff = inverter->timeStamp - oldTimestamp;
        
        if (oldTimestamp > 0 && timeDiff > 0) {
            float energyDiff = newEnergy - oldEnergy;
            
            // Only update if energy increased (avoid negative power on reset)
            if (energyDiff > 0) {
                // Convert energy diff (Wh) to power (W) using time diff (seconds)
                inverter->acPower = (energyDiff / timeDiff) * 3600;
            } else {
                // Fallback to DC power calculation
                inverter->acPower = dcPower;
            }
        } else {
            // First reading or timestamp reset, use DC power
            inverter->acPower = dcPower;
        }
        
        // Cap power at 1000W
        if(inverter->acPower > 1000)
        {
            inverter->acPower = 0;
        }

        //dc mptt 
        inverter->dcMpttVoltage = toFloat(zb_buffer,offset + 42, 2) * DS3_DC_VOLTAGE_FACTOR;
        // freq
        inverter->frequency = toFloat(zb_buffer,offset + 36, 2) / 100;

        // temp * 0.0198 - 23.84
        inverter->temperature = toFloat(zb_buffer,offset + 48, 2) * 0.0198 - 23.84;

        inverter->status = zb_buffer[offset + 58];
        inverter->energy = newEnergy;

        
        
    }
    inverter->polled = true;
    #ifdef DEBUG
    log_inverter(inverter);
    #endif

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
