#include <zb_handler.h>

static EspSoftwareSerial::UART *serial;
// Measured: running the web server and MQTT client while a frame is in flight
// pushed the frame rejection rate from 28% to 49%. The software UART buffers in
// an ISR and any latency there costs bytes, so these windows only yield().
static void zigbee_pause(uint16_t ms)
{
    const uint32_t deadline = millis() + ms;
    while ((int32_t)(millis() - deadline) < 0)
    {
        yield();
    }
}

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
#ifdef DEBUG_FRAMES
    log("S : ");
#endif

#ifdef DEBUG_FRAMES
    log(F("FE"));
    logf("-%02X", buffer_len - 2);
#endif

    // serial->flush();

    // 0xFE is the command suffix
    serial->write(ZNP_SOF);

    // size is buffer len without command 2 bytes
    serial->write(buffer_len - 2);

    for (uint8_t i = 0; i < buffer_len; i++)
    {
        chunk = buffer[i];
#ifdef DEBUG_FRAMES
        logf("-%02X", chunk);
#endif
        serial->write(chunk);
        crc ^= chunk;
    }
#ifdef DEBUG_FRAMES
    logf("-%02X", crc);
    log_line("");
#endif
    serial->write(crc);

    serial->listen();
    zigbee_pause(500);
}

uint16_t zigbee_recv(uint8_t *buffer, uint16_t timeout_ms)
{
    uint16_t index = 0;
    uint16_t size = ZB_MAX_FRAME; // real length is known once byte 1 arrives
    const uint32_t deadline = millis() + timeout_ms;

#ifdef DEBUG_FRAMES
    log(F("R : "));
#endif

    while (index < size)
    {
        if (serial->available() == 0)
        {
            if ((int32_t)(millis() - deadline) >= 0)
            {
                break; // timed out
            }
            yield();
            continue;
        }

        const uint8_t chunk = serial->read();
        buffer[index] = chunk;

        // ZNP announces the payload length in byte 1; total frame is that + 5
        if (index == 1)
        {
            size = chunk + 5;
        }

#ifdef DEBUG_FRAMES
        logf(index == 0 ? "%02X" : "-%02X", chunk);
#endif
        index++;
    }

#ifdef DEBUG
    logf_P(PSTR("R : received %d, declared %d"), index, size);
    log_line("");
#endif

    if (index == 0)
    {
        return 0; // nothing came in, not an error
    }

    // A truncated frame silently shifts every field past the missing byte, so
    // the caller would decode neighbouring bytes as valid measurements.
    // buffer[1] announces the payload length, total frame is that + 5.
    if (index < 5 || index != size)
    {
        logf_P(PSTR("incomplete frame: %d/%d bytes - dropped\n"), index, size);
        return ZB_RECV_INVALID;
    }

    // ZNP frame check sequence: XOR over LEN, CMD0, CMD1 and DATA
    uint8_t fcs = 0;
    for (uint16_t i = 1; i < index - 1; i++)
    {
        fcs ^= buffer[i];
    }
    if (fcs != buffer[index - 1])
    {
        logf_P(PSTR("bad checksum: %02X != %02X - frame dropped\n"), fcs, buffer[index - 1]);
        return ZB_RECV_INVALID;
    }

    return index;
}
