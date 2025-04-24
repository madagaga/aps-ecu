
# APSystems ECU

This is an ESP8266-based project to poll APSystems inverters and publish the data to MQTT. The project supports real-time data collection from multiple APSystems inverter models such as YC600, QS1, and DS3. It also features ZigBee communication and a configurable web interface. The project is based on work from:

- [read-APSystems-YC600-QS1-DS3](https://github.com/patience4711/read-APSystems-YC600-QS1-DS3)
- [ApsYc600-Pythonlib](https://github.com/No13/ApsYc600-Pythonlib)
- [APSystems ZigBee DS3 Poll](https://gitlab.com/moreroid/apsystems/-/blob/main/zigbee/apsystems-zigbee-ds3-poll.txt?ref_type=heads)

## Features
- Supports APSystems actually DS3 inverters.  
- ZigBee communication.
- Configurable via a web interface.
- No data retention or history.
- Supports multiple inverters (may be limited by memory) tested with 3 inverters and 2 panel on each. 
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
The payload sent to the MQTT broker is a JSON object containing the inverter's data. Below is an example structure:

```json
{
  "type": "inverter",
  "serial": "12-34-56-78-9A-BC",
  "id": "12-34",
  "invType": 1,
  "index": 0,
  "polled": 1,
  "power": 350,
  "frequency": 60.00,
  "temperature": 35.50,
  "voltage": 230.00,
  "energy": 1234.56,
  "mac": "AA:BB:CC:DD:EE:FF"
}
```

**Payload Fields**:
- `type`: DS3 or other .
- `serial`: Serial number of the inverter.
- `id`: Identifier of the inverter.
- `invType`: Inverter type (numerical value).
- `index`: Index of the inverter in the system.
- `polled`: Indicates if the inverter has been successfully polled.
- `power`: Current power output (in watts).
- `frequency`: Frequency in hertz (Hz).
- `temperature`: Temperature in degrees Celsius.
- `voltage`: Voltage in volts (V).
- `energy`: Energy produced in kilowatt-hours (kWh).
- `mac`: MAC address of the device that polled the inverter.

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
