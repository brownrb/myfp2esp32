// ----------------------------------------------------------------------
// myFP2ESP32 GENERAL DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// controller_defines.h
// ----------------------------------------------------------------------

#ifndef _controller_defines_h
#define _controller_defines_h

#include <Arduino.h>


// ----------------------------------------------------------------------
// DO NOT CHANGE
// ----------------------------------------------------------------------
enum Oled_States { oled_off, oled_on };

enum Focuser_States { State_Idle, State_InitMove, State_Backlash, State_Moving, State_FinishedMove, State_SetHomePosition, State_DelayAfterMove, State_EndMove };

enum Option_States  { Option_pushbtn_joystick, Option_IRRemote, Option_Display, Option_Temperature, Option_WiFi };

// display_graphic
enum logo_num { nwifi, ntemp, nreboot };    // add nmove later once graphics display is working

enum Display_Types { Type_None, Type_Text, Type_Graphic};

enum tmc2209stallguard { Use_Stallguard, Use_Physical_Switch, Use_None };

// controller modes
#define ACCESSPOINT       1
#define STATIONMODE       2

// STATIC AND DYNAMIC CONTROLLER TCP/IP ADDRESS - Only valid for STATIONMODE
#define DYNAMICIP         1
#define STATICIP          2

// Value Defines
#define V_STOPPED         0               // service state stopped
#define V_RUNNING         1               // service state running
#define V_NOTENABLED      0
#define V_ENABLED         1
#define V_NOTFOUND        0
#define V_FOUND           1
#define V_ON              1
#define V_OFF             0
#define V_FAHRENHEIT      0
#define V_CELSIUS         1
#define V_PARKTIME        60

// I2C
#define I2CDATAPIN        21
#define I2CCLKPIN         22

// MoveTimer, default to timer0
#define MOVE_TIMER        0
// Task Timer, uses timer1
// IRRemote timer; in class, uses Timer 3

// undef
#undef DRVBRD
#undef FIXEDSTEPMODE
#undef STEPSPERREVOLUTION
#undef ENABLE_TEXTDISPLAY
#undef ENABLE_GRAPHICDISPLAY
#undef ENABLE_JOYSTICK1
#undef ENABLE_JOYSTICK2
#undef ENABLE_INFRAREDREMOTE
#undef CONTROLLERMODE
#undef ENABLE_READWIFICONFIG
#undef ENABLE_ELEGANTOTA
#undef ENABLE_DUCKDNS

// Watch dog timer
#define WDT_TIMEOUT             30            // in seconds

// DO NOT CHANGE
#define REBOOTDELAY             2000          // When rebooting controller, wait (2s) before actual reboot
#define moving_in               false
#define moving_out              !moving_in
#define moving_main             moving_in
// TEMPERATURE COMPENSATION
#define TC_DIRECTION_IN         0
#define TC_DIRECTION_OUT        1

// PORTS
#define ASCOMSERVERPORT         4040          // ASCOM Remote port
#define ASCOMDISCOVERYPORT      32227         // UDP
#define MNGSERVERPORT           6060          // Management interface - should not be changed
#define TCPIPSERVERPORT         2020          // TCPIP Server port for myFP2ESP32
#define WEBSERVERPORT           80            // Web server port

// MOTOR SETTINGS
#define MOTORPULSETIME          3             // DO NOT CHANGE
#define DEFAULTSTEPSIZE         50.0          // This is the default setting for the step size in microns
#define MINIMUMSTEPSIZE         0.0
#define MAXIMUMSTEPSIZE         100.0
#define DEFAULTPOSITION         5000L
#define DEFAULTMAXSTEPS         80000L        // 80,000 steps
#define FOCUSERUPPERLIMIT       500000L       // arbitary focuser limit up to 500,000 steps
#define FOCUSERLOWERLIMIT       1024L         // lowest value that maxsteps can be
#define HOMESTEPS               200           // Prevent searching for home position switch never returning, this should be > than # of steps between closed and open
#define HPSWOPEN                0             // hpsw states refelect status of switch
#define HPSWCLOSED              1
#define LEDPULSE                0
#define LEDMOVE                 1
#define PUSHBUTTON_STEPS        1

