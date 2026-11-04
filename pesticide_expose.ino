#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ===== WI-FI DETAILS =====
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ===== THINGSPEAK WRITE API KEY =====
const char* WRITE_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";

// ===== SENSOR PINS =====
#define MQ_PIN 34
#define IR_PIN 27
#define BUZZ 25
#define GREEN 26
#define RED 32

// ===== SETTINGS =====
const int THRESHOLD = 300;
const unsigned long UPLOAD_INTERVAL = 20000;

unsigned long lastUpload = 0;
int baseline = 0;

// ===== READ AVERAGE SENSOR VALUE =====
int readAvg(int pin) {
  long sum = 0;

  for (int i = 0; i < 20; i++) {
    sum += analogRead(pin);
    delay(10);
  }

  return sum / 20;
}

// ===== CONNECT TO WI-FI =====
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection failed.");
    Serial.println("Check the Wi-Fi name, password, and 2.4 GHz support.");
  }
}

// ===== UPLOAD TO THINGSPEAK =====
void uploadToThingSpeak(int value, int diff, bool plant, bool alert) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    WiFi.reconnect();
    return;
  }

  WiFiClientSecure client;

  // For a student prototype only:
  // skips server certificate verification.
  client.setInsecure();

  HTTPClient https;

  String url = "https://api.thingspeak.com/update?api_key=";
  url += WRITE_API_KEY;
  url += "&field1=" + String(value);
  url += "&field2=" + String(diff);
  url += "&field3=" + String(plant ? 1 : 0);
  url += "&field4=" + String(alert ? 1 : 0);

  if (https.begin(client, url)) {
    int httpCode = https.GET();

    if (httpCode > 0) {
      String response = https.getString();

      Serial.print("HTTP code: ");
      Serial.println(httpCode);

      Serial.print("ThingSpeak response: ");
      Serial.println(response);

      if (response.toInt() > 0) {
        Serial.println("Cloud upload successful!");
      } else {
        Serial.println("Upload not accepted. Check the Write API Key and channel.");
        Serial.println("Also check ThingSpeak's update interval.");
      }
    } else {
      Serial.print("HTTPS error: ");
      Serial.println(https.errorToString(httpCode));
    }

    https.end();
  } else {
    Serial.println("Unable to start HTTPS connection.");
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(IR_PIN, INPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);

  // Active-low buzzer: HIGH means OFF
  digitalWrite(BUZZ, HIGH);
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);

  // ESP32 ADC configuration
  analogReadResolution(12);

  // Connect to Wi-Fi
  connectWiFi();

  // Warm up sensor; follow the module's requirements
  Serial.println("MQ-135 warming up...");
  delay(20000);

  baseline = readAvg(MQ_PIN);

  Serial.print("Baseline reading: ");
  Serial.println(baseline);

  Serial.println("System ready!");
}

// ===== MAIN LOOP =====
void loop() {
  int value = readAvg(MQ_PIN);
  int diff = value - baseline;

  // Assumes IR output LOW means plant/object detected
  bool plant = (digitalRead(IR_PIN) == LOW);

  // Alert when an object is detected AND gas reading
  // rises above the baseline by the selected threshold
  bool alert = plant && (diff > THRESHOLD);

  // LED and buzzer control
  digitalWrite(RED, alert ? HIGH : LOW);
  digitalWrite(GREEN, alert ? LOW : HIGH);
  digitalWrite(BUZZ, alert ? LOW : HIGH);

  // Serial Monitor output
  Serial.printf(
    "Plant: %s | MQ-135: %d | Baseline: %d | Change: %d | Alert: %s\n",
    plant ? "YES" : "NO",
    value,
    baseline,
    diff,
    alert ? "ON" : "OFF"
  );

  // Upload once every 20 seconds
  if (millis() - lastUpload >= UPLOAD_INTERVAL) {
    lastUpload = millis();
    uploadToThingSpeak(value, diff, plant, alert);
  }

  delay(1000);
}
