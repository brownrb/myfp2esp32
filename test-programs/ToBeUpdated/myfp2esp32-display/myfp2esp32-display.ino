// myFP2ESP32 FIRMWARE OFFICIAL RELEASE 300
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// © Copyright Pieter P, SPIFFs examples found online
// © Copyright Paul P, 2021-2022. All Rights Reserved. TMC22xx code
//   myfp2esp32-firmware.ino
//   version: 300
//   date:    19-09-2022
// ----------------------------------------------------------------------

// display test


// ----------------------------------------------------------------------
// DO NOT EDIT BELOW THIS LINE
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// CONTROLLER DEFINITIONS AND CONFIGURATION
// ----------------------------------------------------------------------
#include "controller_defines.h"

// uncomment ONE of the following USESSxxxx lines depending upon your lcd type
// For the OLED 128x64 0.96" display with SSD1306 driver, uncomment the following line
#define USE_SSD1306   1

// For the OLED 128x64 1.3" display with SSH1106 driver, uncomment the following line
//#define USE_SSH1106   2


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>
#include <Wire.h>


// ----------------------------------------------------------------------
// DISPLAY
// Library  Text      myOLED
// Library  Graphics  https://github.com/ThingPulse/esp8266-oled-ssd1306
// Optional
// ----------------------------------------------------------------------
#include "display_text.h"
TEXT_DISPLAY *mydisplay;
bool display_status;  



// ----------------------------------------------------------------------
// bool init_display(void);
// initialise display vars and create pointer to the display class
// helper, optional
// ----------------------------------------------------------------------
bool display_init()
{
  mydisplay = new TEXT_DISPLAY(OLED_ADDR);
  return true;
}

// ----------------------------------------------------------------------
// bool display_start(void);
// Display start
// helper, optional
// ----------------------------------------------------------------------
bool display_start(void)
{
  if( display_status )
  {
    Serial.println("helper: display_start: already started");
    return true;
  }
  
  Serial.println("helper: display_start: start");
  if ( mydisplay->start() != true )         // attempt to start the display
  {
    Serial.println("helper: display_start: error starting display");
    display_status = false;             // display did not start
    return false;
  }
  else
  {
    Serial.println("helper: display_start: started");
    display_status = true;
    return true;
  }
  Serial.println("helper: display_start: error display not started");
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
  mydisplay->stop();                          // stop the display
  display_status = false;
  Serial.println("helper: display_stop: display stopped");
}

// ----------------------------------------------------------------------
// void display_off(void);
// Turn display off (blank)
// helper, optional
// ----------------------------------------------------------------------
void display_off()
{
  mydisplay->off();
}

// ----------------------------------------------------------------------
// void display_on(void);
// Turn display on
// helper, optional
// ----------------------------------------------------------------------
void display_on()
{
  mydisplay->on();
}

// ----------------------------------------------------------------------
// void display_update();
// Update display
// helper, optional
// ----------------------------------------------------------------------
void display_update()
{
  mydisplay->update();
}

//-------------------------------------------------
// void load_vars(void);
// Load cached vars, gives quicker access for web pages etc
// when any cached variable changes, it is reloaded
//-------------------------------------------------
void load_vars()
{
  // display
  display_status = false;
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

  delay(1000);
  
  Serial.println("Serial started");


  //-------------------------------------------------
  // Initialise vars
  //-------------------------------------------------
  Serial.println("Set vars and load cached vars");
  load_vars();


  //-------------------------------------------------
  // I2C INTERFACE START
  //-------------------------------------------------
  Serial.println("Start I2C");
  Wire.begin(I2CDATAPIN, I2CCLKPIN);          // pins defined in controller_defines.h


  //-------------------------------------------------
  // DISPLAY
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // call helper, if not defined then helper will return false;
  if ( display_init() == true )
  {
    Serial.println("Start display");
    if ( display_start() == false )
    {
      Serial.println("displaystart() error");
    }
  }
  Serial.println("Setup done, controller is ready");
}

void loop()
{
  if( display_status )
  {
    Serial.println("Update display");
    display_update();
  }
  else
  {
    Serial.println("Display not running");
  }
  delay(3000);
}
