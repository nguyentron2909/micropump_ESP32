#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "ble_callback_class.h"
#include "ble_uuids.h"
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


void setup() {
  Serial.begin(115200);
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
  isRunningChar = pService->createCharacteristic(IS_RUNNING_CHAR, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);



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
}

void loop() {

  target = targetChar->getValue().toFloat();
  rate   = rateChar->getValue().toFloat();
  type   = typeChar->getValue().toInt();
  mode   = modeChar->getValue().toInt();
  
  //type  = typeChar->getData();
  //mode  = modeChar->getData();
  if(deviceConnected)
  {
    Serial.printf("Target:  %f \n", target);
   // Serial.println("Target_uuid: " + targetChar->getUUID().toString());
    Serial.printf("Rate:  %f \n", rate);
   // Serial.println("rate_uuid: " + rateChar->getUUID().toString());

   Serial.printf("Type: %d \n",type);
   Serial.printf("Mode: %d\n", mode);
   Serial.println("-----------------");
  }
  
    delay(5000);
}