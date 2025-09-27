#include <ETH.h>

// ---- Konfigurasi PHY / RMII ----
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1                   // alamat PHY (umumnya 1)
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define ETH_POWER_PIN   -1                  // -1 jika tidak dikendalikan GPIO
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN  // clock 50MHz dari PHY ke GPIO0

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("[ETH] Started");
      ETH.setHostname("esp32-eth");
      break;

    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("[ETH] Link UP (L2 connected)");
      break;

    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.println("[ETH] GOT IP (DHCP)");
      Serial.print("  IP      : "); Serial.println(ETH.localIP());
      Serial.print("  Mask    : "); Serial.println(ETH.subnetMask());
      Serial.print("  Gateway : "); Serial.println(ETH.gatewayIP());
      Serial.print("  DNS     : "); Serial.println(ETH.dnsIP());
      Serial.print("  Speed   : "); Serial.print(ETH.linkSpeed()); Serial.println(" Mb/s");
      Serial.print("  Duplex  : "); Serial.println(ETH.fullDuplex() ? "Full" : "Half");
      Serial.print("  MAC     : "); Serial.println(ETH.macAddress());
      eth_connected = true;
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("[ETH] Link DOWN");
      eth_connected = false;
      break;

    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("[ETH] Stopped");
      eth_connected = false;
      break;

    default:
      break;
  }
}
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[ETH] Init...");

  // Pasang event handler dulu
  WiFi.onEvent(WiFiEvent);

  // Mulai interface Ethernet
  // ETH.begin(addr, powerPin, mdcPin, mdioPin, phyType, clkMode)
  if (!ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE)) {
    Serial.println("[ETH] begin() FAILED");
  } else {
    Serial.println("[ETH] begin() OK, tunggu link & DHCP...");
  }

  // Mulai AsyncWebServer pada port 80
  static AsyncWebServer server(80);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello, World!");
  });
  server.begin();
}

void loop() {
  static uint32_t t = 0;
  if (millis() - t > 2000) {
    t = millis();
    if (eth_connected) {
      Serial.print("[ETH] IP: ");
      Serial.println(ETH.localIP());
    } else {
      Serial.println("[ETH] menunggu link/IP...");
    }
  }
}
