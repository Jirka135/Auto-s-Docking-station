#include <WiFi.h>

const char* ssid = "jirka";
const char* password = "jirka123";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("New client");
  while (client.connected()) {
    if (client.available()) {
      String incomingMessage = client.readStringUntil('\n');
      Serial.println("Received message: " + incomingMessage);
    }
  }

  Serial.println("Client disconnected");
}