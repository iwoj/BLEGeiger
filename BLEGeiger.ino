/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#define PROXIMITY_LIMIT_RSSI -60
#define LED_PIN               21

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "SoundData.h";
#include "XT_DAC_Audio.h";

XT_DAC_Audio_Class DacAudio(25,0);
XT_Wav_Class Geiger(GeigerWav);

int scanTime = 1; //In seconds
int numCloseDevices = 0;
bool runningScan = false;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s", advertisedDevice.toString().c_str());
      
      if (advertisedDevice.haveRSSI()){
        Serial.printf(", Rssi: %d \n", (int)advertisedDevice.getRSSI());
        if ((int)advertisedDevice.getRSSI() > PROXIMITY_LIMIT_RSSI) {
          numCloseDevices++;
        }
      }
      else Serial.printf("\n");
  
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  BLEDevice::init("");

  Geiger.RepeatForever=true;
  runScan();
}

void loop() {
  DacAudio.FillBuffer();
  if (numCloseDevices > 0) {
    Serial.println(numCloseDevices + (String)" device(s) too close!");
    digitalWrite(LED_PIN, HIGH);
    if(Geiger.Playing==false) {
      DacAudio.Play(&Geiger);
    }
  }
  else if (numCloseDevices == 0 && !runningScan) {
    Serial.println("No devices too close.");
    digitalWrite(LED_PIN, LOW);
    DacAudio.StopAllSounds();
  }
}

void runScan() {
  Serial.println("Scanning...");
  numCloseDevices = 0;
  runningScan = true;
  BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->start(scanTime, scanComplete);
}

void scanComplete(BLEScanResults foundDevices) {  
  runningScan = false;
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  runScan();
}
