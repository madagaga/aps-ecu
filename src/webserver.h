#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <logger.h>
#include <config.h>

static ESP8266WebServer server(80); // Create a webserver object that listens for HTTP request on port 80

void webserver_begin();

void webserver_loop();

#endif