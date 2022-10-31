// myFP2ESP32 FIRMWARE OFFICIAL RELEASE 303
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// © Copyright Pieter P, SPIFFs examples found online
// © Copyright Paul P, 2021-2022. All Rights Reserved. TMC22xx code
//   myfp2esp32-firmware.ino
//   version: 303
//   date:    31-10-2022
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// Supports   ESP32 Driver boards only
//            DRV8825, ULN2003, L298N, L9110S, L293DMINI, L293D,
//            ST6128, TMC2209, TMC2225
//
// WiFi Modes ACCESSPOINT, STATIONMODE
//
// Servers    MANAGEMENT, TCPIP, WEB, ASCOM ALPACA, DuckDNS, OTA
//
// Options    Display (text/graphic), Infra-red Remote, In/Out LED's,
//            Joysticks, Push buttons, Temperature Probe
//
// ESP32-C3   Not supported. Code will NOT run on ESP32-C3
//
// Remember to change your target CPU to ESP32
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// SPECIAL LICENSE
// ----------------------------------------------------------------------
// This code is released under license. If you copy or write new code based
// on the code in these files, you MUST include a link to these files AND
// you MUST include references to the author(s) of this code.


// ----------------------------------------------------------------------
// CONTRIBUTIONS
// ----------------------------------------------------------------------
// This project provides you with all the information you need to undertake
// the project. A high level of documentation and support is provided to
// assist your successful implementation of this project. I only ask that
// you consider a small donation in terms as a thank you, which can be done
// via PayPal and sent to user rbb1brown@gmail.com  (Robert Brown).


// ----------------------------------------------------------------------
// BRIEF OVERVIEW: TO PROGRAM THE FIRMWARE
// ----------------------------------------------------------------------
// 1. Configure the Controller Settings and Services in controller_config.h
// 2. Set the target CPU to match the correct CPU for your board
// 3. Compile and upload to your controller
// 4. Upload the sketch data files
//
// SPECIFY DRIVER BOARD
// Enable your driver board [DRVBRD] in controller_config.h
//
// SPECIFY FIXED STEP MODE FOR PRO2ESP32ST6128 and PRO2ESP32R3WEMOS
// Enable your fixed step mode in controller_config.h
//
// SPECIFY MOTOR STEPS PER REVOLUTION
// Enable your motor steps per revolution [MOTOR STEPS PER REVOLUTION]
// in controller_config.h
//
// SPECIFY DISPLAY OPTIONS
// If using a display, enable your display options in controller_config.h
//
// SPECIFY INFRA-RED REMOTE OPTION
// If using an Infra-red remote, ENABLE_INFRAREDREMOTE in controller_config.h
//
// SPECIFY THE CONTROLLER MODE
// Please specify your controller mode in controller_config.h, such as
// ACCESSPOINT or STATIONMODE
//
// CONTROLLER MODE ACCESSPOINT :: WIFI NETWORK CREDENTIALS: SSID AND PASSWORD
// Edit defines/accesspoint_defines.h
//
// CONTROLLER MODE STATION POINT :: SSID AND PASSWORD :: STATIC IP ADDRESS CONFIGURATION
// Edit defines/stationmode_defines.h
//
// READWIFI CONFIG
// If using ReadWifiConfig, create the wificonfig.json file and save the
// file in the /data sub-folder
//
// OTAUPDATES (OVER THE AIR UPDATE)
// If using OTA, edit defines/otaupdates_defines.h
//
// DUCKDNS CONFIGURATION
// If using DuckDNS, edit defines/duckdns_defines.h
//
//
// ----------------------------------------------------------------------
// END OF CONFIG SECTION: DO NOT EDIT BELOW THIS LINE
// DO NOT FORGET TO SET THE TARGET CPU
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// DO NOT EDIT BELOW THIS LINE
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// CONTROLLER DEFINITIONS AND CONFIGURATION
// ----------------------------------------------------------------------
//#include "controller_defines.h"
#include "driver_board.h"                     // include driverboard class definitions
#include "controller_config.h"                // includes boarddefs.h and controller_defines.h


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#include <WiFi.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Wire.h>

#include <esp_task_wdt.h>                     // use esp32 watch dog timer (WDT)


// ----------------------------------------------------------------------
// CONTROLLER CONFIG DATA
// Default Configuration: Included
// ----------------------------------------------------------------------
// You can change the value for SSID/PASSWORD, IPADDRESSES if required
// for accesspoint see defines/accesspoint_defines.h
// for stationmode see defines/stationmode_defines.h

extern int myfp2esp32mode;
#if defined(CONTROLLERMODE)
#if (CONTROLLERMODE == ACCESSPOINT)
#include "defines/accesspoint_defines.h"
#endif // #if (CONTROLLERMODE == ACCESSPOINT)
#if (CONTROLLERMODE == STATIONMODE)
#include "defines/stationmode_defines.h"
#endif // #if (CONTROLLERMODE == STATIONMODE)
#endif // #if defined(CONTROLLERMODE)

// get access to the ControllerData class
#include "controller_data.h"
CONTROLLER_DATA *ControllerData;

// declare the network SSID/PASSWORD
extern char mySSID[];
extern char myPASSWORD[];
extern char mySSID_1[];
extern char myPASSWORD_1[];


// ----------------------------------------------------------------------
// DRIVER BOARD [uses Timer0]
// Default Configuration: Included
// ----------------------------------------------------------------------
#include "driver_board.h"
DRIVER_BOARD *driverboard;
// these capture compile time settings and are required to initialize a board correctly
int myboardnumber   = DRVBRD;                 // define in controller_config.h
int myfixedstepmode = FIXEDSTEPMODE;          // define in controller_config.h
int mystepsperrev   = STEPSPERREVOLUTION;     // define in controller_config.h


// ----------------------------------------------------------------------
// IN-OUT LEDS
// Default Configuration: Included
// ----------------------------------------------------------------------
// Loaded with DriverBoard


// ----------------------------------------------------------------------
// PUSHBUTTONS
// Default Configuration: Included
// ----------------------------------------------------------------------
// Loaded with DriverBoard


// ----------------------------------------------------------------------
// JOYSTICKS
// Default Configuration: Included
// ----------------------------------------------------------------------
// Loaded with DriverBoard


// ----------------------------------------------------------------------
// TEMPERATURE PROBE
// Library  myDallasTemperature
// Default Configuration: Included
// ----------------------------------------------------------------------
#include "temp_probe.h"
TEMP_PROBE *tempprobe;


// ----------------------------------------------------------------------
// ASCOM SERVER
// Default Configuration: Included
// ----------------------------------------------------------------------
#include "ascom_server.h"
ASCOM_SERVER *ascomsrvr;


// ----------------------------------------------------------------------
// TCPIP SERVER
// Default Configuration: Included
// ----------------------------------------------------------------------
#include "tcpip_server.h"                     // do not change or move
TCPIP_SERVER *tcpipsrvr;


// ----------------------------------------------------------------------
// WEB SERVER
// Default Configuration: Included
// ----------------------------------------------------------------------
#include "web_server.h"
WEB_SERVER *websrvr;


// ----------------------------------------------------------------------
// MANAGEMENT SERVER
// Default Configuration: Included
// ----------------------------------------------------------------------
#include "management_server.h"
MANAGEMENT_SERVER *mngsrvr;


