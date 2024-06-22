#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ThingSpeak.h>

#define DHTPIN D4     // Pin where the DHT11 is connected
#define DHTTYPE DHT11 // DHT 11
#define SOIL_MOISTURE_PIN A0 // Pin where the soil moisture sensor is connected
#define PUMP_PIN D1    // Pin where the water pump relay is connected

int hm=40,tm=28;

const char* ssid = "OnePlus 9R";
const char* password = "Shivani#10J";
const char* thingSpeakApiKey = "9WWVPDWRNPCR9EID";
const char* thingSpeakChannelId = "2456271";

WiFiClient client;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  ThingSpeak.begin(client);
}


void loop() {
  // Read soil moisture level
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  //int soilMoisture = analogRead(SOIL_MOISTURE_PIN); // Read moisture level

  // Convert analog reading to percentage (assuming a 10-bit ADC)
  soilMoisture = map(soilMoisture, 1023, 0, 0, 100); // Inverted mapping
  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  humidity = humidity+25;
  //soilMoisture = (soilMoisture/100);
  //soilMoisture = 100 - soilMoisture;
  // Check if any readings failed
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  ThingSpeak.setField(1, soilMoisture);
  ThingSpeak.setField(2, temperature);
  ThingSpeak.setField(3, humidity);

  unsigned long channelId = atol(thingSpeakChannelId);
  // Send data to ThingSpeak
  int httpCode = ThingSpeak.writeFields(atol(thingSpeakChannelId), thingSpeakApiKey);

  //int httpCode = ThingSpeak.writeFields(thingSpeakChannelId, thingSpeakApiKey);
  if (httpCode == 200) {
    Serial.println("Data sent to ThingSpeak successfully!");
  } else {
    Serial.println("Failed to send data to ThingSpeak. HTTP error code " + String(httpCode));
  }

  // Check if soil moisture is below threshold
  if (soilMoisture < 40 && hm>=38 && tm>25) { // Adjust this threshold according to your soil moisture sensor
    Serial.println("Soil moisture  level is low && Humidity & Temp Not OPTIMAL----> Turning on water pump.");
    digitalWrite(PUMP_PIN, HIGH); // Turn on water pump
    delay(5000); // Run pump for 5 seconds
    digitalWrite(PUMP_PIN, LOW); // Turn off water pump
    delay(6000); // Wait for 10 minutes before checking soil moisture again
  }
  else {
    Serial.println("Soil moisture level is okay. No action needed.");
    delay(6000); // Wait for 10 minutes before checking soil moisture again
  }
}

