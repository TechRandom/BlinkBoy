#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi and MQTT server settings
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_BORKER_IP";

// LED Matrix settings
const int WIDTH = 11;
const int HEIGHT = 11;
const int DATA_SIZE = WIDTH * HEIGHT * 3;
uint8_t frameData[DATA_SIZE];
char hexString[DATA_SIZE * 2 + 1] = {0}; // Each byte is represented by 2 hex characters

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setBufferSize(1024);
  client.setCallback(Ack);
  client.subscribe("cast/ack");
}

void setup_wifi() {
  delay(10);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void Ack(char* topic, byte* payload, unsigned int length){
  Serial.write(0x06); // Acknowledge
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "WemosD1Mini-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
    } else {
      delay(5000);
    }
  }
  client.subscribe("cast/ack");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (Serial.available()){
    switch (Serial.read()) {
      case 0x05: // Request for matrix definition
        Serial.println(WIDTH);
        Serial.println(HEIGHT);
        break;

      case 0x41: // 24bpp frame data
        char* buffer = (char*) frameData;
        Serial.readBytes(buffer, DATA_SIZE);
        client.publish("lmcshd/data", frameData, DATA_SIZE);
        break;
    }
  }
}