// ----------------------------------------------------------------------
// DISPLAY
// Library  Text      myOLED
// Library  Graphics  https://github.com/ThingPulse/esp8266-oled-ssd1306
// Optional
// ----------------------------------------------------------------------
#if defined(ENABLE_TEXTDISPLAY)
#include "display_text.h"
TEXT_DISPLAY *mydisplay;
#endif
#if defined(ENABLE_GRAPHICDISPLAY)
#include "display_graphics.h"
GRAPHIC_DISPLAY *mydisplay;
#endif


// ----------------------------------------------------------------------
// IR REMOTE
// Library  myfp2eIRremoteESP8266.zip
// Optional
// ----------------------------------------------------------------------
#if defined(ENABLE_INFRAREDREMOTE)
#include "ir_remote.h"
IR_REMOTE *irremote;
#endif


// ----------------------------------------------------------------------
// OTAUPDATES (OVER THE AIR UPDATE) ELEGANTOTA
// Library https://github.com/ayushsharma82/ElegantOTA
// Optional
// ----------------------------------------------------------------------
// Set initial values for name and password in defines/otaupdates_defines.h
extern const char *OTAName;
extern const char *OTAPassword;


// ----------------------------------------------------------------------
// DUCKDNS [STATIONMODE ONLY]
// Library https://github.com/ayushsharma82/EasyDDNS
// Optional
// ----------------------------------------------------------------------
// Set initial values for duckdnsdomain and duckdnstoken in defines/duckdns_defines.h
extern char duckdnsdomain[];
extern char duckdnstoken[];
#include "defines/duckdns_defines.h"
#if defined(ENABLE_DUCKDNS)
// get access to class definition
#include "duckdns.h"
DUCK_DNS *cntlr_DuckDNS;
#endif


// ----------------------------------------------------------------------
// FOCUSER CONTROLLER DATA
// ----------------------------------------------------------------------
#include "defines/app_defines.h"
// The file contains definitions/settings for
// project_name
// program_version
// program_author

// cached vars for management/web server pages
char devicename[32];
char titlecolor[8];
char subtitlecolor[8];
char headercolor[8];
char textcolor[8];
char backcolor[8];


// At 1st reboot after uploading data files the Management and TCPIP
// servers are enabled and started. For all other options, the options
// are not enabled and not started. If an option is then enabled via the
// Management server, the option state will be saved as enabled.

// On subsequent boots
// Any option that is found enabled will be initialised and started.
// If the start is successful then the status for that option will
// be set to V_RUNNING

// Runtime status flags for devices and servers
// state  = enabled (status can be stopped or running), notenabled (status will be stopped)
// status = v_stopped or v_running, on or off
byte display_status;                          // V_STOPPED
byte irremote_status;                         // V_STOPPED
byte ascomsrvr_status;                        // V_STOPPED
byte duckdns_status;                          // V_STOPPED
byte mngsrvr_status;                          // V_RUNNING
byte ota_status;                              // V_STOPPED
byte tcpipsrvr_status;                        // V_RUNNING
byte websrvr_status;                          // V_STOPPED

// There is no pushbutton state and inout-led state as they are in Driverboard

// ----------------------------------------------------------------------
// Task timer [timer1]
// handles state machine for options (display, temp probe, park, config saves)
// ----------------------------------------------------------------------
#include "tasktimer.h"
extern volatile int update_temp_flag;
extern volatile int update_display_flag;
extern volatile int update_park_flag;
extern volatile int save_board_flag;          // save board_config.jsn
extern volatile int save_cntlr_flag;          // save cntlr_config.jsn
extern volatile int save_var_flag;            // save cntlr_var.jsn
extern volatile int update_wifi_flag;         // check wifi connection - applicable to Station Mode only
extern volatile unsigned int park_maxcount;   // 30s  subject to ControllerData->get_parktime()
extern volatile unsigned int display_maxcount; // 4.5s subject to ControllerData->get_displaypagetime(), default 4500 milliseconds
extern portMUX_TYPE boardMux;
extern portMUX_TYPE cntlrMux;
extern portMUX_TYPE varMux;
extern portMUX_TYPE wifiMux;
extern portMUX_TYPE tempMux;
extern portMUX_TYPE displayMux;               // for the flag, now its time to update page on display
extern portMUX_TYPE displaytimeMux;           // for updating the actual time delay value display_maxcount
extern portMUX_TYPE parkMux;

// Mutex's required for focuser halt and move
volatile bool timerSemaphore = false;                           // move completed=true, still moving or not moving = false;
portMUX_TYPE  timerSemaphoreMux = portMUX_INITIALIZER_UNLOCKED; // protects timerSemaphore
volatile uint32_t stepcount;                                    // number of steps to go
portMUX_TYPE  stepcountMux = portMUX_INITIALIZER_UNLOCKED;      // protects stepcount
volatile bool halt_alert;                                       // indicates if a halt occured when motor was moving
portMUX_TYPE  halt_alertMux = portMUX_INITIALIZER_UNLOCKED;     // protects halt_alert

// FOCUSER
long  ftargetPosition;                        // target position
bool  isMoving;                               // is the motor currently moving (true / false)
float temp;                                   // the last temperature read
int   update_delay_after_move_flag;           // when set to 1, indicates the flag has been set, default = 0, disabled = -1
enum  Display_Types displaytype;              // None, text, graphics

// CONTROLLER
long rssi;                                    // network signal strength in Stationmode
bool reboot_start;                            // flag used to indicate a reboot has occurred
bool filesystemloaded;                        // flag indicator for webserver usage, rather than use SPIFFS.begin() test
char ipStr[16] = "000.000.000.000";           // shared between BT mode and other modes
char systemuptime[12];                        // ddd:hh:mm
IPAddress ESP32IPAddress;
IPAddress myIP;


// ----------------------------------------------------------------------
// PARK, DISPLAY, COILPOWER INTERACTION
// ----------------------------------------------------------------------
// for displays, oled_state controls if the display is on or off
// This is done via the use "move", "park" and "parktime".
// Park = false display is on
// Park = true  display is off
// When the motor moves, park = false and display turned on
// When the motor stops, parktime flag is set to 0, task timer starts counting to 30s
// When count is reached parktime flag is set to 1
// In loop(), parktime flag is then disabled, park set to to 1, display turns off
// "park" is also used to control the enable/disable of Coil Power
// The "park" feature is enabled/disabled in the Management Server
Oled_States oled_state = oled_off;


// ----------------------------------------------------------------------
// HELPER FUNCTIONS
// USED THROUGHOUT THIS CODE AND REFERENCED BY CLASSES
// required because the destination class might not exist depending on
// the controllerconfig.h settings.
// Using a help function, the class can be hidden from the calling
// request, and appropriate true/false returned according to if
// the class has been defined or not.
// ----------------------------------------------------------------------


// --------------------------------------------------------------
// support function : void fmemcpy( char *, char *, int );
// fast memcpy, null terminate destination
// --------------------------------------------------------------
void fmemcpy( char *target, char *source, int i)
{
  char *dest = target;
  const char *org = source;
  while (i--)
  {
    *dest++ = *org++;
  }
  *dest = 0x00;
}

// ----------------------------------------------------------------------
// bool init_display(void);
// initialise display vars and create pointer to the display class
// helper, optional
// ----------------------------------------------------------------------
bool display_init()
{
  displaytype = Type_None;
#if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
#if defined(ENABLE_TEXTDISPLAY)
  displaytype = Type_Text;
  mydisplay = new TEXT_DISPLAY(OLED_ADDR);
  return true;
#endif
#if defined(ENABLE_GRAPHICDISPLAY)
  displaytype = Type_Graphic;
  mydisplay = new GRAPHIC_DISPLAY(OLED_ADDR);
  return true;
#endif
#endif // #if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  return false;
}

