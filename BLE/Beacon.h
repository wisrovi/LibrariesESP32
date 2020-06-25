#define BLuetooth


#ifdef BLuetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "WiFi.h"
String idBLE;
boolean BeaconEncontrado = false;


int scanTime = 5; //In seconds
BLEScan* pBLEScan;

void configBLEscan() {
  BLEDevice::init("ivAdventure");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  uint64_t chipid = ESP.getEfuseMac();
  idBLE = String((uint32_t)chipid);
  String mac = WiFi.macAddress();
  idBLE = mac.substring(mac.length() - 2, mac.length() - 1) +  idBLE   +    mac.substring(mac.length() - 1, mac.length());

  Serial.print("ID buscar BLE: ");
  idBLE.toLowerCase();
  Serial.println(idBLE);
}

void scanBeacon() {
  //Serial.println("Scaneando Beacon");
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  int count = foundDevices.getCount();
  if (count > 0 ) {
    //Serial.print("Se han encontrado ");
    //Serial.print(count);
    //Serial.println(" dispositivos");
    for (int i = 0; i < count; i++) {
      BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
      String datosDispositivo = advertisedDevice.toString().c_str();

      if (true) {
        int indice = datosDispositivo.indexOf("Address:");
        if (indice >= 0) {
          String address = datosDispositivo.substring( indice + 9, indice + 26  );
          if (address.length() > 0) {
            //Serial.print(address);
            //Serial.print("-");
            indice = datosDispositivo.indexOf("manufacturer data:");
            if (indice >= 0) {
              String serviceUUID = datosDispositivo.substring(  indice  );
              if (serviceUUID.length() > 0) {
                indice = serviceUUID.indexOf(idBLE);
                if (indice >= 0) {
                  BeaconEncontrado = true;
                }
              }
            }
            //Serial.println();
          }
        }
      }else{
        Serial.println(datosDispositivo);
      }
    }
  }
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

  if (BeaconEncontrado) {
    //Serial.println("BLE encontrado");
  }
}
#endif
