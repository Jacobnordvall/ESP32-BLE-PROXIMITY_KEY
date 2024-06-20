    /*----|                             __   _____              __ _       
    |  ___|                            / _| /  __ \            / _(_)      
    | |_ __ _ _ __   ___ _   _    __ _| |_  | /  \/ ___  _ __ | |_ _  __ _ 
    |  _/ _` | '_ \ / __| | | |  / _` |  _| | |    / _ \| '_ \|  _| |/ _` |
    | || (_| | | | | (__| |_| | | (_| | |   | \__/\ (_) | | | | | | | (_| |
    \_| \__,_|_| |_|\___|\__, |  \__,_|_|    \____/\___/|_| |_|_| |_|\__, |
                        __/ |                                       __/ |
                        |___/                                       |___*/


    //#define COMPILE_SCANNER  //Compile the scanner instead? to find ble devices like ble beacons easily.
    //The apple ones are unreliable since they dont use a static address. Buy something like a tile or some android one on on aliexpress

    // See the following for generating UUIDs:
    // https://www.uuidgenerator.net/
    #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
    #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


    #define DEFAULT_ADDRESS "53:b9:e8:cc:67:02" // This gets overriden by address saved in preferences, if that exists. last bluetooth pairing.
    #define ADDRESS_SECOND  "fa:2b:c1:34:ac:84" // Anything you want to detect with a static address
    #define ADDRESS_THIRD   "fa:2b:c1:34:ac:84" // Anything you want to detect with a static address

    #define BLE_NAME "ESP-PROXIMITY-KEY" // Bluetooth device name
    #define BLE_PIN 123456 // The pin to pair //!(NEEDS TO BE 6 DIGITS)

    #define BLE_SCAN_DURATION 1.5 //how many seconds each scan should go for
    #define BLE_SCAN_DELAY 500 //how long in MS it should wait between scans.
    #define BLE_RETRY_COUNT 5 //how many scans can the devices not be found/accepted for until it deactivates again.
    const int RSSI_THRESHOLD[] = {-85, -50, -50}; // Signal strength required to accept device found: DEFAULT -> SECOND -> THIRD. Lets you tune each device.

    #define BLE_ACTIVE_SCAN true // Active scan uses more power, but get results faster
    #define CHECK_ONCE false //stops when paired device found: BLE off, Light sleep to only keep relay pin powered.
    #define RELAY_PIN 22 // GPIO pin that gets triggered when device found. (Could be useful to have this be the led pin)


    // Wiring the relay:
    //   
    // POWER IT from the 3V3 and dedicated gnd pins (or any external source)
    // ONLY use the "RELAY_PIN" to give it a signal to trigger, GPIO pins cant output enough current to power a relay coil.