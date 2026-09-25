
# APSystems ECU

This is an ESP8266-based project to poll APSystems inverters and publish the data to MQTT. The project supports real-time data collection from multiple APSystems inverter models such as YC600, QS1, and DS3. It also features ZigBee communication and a configurable web interface. The project is based on work from:

- [read-APSystems-YC600-QS1-DS3](https://github.com/patience4711/read-APSystems-YC600-QS1-DS3)
- [ApsYc600-Pythonlib](https://github.com/No13/ApsYc600-Pythonlib)
- [APSystems ZigBee DS3 Poll](https://gitlab.com/moreroid/apsystems/-/blob/main/zigbee/apsystems-zigbee-ds3-poll.txt?ref_type=heads)
- [OpenAPS](https://github.com/bolkedebruin/openaps): the APsystems frame layer (`FB FB ... FE FE` framing and checksum), the DS3 field layout and scales, the fault bits, the `0xDC` model query and the watchdog / online tracking approach are modelled on its reverse-engineered codec

## Features
- Supports APSystems actually DS3 inverters.  
- ZigBee communication.
- Configurable via a web interface.
- No data retention or history.
- Supports up to 64 inverters (12 bytes of RAM each); tested with 3 inverters and 2 panels on each.
- Short addresses are saved after pairing: no pairing on the next boot.
- The zigbee module is reinitialised when it stops answering.
- Publishes data to mqtt - actually user/password not supported 

## Installation
The project can be installed using PlatformIO with the ESP8266 Board package. Both the firmware and data partition are required for setup.

1. Clone the repository.
2. Install the ESP8266 Board package in PlatformIO.
3. Build and flash the project, including the data partition.

You can flash the firmware without pre-configuring Wi-Fi settings.

## Configuration
The configuration is stored in the `LittleFS` file system of the ESP8266.

### Web Interface
After flashing the firmware, the device will start in default mode. An access point (AP) named "APS_ECU" with the password "12345678" will be available. Connect to this AP and access the web interface by entering the default IP address `192.168.4.1` in your browser.

Through the web interface, you can:
- Configure Wi-Fi connection.
- Set up the MQTT broker and topics.
- Add inverters for monitoring.

Once Wi-Fi is configured, you can access the local web server via the device's IP address on your network.

### MQTT Topics
The MQTT topics for the published data are defined in the web interface. They follow a structure that you can customize based on your configuration.

#### Example MQTT Payload
One JSON object per inverter and per poll (DS3, values from a real frame):

```json
{
  "type": "inverter",
  "serial": "703000080835",
  "addr": "7603",
  "model": 32,
  "online": true,
  "lqi": 115,
  "power": 21,
  "reactive": 29,
  "voltage": 239.9,
  "frequency": 50.01,
  "temperature": 27.6,
  "counter": 1028,
  "status": "0000000000",
  "faults": 0,
  "energy": 9,
  "deviceID": "AA:BB:CC:DD:EE:FF",
  "panels": [
    {"voltage": 34.36, "current": 0.352, "energy": 4},
    {"voltage": 34.32, "current": 0.481, "energy": 5}
  ]
}
```

When an inverter stops answering (30 failed polls in a row) a single message is sent:

```json
{"type": "inverter", "serial": "703000080835", "online": false, "deviceID": "AA:BB:CC:DD:EE:FF"}
```

**Payload Fields**:
- `serial`: inverter serial, as printed on the label.
- `addr`: zigbee short address assigned at pairing.
- `model`: model code reported by the inverter (32 = DS3, 33 = DS3-H, 34 = DS3-L), 0 until identified.
- `lqi`: link quality of the reply, 0-255.
- `power`: AC power in W, as measured by the inverter. `reactive`: reactive power in VAR.
- `voltage` (V), `frequency` (Hz), `temperature` (degC): AC side.
- `counter`: inverter uptime counter in seconds.
- `status`: raw status bytes (hex). `faults`: bit mask decoded from them:
  1 AC over-voltage, 2 AC under-voltage, 4 over-frequency, 8 under-frequency, 16 grid relay,
  32 DC bus, 64 DC contactor, 128 DC ground, 256 isolation.
- `energy`: energy counter of the inverter in Wh, total and per panel. **Not a lifetime total**:
  the inverter resets it by itself, the consumer has to accumulate deltas.
- `panels`: DC voltage (V), current (A) and energy (Wh) per input.
- `deviceID`: MAC address of the ESP.

Only DS3 replies are decoded; other models are identified and logged but not published.

## Usage
1. **Flashing the Firmware**: Flash the firmware without Wi-Fi pre-configured if desired.
2. **Wi-Fi Configuration**: After flashing, connect to the "APS_ECU" access point and use the web interface to configure Wi-Fi settings.
3. **Accessing the Local Web Server**: Once the device is connected to your network, access the web interface through the device’s IP address for configuration.
4. **Monitoring and MQTT**: The system will automatically monitor DS3 inverters and push their data to the specified MQTT broker.

## Dependencies
- `plerup/EspSoftwareSerial@^8.1.0`
- `knolleary/PubSubClient@^2.8`

## pinouts  
- RX pin D7 
- TX pin D8
- ZB_RESET pin D5
