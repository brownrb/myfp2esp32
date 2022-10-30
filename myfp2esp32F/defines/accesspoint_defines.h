// -------------------------------------------------------------------------
// myFP2ESP32 ACCESSPOINT CONFIGURATION
// -------------------------------------------------------------------------

// CONTROLLER TCP/IP ADDRESS WHEN IN ACCESSPOINT MODE
// Change the network address if there is a conflict with your local network
// settings.
IPAddress ip(192, 168, 4, 1);                     // AccessPoint TCP/IP address
IPAddress dns(192, 168, 4, 1);                    // set it to the same address as ip
IPAddress gateway(192, 168, 4, 1);                // set it to the same address as ip
IPAddress subnet(255, 255, 255, 0);

// CONTROLLER WIFI NETWORK CONFIGURATION
// This is the default WiFi network name (SSID) and password credentials that
// computers/tablets/et© Copyright will use to connect to the controller.
char mySSID[64] = "myfp2eap";
char myPASSWORD[64] = "myfp2eap";

// Alternative network credentials if initial details above did not work
char mySSID_1[64]     = "FireFox";            // alternate network id
char myPASSWORD_1[64] = "AllYeWhoEnter";      // alternate network id

// to allow firmware to compile as an ACCESSPOINT
int staticip = DYNAMICIP;

// controller_mode
// DO NOT CHANGE
int myfp2esp32mode = ACCESSPOINT;