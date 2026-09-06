#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <logger.h>
#include <config.h>

void webserver_begin();

void webserver_loop();

#endif