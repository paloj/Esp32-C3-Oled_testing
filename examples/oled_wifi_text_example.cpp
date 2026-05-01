// ESP32-C3 OLED WiFi Text Control Example (WORKING)
// This is a backup of the working code before adding the door signal feature.
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WebServer.h>

// OLED I2C pins on this board:
// SDA = GPIO5
// SCL = GPIO6
//
// OLED visible area is 72x40 pixels, mapped inside a 128x64 SSD1306 framebuffer.
// For this board, the correct visible window offset is:
//   X_OFFSET = 28
//   Y_OFFSET = 24   // (empirically determined)
//
// If you see missing/cut-off text, adjust Y_OFFSET slightly.

const char* ssid = "U6_Home1";
const char* password = "vaaramentie1a2";

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    U8X8_PIN_NONE,
    6, // SCL
    5  // SDA
);

constexpr int OLED_W = 72;
constexpr int OLED_H = 40;
constexpr int X_OFFSET = 28; // Correct for this board: (128-72)/2
constexpr int Y_OFFSET = 24; // Correct for this board: empirically determined

WebServer server(80);
String displayText1 = "ESP32-C3";
String displayText2 = "72x40 OLED";

void setup() {
    Serial.begin(115200);
    delay(1000);

    u8g2.begin();
    u8g2.setBusClock(400000);
    u8g2.setContrast(255);
    u8g2.setFont(u8g2_font_6x10_tf);

    // Connect to WiFi
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int wifiTries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        wifiTries++;
        if (wifiTries > 40) {
            Serial.println("\nWiFi connection failed!");
            break;
        }
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\nWiFi connected! IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi not connected. Check credentials or signal.");
    }

    // Web page to control display text
    server.on("/", HTTP_GET, []() {
        String html = "<html><head><title>OLED Text Control</title></head><body>";
        html += "<h2>ESP32-C3 OLED Text Control</h2>";
        html += "<form action='/set' method='POST'>";
        html += "Line 1: <input name='line1' value='" + displayText1 + "'><br>";
        html += "Line 2: <input name='line2' value='" + displayText2 + "'><br>";
        html += "<input type='submit' value='Update'>";
        html += "</form>";
        html += "<p>Current IP: " + WiFi.localIP().toString() + "</p>";
        html += "</body></html>";
        server.send(200, "text/html", html);
    });
    server.on("/set", HTTP_POST, []() {
        if (server.hasArg("line1")) displayText1 = server.arg("line1");
        if (server.hasArg("line2")) displayText2 = server.arg("line2");
        server.sendHeader("Location", "/");
        server.send(303);
    });
    server.begin();
    Serial.println("Web server started on port 80");
}

void loop() {
    server.handleClient();
    u8g2.clearBuffer();
    u8g2.drawFrame(X_OFFSET, Y_OFFSET, OLED_W, OLED_H);
    u8g2.setCursor(X_OFFSET + 6, Y_OFFSET + 14);
    u8g2.print(displayText1.c_str());
    u8g2.setCursor(X_OFFSET + 6, Y_OFFSET + 28);
    u8g2.print(displayText2.c_str());
    u8g2.sendBuffer();
    delay(100);
}
