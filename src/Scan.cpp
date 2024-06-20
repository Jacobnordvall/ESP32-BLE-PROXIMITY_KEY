#include <Config.h>
#ifdef COMPILE_SCANNER

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <HardwareSerial.h>

int scanTime = 2; // In seconds
BLEScan* pBLEScan;

String nearestDeviceName = "";
String nearestDeviceAddress = "";
int nearestDeviceRSSI = -100; // Start with a very low RSSI

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        int rssi = advertisedDevice.getRSSI();
        if (rssi > nearestDeviceRSSI) {
            nearestDeviceName = advertisedDevice.haveName() ? advertisedDevice.getName().c_str() : "Unknown";
            nearestDeviceAddress = advertisedDevice.getAddress().toString().c_str();
            nearestDeviceRSSI = rssi;
        }
    }
};

void setup() {
    Serial.begin(9600);
    Serial.println("Scanning for BLE devices...");

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); // Active scan to get more data
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // Less than or equal to setInterval value
}

void loop() {
    // Reset nearest device info for each scan
    nearestDeviceName = "";
    nearestDeviceAddress = "";
    nearestDeviceRSSI = -100;

    // Start scanning
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

    // Output the nearest device information
    Serial.println("Nearest BLE device found:");
    Serial.print("Name: ");
    Serial.println(nearestDeviceName);
    Serial.print("Address: ");
    Serial.println(nearestDeviceAddress);
    Serial.print("RSSI: ");
    Serial.println(nearestDeviceRSSI);

}

#endif