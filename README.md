# Pesticide Exposure Prototype

An ESP32 prototype that reads an MQ-135 gas sensor and an IR detector, signals an alert with LEDs and a buzzer, and can send readings to ThingSpeak.

> **Safety and measurement note:** An MQ-135 is a general-purpose gas/air-quality sensor and this sketch is not a validated pesticide detector or a safety instrument. Do not use it to determine whether an area is safe or to replace certified monitoring equipment.

## Hardware and pins

- ESP32 development board with 2.4 GHz Wi-Fi
- MQ-135 analog output: GPIO 34
- IR detector output: GPIO 27 (code treats LOW as object detected)
- Active-low buzzer: GPIO 25
- Green LED: GPIO 26
- Red LED: GPIO 32

Use appropriate current-limiting resistors for LEDs and follow the sensor module's voltage requirements.

## Libraries and setup

The sketch uses the ESP32 Arduino core (`WiFi`, `WiFiClientSecure`, and `HTTPClient`).

1. Open `pesticide_expose.ino` in Arduino IDE and select your ESP32 board and port.
2. Replace `YOUR_WIFI_SSID`, `YOUR_WIFI_PASSWORD`, and `YOUR_THINGSPEAK_WRITE_API_KEY` with your own values locally. Keep credentials out of public commits.
3. Set up a ThingSpeak channel with fields 1-4 for sensor value, change from baseline, IR/object state, and alert state.
4. Review the `THRESHOLD` value and allow for the sensor's required warm-up time.
5. Upload and open Serial Monitor at 115200 baud.

The sketch averages 20 sensor readings, records a startup baseline after a 20-second warm-up, and triggers the alert when the IR input indicates an object and the MQ-135 reading rises above the baseline threshold. It attempts to upload every 20 seconds.

## Security note

The current sketch calls `WiFiClientSecure::setInsecure()`, which disables TLS certificate verification. This is not suitable for security-sensitive deployments; use certificate validation for a real deployment.