// ----------------------------------------------------------------------
// bool display_start(void);
// Display start
// helper, optional
// ----------------------------------------------------------------------
bool display_start(void)
{
  if ( display_status == V_RUNNING )
  {
    DEBUG_println("helper display_start: display_status = Running");
    return true;
  }

  DEBUG_println("helper display_start: display_status = Stopped");

  portENTER_CRITICAL(&displayMux);             // display_status has to be stopped to get here
  update_display_flag = -1;
  portEXIT_CRITICAL(&displayMux);

#if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  if ( ControllerData->get_display_enable() == V_ENABLED) // only start the display if is enabled in ControllerData
  {
    DEBUG_println("helper display_start: start display");
    if ( mydisplay->start() != true )         // attempt to start the display
    {
      DEBUG_println("helper display_start: display failed to start");
      display_status = V_STOPPED;             // display did not start
      portENTER_CRITICAL(&displayMux);
      update_display_flag = -1;
      portEXIT_CRITICAL(&displayMux);
      return false;
    }
    else
    {
      DEBUG_println("helper display_start: display started");
      display_status = V_RUNNING;
      portENTER_CRITICAL(&displayMux);
      update_display_flag = 0;
      portEXIT_CRITICAL(&displayMux);
      return true;
    }
  }
#endif // #if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  return false;
}


// ----------------------------------------------------------------------
// void display_stop(void);
// Stop disables the display and release() display,
// sets ControllerData->display_enable state to V_NOTENABLED
// sets display_status to V_STOPPED
// helper, optional
// ----------------------------------------------------------------------
void display_stop(void)
{
#if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  portENTER_CRITICAL(&displayMux);
  update_display_flag = -1;
  portEXIT_CRITICAL(&displayMux);
  mydisplay->stop();                          // stop the display
  display_status = V_STOPPED;
#endif // #if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
}

// ----------------------------------------------------------------------
// void display_update();
// Update display
// helper, optional
// ----------------------------------------------------------------------
void display_update()
{
#if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  if ( oled_state == oled_off)
  {
    DEBUG_println("helper: display_update: oled_off: update ignored");
    return;
  }
  mydisplay->update();
#endif
}

// ----------------------------------------------------------------------
// void display_update_position(long);
// Update display with focuser position
// helper, optional
// ----------------------------------------------------------------------
void display_update_position(long fposition)
{
#if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  if ( oled_state == oled_off)
  {
    DEBUG_println("helper: display_update_position: oled_off: update ignored");
    return;
  }

  // if update position when moving enabled, then update position
  if ( ControllerData->get_displayupdateonmove() == V_ENABLED)
  {
    mydisplay->update_position(fposition);
  }
#endif
}

// ----------------------------------------------------------------------
// void display_off(void);
// Turn display off (blank)
// helper, optional
// ----------------------------------------------------------------------
void display_off()
{
#if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  mydisplay->off();
#endif
}

// ----------------------------------------------------------------------
// void display_on(void);
// Turn display on
// helper, optional
// ----------------------------------------------------------------------
void display_on()
{
#if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  mydisplay->on();
#endif
}

// ----------------------------------------------------------------------
// void display_clear(void);
// Turn display clear
// helper, optional
// ----------------------------------------------------------------------
void display_clear()
{
#if defined(ENABLE_TEXTDISPLAY) || defined(ENABLE_GRAPHICDISPLAY)
  mydisplay->clear();
#endif
}

// ----------------------------------------------------------------------
// bool irremote_init(void);
// Initialise infra-red remote
// helper, optional
// ----------------------------------------------------------------------
bool irremote_init()
{
#if defined(ENABLE_INFRAREDREMOTE)
  if ( ControllerData->get_brdirpin() != -1 ) // create the class instance
  {
    irremote = new IR_REMOTE();
    return true;
  }
#endif
  return false;
}

// ----------------------------------------------------------------------
// bool irremote_start(void);
// Initialise and start Infra red Remote
// helper, optional
// ----------------------------------------------------------------------
bool irremote_start(void)
{
#if defined(ENABLE_NFRAREDREMOTE)
  if ( irremote_status == V_RUNNING )
  {
    return true;                              // already started
  }
  irremote->start();
  return true;
#endif
  return false;
}

// ----------------------------------------------------------------------
// bool irremote_update(void);
// Read Infra red Remote
// helper, optional
// ----------------------------------------------------------------------
void irremote_update(void)
{
#if defined(ENABLE_INFRAREDREMOTE)
  if ( irremote_status == V_STOPPED )
  {
    return;
  }
  irremote->update();
#endif
}


// ----------------------------------------------------------------------
// bool duckdns_start(void);
// Start Duck DNS service, only starts service if it has been enabled in the firmware
// There is no stop()
// helper, optional
// ----------------------------------------------------------------------
bool duckdns_start()
{
#if defined(ENABLE_DUCKDNS)
  if ( duckdns_status == V_RUNNING )
  {
    return true;                              // already started
  }

  if ( ControllerData->get_duckdns_enable() == V_ENABLED )
  {
    cntlr_DuckDNS = new DUCK_DNS();           // create instance
    if ( cntlr_DuckDNS->start() == true )     // start the class
    {
      DEBUG_println("duckdns_start: started");
      duckdns_status = V_RUNNING;
      return true;
    }
  }
#endif
  duckdns_status = V_STOPPED;
  return false;
}

// ----------------------------------------------------------------------
// void duckdns_refresh_time(unsigned int);
// Set value of refresh time, which is the time bewteen sending updates to duckdns server
// helper, optional
// ----------------------------------------------------------------------
void duckdns_refreshtime(int rt)
{
  // limit range from 60s to 3600 (1hr), default is 120s
  rt = (rt < 60)   ?   60 : rt;
  rt = (rt > 3600) ? 3600 : rt;
  // if a different refresh time then save it
  if ( ControllerData->get_duckdns_refreshtime() != rt )
  {
    ControllerData->set_duckdns_refreshtime(rt);
  }
#if defined(ENABLE_DUCKDNS)
  if ( duckdns_status == V_RUNNING )
  {
    unsigned long dtime = ControllerData->get_duckdns_refreshtime() * 1000;
    cntlr_DuckDNS->set_refresh(dtime);
  }
#endif
}


// ----------------------------------------------------------------------
// bool readwificonfig( char *, char *, bool);
// READ WIFICONFIG SSID/PASSWORD FROM FILE
// Requires wificonfig.json
// helper, optional
// ----------------------------------------------------------------------
bool readwificonfig( char* xSSID, char* xPASSWORD, bool retry )
{
  bool mstatus = false;
#if defined(ENABLE_READWIFICONFIG)
  const String filename = "/wificonfig.json";
  String SSID_1, SSID_2;
  String PASSWORD_1, PASSWORD_2;

  if ( filesystemloaded == false )                    // check if SPIFFS was started
  {
    return mstatus;
  }

  File f = SPIFFS.open(filename, "r");
  if (!f)
  {
    DEBUG_println("readwificonfig() file not found");
  }
  else
  {
    String data = f.readString();
    f.close();

    // Using JSON assistant - https://arduinojson.org/v5/assistant/
    // 188 * 2
    DynamicJsonDocument doc(400);
    DeserializationError jerror = deserializeJson(doc, data);
    if (jerror)
    {
      DEBUG_println("readwificonfig() deserialise error");
    }
    else
    {
      // Decode JSON/Extract values
      SSID_1     =  doc["mySSID"].as<const char*>();
      PASSWORD_1 =  doc["myPASSWORD"].as<const char*>();
      SSID_2     =  doc["mySSID_1"].as<const char*>();
      PASSWORD_2 =  doc["myPASSWORD_1"].as<const char*>();
      if ( retry == false )
      {
        // get first pair
        SSID_1.toCharArray(xSSID, SSID_1.length() + 1);
        PASSWORD_1.toCharArray(xPASSWORD, PASSWORD_1.length() + 1);
        mstatus = true;
      }
      else
      {
        // get second pair
        SSID_2.toCharArray(xSSID, SSID_2.length() + 1);
        PASSWORD_2.toCharArray(xPASSWORD, PASSWORD_2.length() + 1);
        mstatus = true;
      }
    }
  }
#endif // #if defined(ENABLE_READWIFICONFIG)   
  return mstatus;
}


