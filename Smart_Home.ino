#include <PubSubClient.h>
#include <WiFi.h>
#include "DHT.h"

#define DHTTYPE DHT11

unsigned long previousMillis = 0;  // Store the last time data was published
const long interval = 300000;      // 5 minutes in milliseconds

// WiFi and MQTT settings
const char* WIFI_SSID = "WIFISSID";  // Your WiFi SSID
const char* WIFI_PASSWORD = "Password";  // Your WiFi password
const char* MQTT_SERVER = "34.59.5.29";  // VM instance public IP
const char* MQTT_TOPIC = "iot";  // MQTT topic
const int MQTT_PORT = 1883;  // Non-TLS port

// Pin assignments
const int dht11Pin = 21;  // DHT11 sensor pin
const int ledPinY = 18;   // Yellow LED (Cool Mode)
const int ledPinG = 19;   // Green LED (Dry Mode)
const int relayPin = 32;  // Relay pin for fan

char buffer[256] = "";  // Buffer for MQTT messages

DHT dht(dht11Pin, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// Connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT reconnect function
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);  // Serial communication
  dht.begin();  // Initialize DHT sensor
  setup_wifi();  // Connect to WiFi
  client.setServer(MQTT_SERVER, MQTT_PORT);  // Set MQTT broker

  pinMode(ledPinY, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  // Initially turn off fan
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

  // Read sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from the DHT sensor");
    return;
  }

  // Control system based on temperature and humidity
  const char* mode;
  if (temperature >= 30 && humidity < 40) {
    // Cool mode: Yellow LED ON, Fan OFF
    digitalWrite(ledPinY, HIGH);
    digitalWrite(ledPinG, LOW);
    digitalWrite(relayPin, HIGH);
    mode = "Cool Mode";
  } 
  else if (temperature >= 30 && humidity >= 40) {
    // Dry mode: Green LED ON, Fan OFF
    digitalWrite(ledPinY, LOW);
    digitalWrite(ledPinG, HIGH);
    digitalWrite(relayPin, HIGH);
    mode = "Dry Mode";
  }
  else if (temperature > 26 && temperature < 30) {
    // Save mode: Fan ON
    digitalWrite(ledPinG, LOW);
    digitalWrite(ledPinY, LOW);
    digitalWrite(relayPin, LOW);
    mode = "Fan ON";
  } 
  else {
    // System OFF
    digitalWrite(ledPinY, LOW);
    digitalWrite(ledPinG, LOW);
    digitalWrite(relayPin, HIGH);
    mode = "System OFF";
  }

  // Publish a single JSON message containing all sensor readings and system states
  snprintf(buffer, sizeof(buffer),
          "{\"temperature\": %.2f, \"humidity\": %.2f, \"fan\": \"%s\", \"LED_Y\": \"%s\", \"LED_G\": \"%s\", \"mode\": \"%s\"}",
          temperature, humidity,
          digitalRead(relayPin) == LOW ? "ON" : "OFF",
          digitalRead(ledPinY) == HIGH ? "ON" : "OFF",
          digitalRead(ledPinG) == HIGH ? "ON" : "OFF",
          mode);

  client.publish(MQTT_TOPIC, buffer);
  Serial.println(buffer);
  }
  // delay(5000);  // Delay for 5 seconds before the next reading
}
