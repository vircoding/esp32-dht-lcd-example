#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define DHT_PIN 4
#define DHT_TYPE DHT11
#define BUTTON_PIN 5

LiquidCrystal lcd(13, 12, 14, 27, 26, 25);
DHT dht(DHT_PIN, DHT_TYPE);

float hum = 0;
float tempC = 0;
float tempF = 0;

volatile int screen = 0;
volatile unsigned long lastButtonPress = 0;

void IRAM_ATTR buttonISR() {
  if (millis() - lastButtonPress > 200) {
    screen++;
    lastButtonPress = millis();
  }
}

void readDHTTask(void *parameter) {
  while (1) {
    // Read Humidity
    hum = dht.readHumidity();

    // Read Temperature in Celsius
    tempC = dht.readTemperature();

    // Read Temperature in Fahrenheit
    tempF = dht.readTemperature(true);

    // Check wheter the reading is succesfully or not
    if (isnan(hum) || isnan(tempC) || isnan(tempF)) {
      Serial.println("Failed to read from DHT sensor");
    } else {
      Serial.print("Humidity: ");
      Serial.print(hum);
      Serial.print("%");

      Serial.print("  |  ");

      Serial.print("Temperature: ");
      Serial.print(tempC);
      Serial.print("ºC  ~  ");
      Serial.print(tempF);
      Serial.println("ºF");
    }

    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void displayTask(void *parameter) {
  while (1) {
    lcd.clear();

    if (screen > 2) screen = 0;

    switch (screen) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("Humidity:");
        lcd.setCursor(0, 1);
        lcd.print(hum);
        lcd.print(" %");
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Temperature C:");
        lcd.setCursor(0, 1);
        lcd.print(tempC);
        lcd.print(" C");
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("Temperature F:");
        lcd.setCursor(0, 1);
        lcd.print(tempF);
        lcd.print(" F");
        break;
    }

    vTaskDelay(400 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  lcd.begin(16, 2);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  xTaskCreatePinnedToCore(readDHTTask, "ReadDHT", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(displayTask, "Display", 10000, NULL, 1, NULL, 1);

  Serial.println("Ready");
}

void loop() {}