// ----------------------------------------------------------------------
// get wifi signal strength (in stationmode)
// long getrssi(int);
// ----------------------------------------------------------------------
long getrssi()
{
  long strength = WiFi.RSSI();
  return strength;
}


// ----------------------------------------------------------------------
// void get_systemuptime(void);
// CALCULATE SYSTEM UPTIME
// Outputs:  String systemuptime as dd:hh:mm   (days:hours:minutes)
// ----------------------------------------------------------------------
void get_systemuptime()
{
  unsigned long elapsedtime = millis();
  int systemuptime_m = int((elapsedtime / (1000 * 60)) % 60);
  int systemuptime_h = int((elapsedtime / (1000 * 60 * 60)) % 24);
  int systemuptime_d = int((elapsedtime / (1000 * 60 * 60 * 24)) % 365);
  snprintf(systemuptime, 10, "%03d:%02d:%02d", systemuptime_d, systemuptime_h, systemuptime_m);
}


// ----------------------------------------------------------------------
// TimeCheck
// Delay after move only, All other states and times handled by task timer
// no longer used to implement delays
// ----------------------------------------------------------------------
byte TimeCheck(unsigned long x, unsigned long Delay)
{
  unsigned long y = x + Delay;
  unsigned long z = millis();                 // pick current time

  if ((x > y) && (x < z))
    return 0;                                 // overflow y
  if ((x < y) && ( x > z))
    return 1;                                 // overflow z

  return (y < z);                             // no or (z and y) overflow
}


// ----------------------------------------------------------------------
// void reboot_esp32(int);
// reboot controller
// ----------------------------------------------------------------------
void reboot_esp32(int reboot_delay)
{
  if ( isMoving == true )
  {
    driverboard->end_move();
  }
  // save the focuser settings immediately
  ControllerData->SaveNow(driverboard->getposition(), driverboard->getdirection());
  DEBUG_println("reboot_esp32: rebooting");
  delay(reboot_delay);
  ESP.restart();
}


// ----------------------------------------------------------------------
// void steppermotormove(byte);
// move motor without updating position
// only used by sethomeposition and backlash
// ignores ledmode
// ----------------------------------------------------------------------
void steppermotormove(byte ddir )             // direction moving_in, moving_out ^ reverse direction
{
  if ( driverboard->get_leds_loaded() == true)
  {
    ( ddir == moving_in ) ? digitalWrite(ControllerData->get_brdinledpin(), 1) : digitalWrite(ControllerData->get_brdoutledpin(), 1);
  }
  driverboard->movemotor(ddir, false);
  if ( driverboard->get_leds_loaded() == true)
  {
    ( ddir == moving_in ) ? digitalWrite(ControllerData->get_brdinledpin(), 0) : digitalWrite(ControllerData->get_brdoutledpin(), 0);
  }
}


//-------------------------------------------------
// void load_vars(void);
// Load cached vars, gives quicker access for web pages etc
// when any cached variable changes, it is reloaded
//-------------------------------------------------
void load_vars()
{
  reboot_start = true;                        // booting up
  halt_alert = false;
  isMoving = false;
  update_delay_after_move_flag = -1;
  update_wifi_flag = 0;

  // ascom server
  ascomsrvr_status = V_STOPPED;

  // Controller Data
  save_cntlr_flag = -1;
  save_var_flag   = -1;
  save_board_flag = -1;

  // display
  oled_state  = oled_on;
  displaytype = Type_None;
  display_status = V_STOPPED;
  portENTER_CRITICAL(&displayMux);
  update_display_flag = -1;
  portEXIT_CRITICAL(&displayMux);

  // duckdns
  duckdns_status = V_STOPPED;

  // irremote
  irremote_status = V_STOPPED;

  // management server
  mngsrvr_status = V_STOPPED;

  // ota
  ota_status = V_STOPPED;

  // park
  update_park_flag = -1;

  // tcpip server
  tcpipsrvr_status = V_STOPPED;

  // temperature probe
  temp = 20.0;
  ControllerData->set_tcavailable(V_NOTENABLED);
  update_temp_flag = -1;

  // webserver
  websrvr_status = V_STOPPED;

  // cached vars
  String tmp;
  tmp = ControllerData->get_devicename();
  tmp.toCharArray(devicename, tmp.length() + 1 );

  tmp = ControllerData->get_wp_titlecolor();
  tmp.toCharArray(titlecolor, tmp.length() + 1 );

  tmp = ControllerData->get_wp_subtitlecolor();
  tmp.toCharArray(subtitlecolor, tmp.length() + 1 );
  
  tmp = ControllerData->get_wp_headercolor();
  tmp.toCharArray(headercolor, tmp.length() + 1 );

  tmp = ControllerData->get_wp_textcolor();
  tmp.toCharArray(textcolor, tmp.length() + 1 );

  tmp = ControllerData->get_wp_backcolor();
  tmp.toCharArray(backcolor, tmp.length() + 1 );
}