// DISPLAY
#define OLED_ADDR               0x3C          // some displays maybe at 0x3D or 0x3F, use I2Cscanner to find the correct address        
#define V_DISPLAYPAGETIMEMIN    2             // 2s minimum oled page display time
#define V_DISPLAYPAGETIMEMAX    10            // 10s maximum oled page display time
#define DISPLAYUPDATEONMOVE     15            // number of steps before refreshing position when moving if oledupdateonmove is 1
#define DEFAULTDISPLAYPAGETIMEMIN 4

// DUCKDNS SERVICE
#define DUCKDNS_REFRESHRATE     120           // duck dns, check ip address every 2 minutes for an update

// MANAGEMENT SERVICE
#define MSREBOOTPAGEDELAY       20000L        // management service reboot page, time in seconds before reboot occurs
//#define MAXMANAGEMENTPAGESIZE   3700        // largest = /msindex2 = 3568
//#define MAXCUSTOMBRDJSONSIZE    300

// SERIAL PORT
#define SERIALPORTSPEED         115200        // 9600, 14400, 19200, 28800, 38400, 57600, 115200

// TEMPERATURE PROBE
#define DEFAULTTEMPREFRESHTIME  30            // refresh rate between temperature conversions - 30 timeslices = 3s
#define DEFAULTTEMPRESOLUTION   10            // Set the default DS18B20 resolution to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625

// DELAY TIME BEFORE CHANGES ARE WRITTEN TO SPIFFS FILE
#define DEFAULTSAVETIME         600           // 600 timeslices, 10 timeslices per second = 600 / 10 = 60 seconds

// DEFAULT PARK TIME (Can be changed in Management Server)
#define DEFAULTPARKTIME         120           // 30-300s

// defines for ASCOMSERVER, WEBSERVER
#define NORMALWEBPAGE           200
#define FILEUPLOADSUCCESS       300
#define BADREQUESTWEBPAGE       400
#define NOTFOUNDWEBPAGE         404
#define INTERNALSERVERERROR     500

#define LISTSHORT               1
#define LISTLONG                2


// ----------------------------------------------------------------------
// DO NOT CHANGE
// ----------------------------------------------------------------------
extern const char *DEFAULTTITLECOLOR;
extern const char *DEFAULTSUBTITLECOLOR;
extern const char *DEFAULTHEADERCOLOR;
extern const char *DEFAULTTEXTCOLLOR;
extern const char *DEFAULTBACKCOLOR;

extern const char *project_name;
extern const char *program_version;
extern const char *program_author;

extern const char* filesysnotloadedstr;
extern const char* TEXTPAGETYPE;
extern const char* PLAINTEXTPAGETYPE;
extern const char* JSONTEXTPAGETYPE;
extern const char* JSONPAGETYPE;

extern const char* H_FILENOTFOUNDSTR;
extern const char* H_FSNOTLOADEDSTR;
extern const char *H_ISMOVINGPG;

extern const char *T_CONTROLLERMODE;
extern const char *T_ACCESSPOINT;
extern const char *T_STATIONMODE;
extern const char *T_OFF;
extern const char *T_ON;
extern const char *T_DISABLED;
extern const char *T_ENABLED;
extern const char *T_CELSIUS;
extern const char *T_FAHRENHEIT;
extern const char *T_FOUND;
extern const char *T_NOTFOUND;
extern const char *T_NO;
extern const char *T_YES;
extern const char *T_STOPPED;
extern const char *T_RUNNING;


// -----------------------------------------------------------------------
// DEBUGGING
// CAUTION: DO NOT ENABLE DEBUGGING INFORMATION
// -----------------------------------------------------------------------
// Remove comment to enable messages to Serial port

#define BOOTPRINT         1

//#define DEBUGPRINT       2

#define ERRORPRINT       3

// -----------------------------------------------------------------------
// DO NOT CHANGE
// -----------------------------------------------------------------------
#ifdef  BOOTPRINT
#define boot_msg_print(...)   Serial.print(__VA_ARGS__)
#define boot_msg_println(...) Serial.println(__VA_ARGS__)
#else
#define boot_msg_print(...)
#define boot_msg_println(...)
#endif

#ifdef  DEBUGPRINT
#define DEBUG_print(...)   Serial.print(__VA_ARGS__)
#define DEBUG_println(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG_print(...)
#define DEBUG_println(...)
#endif

#ifdef  ERRORPRINT
#define ERROR_print(...)   Serial.print(__VA_ARGS__)
#define ERROR_println(...) Serial.println(__VA_ARGS__)
#else
#define ERROR_print(...)
#define ERROR_println(...)
#endif

#endif // _controller_defines_h
