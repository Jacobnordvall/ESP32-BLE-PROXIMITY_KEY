#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>

#include <Config.h>
#ifndef COMPILE_SCANNER


Preferences preferences;     // Create a Preferences object
String PairedDeviceNVS = ""; // Device address from nvs

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool pairingSuccess = false;
uint32_t value = 0;
void resetEsp(void *pvParameters);
int TriesSincePairedFound = 0;
bool foundThisScan[50];



//*---------------------------------------------------------------------- Methods ----------------------------------------------------------------------*


void saveAddress(String Address)
{
  preferences.begin("my-app", false); // "my-app" is the namespace, false for read/write mode
  Serial.println("Writing to NVS...");
  preferences.putString("LP", Address);
  Serial.println("String written to NVS: " + Address);
  preferences.end(); 
  Serial.println("NVS write complete");

  // Create a FreeRTOS task that will start in 5 seconds to restart the esp.
  Serial.println("Restarting in 1sec...");
  xTaskCreate(resetEsp, "Task", 2048, NULL, 1, NULL);

}


// Task function that will run after a delay
void resetEsp(void * pvParameters)
{
  // Wait for 1 seconds without blocking other tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  ESP.restart();
}


bool containsTrue(const bool* array, size_t size) 
{
  for (size_t i = 0; i < size; ++i) {
    if (array[i]) {
      return true;
    }
  }
  return false;
}




// *----------------------------------------------------------------- Device Call backs -----------------------------------------------------------------*


class HelpFullFunctions
{
  public:
  String convertAddress(esp_ble_gatts_cb_param_t *param){
        String temp = ""; //just need a temp string to mash it all into.
        for (int i = 0; i < 6; i++)
        {
          // Convert each byte to a two-character hexadecimal string
          if (param->connect.remote_bda[i] < 16)
            temp += "0"; // Add leading zero if necessary
            temp += String(param->connect.remote_bda[i], HEX);
          if (i < 5)            
            temp += ":";
        }
        return temp;
  }

  // Function to convert address to string
  String addressToString(const uint8_t* addr) {
  char str[18];
  sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", 
          addr[0], addr[1], addr[2], 
          addr[3], addr[4], addr[5]);
  return String(str);
}

};

class MyServerCallbacks: public BLEServerCallbacks {

    void onConnect(BLEServer* pServer,esp_ble_gatts_cb_param_t *param) {
      deviceConnected = true;
      oldDeviceConnected = true;
      Serial.println("Device connected");
        

      //*Prints the address of the connected device to the serial monitor 
      HelpFullFunctions printAddress;

      Serial.print("Connected to: ");
      if (param->connect.remote_bda)
      { 
        String Address = printAddress.convertAddress(param);
        Serial.println(Address);
        Serial.println("");
      } 
      //return;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
      Serial.println("");
      Serial.println("--------------------");
      Serial.println("");
    }

    //Get the device name and address
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("Advertised Device: ");
      Serial.println(advertisedDevice.toString().c_str());
    }

};


// *------------------------------------------------------------- Pairing callbacks  ----------------------------------------------------------------*

//On successful pairing, write to console "Successful pairing"
class MySecurityCallbacks : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() {
    Serial.println("Passkey requested");
    return 123456; // Your predefined passkey
  }

  void onPassKeyNotify(uint32_t pass_key) {
    Serial.print("Passkey notified: ");
    Serial.println(pass_key);
  }

  bool onConfirmPIN(uint32_t pass_key) {
    Serial.print("Confirming PIN: ");
    Serial.println(pass_key);
    return true; // Return true if pass_key is correct
  }

  bool onSecurityRequest() {
    Serial.println("Security request");
    return true; // Always accept security requests
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl) {
    if (auth_cmpl.success) {
      HelpFullFunctions printaddress;
      String addrStr = printaddress.addressToString(auth_cmpl.bd_addr);
      addrStr.toLowerCase();
      Serial.print("Authentication successful, address: ");
      Serial.println(addrStr);
      saveAddress(addrStr);
    } 
    else {
      Serial.println("Authentication complete: failed");    
    }
  }
};


//*------------------------------------------------------------------ Device lost -------------------------------------------------------------------*



void ProximityCheckup(bool ConnectionFound)
{
 if(ConnectionFound)
 {
   TriesSincePairedFound = 0;
   digitalWrite(RELAY_PIN, HIGH);

   if(CHECK_ONCE)
   {
     Serial.println("Going night night, job done");
     esp_bt_controller_disable();
     esp_bt_controller_deinit();   

     gpio_deep_sleep_hold_en();
     gpio_hold_en((gpio_num_t) RELAY_PIN); 

     delay(1000);
     esp_light_sleep_start();
   }
 }
 else if (!CHECK_ONCE)
 {  
   if(TriesSincePairedFound >= BLE_RETRY_COUNT) {
    digitalWrite(RELAY_PIN, LOW); }
   else {
    TriesSincePairedFound++;
    Serial.println("Trying to find last detected device: " + (String)TriesSincePairedFound + "/" + BLE_RETRY_COUNT);
   }   
 }

}