//-------------------------------------------------
// void setup(void)
//-------------------------------------------------
void setup()
{
  // serial port is used for runtime messages and debugging support
  Serial.begin(115200);
  while ( !Serial )
  {
    ;                                         // wait for serial port to start
  }

  boot_msg_println("Serial started");
  filesystemloaded = false;

  //-------------------------------------------------
  // READ FOCUSER CONFIGURATION SETTINGS FROM CONFIG FILES
  //-------------------------------------------------
  // create pointer to the class and start
  boot_msg_println("ControllerData start");
  ControllerData = new CONTROLLER_DATA();


  //-------------------------------------------------
  // Initialise vars
  //-------------------------------------------------
  boot_msg_println("Set vars and load cached vars");
  load_vars();


  //-------------------------------------------------
  // CONTROLLERMODE
  //-------------------------------------------------
  boot_msg_print("Controller mode ");
  boot_msg_println(myfp2esp32mode);


  //-------------------------------------------------
  // WIFICONFIG READ
  //-------------------------------------------------
  // read network credentials
#if  defined(ENABLE_READWIFICONFIG)
  // read mySSID,myPASSWORD from file, otherwise use defaults
  boot_msg_println("Load readwificonfig");
  readwificonfig(mySSID, myPASSWORD, false);
#endif // #if defined(ENABLE_READWIFICONFIG)


  //-------------------------------------------------
  // ACCESSPOINTMODE START
  //-------------------------------------------------
  if (myfp2esp32mode == ACCESSPOINT)
  {
    boot_msg_println("Load accesspoint");
    WiFi.config(ip, dns, gateway, subnet);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(mySSID, myPASSWORD);
  }


  //-------------------------------------------------
  // STATIONMODE START
  // Setup Stationmode, as a station connecting to an existing wifi network
  //-------------------------------------------------
  if (myfp2esp32mode == STATIONMODE)
  {
    boot_msg_println("Load station mode");
    WiFi.mode(WIFI_STA);                      // log on to LAN
    if (staticip == STATICIP)                 // if staticip then set this up before starting
    {
      DEBUG_println("Use static ip");
      WiFi.config(ip, dns, gateway, subnet);
      delay(50);
    }

    // attempt to connect using mySSID and myPASSWORD
    WiFi.begin(mySSID, myPASSWORD);
    delay(1000);
    for (int attempts = 0; WiFi.status() != WL_CONNECTED; attempts++)
    {
      delay(500);
      if (attempts > 9)                       // if this attempt is 10 or more tries
      {
        delay(500);
        break;                                // jump out of this loop
      }
    }

    // check if connected after using first set of credentials - if not connected try second pair of credentials
    if ( WiFi.status() != WL_CONNECTED )
    {
#if defined(ENABLE_READWIFICONFIG)
      // try alternative credentials, mySSID_1, myPASSWORD_1 in the wificonfig.json file
      readwificonfig(mySSID, myPASSWORD, true);
#else
      // there was no wificonfig.json file specified, so we will try again with
      // the 2nd pair of credentials and reboot after 10 unsuccessful attempts to log on
      memset( mySSID, 0, 64);
      memset( myPASSWORD, 0, 64);
      memcpy( mySSID, mySSID_1, (sizeof(mySSID_1) / sizeof(mySSID_1[0]) ));
      memcpy( myPASSWORD, myPASSWORD_1, (sizeof(myPASSWORD_1) / sizeof(myPASSWORD_1[0])) );
#endif // #ifdef ENABLE_READWIFICONFIG
      WiFi.begin(mySSID, myPASSWORD);         // attempt to start the WiFi
      delay(1000);
      for (int attempts = 0; WiFi.status() != WL_CONNECTED; attempts++)
      {
        delay(1000);
        if (attempts > 9)                     // if this attempt is 10 or more tries
        {
          ERROR_println("could not log on to network - rebooting");
          delay(2000);
          reboot_esp32(2000);;
        }
      }
    }
  }


  //-------------------------------------------------
  // CONNECTION DETAILS
  // connection established
  //-------------------------------------------------
  ESP32IPAddress = WiFi.localIP();
  snprintf(ipStr, sizeof(ipStr), "%i.%i.%i.%i",  ESP32IPAddress[0], ESP32IPAddress[1], ESP32IPAddress[2], ESP32IPAddress[3]);
  boot_msg_print("IP of controller ");
  boot_msg_println(ipStr);

  DEBUG_print("Hostname ");
  DEBUG_println(WiFi.getHostname());


  //-------------------------------------------------
  // SYSTEM UP TIME START
  //-------------------------------------------------
  boot_msg_println("Start system up time");
  get_systemuptime();                         // days:hours:minutes


  //-------------------------------------------------
  // SETUP DRIVER BOARD
  //-------------------------------------------------
  boot_msg_print("Load driver board ");
  boot_msg_println(DRVBRD);
  // ensure targetposition will be same as focuser position
  // else after loading driverboard focuser will start moving immediately
  ftargetPosition = ControllerData->get_fposition();
  driverboard = new DRIVER_BOARD();
  driverboard->start(ControllerData->get_fposition());

  // Range checks for safety reasons
  ControllerData->set_brdstepmode((ControllerData->get_brdstepmode() < 1 ) ? 1 : ControllerData->get_brdstepmode());
  ControllerData->set_coilpower_enable((ControllerData->get_coilpower_enable() >= 1) ?  1 : 0);
  ControllerData->set_reverse_enable((ControllerData->get_reverse_enable() >= 1) ?  1 : 0);
  int pgtime = ControllerData->get_displaypagetime();
  pgtime = (pgtime < V_DISPLAYPAGETIMEMIN) ? V_DISPLAYPAGETIMEMIN : pgtime;
  pgtime = (pgtime > V_DISPLAYPAGETIMEMAX) ? V_DISPLAYPAGETIMEMAX : pgtime;
  ControllerData->set_displaypagetime(pgtime);
  ControllerData->set_maxstep((ControllerData->get_maxstep() < FOCUSERLOWERLIMIT) ? FOCUSERLOWERLIMIT : ControllerData->get_maxstep());
  ControllerData->set_stepsize((float)(ControllerData->get_stepsize() < 0.0 ) ? 0 : ControllerData->get_stepsize());
  ControllerData->set_stepsize((float)(ControllerData->get_stepsize() > MAXIMUMSTEPSIZE ) ? MAXIMUMSTEPSIZE : ControllerData->get_stepsize());

  // Set coilpower
  if (ControllerData->get_coilpower_enable() == V_NOTENABLED)
  {
    boot_msg_println("setup: coilpower OFF");
    driverboard->releasemotor();
  }
  else
  {
    boot_msg_println("setup: coilpower ON");
    driverboard->enablemotor();
  }

  // ensure driverboard position is same as setupData
  // set focuser position in DriverBoard
  driverboard->setposition(ControllerData->get_fposition());


  //-------------------------------------------------
  // SETUP IN-OUT LEDS
  // Included by default
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // Now part of DriverBoard, and initialised/enabled/stopped there


  //-------------------------------------------------
  // SETUP PUSHBUTTONS
  // active high when pressed
  // Included by default
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // Now part of DriverBoard, and initialised/enabled/stopped there


  //-------------------------------------------------
  // SETUP JOYSTICKS
  // Included by default
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // Now part of DriverBoard, and initialised/enabled/stopped there


  //-------------------------------------------------
  // I2C INTERFACE START
  //-------------------------------------------------
  boot_msg_println("Start I2C");
  Wire.begin(I2CDATAPIN, I2CCLKPIN);          // pins defined in controller_defines.h


  //-------------------------------------------------
  // DISPLAY
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // call helper, if not defined then helper will return false;
  if ( display_init() == true )
  {
    boot_msg_println("Start display");
    if ( display_start() == false )
    {
      ERROR_println("displaystart() error");
    }
  }


  //-------------------------------------------------
  // SETUP INFRA-RED REMOTE
  // Optional, use helper function
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // call helper, if not defined then helper will return false;
  if ( irremote_init() == true )
  {
    boot_msg_println("Start IRRemote");
    irremote_status = irremote_start();
    if ( irremote_status == false )
    {
      ERROR_println("irremote_start() error");
    }
  }


  //-------------------------------------------------
  // SETUP DUCKDNS
  // Dependancy: WiFi has to be up and running before starting service
  // Optional, use helper function
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // check if duckdns is loaded at boot time
  if ( ControllerData->get_duckdns_enable() == V_ENABLED )
  {
    boot_msg_println("Start DUCKDNS");
    // call helper, if not defined then helper will return false;
    duckdns_status = duckdns_start();
    if ( duckdns_status != V_RUNNING )
    {
      ERROR_println("duckdns_start() error");
    }
  }


  //-------------------------------------------------
  // SETUP TEMPERATURE PROBE
  // Included by default
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // create the pointer to the class
  tempprobe = new TEMP_PROBE(ControllerData->get_brdtemppin());

  // initialise the probe
  if ( tempprobe->init() == true )
  {
    DEBUG_println("boot: temp probe init ok");
    if (ControllerData->get_tempprobe_enable() == V_ENABLED )
    {
      DEBUG_println("boot: temp: enabled");

      // probe is enabled, so start the probe
      DEBUG_println("boot: Start temperature probe");
      if ( tempprobe->start() == true )
      {
        update_temp_flag = 0;
        temp = tempprobe->read();
      }
      else
      {
        DEBUG_println("boot: start temp probe error");
      }
    }
    else
    {
      DEBUG_println("boot: temp: NOT enabled in controllerdata");
    }
  }
  else
  {
    DEBUG_println("boot: tempprobe init failed");
  }

  //-------------------------------------------------
  // ASCOM ALPACA SERVER START
  // Dependancy: WiFi must be running before starting server
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // create the pointer to the class
  ascomsrvr = new ASCOM_SERVER();
  // check if ascom server is to be started at boot time
  if ( ControllerData->get_ascomsrvr_enable() == V_ENABLED)
  {
    boot_msg_println("Start ascom alpaca server");
    ascomsrvr_status = ascomsrvr->start();
    if ( ascomsrvr_status != V_RUNNING )
    {
      ERROR_println("ascomserver start() error");
    }
  }


  //-------------------------------------------------
  // OTA UPDATES
  // Dependancy: WiFi and Management Server
  // Optional, managed by Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // OTA if enabled, will be started by Management Server


  //-------------------------------------------------
  // MANAGEMENT SERVER START
  // Dependancy: WiFi has to be up and running before starting
  // Default state:  Enabled: Started
  //-------------------------------------------------
  // create pointer to class
  mngsrvr = new MANAGEMENT_SERVER();
  // check if management server is to be started at boot time
  if ( ControllerData->get_mngsrvr_enable() == V_ENABLED)
  {
    boot_msg_println("management server enabled");
    boot_msg_println("starting management server");
    mngsrvr_status = mngsrvr->start(ControllerData->get_mngsrvr_port());
    if ( mngsrvr_status != V_RUNNING )
    {
      ERROR_println("Management server start() error");
    }
  }
  else
  {
    DEBUG_println("management server is NOT enabled");
    DEBUG_println("management server will now be enabled, but not started");
    ControllerData->set_mngsrvr_enable(V_ENABLED);
  }


  //-------------------------------------------------
  // TCP/IP SERVER START
  // Dependancy: WiFi has to be up and running before starting TCP/IP server
  // Default state:  Enabled: Started
  //-------------------------------------------------
  // create pointer to class
  tcpipsrvr = new TCPIP_SERVER();
  // check if tcpip server is to be started at boot time
  if ( ControllerData->get_tcpipsrvr_enable() == V_ENABLED)
  {
    boot_msg_println("Start TCPIP Server");
    tcpipsrvr_status = tcpipsrvr->start(ControllerData->get_tcpipsrvr_port());
    if ( tcpipsrvr_status != V_RUNNING )
    {
      ERROR_println("tcpip server start error");
    }
  }

  //-------------------------------------------------
  // WEBSERVER START
  // Dependancy: WiFi has to be up and running before starting Webserver service
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // create pointer to class
  websrvr = new WEB_SERVER();
  // check if web server is to be started at boot time
  if ( ControllerData->get_websrvr_enable() == V_ENABLED)
  {
    boot_msg_println("Start Web Server");
    websrvr_status = websrvr->start(ControllerData->get_websrvr_port());
    if ( websrvr_status != true )
    {
      ERROR_println("web server start() error");
    }
  }


  //-------------------------------------------------
  // TASK TIMER START
  // Should be the last to start
  //-------------------------------------------------
  boot_msg_println("Start task timer");
  init_task_timer();


  //-------------------------------------------------
  // WATCH DOG TIMER START
  //-------------------------------------------------
  esp_task_wdt_init(WDT_TIMEOUT, true);       // enable panic reboot, dump registers, serial output - exception decoder
  esp_task_wdt_add(NULL);                     // add code to watch dog timer

  reboot_start = false;                       // we have finished the reboot now

  boot_msg_println("Setup done, controller is ready");
}

