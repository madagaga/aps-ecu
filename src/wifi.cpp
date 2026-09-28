#include <wifi.h>

// soft-AP addressing
static IPAddress local_IP(192, 168, 4, 1);
static IPAddress subnet(255, 255, 255, 0);

static char wifi_IP[16];  // 255.255.255.255
static char wifi_MAC[18]; // AA:BB:CC:DD:EE:FF

void wifi_startAP() {
    // start softAP
    WiFi.mode(WIFI_AP);    
    logf_P(PSTR("Starting softAP : %s\n"),WiFi.softAPConfig(local_IP, local_IP, subnet) ? "Ready" : "Failed!");
    WiFi.softAP("APS_ECU", "12345678");
    WiFi.softAPIP().toString().toCharArray(wifi_IP, 16);
    logf_P(PSTR("Soft-AP IP address = %s\n"), wifi_IP);
}

void wifi_setup(const char *ssid, const char *password) {
    logf_P(PSTR("Connecting to '%s'\n"), ssid);
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
        log(".");
        delay(500);
    }
    logf_P(PSTR("\nConnected to %s\n"), WiFi.SSID().c_str());
    WiFi.localIP().toString().toCharArray(wifi_IP, 16);
    logf_P(PSTR("IP address: %s\n"), wifi_IP);
    WiFi.macAddress().toCharArray(wifi_MAC, 18);
    return true;
}

uint8_t wifi_ap_clients()
{
    return WiFi.softAPgetStationNum();
}

char * getIP()
{
    return wifi_IP;
}

char * getMAC()
{
    return wifi_MAC;
}
