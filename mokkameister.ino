/*
 * Mokkameister hardware button,
 * based on an esp8266
 *
 * Is offline, wakes up by pushing giant red button,
 * Connects to WIFI, POSTs to mokkameister backend.
 *
 */

#include <ESP8266WiFi.h>
#include <Ticker.h>

#include "secrets.h"

const char* host = "mokkameister.herokuapp.com";
const char* path = "/coffee-button/";
const int httpPort = 80;

const int POWERPIN = 4;

#define RETRYCOUNT 3
#define SPININTERVAL 0.2
#define LEDCOUNT 3

const int LEDPINS[] = { 5, 6, 7 };

Ticker spinticker;

volatile int spincount;

void spinLEDs() {
    spincount++;

    for (int i = 0; i < LEDCOUNT; i++) {
        if (spincount % LEDCOUNT == i) {
            digitalWrite(LEDPINS[i], HIGH);
        } else {
            digitalWrite(LEDPINS[i], LOW);
        }
    }
}

void stopSpinner() {
    spinticker.detach();
    for (int i = 0; i < LEDCOUNT; i++) {
        digitalWrite(LEDPINS[i], LOW);
    }
}

void flashFailure() {
    int blinkCount = 12;

    while (blinkCount-- > 0) {
        for (int led = 0; led < LEDCOUNT; led++) {
            if (blinkCount % 2 == 0) {
                digitalWrite(LEDPINS[led], LOW);
            } else {
                digitalWrite(LEDPINS[led], HIGH);
            }
        }
        delay(300);
    }

}

void wifiConnect() {
    Serial.print("\n\nConnecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void keepPowerOn() {
    pinMode(POWERPIN, OUTPUT);
    digitalWrite(POWERPIN, HIGH);
}

void powerOff() {
    digitalWrite(POWERPIN, LOW);
}

boolean doHttpPost() {
    Serial.print("connecting to ");
    Serial.println(host);

    WiFiClient client;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return false;
    }

    Serial.print("Requesting URL: ");
    Serial.print(host);
    Serial.println(path);

    client.print(String("POST ") + path + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Content-Type: application/x-www-form-urlencoded\r\n" +
                 "Connection: close\r\n"
                 "Content-Length: 19\r\n" +   //TODO: calc length
                 "\r\n" +
                 "secret=" + secret + "\r\n" +
                 "\r\n");
    delay(50);

    while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");

    delay(200);

    client.stop();
    return true;
}

void setup() {
    keepPowerOn();

    for (int i = 0; i < LEDCOUNT; i++) {
        pinMode(LEDPINS[i], OUTPUT);
    }

    Serial.begin(115200);
    delay(10);

    spinticker.attach(SPININTERVAL, spinLEDs);
    wifiConnect();

    // Retry RETRYCOUNT times:
    for (int i = 0; i < RETRYCOUNT; i++) {
        if (doHttpPost()) {
            break;
        } else if (i == RETRYCOUNT - 1) {
            stopSpinner();
            flashFailure();
        }
    }

    stopSpinner();
    powerOff();
}

void loop() {}
