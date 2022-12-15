#include <ArduinoJson.h>
#include <HTTP_Method.h>
#include <Uri.h>
#include <WebServer.h>
#include <heltec.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32Servo.h>
#include <Wire.h>               
#include "HT_SSD1306Wire.h"

#include <DHT.h>
// Definit la broche de l'Arduino sur laquelle la broche DATA du capteur est reliee 
#define DHTPIN 21 //seule variable à changer
// 0 14~ 37~ 5 12 21
// Definit le type de capteur utilise
#define DHTTYPE DHT11

// Declare un objet de type DHT Il faut passer en parametre du constructeur de l'objet la broche et le type de capteur
DHT dht(DHTPIN, DHTTYPE);

Servo servoWindow;   //On crée un objet servo appelé servoWindow
Servo servoHeater;

const char *ssid = "Galaxy S10";
const char *password = "dicacas21";
WebServer server(80);
DynamicJsonDocument doc(1024);
char buffer[1024];
float temperature;
float humidity;
bool windowIsOpen;
bool heaterIsOpen;

// OLED
SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

void handleRoot()
{
    doc["values"]["temperature"] = temperature;
    doc["values"]["humidity"] = humidity;
    doc["servos"]["window"] = windowIsOpen;
    doc["servos"]["heater"] = heaterIsOpen;
    doc["status"] = status();

    serializeJson(doc, buffer);
    server.send(200, "application/json", buffer);
    doc.clear();
    Serial.println("GET 200 / ");
}

void handleServoWindowClose() {
  closeWindow();
  
  doc["type"] = "window"; doc["isOpen"] = windowIsOpen;  
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
  doc.clear();
}

void handleServoWindowOpen() {
  openWindow();
  
  doc["type"] = "window"; doc["isOpen"] = windowIsOpen;  
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
  doc.clear();
}

void handleServoHeaterClose() {
  closeHeater();
    
  doc["type"] = "heater"; doc["isOpen"] = heaterIsOpen;  
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
  doc.clear();
}

void handleServoHeaterOpen() {
  openHeater();
  
  doc["type"] = "heater"; doc["isOpen"] = heaterIsOpen;  
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
  doc.clear();
}

void handleNotFound()
{
    server.send(404, "text/plain", "404: Not Found");
    Serial.println("GET 404");
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(5, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(14, OUTPUT);

    // Initialising the UI will init the display too.
    display.init();
    display.setFont(ArialMT_Plain_10);
    
    WiFi.mode(WIFI_STA); // Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, WiFi.localIP().toString());
    display.display();

    server.enableCORS();
    server.on("/", handleRoot);        // Chargement de la page d'accueil
    server.on("/window/close", handleServoWindowClose);
    server.on("/window/open", handleServoWindowOpen);
    server.on("/heater/close", handleServoHeaterClose); 
    server.on("/heater/open", handleServoHeaterOpen);
    server.onNotFound(handleNotFound); // Chargement de la page "Not found"
    server.begin();                    // Initialisation du serveur web
    Serial.println("Serveur web actif");

    // Initialise la capteur DHT11
    dht.begin();
    servoHeater.attach(5); //On associe le servo du chauffage à la broche 5
    servoWindow.attach(12); //On associe le servo de la fenêtre à la broche 12
    servoWindow.write(0); // on initialise le servo sur l'angle 0
    servoHeater.write(0);
    digitalWrite(14, HIGH);
}

float roundToTwoDecimals(float value) {
  return roundf(value * 100) / 100; 
}

bool isWindowOpen(){
  if (servoWindow.read() > 70){
     return true;
     Serial.println("isWindowOpen > 70");
  } return false;
}

bool isHeaterOpen() {
  if (servoHeater.read() > 110){
    return true;
    Serial.println("isHeaterOpen > 110");
 } return false;
}

void openWindow(){
    servoWindow.write(80);
    Serial.println("openWindow");
    windowIsOpen = isWindowOpen();
}

void closeWindow(){
    servoWindow.write(0);
    Serial.println("closeWindow");
    windowIsOpen = isWindowOpen();
}

void openHeater(){
    servoHeater.write(120);
    Serial.println("openHeater");
    heaterIsOpen = isHeaterOpen();
    digitalWrite(14, HIGH);
}

void closeHeater(){
    servoHeater.write(0);
    Serial.println("closeHeater");
    heaterIsOpen = isHeaterOpen();
    digitalWrite(14, LOW);
}

String status() { 
  if (isWindowOpen() && isHeaterOpen()) {
    return "NOK";
  } return "OK";
}


void loop()
{
    temperature = roundToTwoDecimals(dht.readTemperature());
    humidity =  dht.readHumidity();
    windowIsOpen = isWindowOpen();
    heaterIsOpen = isHeaterOpen();
    server.handleClient();
}
