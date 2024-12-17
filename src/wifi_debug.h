#ifndef WIFI_DEGUG_H
#define WIFI_DEGUG_H

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Micropump baby";
const char* password = "12345678";

WebServer server(80);

float rotationRate = 0.0;
float targetDistance = 0.0;
short type = 0;
short mode = 0;
bool isPumping = false;

String webpage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Web Server</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #222; color: #fff; }
        input, button { margin: 10px; padding: 5px; font-size: 16px; }
        button { cursor: pointer; background-color: #28a745; color: #fff; border: none; }
        button.off { background-color: #dc3545; }
    </style>
</head>
<body>
    <h1>Micropump debugging</h1>
    
    <form action="/update" method="GET">
        <label>Rate (ml/s): </label>
        <input type="number" step="any" name="rate" value="%RATE%">
        <br>
        <label>Target (ml): </label>
        <input type="number" step="any" name="target" value="%TARGET%">
        <br>
        <label>Type (1->5): </label>
        <input type="number" name="type" value="%TYPE%">
        <br>
        <label>Mode (1 or 0): </label>
        <input type="number" name="direction" value="%DIRECTION%">
        <br>
        
    </form>

    <form action="/toggle" method="POST">
        <button class="%STATE%" type="submit">%TOGGLE_STATE%</button>
    </form>

    <form action="/update" method="GET">
          <button type="submit">Send data to ESP32</button>
    </form>
</body>
</html>
)rawliteral";

String processor(const String& var) {
  if (var == "RATE") return String(rotationRate);
  if (var == "TARGET") return String(targetDistance);
  if (var == "TYPE") return String(type);
  if (var == "DIRECTION") return String(mode);
  if (var == "TOGGLE_STATE") return !isPumping ? "Start pump" : "Stop pump";
  if (var == "STATE") return isPumping ? "off" : "on";
  return String();
}

void handleRoot() {
  String page = webpage;
  page.replace("%RATE%", processor("RATE"));
  page.replace("%TARGET%", processor("TARGET"));
  page.replace("%TYPE%", processor("TYPE"));
  page.replace("%DIRECTION%", processor("DIRECTION"));
  page.replace("%TOGGLE_STATE%", processor("TOGGLE_STATE"));
  page.replace("%STATE%", processor("STATE"));
  server.send(200, "text/html", page);
}

void handleUpdate() {
  if (server.hasArg("rate")) rotationRate = server.arg("rate").toFloat();
  if (server.hasArg("target")) targetDistance = server.arg("target").toFloat();
  if (server.hasArg("type")) type = server.arg("type").toInt();
  if (server.hasArg("direction")) mode = server.arg("direction").toInt();

  Serial.printf("Rate: %f \n",rotationRate);
  Serial.printf("Target: %f \n",targetDistance);
  Serial.printf("Type: %d \n",type);
  Serial.printf("Mode: %d \n",mode);
  Serial.printf("ispumping: %d \n",isPumping);
  Serial.println("*********************************");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggle() {
  isPumping = !isPumping;
  server.sendHeader("Location", "/");
  server.send(303);
}


#endif //WIFI_DEGUG_H