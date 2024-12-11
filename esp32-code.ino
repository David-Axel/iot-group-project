#define BLYNK_TEMPLATE_ID "TMPL6YA67DQXr"    // Replace with your Blynk Template ID
#define BLYNK_TEMPLATE_NAME "white noise"   // Replace with your Blynk Template Name
#define BLYNK_AUTH_TOKEN "ho-1mVQB7sdEeFnKqE3pwZAAeU-K0Sr8"  // Replace with your Blynk Auth Token

#include <WiFi.h>
#include <ThingSpeak.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Wi-Fi credentials
const char *ssid = "Rami";          // Replace with your Wi-Fi SSID
const char *password = "12345678"; // Replace with your Wi-Fi password

// ThingSpeak configuration
unsigned long channelID = 2722708;        // Replace with your ThingSpeak Channel ID
const char *apiKey = "LC3W47N99RB740X8";  // Replace with your ThingSpeak Read API Key

// Virtual pin assignment for Blynk
#define V0 0  // Virtual pin for sound control
#define V2 2  // Virtual pin for fan state
#define V4 4  // Virtual pin for temperature

// Pin definitions for sensors and actuators
#define DHTPIN 4           // Pin for DHT sensor
#define DHTTYPE DHT11      // DHT11 sensor type
#define RELAY_PIN 26       // Valid GPIO pin for motor relay control (e.g., GPIO26 on ESP32)

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
BlynkTimer timer;

// Variables for sensor readings
float fanTemperatureThreshold = 70; // Fan threshold in Fahrenheit
bool motorState = LOW; // Stores motor state

// Function to reconnect Wi-Fi if disconnected
void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Still reconnecting...");
    }
    Serial.println("WiFi reconnected!");
  }
}

// Function to read sensor data and send it to Blynk and ThingSpeak
void updateSensorData() {
  // Read temperature (Fahrenheit)
  float temperatureF = dht.readTemperature(true);

  // Validate sensor readings
  if (isnan(temperatureF)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Control the fan based on temperature
  motorState = (temperatureF > fanTemperatureThreshold);
  digitalWrite(RELAY_PIN, motorState);

  // Print data to Serial Monitor
  Serial.print("Temperature (F): ");
  Serial.print(temperatureF);
  Serial.print(", Fan State: ");
  Serial.println(motorState ? "ON" : "OFF");

  // Send data to Blynk
  Blynk.virtualWrite(V4, temperatureF);        // Update temperature to V4
  Blynk.virtualWrite(V2, motorState ? "ON" : "OFF"); // Update fan state to V2

  // Send data to ThingSpeak
  ThingSpeak.setField(1, temperatureF);
  ThingSpeak.setField(2, motorState);

  int response = ThingSpeak.writeFields(channelID, apiKey);
  if (response == 200) {
    Serial.println("Data sent to ThingSpeak successfully.");
  } else {
    Serial.print("Error sending data to ThingSpeak: ");
    Serial.println(response);
  }
}

// Function to read Field 5 (for sound control) and Field 2 (for motor control) from ThingSpeak
void controlFromThingSpeak() {
  // Read Field 5 for sound control
  float field5 = ThingSpeak.readFloatField(channelID, 5);  // Read the fifth field from ThingSpeak
  Serial.print("Field 5 (Sound Control): ");
  Serial.println(field5);

  // Control sound playback
  if (field5 == 32) {
    Serial.println("Field 5 is 32. Playing noise...");
    Blynk.virtualWrite(V0, 1); // Play noise
  } else {
    Blynk.virtualWrite(V0, 0); // Stop noise
  }

  // Read Field 2 for motor control
  float field2 = ThingSpeak.readFloatField(channelID, 2);  // Read the second field from ThingSpeak
  Serial.print("Field 2 (Motor State): ");
  Serial.println(field2);

  // Control the motor state based on Field 2
  if (field2 == 1) {
    motorState = HIGH; // Turn on the motor
    Serial.println("Motor ON (based on ThingSpeak Field 2)");
  } else {
    motorState = LOW;  // Turn off the motor
    Serial.println("Motor OFF (based on ThingSpeak Field 2)");
  }
  digitalWrite(RELAY_PIN, motorState); // Apply motor state
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("WiFi connected!");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("Blynk connected!");

  // Set up timers
  timer.setInterval(15000L, updateSensorData);         // Update sensor data every 15 seconds
  timer.setInterval(30000L, controlFromThingSpeak);    // Control from ThingSpeak every 30 seconds
}

void loop() {
  reconnectWiFi(); // Ensure Wi-Fi connection
  Blynk.run();
  timer.run();
}