//*---------------------------------------------------------------------- Setup ---------------------------------------------------------------------*

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT); 
  digitalWrite(RELAY_PIN, LOW);

  //Read preferences for last paired and saved device address
  preferences.begin("my-app", false); // "my-app" is the namespace, false for read/write mode
  PairedDeviceNVS = preferences.getString("LP", "");
  Serial.println("String read from NVS: " + PairedDeviceNVS);
  preferences.end();

  if(PairedDeviceNVS == "") {
    PairedDeviceNVS = DEFAULT_ADDRESS; //if there is no saved address then it defaults to the default_address
    Serial.println("No address saved in NVS, Using default: " + PairedDeviceNVS);
  }

  // Create the BLE Device
  BLEDevice::init(BLE_NAME);

  // Create the BLE Server and security! 
  pServer = BLEDevice::createServer();
  Serial.println("BLE Server created");
  BLEDevice::setSecurityCallbacks(new MySecurityCallbacks());
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("BLE Server callbacks set");
  Serial.println("");
  Serial.println(""); 
   
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Set access permissions to encrypted
  pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // Adjusted advertising interval min (7.5ms * 0x06)
  pAdvertising->setMinPreferred(0x12);  // Adjusted advertising interval max (7.5ms * 0x12)
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");

  // Security aka pin to enter on device
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setStaticPIN(BLE_PIN); 
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND); //! 'ESP_LE_AUTH_REQ_SC_ONLY' Makes phone hidden for scans

  Serial.println("Set up complete");
  Serial.println("--------------------");
  Serial.println("");
}


//*------------------------------------------------------------------ Main Loop ---------------------------------------------------------------------*


void loop() {

    // notify changed value
    if (deviceConnected) {
        pCharacteristic->setValue((uint8_t*)&value, 4);
        pCharacteristic->notify();
        value++;
        delay(10); // Adjusted delay to reduce potential congestion
    }

    // Disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // Restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
        
    }

    // Connecting
    if (deviceConnected && !oldDeviceConnected) {
        // Do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    // If device is disconnected scan for new devices and list in serial monitor
    if (!deviceConnected){
      BLEScan* pBLEScan = BLEDevice::getScan(); // Create new scan
      pBLEScan->setActiveScan(BLE_ACTIVE_SCAN); // Active scan uses more power, but get results faster
      BLEScanResults foundDevices = pBLEScan->start(BLE_SCAN_DURATION); // Scan for x seconds
      Serial.print("Devices found: ");
      Serial.println(foundDevices.getCount());
      Serial.println("--------------------");
      Serial.println("");
     

      for( int i = 0; i < 50;  ++i ) //Clear the array from previous detects
        foundThisScan[i] = false;

      for (int i = 0; i < foundDevices.getCount(); i++) {
       
        String address = foundDevices.getDevice(i).getAddress().toString().c_str();
        int rssi = foundDevices.getDevice(i).getRSSI();
        //Serial.println(address + " RSSI " + rssi); //use this to see all the devices it scanned with address & rssi
        
        // Check address against the address of the device you are looking for        
        if(PairedDeviceNVS != "" && address == PairedDeviceNVS && rssi > RSSI_THRESHOLD[0])
        {
          Serial.println("Found a paired device saved in NVS: " + address + " RSSI: " + rssi);
          foundThisScan[i] = true;
        }
        else if ((address == ADDRESS_SECOND && rssi > RSSI_THRESHOLD[1]) || (address == ADDRESS_THIRD && rssi > RSSI_THRESHOLD[2])) {
          Serial.println("Found a hardcoded device: " + address + " RSSI: " + rssi);
          foundThisScan[i] = true;
          String x = "x"; //! Do not remove this or the code will break and a black hole will open up and swallow the universe. It's important, trust me im a magician.
        }                 //! TIMES TRIED TO REMOVE AND FAILED [4]
        else
        { 
          foundThisScan[i] = false;
        }     
      }
     
      
 
      // Check if the array contains at least one true value      
      if (containsTrue(foundThisScan, 50)) { 
        ProximityCheckup(true); }   
      else {
        ProximityCheckup(false); }


      delay(BLE_SCAN_DELAY); //! Slows time and space down to a crawling halt so the program has to wait X sec after each run to let space time catch up.
    }

}
#endif