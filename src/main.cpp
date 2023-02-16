//MAC 24:0A:C4:F9:56:94
#include <esp_now.h>
#include <WiFi.h>
#include <M5Stack.h>
#include "bala.h"
#include "PinDefinitionsAndMore.h" 
#include <ir/IRremote.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <wire.h>

uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0xAD, 0x45, 0xD4};

using namespace std;
TFT_eSprite display = TFT_eSprite(&M5.Lcd);
std::stringstream data;
// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  data.str("");
  data << (const char*)incomingData;
  Serial.println(data.str().c_str());
}


void vypis(const char *text,int posx,int posy){
  display.fillSprite(TFT_BLACK);
  display.setCursor(posx, posy);
  display.drawString(text, 0, 0);
  display.pushSprite(0, 0);
}

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);
  vypis("Start",10,10);
  M5.begin();
  Wire.begin();
  display.createSprite(300,100);
  display.fillSprite(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(3);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  vypis("Hledam kamarada",10,10);
  esp_now_register_recv_cb(OnDataRecv);

}

void loop()
{
  // Send a message
  vypis("Povídáme si",10,10);
  const char* outgoingData = "Hello from ESP32";
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*) outgoingData, strlen(outgoingData) + 1);
  if (result == ESP_OK) {
    Serial.println("odesláno");
  }
  
  // Wait for 5 seconds before sending the next message
  delay(5000);  
}