// Libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
 
// Credentials
char* deviceId     = "device_id";             // * set your device id (will be the MQTT client username)
char* deviceSecret = "device_secret";         // * set your device secret (will be the MQTT client password)
char* outTopic     = "devices/device_id/set"; // * MQTT channel where physical updates are published
char* inTopic      = "devices/device_id/get"; // * MQTT channel where lelylan updates are received
char* clientId     = "7dslk2dfksd";             // * set a random string (max 23 chars, will be the MQTT client id)

// WiFi name & password
const char* ssid = "your_WiFi_name";
const char* password = "your_WiFi_password"; 
 
// Sketch logic
char* payloadOn  = "{\"properties\":[{\"id\":\"518be5a700045e1521000001\",\"value\":\"on\"}]}";
char* payloadOff = "{\"properties\":[{\"id\":\"518be5a700045e1521000001\",\"value\":\"off\"}]}";

byte server[] = { 178, 62, 108, 47 }; // MQTT server address
 
// WiFi Client
WiFiClient wifiClient;
 
// MQTT
void callback(char* topic, byte* payload, unsigned int length); // subscription callback
PubSubClient client(server, 1883, callback, wifiClient);         // mqtt client
 
// Pins
int outPin = 5; // led
 
// Logic
int state = HIGH;     // current state of the output pin
int reading;          // current reading from the input pin
int previous = LOW;   // previous reading from the input pin
long time = 0;        // the last time the output pin was toggled
long debounce = 200;  // the debounce time, increase if the output flickers
 
void setup() {
  Serial.begin(115200);
  delay(500);
 
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
  
  // MQTT server connection
  lelylanConnection();      
  pinMode(outPin, OUTPUT);  // led pin setup
}
 
void loop() {

  // Keep connection open
  lelylanConnection();
 
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
 
/* MQTT publish */
void lelylanPublish(char* value) {
  if (value == "on")
    client.publish(outTopic, payloadOn); // light on
  else
    client.publish(outTopic, payloadOff); // light off
}
 
/* MQTT subscribe */
void lelylanSubscribe() {
  client.subscribe(inTopic);
}
 
/* Receive Lelylan message and confirm the physical change */
void callback(char* topic, byte* payload, unsigned int length) {
  // copu the payload content into a char*
  char* json;
  json = (char*) malloc(length + 1);
  memcpy(json, payload, length);
  json[length] = '\0';
 
  // update the physical status and confirm the executed update
  if (String(payloadOn) == String(json)) {
    Serial.println("[LELYLAN] Led turned on");
    lelylanPublish("on");
    state = HIGH;
  } else {
    Serial.println("[LELYLAN] Led turned off");
    lelylanPublish("off");
    state = LOW;
  }
 
  digitalWrite(outPin, state);
  free(json);
}
