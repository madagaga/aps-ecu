#include <zb_handler.h>

static EspSoftwareSerial::UART *serial;

void zigbee_begin()
{
    serial = new EspSoftwareSerial::UART();
    serial->begin(115200, EspSoftwareSerial::SWSERIAL_8N1, RXD2, TXD2, false, 250, 0);
    pinMode(ZB_RESET, OUTPUT); // resetpin cc2530
    digitalWrite(ZB_RESET, HIGH);
}

void zigbee_reset()
{
    digitalWrite(ZB_RESET, LOW);
    delay(500);
    digitalWrite(ZB_RESET, HIGH);
    log_line("zigbee module reset");
    delay(500);
}

void zigbee_flush()
{
    serial->flush();
}

void zigbee_send(const uint8_t *buffer, uint8_t buffer_len)
{
    // copy data
    uint8_t chunk = 0;
    uint8_t crc = 0;
    crc ^= (buffer_len - 2);
    Serial.print("S : ");

#ifdef DEBUG
    Serial.print(F("FE"));
    Serial.printf("-%02X", buffer_len - 2);
#endif

    // serial->flush();

    // 0xFE is the command suffix
    serial->write(ZNP_SOF);

    // size is buffer len without command 2 bytes
    serial->write(buffer_len - 2);

    for (uint8_t i = 0; i < buffer_len; i++)
    {
        chunk = buffer[i];
#ifdef DEBUG
        Serial.printf("-%02X", chunk);
#endif
        serial->write(chunk);
        crc ^= chunk;
    }
#ifdef DEBUG
    Serial.printf("-%02X", crc);
    Serial.println();
#endif
    serial->write(crc);

    delay(500);
    serial->listen();
}

uint8_t zigbee_recv(uint8_t *buffer)
{

    uint16_t index = 0;
    uint16_t size = 255;
    uint8_t chunk;
#ifdef DEBUG
    log(F("R : "));
#endif
    uint8_t tries = 0;
    while (serial->available() == 0 && tries < 3)
    {
        delay(500); // we wait if there comes more data
        tries++;
#ifdef DEBUG
        log(F("."));
#endif
    }
    if (tries == 3)
        size = 0;

    while (index < size)
    {
        if (serial->available() == 0 && tries < 3)
        {

            delay(100);
            tries++;
#ifdef DEBUG
            log(F("."));
#endif
            continue;
        }

        if (tries == 3)
            break;

        if (index < size)
        {
            chunk = serial->read();
            buffer[index] = chunk;

            // size is buffer len without command 2 bytes and crc 1 byte
            if (index == 1)
            {
                size = chunk + 5;
            }

#ifdef DEBUG
            if (index == 0)
                Serial.printf("%02X", chunk);
            else
                Serial.printf("-%02X", chunk);
#endif

            index++;
        }
        else
        {
#ifdef DEBUG
            log_line("Inverter::_read");
#endif
            serial->read();
        }
    }

#ifdef DEBUG
    Serial.printf("  |  index : %d  |  size : %d ", index, size - 5);
    Serial.println();
#endif

    return index;
}
