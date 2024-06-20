
# ESP32-BLE-PROXIMITY_KEY
 
 Turn your esp32 into a ble proximity sensor.

 Pair your phone or define hardcoded ble addresses.



## Config

| Name & Type                 | Description                |
| :--------                   | :------------------------- |
| `DEFAULT_ADDRESS`   String  | Default address, Pairing a new device replaces this (Saves in NVS) |
| `ADDRESS_SECOND`    String  | Hardcoded address from something like a ble beacon  |
| `ADDRESS_THIRD`     String  | Hardcoded address from something like a ble beacon  |
| `BLE_NAME`          String  | Bluetooth name it appears with  |
| `BLE_PIN`           Int     | The pin to pair (Needs to be 6 digits)  |
| `BLE_SCAN_DURATION` Float   | how many seconds each scan should go for|
| `BLE_SCAN_DELAY`    Int     | how long in MS it should wait between scans.|
| `BLE_RETRY_COUNT`   Int     | X Scans devices not found/accepted until it deactivate again|
| `SSI_THRESHOLD[]`   Int[]   | Set signal strength required per address. Default -> Second -> Third|
| `BLE_ACTIVE_SCAN`   Bool    | Active scan uses more power, but get results faster|
| `CHECK_ONCE`        Bool    | stops when paired device found: BLE off, Light sleep to only keep relay pin powered.|
| `RELAY_PIN `        Int     | GPIO pin that gets triggered when a device is found & accepted|

Check Config.h if this list is confusing. its neater there and its more obvious how it works.





## Notes
This is a vscode platformIO project.

this uses Better Comments from Aaron Bond.