// ----------------------------------------------------------------------
// void check_options();
// handle all device checks and updates
// ----------------------------------------------------------------------
void check_options()
{
  static Option_States OptionState = Option_pushbtn_joystick;
  static int o_mux;                           // mutex for option states

  // Option state engine
  switch (OptionState)
  {
    case Option_pushbtn_joystick:
      // these are mutually exclusive, so use if else
      if ( driverboard->get_pushbuttons_loaded() == true )
      {
        driverboard->update_pushbuttons();
      }
      else if ( driverboard->get_joystick1_loaded() == true )
      {
        driverboard->update_joystick1();
      }
      else if ( driverboard->get_joystick2_loaded() == true )
      {
        driverboard->update_joystick2();
      }
      OptionState = Option_IRRemote;
      break;

    case Option_IRRemote:
      // use helpers because optional
      irremote_update();
      OptionState = Option_Display;
      break;

    case Option_Display:
      if ( display_status == V_RUNNING )
      {
        // display is optional so need to use helper
        portENTER_CRITICAL(&displayMux);
        o_mux = update_display_flag;
        portEXIT_CRITICAL(&displayMux);
        if ( o_mux == 1 )
        {
          portENTER_CRITICAL(&displayMux);
          update_display_flag = 0;
          portEXIT_CRITICAL(&displayMux);
          // use helper
          display_update();
        }
      }
      OptionState = Option_Temperature;
      break;

    case Option_Temperature:
      if ( tempprobe->get_state() == V_RUNNING )
      {
        portENTER_CRITICAL(&tempMux);
        o_mux = update_temp_flag;
        portEXIT_CRITICAL(&tempMux);
        if ( o_mux == 1 )
        {
          portENTER_CRITICAL(&tempMux);
          update_temp_flag = 0;
          portEXIT_CRITICAL(&tempMux);
          // read temp AND check Temperature Compensation
          temp = tempprobe->update();
        }
      }
      OptionState = Option_WiFi;
      break;

    case Option_WiFi:
      // wifi connect check
      portENTER_CRITICAL(&wifiMux);
      o_mux = update_wifi_flag;
      portEXIT_CRITICAL(&wifiMux);
      if ( o_mux == 1)
      {
        if ( myfp2esp32mode == STATIONMODE )
        {
          if ( WiFi.status() != WL_CONNECTED )
          {
            ERROR_println("WiFi not connected: attempt reconnect");
            WiFi.disconnect();
            WiFi.reconnect();
          }
        }
        update_wifi_flag = 0;
      }
      OptionState = Option_pushbtn_joystick;
      break;

    default:
      OptionState = Option_pushbtn_joystick;
      break;
  }
}

