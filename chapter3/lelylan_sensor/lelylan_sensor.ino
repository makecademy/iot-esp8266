// Libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
 
// Credentials
char* deviceId     = "device_id";             // * set your device id (will be the MQTT client username)
char* deviceSecret = "device_secret";         // * set your device secret (will be the MQTT client password)
char* outTopic     = "devices/device_id/set"; // * MQTT channel where physical updates are published
char* inTopic      = "devices/device_id/get"; // * MQTT channel where lelylan updates are received
char* clientId     = "7dslk2dfksd";             // * set a random string (max 23 chars, will be the MQTT client id)

// WiFi name & password
const char* ssid = "your_WiFi_name";
const char* password = "your_WiFi_password"; 

// MQTT server address
byte server[] = { 178, 62, 108, 47 }; 
 
// WiFi client
WiFiClient wifiClient;

// Pin & type
#define DHTPIN 5
#define DHTTYPE DHT11
 
// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE, 15);
 
// MQTT communication
void callback(char* topic, byte* payload, unsigned int length); // subscription callback
PubSubClient client(server, 1883, callback, wifiClient);         // mqtt client
 
void setup() {
  Serial.begin(115200);
  delay(500);
  
  // Init DHT 
  dht.begin();
 
  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  lelylanConnection();      // MQTT server connection
}
 
void loop() {

  // Keep connection open
  lelylanConnection();
  
  // Reading temperature and humidity
  int h = dht.readHumidity();
  
  // Read temperature as Celsius
  int t = dht.readTemperature();
  
  // Messages for MQTT
  String temperature  = "{\"properties\":[{\"id\":\"552b72f1c70d1fd785000003\",\"value\":\"" + String(t) + "\"}]}";
  String humidity = "{\"properties\":[{\"id\":\"552b7315c70d1f4ee0000003\",\"value\":\"" + String(h) + "\"}]}";
 
  // Publish temperature
  client.publish(outTopic, (char *) temperature.c_str());
  delay(100);
  
  // Publish humidity
  client.publish(outTopic, (char *) humidity.c_str());
  delay(10000);
  
}
 
/* MQTT server connection */
void lelylanConnection() {
  // add reconnection logics
  if (!client.connected()) {
    // connection to MQTT server
    if (client.connect(clientId, deviceId, deviceSecret)) {
      Serial.println("[PHYSICAL] Successfully connected with MQTT");
      lelylanSubscribe(); // topic subscription
    }
  }
  client.loop();
}

/* MQTT subscribe */
void lelylanSubscribe() {
  client.subscribe(inTopic);
}
 
/* Receive Lelylan message and confirm the physical change */
void callback(char* topic, byte* payload, unsigned int length) {

}
