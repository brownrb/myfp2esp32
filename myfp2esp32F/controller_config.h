// ----------------------------------------------------------------------
// myFP2ESP32 FOCUSER CONFIG DEFINITIONS
// SPECIFY OPTIONS AND CONTROLLER MODE HERE
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// controller_config.h
// ----------------------------------------------------------------------

#include <Arduino.h>
#include "boarddefs.h"
#include "controller_defines.h"

#ifndef _controller_config_h
#define _controller_config_h

// Caution: Do not enable a feature if the associated hardware circuits are
// not fitted on the board. Enable or disable the specific options below


// ----------------------------------------------------------------------
// BOARD DEFINES [do not change names]
// ----------------------------------------------------------------------
// Uncomment only your board - ONLY ONE BOARD SHOULD BE UNCOMMENTED

// ESP32 Boards
#define DRVBRD 	PRO2ESP32DRV8825
//#define DRVBRD 	PRO2ESP32ULN2003
//#define DRVBRD 	PRO2ESP32L298N
//#define DRVBRD 	PRO2ESP32L293DMINI
//#define DRVBRD 	PRO2ESP32L9110S
//#define DRVBRD	PRO2ESP32R3WEMOS				        // fixed step mode
//#define DRVBRD	PRO2ESP32TMC2225
//#define DRVBRD	PRO2ESP32TMC2209
//#define DRVBRD	PRO2ESP32TMC2209P				        // this is for Paul using TMC2209 - 58.jsn
//#define DRVBRD	PRO2ESP32ST6128					        // this is board for CLOSED LOOP ST6128 driver. fixed step mode
//#define DRVBRD 	CUSTOMBRD

// On bootup following a controller firmware update, a default driver board file
// is created based on your board selection above.
// In the MANAGEMENT server you can edit the board pin numbers and save the
// config [only if you know what you are doing].
// No changes are necessary if you are using one of the available PCBoards for this project


// ----------------------------------------------------------------------
// FIXEDSTEPMODE
// ----------------------------------------------------------------------
// Some boards have a step mode which is hardwired. If the hardwired was 
// for 1/16 step mode then enable FIXEDSTEPMODE to 16.
// For PRO2ESP32ST6128 and PRO2ESP32R3WEMOS boards, set the fixedmode value
// to match the stepmode jumper settings on the board [or external driver board]

// For all other boards, set FIXEDSTEPMODE to the step mode you will use with
// the stepper motor

#define FIXEDSTEPMODE     1
//#define FIXEDSTEPMODE   2
//#define FIXEDSTEPMODE   4
//#define FIXEDSTEPMODE   8
//#define FIXEDSTEPMODE   16
//#define FIXEDSTEPMODE   32
//#define FIXEDSTEPMODE   64                      // for future release
//#define FIXEDSTEPMODE   128                     // for future release
//#define FIXEDSTEPMODE   256                     // for future release


// ----------------------------------------------------------------------
// MOTOR STEPS PER REVOLUTION : THIS MUST BE CORRECT FOR MOTOR TO STEP PROPERLY
// Applicable to the following boards
// PRO2ESP32ULN2003, PRO2ESP32L298N, PRO2ESP32L293DMINI, PRO2ESP32L9110S
// ----------------------------------------------------------------------
// stepper motor steps per full revolution using full steps, applies to all boards

//#define STEPSPERREVOLUTION  2048                // 28BYJ-48 stepper motor unipolar with ULN2003 board
//#define STEPSPERREVOLUTION	200        	        // NEMA17 FULL STEPPED
//#define STEPSPERREVOLUTION	400        	        // NEMA14HM11-0404S 0.9 motor FULL STEPPED
//#define STEPSPERREVOLUTION 	1028                // 17HS13-0404S-PG5
#define STEPSPERREVOLUTION 	5370                // NEMA17HS13-0404S-PG27
//#define STEPSPERREVOLUTION 	1036                // NEMA14HS13-0804S-PG5
//#define STEPSPERREVOLUTION 	1036                // NEMA16HS13-0604S-PG5


