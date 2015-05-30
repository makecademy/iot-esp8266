/***************************************************

Written by Marco Schwartz for Open Home Automation.  
BSD license, all text above must be included in any redistribution

Based on the original sketches supplied with the ESP8266/Arduino 
implementation written by Ivan Grokhotkov      

****************************************************/

// Libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
 
// Credentials
String deviceId     = "device_id";             // * set your device id (will be the MQTT client username)
String deviceSecret = "device_secret";         // * set your device secret (will be the MQTT client password)
String clientId     = "7dslk2dfksd";        // * set a random string (max 23 chars, will be the MQTT client id)

// WiFi name & password
const char* ssid = "your_WiFi_name";
const char* password = "your_WiFi_password"; 
 
// Sketch logic
char* payloadOn  = "{\"properties\":[{\"id\":\"518be5a700045e1521000001\",\"value\":\"on\"}]}";
char* payloadOff = "{\"properties\":[{\"id\":\"518be5a700045e1521000001\",\"value\":\"off\"}]}";

// Topics
String outTopic     = "devices/" + deviceId + "/set"; // * MQTT channel where physical updates are published
String inTopic      = "devices/" + deviceId + "/get"; // * MQTT channel where lelylan updates are received

// MQTT server address
IPAddress server(178, 62, 108, 47);
 
// WiFi Client
WiFiClient wifiClient;
 
// MQTT
PubSubClient client(server);

// Pins
int outPin = 5; // led
 
// Logic
int state = HIGH;     // current state of the output pin
int reading;          // current reading from the input pin
int previous = LOW;   // previous reading from the input pin
long time = 0;        // the last time the output pin was toggled
long debounce = 200;  // the debounce time, increase if the output flickers

// Callback
void callback(const MQTT::Publish& pub) {
  
  // Copy the payload content into a char*
  char* json;
  json = (char*) malloc(pub.payload_len() + 1);
  memcpy(json, pub.payload(), pub.payload_len());
  json[pub.payload_len()] = '\0';
 
  // Update the physical status and confirm the executed update
  boolean state;
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
 
void setup() {
  Serial.begin(115200);
  delay(500);
 
  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  // Set callback
  client.set_callback(callback);
  
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
    if (client.connect(MQTT::Connect(clientId)
	             .set_auth(deviceId, deviceSecret))) {
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

// 