void loop()
{
  static Focuser_States FocuserState = State_Idle;
  static uint32_t backlash_count = 0;
  static bool     DirOfTravel = (bool) ControllerData->get_focuserdirection();
  static bool     Parked = true;              // focuser is parked
  static uint32_t TimeStampdelayaftermove = 0;
  static bool     tms = false;                // timersemaphore, used by movetimer
  static uint8_t  updatecount = 0;
  static uint32_t steps = 0;
  static uint32_t damcounter = 0;
  static int t_mux;                           // mutex for focuser states
  static int stepstaken = 0;                  // used in finding Home Position Switch
  static bool hpswstate  = false;

  esp_task_wdt_reset();                       // watch dog timer reset

  // handle all the server loop checks, for new client or client requests

  // check ascom server for new clients
  ascomsrvr->loop();                          // clients
  ascomsrvr->check_alpaca();                  // discovery

  // check management server for new clients
  mngsrvr->loop(Parked);

  // check TCPIP Server for new clients
  tcpipsrvr->loop(Parked);

  // check Web Server for new clients
  websrvr->loop(Parked);

  check_options();

  // Focuser state engine
  switch (FocuserState)
  {
    case State_Idle:
      if (driverboard->getposition() != ftargetPosition)
      {
        // prepare to move focuser
        Parked = false;
        oled_state = oled_on;
        isMoving = true;
        driverboard->enablemotor();
        FocuserState = State_InitMove;
        DEBUG_println("go init_move");
        DEBUG_print("From Position:");
        DEBUG_println(driverboard->getposition());
        DEBUG_print("to Target:");
        DEBUG_println(ftargetPosition);
      }
      else
      {
        // focuser stationary, isMoving is false
        isMoving = false;

        // focuser stationary. isMoving is 0
        if (ControllerData->SaveConfiguration(driverboard->getposition(), DirOfTravel)) // save config if needed
        {
          DEBUG_println("config saved");
        }

        // park can be enabled or disabled (management server)
        // park controls coil power off and display off after elapsed 30s following a move
        // if park is enabled, 30s after a move ends, coilpower(if enabled) and display get turned off
        // if park is not enabled, state of coilpower and display are not altered

        // check if parking is enabled
        if ( ControllerData->get_park_enable() == true )
        {
          // parking is enabled in ControllerData
          // state_delayaftermove sets Parked false and sets park flag to 0
          if (Parked == false)
          {
            // check parked flag state for 1 (means park time delay is expired)
            portENTER_CRITICAL(&parkMux);
            t_mux = update_park_flag;
            portEXIT_CRITICAL(&parkMux);
            if ( t_mux == 1 )
            {
              // 30s wait is over, disable park flag
              // set flag to -1 so task timer no longer counts this flag
              portENTER_CRITICAL(&parkMux);
              update_park_flag = -1;
              portEXIT_CRITICAL(&parkMux);
              DEBUG_println("loop: park 30s expired: parking now");

              // park focuser if parking is enabled
              Parked = true;
              DEBUG_println("loop: Parked=True");

              // handle coil power
              // Coil Power Status ON  - Controller does move, coil power remains on
              // Coil Power Status OFF - Controller enables coil power, moves motor, after 30s elapsed releases power to motor
              if ( ControllerData->get_coilpower_enable() == V_NOTENABLED )
              {
                driverboard->releasemotor();
                DEBUG_println("loop: coilpower=released");
              }

              // turn off display
              oled_state = oled_off;

              // focuser is parked, coil power off, display off

            }
          }
        }
      }
      break;

    case State_InitMove:
      isMoving = true;
      backlash_count = 0;
      DirOfTravel = (ftargetPosition > driverboard->getposition()) ? moving_out : moving_in;
      driverboard->enablemotor();
      if (ControllerData->get_focuserdirection() != DirOfTravel)
      {
        ControllerData->set_focuserdirection(DirOfTravel);
        // move is in opposite direction
        if ( DirOfTravel == moving_in)
        {
          // check for backlash-in enabled
          if (ControllerData->get_backlash_in_enable())
          {
            // get backlash in steps
            backlash_count = ControllerData->get_backlashsteps_in();
          }
        }
        else
        {
          // check for backlash-out enabled
          if (ControllerData->get_backlash_out_enable())
          {
            // get backlash out steps
            backlash_count = ControllerData->get_backlashsteps_out();
          }
        } // if ( DirOfTravel == moving_in)

        // check for graphics display, screen output is different
        if ( displaytype == Type_Graphic )
        {
          // Holgers code: This is for a graphics display
          if (DirOfTravel != moving_main && backlash_count)
          {
            uint32_t sm = ControllerData->get_brdstepmode();
            uint32_t bl = backlash_count * sm;
            DEBUG_print("bl: ");
            DEBUG_print(bl);
            DEBUG_print(" ");

            if (DirOfTravel == moving_out)
            {
              backlash_count = bl + sm - ((ftargetPosition + bl) % sm); // Trip to tuning point should be a fullstep position
            }
            else
            {
              backlash_count = bl + sm + ((ftargetPosition - bl) % sm); // Trip to tuning point should be a fullstep position
            }
            DEBUG_print("backlash_count: ");
            DEBUG_print(backlash_count);
            DEBUG_print(" ");
          } // if (DirOfTravel != moving_main && backlash_count)
          else
          {
            DEBUG_println("false");
          }
        } // if ( displaytype == Type_Graphic )
      } // if (ControllerData->get_focuserdirection() != DirOfTravel)

      // calculate number of steps to move
      // if target pos > current pos then steps = target pos - current pos
      // if target pos < current pos then steps = current pos - target pos
      steps = (ftargetPosition > driverboard->getposition()) ? ftargetPosition - driverboard->getposition() : driverboard->getposition() - ftargetPosition;

      // Error - cannot combine backlash steps to steps because that alters position
      // Backlash move SHOULD NOT alter focuser position as focuser is not actually moving
      // backlash is taking up the slack in the stepper motor/focuser mechanism, so position is not actually changing
      if ( backlash_count != 0 )
      {
        DEBUG_println("go backlash");
        FocuserState = State_Backlash;
      }
      else
      {
        // if target pos > current pos then steps = target pos - current pos
        // if target pos < current pos then steps = current pos - target pos
        driverboard->initmove(DirOfTravel, steps);
        DEBUG_print("Steps: ");
        DEBUG_println(steps);
        DEBUG_println("go moving");
        FocuserState = State_Moving;
      }
      break;

    case State_Backlash:
      DEBUG_print("State_Backlash: Steps=");
      DEBUG_println(backlash_count);
      while ( backlash_count != 0 )
      {
        steppermotormove(DirOfTravel);                          // take 1 step and do not adjust position
        delayMicroseconds(ControllerData->get_brdmsdelay());    // ensure delay between steps
        backlash_count--;
        if (driverboard->hpsw_alert() )                         // check if home position sensor activated?
        {
          DEBUG_println("HPS_alert() during backlash move");
          portENTER_CRITICAL(&timerSemaphoreMux);
          timerSemaphore = false;                               // move finished
          portEXIT_CRITICAL(&timerSemaphoreMux);
          backlash_count = 0;                                   // drop out of while loop
          FocuserState = State_Moving;                          // change state to State_Moving and handle HPSW
        }
      }
      if ( FocuserState == State_Backlash )                     // finished backlash move, so now move motor #steps
      {
        DEBUG_println("Backlash done");
        DEBUG_print("Initiate motor move- steps: ");
        DEBUG_println(steps);
        driverboard->initmove(DirOfTravel, steps);
        DEBUG_println("go moving");
        FocuserState = State_Moving;
      }
      else
      {
        // FocuserState is State_Moving - timerSemaphore is false. is then caught by if(driverboard->hpsw_alert() ) and HPSW is processed
      }
      break;

    case State_Moving:
      portENTER_CRITICAL(&timerSemaphoreMux);
      tms = timerSemaphore;
      portEXIT_CRITICAL(&timerSemaphoreMux);
      if ( tms == true )
      {
        // move has completed, the driverboard keeps track of focuser position
        DEBUG_println("Move done");
        // disable interrupt timer that moves motor
        driverboard->end_move();
        DEBUG_println("go delayaftermove");
        // cannot use task timer for delayaftermove, as delayaftermove can be less than 100ms
        // task timer minimum time slice is 100ms, so use timestamp instead
        TimeStampdelayaftermove = millis();
        FocuserState = State_DelayAfterMove;
      }
      else
      {
        // still moving - timer semaphore is false
        // check for halt_alert which is set by tcpip_server or web_server
        if ( halt_alert )
        {
          DEBUG_println("halt_alert");
          // reset halt_alert flag
          portENTER_CRITICAL(&halt_alertMux);
          halt_alert = false;
          portEXIT_CRITICAL(&halt_alertMux);
          // disable interrupt timer that moves motor
          driverboard->end_move();
          // check for < 0
          if ( driverboard->getposition() < 0 )
          {
            driverboard->setposition(0);
          }
          ftargetPosition = driverboard->getposition();
          ControllerData->set_fposition(driverboard->getposition());

          // we no longer need to keep track of steps here or halt because driverboard updates position on every move
          TimeStampdelayaftermove = millis();           // handle delayaftermove using TimeCheck
          FocuserState = State_DelayAfterMove;
        } // if ( halt_alert )

        // check for home postion switch
        if ( driverboard->hpsw_alert() )
        {
          // hpsw is activated
          // disable interrupt timer that moves motor
          driverboard->end_move();
          if ( ControllerData->get_hpswmsg_enable() == V_ENABLED )
          {
            DEBUG_println("HPSW activated");
            if (driverboard->getposition() > 0)
            {
              DEBUG_println("HP Sw=1, Pos not 0");
            }
            else
            {
              DEBUG_println("HP Sw=1, Pos=0");
            } // if (driverboard->getposition() > 0)
          }
          ftargetPosition = 0;
          driverboard->setposition(0);
          ControllerData->set_fposition(0);
          // check if display home position messages is enabled
          if ( ControllerData->get_hpswitch_enable() == V_ENABLED )
          {
            DEBUG_println("HP Sw=1, Pos=0");
          }
          if ( ControllerData->get_brdnumber() == PRO2ESP32TMC2209 || ControllerData->get_brdnumber() == PRO2ESP32TMC2209P )
          {
#if defined(USE_STALL_GUARD)
            // focuser is at home position, no need to handle set position, simple
            if ( ControllerData->get_hpswitch_enable() == V_ENABLED )
            {
              DEBUG_println("Stall Guard: Pos = 0");
            }
            TimeStampdelayaftermove = millis();
            FocuserState = State_DelayAfterMove;
#else
            // not stall guard, must be a physical switch then we should jump to set home position
            if ( ControllerData->get_hpswitch_enable() == V_ENABLED )
            {
              DEBUG_println("go SetHomePosition");
            }
            FocuserState = State_SetHomePosition;
#endif // #if defined(USE_STALL_GUARD)
          }
          else // not a tmc2209 board
          {
            // check for a home position switch
            if ( ControllerData->get_hpswitch_enable() == V_ENABLED )
            {
              DEBUG_println("go SetHomePosition");
            }
            FocuserState = State_SetHomePosition;
          } // if ( ControllerData->get_brdnumber() == PRO2ESP32TMC2209 || ControllerData->get_brdnumber() == PRO2ESP32TMC2209P )
        } // if (driverboard->hpsw_alert() )

        // check for < 0
        if ( driverboard->getposition() < 0 )
        {
          portENTER_CRITICAL(&halt_alertMux);
          halt_alert = true;
          portEXIT_CRITICAL(&halt_alertMux);
        }

        // if the update position on display when moving is enabled, then update the display
        updatecount++;
        // update every 15th move to avoid overhead
        if ( updatecount > DISPLAYUPDATEONMOVE )
        {
          updatecount = 0;
          // use helper
          display_update_position(driverboard->getposition());
        }
      }
      break;

    case State_SetHomePosition:                         // move out till home position switch opens
      if ( ControllerData->get_hpswitch_enable() == V_ENABLED)
      {
        // check if display home position switch messages is enabled
        if ( ControllerData->get_hpswmsg_enable() == V_ENABLED)
        {
          DEBUG_println("HP Sw=0, Mov out");
        }
        // HOME POSITION SWITCH IS CLOSED - Step out till switch opens then set position = 0
        stepstaken = 0;                                 // Count number of steps to prevent going too far
        DirOfTravel = !DirOfTravel;                     // We were going in, now we need to reverse and go out
        hpswstate = HPSWCLOSED;                         // We know we got here because switch was closed
        while ( hpswstate == HPSWCLOSED )               // while hpsw = closed = true = 1
        {
          if ( ControllerData->get_reverse_enable() == V_NOTENABLED )
          {
            steppermotormove(DirOfTravel);              // take 1 step
          }
          else
          {
            steppermotormove(!DirOfTravel);
          }

          delayMicroseconds(ControllerData->get_brdmsdelay()); // Ensure delay between steps

          stepstaken++;                                 // increment steps taken
          if ( stepstaken > HOMESTEPS )                 // this prevents the endless loop if the hpsw is not connected or is faulty
          {
            if ( ControllerData->get_hpswitch_enable() == V_ENABLED )
            {
              DEBUG_println("HP Sw=0, Mov out err");
            }
            hpswstate = HPSWOPEN;
          }
          else
          {
            hpswstate = driverboard->hpsw_alert();      // hpsw_alert returns true if closed, false = open
          }
        }
        if ( ControllerData->get_hpswitch_enable() == V_ENABLED )
        {
          DEBUG_print("HP Sw, Mov out steps: ");
          DEBUG_println(stepstaken);
          DEBUG_println("HP Sw=0, Mov out ok");
        }
        ftargetPosition = 0;
        driverboard->setposition(0);
        ControllerData->set_fposition(0);
        ControllerData->set_focuserdirection(DirOfTravel);        // set direction of last move
        if ( ControllerData->get_hpswmsg_enable() == V_ENABLED)
        {
          DEBUG_println("HP Sw=0, Mov out ok");
        }
      } //  if( ControllerData->get_homepositionswitch() == 1)
      TimeStampdelayaftermove = millis();
      FocuserState = State_DelayAfterMove;
      if ( ControllerData->get_hpswmsg_enable() == V_ENABLED)
      {
        DEBUG_println("go delayaftermove");
      }
      break;

    case State_DelayAfterMove:
      // apply Delayaftermove, this MUST be done here in order to get accurate timing for delayaftermove
      // the task timer runs on 100ms slices, so cannot be used to control delayaftermove, this is why
      // a timecheck is used instead
      if ( ControllerData->get_delayaftermove_enable() == 1 )
      {
        if (TimeCheck(TimeStampdelayaftermove, ControllerData->get_delayaftermove_time()))
        {
          damcounter = 0;
          FocuserState = State_EndMove;
        }
        // keep looping around till timecheck for delayaftermove succeeds
        // BUT ensure there is a way to exit state if delayaftermove fails to timeout
        // a loop cycle is 1-4ms
        damcounter++;
        if ( damcounter > 255 )
        {
          damcounter = 0;
          FocuserState = State_EndMove;
        }
      }
      else
      {
        // delay after move is disabled
        FocuserState = State_EndMove;
      }
      break;

    case State_EndMove:
      isMoving = false;
      // is parking enabled in controller?
      if ( ControllerData->get_park_enable() == true )
      {
        DEBUG_println("State_EndMove: park is enabled, set update_park_flag 0 to start the count");
        portENTER_CRITICAL(&parkMux);
        update_park_flag = 0;
        portEXIT_CRITICAL(&parkMux);
      }
      FocuserState = State_Idle;
      break;

    default:
      FocuserState = State_Idle;
      break;
  }
} // end Loop()
