#include "BLEDevice.h"
#include <stdint.h>
#ifndef BLE_CALLBACK_CLASS_H
#define BLE_CALLBACK_CLASS_H

bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected!");
    BLEDevice::stopAdvertising();
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
    BLEDevice::startAdvertising();
  }
};


#endif //BLE_CALLBACK_CLASS_H