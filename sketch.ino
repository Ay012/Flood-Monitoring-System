#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

const char* thingSpeakApiKey = "placeholder";
const char* thingSpeakServer = "http://api.thingspeak.com/update";

const char* discordWebhookUrl = "placeholder2";

#define TRIG_PIN    5
#define ECHO_PIN    18
#define LED_GREEN   25
#define LED_YELLOW  26
#define LED_RED     27
#define BUZZER_PIN  14

const float SENSOR_HEIGHT  = 200.0;
const float WARNING_LEVEL  = 120.0;
const float DANGER_LEVEL   = 160.0;

const unsigned long SENSOR_INTERVAL = 2000;
const unsigned long CLOUD_INTERVAL  = 16000;
const unsigned long BLINK_INTERVAL  = 300;

unsigned long lastSensorCheck = 0;
unsigned long lastCloudUpdate = 0;
unsigned long lastBlinkTime   = 0;
bool alertSent   = false;
bool wasInDanger = false;
bool blinkState  = false;

float waterLevel = 0;
String status = "SAFE";

float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return (duration * 0.0343) / 2.0;
}

void updateLocalIndicators() {
  if (waterLevel >= DANGER_LEVEL) {
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    status = "DANGER";
  } else if (waterLevel >= WARNING_LEVEL) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    status = "WARNING";
  } else {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    status = "SAFE";
  }
}

void blinkRedLED() {
  if (status == "DANGER") {
    if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
      lastBlinkTime = millis();
      blinkState = !blinkState;
      digitalWrite(LED_RED, blinkState ? HIGH : LOW);
    }
  } else {
    digitalWrite(LED_RED, LOW);
    blinkState = false;
  }
}

void sendToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  int statusCode = (status == "DANGER") ? 2 : (status == "WARNING") ? 1 : 0;
String url = String(thingSpeakServer) + "?api_key=" + thingSpeakApiKey +
             "&field1=" + String(waterLevel, 1) + "&field2=" + String(statusCode);
  http.begin(url);
  int httpCode = http.GET();
  Serial.printf("ThingSpeak update -> HTTP %d\n", httpCode);
  http.end();
}

void sendDiscordAlert() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  https.begin(client, discordWebhookUrl);
  https.addHeader("Content-Type", "application/json");
  String body = "{\"content\":\"FLOOD ALERT: Water level at dam has reached "
                + String(waterLevel, 1) + " cm - danger threshold exceeded!\"}";
  int httpCode = https.POST(body);
  Serial.printf("Discord danger alert -> HTTP %d\n", httpCode);
  https.end();
}

void sendDiscordNormal() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  https.begin(client, discordWebhookUrl);
  https.addHeader("Content-Type", "application/json");
  String body = "{\"content\":\"WATER LEVEL NORMAL: Dam water level has dropped to "
                + String(waterLevel, 1) + " cm - danger has passed.\"}";
  int httpCode = https.POST(body);
  Serial.printf("Discord normal alert -> HTTP %d\n", httpCode);
  https.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void loop() {
  unsigned long now = millis();
  blinkRedLED();

  if (now - lastSensorCheck >= SENSOR_INTERVAL) {
    lastSensorCheck = now;
    float distance = readDistanceCM();
    if (distance < 0) {
      Serial.println("Sensor read error - check wiring");
    } else {
      waterLevel = SENSOR_HEIGHT - distance;
      if (waterLevel < 0) waterLevel = 0;
      Serial.printf("Distance: %.1f cm | Water level: %.1f cm | Status: %s\n",
                    distance, waterLevel, status.c_str());
      updateLocalIndicators();

      if (status == "DANGER" && !alertSent) {
        sendDiscordAlert();
        alertSent = true;
        wasInDanger = true;
      } else if (status != "DANGER") {
        alertSent = false;
        if (wasInDanger) {
          sendDiscordNormal();
          wasInDanger = false;
        }
      }
    }
  }

  if (now - lastCloudUpdate >= CLOUD_INTERVAL) {
    lastCloudUpdate = now;
    sendToThingSpeak();
  }
}
