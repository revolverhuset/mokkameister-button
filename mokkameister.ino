/*
 * Mokkameister hardware button,
 * based on an esp8266
 *
 * Is offline, wakes up by pushing giant red button,
 * Connects to WIFI, POSTs to mokkameister backend.
 *
 */

#include <ESP8266WiFi.h>
#include "secrets.h"

const char* host = "mokkameister.herokuapp.com";
const char* path = "/coffee-button/";

const int httpPort = 80;

void setup() {
    Serial.begin(115200);
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {

    Serial.print("connecting to ");
    Serial.println(host);

    WiFiClient client;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
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

    delay(5000);

    //TODO: DIE!
}