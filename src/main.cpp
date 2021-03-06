#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SLEEP_TIME_SEC 60 //Send data every X sec.

#define WIFI_SSID "<wifi_ssid>"
#define WIFI_PASS "<wifi_pass>"

#define MQTT_SERVER "<mqtt_server>"
#define MQTT_PORT <mqtt_port>

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_BME280 bme;

void goToSleep() {
  Serial.print("Going to sleep for ");
  Serial.print(SLEEP_TIME_SEC);
  Serial.println(" seconds...");

  bme.setSampling(Adafruit_BME280::MODE_SLEEP);
  WiFi.disconnect(true);

  delay(100);

  ESP.deepSleep(SLEEP_TIME_SEC * 1000000);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to: ");
  Serial.print(WIFI_SSID);

  int connCount = 0;
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    connCount++;
    Serial.print(".");

    if(connCount > 50) {
      Serial.print("Not connected!");
      goToSleep();
    }
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(A0, INPUT);

  client.setServer(MQTT_SERVER, MQTT_PORT);
  bme.begin(0x76);

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  long rssi = WiFi.RSSI();
  float voltage = (analogRead(A0) / 1023.0) * 4.3; // 120k resistor

  Serial.print("temperature: ");
  Serial.println(temperature);
  Serial.print("humidity: ");
  Serial.println(humidity);
  Serial.print("pressure: ");
  Serial.println(pressure);
  Serial.print("rssi: ");
  Serial.println(rssi);
  Serial.print("voltage: ");
  Serial.println(voltage);

  connCount = 0;

  while(!client.connected()) {
    if(connCount > 5) {
      Serial.println("MQTT connection failed");
      goToSleep();
    }

    Serial.print("Attempting MQTT connection... ");
    
    if (client.connect("ESP8266Client-espWearher")) {
      Serial.println("connected");

      char buff[32];

      snprintf (buff, 32, "%f", temperature);
      client.publish("espWeather/temperature", buff, true);
      
      snprintf (buff, 32, "%f", humidity);
      client.publish("espWeather/humidity", buff, true);
      
      snprintf (buff, 32, "%f", pressure);
      client.publish("espWeather/pressure", buff, true);

      snprintf (buff, 32, "%ld", rssi);
      client.publish("espWeather/rssi", buff, true);

      snprintf (buff, 32, "%f", voltage);
      client.publish("espWeather/voltage", buff, true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 seconds");
      connCount++;
      delay(1000);
    }
  }

  if(client.connected()) {
    client.disconnect();
  }

  goToSleep();
}

void loop() {
  // Never gonna happen
}
