#include <WiFi.h>
#include <ThingSpeak.h>
#include "DHT.h"

// Pin definitions
#define SOIL_MOISTURE_PIN 34       // Analog input for soil moisture
#define DHTPIN 21                  // Digital pin for DHT11
#define DHTTYPE DHT11
#define MQ2_PIN 19                 // Digital pin for MQ2 gas sensor
#define GREEN_LED_PIN 18          // Green LED - ON when gas is detected
#define RED_LED_PIN 5             // Red LED - ON when soil is dry

// WiFi and ThingSpeak credentials
const char* ssid = "YOUR_WIFI_SSID";         // Replace with your Wi-Fi SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your Wi-Fi password
unsigned long myChannelNumber = YOUR_CHANNEL_ID; // Replace with your ThingSpeak Channel ID
const char* myWriteAPIKey = "YOUR_WRITE_API_KEY"; // Replace with your ThingSpeak Write API Key

DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;

void setup() {
  Serial.begin(115200);
  
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);  // Range: ~0 (wet) to ~4095 (dry)
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int gasDetected = digitalRead(MQ2_PIN);            // LOW = Gas Detected

  // ======== Display Sensor Readings ========
  Serial.println("======== Sensor Readings ========");
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.println(" (Lower = Wetter)");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Gas Detected: ");
  Serial.println(gasDetected == LOW ? "YES" : "NO");

  // ======== Soil Moisture Logic (Dry = Red LED ON) ========
  if (soilMoisture > 3000) {
    digitalWrite(RED_LED_PIN, HIGH);
    Serial.println("Dry soil detected - RED LED ON");
  } else {
    digitalWrite(RED_LED_PIN, LOW);
    Serial.println("Wet soil - RED LED OFF");
  }

  // ======== Gas Detection Logic (Detected = Green LED ON) ========
  if (gasDetected == HIGH) {
    digitalWrite(GREEN_LED_PIN, HIGH);  // Gas detected → LED ON
    Serial.println("Gas Detected");
  } else {
    digitalWrite(GREEN_LED_PIN, LOW);   // No gas → LED OFF
    Serial.println("No Gas");
  }

  // ======== Send Data to ThingSpeak ========
  ThingSpeak.setField(1, soilMoisture);  // Field 1: Soil Moisture
  ThingSpeak.setField(2, temperature);   // Field 2: Temperature
  ThingSpeak.setField(3, humidity);      // Field 3: Humidity
  ThingSpeak.setField(4, gasDetected == LOW ? 1 : 0); // Field 4: Gas Detected (1 for YES, 0 for NO)

  // Update ThingSpeak channel
  int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (httpCode == 200) {
    Serial.println("Data successfully sent to ThingSpeak");
  } else {
    Serial.println("Error sending data to ThingSpeak");
  }

  delay(2000); // Wait before next reading
}