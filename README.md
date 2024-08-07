# Yet another apsystem ecu

This is an ESP8266 based project to poll APSystems inverters and publish the data to mqtt.
Based on work of :  
- https://github.com/patience4711/read-APSystems-YC600-QS1-DS3
- https://github.com/No13/ApsYc600-Pythonlib
- https://gitlab.com/moreroid/apsystems/-/blob/main/zigbee/apsystems-zigbee-ds3-poll.txt?ref_type=heads

## Features

- Supports APSystems YC600, QS1 and DS3 inverters
- Supports ZigBee communication
- Supports multiple inverters
- Publishes data to mqtt
- Configurable via web interface

## Installation

First you need to install the ESP8266 Board package in platformio.

Then you can load the sketch from the platformio.

## Configuration

The configuration is stored in the `LittleFS` of the ESP8266.

### Web interface

You can access the web interface by entering the IP address of your ESP8266 in your browser.

The web interface allows you to configure the wifi connection, the mqtt broker, the inverters and the MQTT topics.

### MQTT topics

The MQTT topics for the published data are defined in the web interface.

The topics are constructed like this:

TODO



Its a work in progress actually tested with 3 DS3.




