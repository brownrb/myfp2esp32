// ----------------------------------------------------------------------
// myFP2ESP32 TEXT_DISPLAY (OLED 0.96", 128x64, 16 chars by 8 lines)
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2021. All Rights Reserved.
// displays.cpp
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>
#include <Wire.h>
#include "controller_defines.h"

// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
// ** #include <mySSD1306Ascii.h>
// ** #include <mySSD1306AsciiWire.h>
// get class description
#include "display_text.h"


// -----------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------
// DO NOT ENABLE DEBUGGING INFORMATION.

// Remove comment to enable messages to Serial port
//#define DISPLAY_PRINT       1

// -----------------------------------------------------------------------
// DO NOT CHANGE
// -----------------------------------------------------------------------
#ifdef  DISPLAY_PRINT
#define DISPLAY_print(...)   Serial.print(__VA_ARGS__)
#define DISPLAY_println(...) Serial.println(__VA_ARGS__)
#else
#define DISPLAY_print(...)
#define DISPLAY_println(...)
#endif


// ----------------------------------------------------------------------
// EXTERNS
// ----------------------------------------------------------------------
extern bool display_status;

// ----------------------------------------------------------------------
// DATA
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// CLASS
// ----------------------------------------------------------------------
// ** TEXT_DISPLAY::TEXT_DISPLAY(uint8_t addr) : _addr(addr)
TEXT_DISPLAY::TEXT_DISPLAY(uint8_t addr)
{
  this->_addr = addr;
  this->_found = false;
  this->_state = false;
}


// ----------------------------------------------------------------------
// START DISPLAY
// bool start(void);
// ----------------------------------------------------------------------
bool TEXT_DISPLAY::start()
{
  // prevent any attempt to start if already started
  Serial.println("display: start()");
  if ( this->_state  )
  {
    Serial.println("display: start: already running");
    return true;
  }

  // set defaults, if display is found then these will change
  this->_found = false;

  // check if display is present
  Serial.println("display: start: search for display");
  Wire.beginTransmission(_addr);
  if (Wire.endTransmission() != 0)
  {
    Serial.println("display: start: error display not found");
    this->_found = false;
    return false;
  }
  else
  {
    Serial.print("display: start: display found ");
    Serial.print("at address ");
    Serial.println(this->_addr, HEX);

    _display = new SSD1306AsciiWire();
    // Setup the OLED 128x64 0.96" display using the SSD1306 driver
#if defined(USE_SSD1306)
    _display->begin(&Adafruit128x64, _addr);
#else
    // Setup the OLED 128x64 1.3" display using the SSH1106 driver
    _display->begin(&SH1106_128x64, _addr);
#endif
    this->_found = true;
    this->_state = true;
    //_display->set400kHz();
    _display->setFont(Adafruit5x7);
    _display->clear();                                    // clear also sets cursor at 0,0
    _display->Display_Normal();                           // black on white
    _display->Display_On();                               // display ON
    _display->Display_Rotate(0);                          // portrait, not rotated
    _display->Display_Bright();
    _display->set2X();
    _display->println("myFP2ESP");
    Serial.println("TEXT_DISPLAY::start(), display found, running");
    return true;
  }
}


// -----------------------------------------------------------------------
// STOP DISPLAY
// void stop(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::stop(void)
{
  delete _display;
  this->_state = false;
  DISPLAY_println("display: stopped");
}

// -----------------------------------------------------------------------
// DISPLAY CLEAR
// void clear(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::clear(void)
{
  if ( this->_state )
  {
    _display->clear();
    Serial.println("display: clear");
  }
}

// -----------------------------------------------------------------------
// DISPLAY OFF
// void off(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::off(void)
{
  if ( this->_state )
  {
    _display->Display_Off();
    Serial.println("display: off");
  }
}

// -----------------------------------------------------------------------
// DISPLAY ON
// void on(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::on( void )
{
  if ( this->_state )
  {
    _display->Display_On();
  }
}

// -----------------------------------------------------------------------
// UPDATE DISPLAY PAGE
// void update(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::update()
{
  static int lp = 0;
  if ( this->_state )
  {
    _display->clear();                                    // clear also sets cursor at 0,0
    _display->set2X();
    _display->println(lp);
    lp++;
  }
  else 
  {
    Serial.println("display: update: error: display not running");
  }
}