// ----------------------------------------------------------------------
// DISPLAY AND DRIVER TYPE
// To use the text display, install the myOLED library
// To use the graphics display, download and install this library
// https://github.com/ThingPulse/esp8266-oled-ssd1306
// If NOT using a display go to INFRA-RED REMOTE CONTROLLER
// ----------------------------------------------------------------------

// To enable the OLED TEXT display (SSD1306 or SSH1106 chip) uncomment the next line
//#define ENABLE_TEXTDISPLAY        1

// To enable the OLED GRAPHIC display (SSD1306 or SSH1106 chip) uncomment the next line
//#define ENABLE_GRAPHICDISPLAY     2

// uncomment ONE of the following USESSxxxx lines depending upon your lcd type
// For the OLED 128x64 0.96" display with SSD1306 driver, uncomment the following line
#define USE_SSD1306   1

// For the OLED 128x64 1.3" display with SSH1106 driver, uncomment the following line
//#define USE_SSH1106   2


// ----------------------------------------------------------------------
// INFRA-RED REMOTE CONTROLLER
// If not using Infra-red controller, go to CONTROLLER MODE
// ----------------------------------------------------------------------
// To enable the Infrared remote controller [ESP32 only], uncomment the next line
//#define ENABLE_INFRAREDREMOTE   3


// ----------------------------------------------------------------------
// CONTROLLER MODE
// Settings for Controller Modes are in defines folder
// ----------------------------------------------------------------------
// The following controller modes are MUTUALLY EXCLUSIVE and cannot be combined

// to work as an access point, uncomment ACCESSPOINT
// values are found in defines/accesspointmode_defines.h
//#define CONTROLLERMODE  ACCESSPOINT

// to work as a station accessing an existing network, uncomment STATIONMODE
// values are found in defines/stationmode_defines.h
#define CONTROLLERMODE  STATIONMODE


// ----------------------------------------------------------------------
// READ WIFI CONFIG
// ----------------------------------------------------------------------
// to enable reading SSID and PASSWORD from file wificonfig.json at
// boot time, uncomment the following file
#define ENABLE_READWIFICONFIG   1


// -----------------------------------------------------------------------
// OTA UPDATE (OVER THE AIR UPDATE)
// If not using OTA, go to DUCKDNS
// Settings for OTA UPDATE are in defines/otaupdate_defines.h
// -----------------------------------------------------------------------
// ElegantOTA uses significant code and data space (dependency: Management Server)
// To use ElegantOTA uncomment the next line
//#define ENABLE_ELEGANTOTA      1


// -----------------------------------------------------------------------
// DUCKDNS
// If not using DuckDNS, goto TMC2209
// Settings for DUCKDNS are in defines/duckdns_defines.h
// -----------------------------------------------------------------------
// Cannot use DuckDNS with ACCESSPOINT
//#define ENABLE_DUCKDNS 	1


// ----------------------------------------------------------------------
// TMC2209 HOME POSITION SWITCH OPTIONS
// If not using the TMC2209 driver chip, then this part is finished
// ----------------------------------------------------------------------
// Configure this in the Management Server when controller boots up
// Default settings are Disabled, No Home Position switch and stall value of 100


// ----------------------------------------------------------------------
// DO NOT CHANGE
// CHECK BOARD AND HW OPTIONS
// ----------------------------------------------------------------------
// DRVBRD must be defined
#ifndef DRVBRD
#error //err: No DRVBRD is defined
#endif // #ifndef DRVBRD

// Select the driver chip for the display
#if defined(USE_SSD1306)
#if defined(USE_SSH1106)
#error // err: Define either USE_SSD1306 or USE_SSH1106 if using an OLED DISPLAY
#endif // #ifdef USE_SSH1106
#endif // #ifdef USE_SSD1306

// cannot run Duckdns with ACCESSPOINT
#if (CONTROLLERMODE == ACCESSPOINT)
#if defined(ENABLE_DUCKDNS)
#error // err: DUCKDNS only works with STATIONMODE
#endif
#if defined(ENABLE_READWIFICONFIG)
#error // err: READWIFICONFIG only available in STATIONMODE
#endif
#endif

#endif
