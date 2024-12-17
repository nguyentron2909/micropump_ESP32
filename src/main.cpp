//#define _GLIB  CXX_USE_CXX11_ABI 0
#include <Arduino.h>

/**************************************
//chọn 1 trong 2 phưong thức điều khiển
//bluetooth hoặc là Webserver
//chọn cái nào thì comment cái còn lại 
***************************************/
#define BLE
//#define WEB

#ifdef BLE
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>

  #include "ble_callback_class.h"
  #include "ble_uuids.h"

#endif

#ifdef WEB
  #include <WiFi.h>
  #include "wifi_debug.h"

#endif

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

const int DIR_PIN = 32;
const int STEP_PIN = 33;
const int ENABLE_PIN = 25;


int screwType = 2;
int motorDirection ; 
float stepsPerRevolution = 200.0;

float calculateDelayTime(float rate, float target, int type, float stepsPerRevolution)
{
 
  float leadScrewPitch = 2.0;
  float microsteps = 16.0;

  float stepsPerRevFactor = (type == 1) ? 40.0 : 47.5;
  float delayTime = (target * leadScrewPitch*1e6) / (rate * stepsPerRevFactor * stepsPerRevolution * microsteps);
  Serial.println(delayTime);
  return delayTime;
}



void setup() {

  Serial.begin(115200);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
    
  #ifdef BLE
  Serial.println("Starting BLE work!");
  BLEDevice::init("MMMMMMMicropump 2024");                      //make a server with the name in "___"
  
  BLEServer *pServer = BLEDevice::createServer();               // create the server
  pServer->setCallbacks(new MyServerCallbacks());               //set a call back

  BLEService *pService = pServer->createService(SERVICE_UUID);  //make a service for the characteristics
  
  // 5 data channels
  targetChar = pService->createCharacteristic(TARGET_CHAR, BLECharacteristic::PROPERTY_WRITE);
  rateChar = pService->createCharacteristic(RATE_CHAR, BLECharacteristic::PROPERTY_WRITE);
  typeChar = pService->createCharacteristic(TYPE_CHAR, BLECharacteristic::PROPERTY_WRITE);
  modeChar = pService->createCharacteristic(MODE_CHAR, BLECharacteristic::PROPERTY_WRITE);
  isPumpingChar = pService->createCharacteristic(IS_PUMPING_CHAR, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);


  pService->start();
  Serial.println("service started!");
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  //make the bluetooth visable to the things
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x00);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  #endif

  #ifdef WEB
  // Setup Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Define server routes
  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.on("/toggle", HTTP_POST, handleToggle);

  // Start the server
  server.begin();
  Serial.println("Web Server Started");
  #endif
}

void loop() {

  #ifdef BLE
    targetDistance = String(targetChar->getValue().c_str()).toFloat();
    rotationRate   = String(rateChar->getValue().c_str()).toFloat();
    type   = String(typeChar->getValue().c_str()).toInt();
    mode   = String(modeChar->getValue().c_str()).toInt();
    isPumping = String(isPumpingChar->getValue().c_str()).toInt();
    
    if(deviceConnected)
    {
      Serial.printf("Rate:  %f \n", rotationRate);
      Serial.printf("Target:  %f \n", targetDistance);
    
      Serial.printf("Type: %d \n",type);
      Serial.printf("Mode: %d\n", mode);
      Serial.printf("Running: %d\n",isPumping);
      Serial.println("-----------------");
    }
  #endif

  #ifdef WEB
  //todo: 
  // add whatever the fuck is needed to make it work on a web server
  // this was suppose to be Khanh's fucking job
  // but the motherfucker betrayed us
  #endif
  while (isPumping) 
  {
    Serial.printf("Running: %d\n",isPumping);
    // Kiểm tra số lần chạm và in ra kết quả
    Serial.println(calculateDelayTime(rotationRate, targetDistance, screwType, stepsPerRevolution));
    delay(20);
    motorDirection = 1;
    digitalWrite(ENABLE_PIN, LOW);
    digitalWrite(DIR_PIN, -motorDirection );
    
    float runtime = targetDistance * 1e6 / rotationRate; // Convert to microseconds
    float timedelay = calculateDelayTime(rotationRate, targetDistance, screwType, stepsPerRevolution);

    unsigned long startTime = micros();
    while (micros() - startTime < runtime)
    { 
      #ifdef BLE
      isPumping = String(isPumpingChar->getValue().c_str()).toInt();
      if(!isPumping || !deviceConnected) break; //if user pressed stop, stop
      #endif

      Serial.print((micros() - startTime) / 1e6, 6); // Print in seconds with microsecond precision
      Serial.print(" s ");
      Serial.print("\t"); Serial.print(runtime/ 1e6, 6);Serial.println(" s ");
      
      // Step the motor
      for(int step = 0; step < stepsPerRevolution; step++)
      {
          digitalWrite(STEP_PIN, HIGH);
          delayMicroseconds(timedelay/2);
          digitalWrite(STEP_PIN, LOW);
          delayMicroseconds(timedelay/2);
      }
     
    
      // return to original position
      motorDirection = 0;
      digitalWrite(ENABLE_PIN, LOW);
      digitalWrite(DIR_PIN, motorDirection );
    }
    if(isPumping)
    {
      Serial.println("Complete!");
      isPumping = 0; //not pumping anymore
      isPumpingChar->setValue(isPumping); //update the ble server
      delay(5000);
    }
    else 
    {
      Serial.println("Canceled!");
      if(!deviceConnected) Serial.println("Device disconnected.");
      delay(5000);
    }
    /*
    timedelay = calculateDelayTime(0.167, targetDistance, screwType, stepsPerRevolution);
    startTime = micros();

    while (micros() - startTime < 30e6)
    { 
      for(int step = 0; step < stepsPerRevolution; step++)
      {
          digitalWrite(STEP_PIN, HIGH);
          delayMicroseconds(timedelay/2);
          digitalWrite(STEP_PIN, LOW);
          delayMicroseconds(timedelay/2);
      }
    }
    */
    
  }

  
  delay(1000);
}






