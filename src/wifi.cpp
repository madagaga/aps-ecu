#include <wifi.h>

void wifi_startAP() {
    // start softAP
    WiFi.mode(WIFI_AP);
    log("Starting softAP\n");
    log_line(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
    WiFi.softAP("APS_ECU", "12345678");
    WiFi.softAPIP().toString().toCharArray(wifi_IP, 16);
    Serial.printf("Soft-AP IP address = %s\n", wifi_IP);
}

void wifi_setup(const char *ssid, const char *password) {
    Serial.printf("Connecting to '%s'\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.begin(ssid, password);    
    WiFi.setAutoReconnect(true);
}

bool wifi_connect(uint8_t max_tries) {
    
    uint8_t tries = 0;
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        if (tries >= max_tries) {
            return false;
        }
        tries++;
        Serial.print(".");
        delay(500);
    }
    Serial.printf("\nConnected to %s\n", WiFi.SSID().c_str());
    WiFi.localIP().toString().toCharArray(wifi_IP, 16);
    Serial.printf("IP address: %s\n", wifi_IP);
    WiFi.macAddress().toCharArray(wifi_MAC, 18);
    return true;
}

char * getIP()
{
    return wifi_IP;
}

char * getMAC()
{
    return wifi_MAC;
}