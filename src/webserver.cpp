#include <webserver.h>

void handleConfig(){

  String s = server.arg("plain");
  File file = LittleFS.open(CONFIG_PATH, "w");
  file.print(s);
  file.close();

  log_line("Config updated");
  server.send(200, "text/plain", "Configuration updated");
  system_restart();
}

void handleInverterConfig(){

  String s = server.arg("plain");
  File file = LittleFS.open(INVERTER_PATH, "w");
  file.print(s);
  file.close();

  log_line("Config updated");
  server.send(200, "text/plain", "Configuration updated");
  system_restart();
}


bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.printf("handleFileRead: %s\n",path.c_str());
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = path.endsWith(".html") ? "text/html" : "text/plain";            // Get the MIME type
  if (LittleFS.exists(path)) {                            // If the file exists
    File file = LittleFS.open(path, "r");                 // Open it
    server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  log_line("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}


void webserver_begin()
{
    server.on("/config", HTTP_POST, handleConfig);
    server.on("/inverters", HTTP_POST, handleInverterConfig);
    server.onNotFound([]() {                                  // If the client requests any URI
        if (!handleFileRead(server.uri()))                    // send it if it exists
            server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });
    server.begin();
}

void webserver_loop()
{
    server.handleClient();
}