#include "DHT.h"

// Pin definitions
#define DHTPIN 4           // Pin for DHT sensor
#define DHTTYPE DHT22      // DHT22 sensor type
#define NOISE_PIN A0       // Analog pin for sound sensor
#define BUTTON_PIN 12      // Digital pin for button
#define LIGHT_PIN 10       // Digital pin for light/LED (PWM-capable)
#define RELAY_PIN 8        // Digital pin for motor relay control

DHT dht(DHTPIN, DHTTYPE);

// Variables for button and light state
bool lightState = LOW;      // Initial state of the light (off)
bool buttonPressed = false; // Prevent repeated toggles on a single press

// Variables for relay and motor control
bool motorState = LOW;      // Initial state of the motor (off)

// Temperature threshold for fan control
const float fanTemperatureThreshold = 26.5; // Adjust based on your requirements

// LED brightness adjustment parameters
const int fadeDelay = 20; // Delay between brightness changes in milliseconds
const int maxBrightness = 255; // Maximum brightness for the LED

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button with pull-up resistor
  pinMode(LIGHT_PIN, OUTPUT);        // Light pin as output
  pinMode(RELAY_PIN, OUTPUT);        // Relay pin as output

  // Ensure the light and motor are off initially
  digitalWrite(LIGHT_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  Serial.println("Setup complete. Monitoring sensors and controlling devices...");
}

void loop() {
  // Read temperature and humidity from DHT sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    // Log temperature and humidity
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.println(humidity);

    // Control fan based on temperature
    if (temperature > fanTemperatureThreshold) {
      motorState = HIGH; // Turn on fan
    } else {
      motorState = LOW; // Turn off fan
    }
    digitalWrite(RELAY_PIN, motorState); // Control the relay for motor
    Serial.print("Fan State: ");
    Serial.println(motorState ? "ON" : "OFF");
  }

  // Read sound level from noise sensor
  int soundLevel = analogRead(NOISE_PIN);
  Serial.print("Sound Level: ");
  Serial.println(soundLevel);

  // Toggle light state with button press
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && !buttonPressed) {
    lightState = !lightState; // Toggle light state
    simulateSunriseSunset(lightState); // Adjust brightness gradually

    Serial.print("Light State: ");
    Serial.println(lightState ? "ON (Simulating Sunrise)" : "OFF (Simulating Sunset)");

    buttonPressed = true; // Set flag to prevent repeated toggles
    delay(200);           // Debounce delay
  } else if (buttonState == HIGH) {
    buttonPressed = false; // Reset flag when button is released
  }

  // Delay before the next loop iteration
  delay(1000); // Adjust as needed
}

void simulateSunriseSunset(bool turnOn) {
  if (turnOn) {
    // Gradually increase brightness (sunrise)
    for (int brightness = 0; brightness <= maxBrightness; brightness++) {
      analogWrite(LIGHT_PIN, brightness);
      delay(fadeDelay);
    }
  } else {
    // Gradually decrease brightness (sunset)
    for (int brightness = maxBrightness; brightness >= 0; brightness--) {
      analogWrite(LIGHT_PIN, brightness);
      delay(fadeDelay);
    }
  }
}
