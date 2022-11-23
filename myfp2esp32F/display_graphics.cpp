// ----------------------------------------------------------------------
// myFP2ESP32 GRAPHICS DISPLAY CLASS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2021. All Rights Reserved.
// displays.cpp
// Optional, not part of default configuration
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// REQUIRES THE FOLLOWING LIBRARY
// ----------------------------------------------------------------------
// Download and install the library from https://github.com/ThingPulse/esp8266-oled-ssd1306


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include "controller_config.h"                  // includes boarddefs.h and controller_defines.h

#if defined(ENABLE_GRAPHICDISPLAY)
#include "SPIFFS.h"


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
// CONTROLLER CONFIG CLASS
// ----------------------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;


// ----------------------------------------------------------------------
// DRIVER BOARD CLASS
// ----------------------------------------------------------------------
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;


// ----------------------------------------------------------------------
// TEMPERATURE CLASS
// ----------------------------------------------------------------------
#include "temp_probe.h"
extern TEMP_PROBE *tempprobe;


// ----------------------------------------------------------------------
// TCPIP CLASS
// ----------------------------------------------------------------------
#include "tcpip_server.h"
extern TCPIP_SERVER *tcpipsrvr;


// For an optional class should not have access to the update flags etc
// For an optional class, keep serial messages to a minimum


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
// bitmaps for display : sky states, wind states, raining, boot-wifi, reboot
#include "images.h"
#include "display_graphics.h"


// ----------------------------------------------------------------------
// EXTERNS
// ----------------------------------------------------------------------
extern char mySSID[];
extern long  ftargetPosition;
extern float temp;
extern bool  filesystemloaded;


// ----------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------
#define MAX_WIDTH     127
#define MAX_HEIGHT    63


// ----------------------------------------------------------------------
// I2C defined in controller_defines.h
// ----------------------------------------------------------------------
// OLED_ADDR is defined in controller_defines.h
// I2CDATAPIN        21
// I2CCLKPIN         22


// ----------------------------------------------------------------------
// Determine the number of connected clients to the focuser
// ----------------------------------------------------------------------
// If one wants to find if there are any clients connected, so the display
// can be turned off, use
//
// if( tcpipsrvr->get_clients() == true) { // yes there is } else { // no }
//
// see below in - void GRAPHIC_DISPLAY::draw_main_update(Oled_States oledstate)


// ----------------------------------------------------------------------
// images.h
// Holds images - icons
// ----------------------------------------------------------------------
// WiFi_Logo_bits[]
// i_wifi[]
// i_temp[]
// i_reboot


// ----------------------------------------------------------------------
// DISPLAY CLASS
// ----------------------------------------------------------------------
GRAPHIC_DISPLAY::GRAPHIC_DISPLAY(uint8_t addr) : _addr(addr)
{
  this->_found = false;             // display found
  this->_state = false;             // display running
}

// ----------------------------------------------------------------------
// bool start(void);
// START DISPLAY
// ----------------------------------------------------------------------
bool GRAPHIC_DISPLAY::start(void)
{
  if ( this->_state )
  {
    return true;
  }

  this->_found = false;

  // if disabled then do not start
  if ( ControllerData->get_display_enable() == V_NOTENABLED)
  {
    DISPLAY_println("GRAPHIC_DISPLAY::start(), display not enabled");
    return false;
  }

  // check if display is present
  Wire.beginTransmission(_addr);
  if (Wire.endTransmission() != 0)
  {
    ERROR_println("GRAPHIC_DISPLAY::start(), display not found");
    this->_found = false;
    return false;
  }
  else
  {
#if defined(USE_SSD1306)
    _display = new SSD1306Wire(_addr, I2CDATAPIN, I2CCLKPIN, GEOMETRY_128_64);
#else
    _display = new SH1106Wire(_addr, I2CDATAPIN, I2CCLKPIN, GEOMETRY_128_64);
#endif
    this->_found = true;
    this->_state = true;
    _display->init();
    delay(1000);
    _display->flipScreenVertically();
    _display->setFont(ArialMT_Plain_10);
    _display->setTextAlignment(TEXT_ALIGN_LEFT);
    _display->clear();
    _display->setTextAlignment(TEXT_ALIGN_CENTER);
    _display->setFont(ArialMT_Plain_10);
    _display->drawString(0, 0, "myFP2ESP32: " + String(program_version));
    _display->drawString(0, 12, "SSID: " + String(mySSID));
    //_display->drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits); // draw wifi logo
    display_draw_xbm(nwifi, 34, 14);
    _display->display();
    DISPLAY_println("GRAPHIC_DISPLAY::start(), display found, running");
  }
  return true;
}


// -----------------------------------------------------------------------
// void stop(void);
// STOP DISPLAY
// -----------------------------------------------------------------------
void GRAPHIC_DISPLAY::stop(void)
{
  delete _display;
  this->_state = false;
}

// -----------------------------------------------------------------------
// void clear(void);
// DISPLAY CLEAR
// -----------------------------------------------------------------------
void GRAPHIC_DISPLAY::clear(void)
{
  if ( this->_state )
  {
    _display->clear();
  }
}


// -----------------------------------------------------------------------
// void off(void);
// DISPLAY OFF
// -----------------------------------------------------------------------
void GRAPHIC_DISPLAY::off(void)
{
  if (this->_state )
  {
    _display->displayOff();
  }
}

// -----------------------------------------------------------------------
// void on(void);
// DISPLAY ON
// -----------------------------------------------------------------------
void GRAPHIC_DISPLAY::on(void)
{
  if ( this->_state )
  {
    _display->displayOn();
  }
}

// -----------------------------------------------------------------------
// void update(void);
// UPDATE DISPLAY PAGE
// -----------------------------------------------------------------------
void GRAPHIC_DISPLAY::update()
{
  if ( this->_state )
  {
    draw_main_update();
  }
  else
  {
    _display->clear();
    _display->display();
  }
}

// -----------------------------------------------------------------------
// void update_position(long);
// UPDATE POSITION
// writes focuser position at specific location on display when the focuser is moving
// pixels: x = 0 - 127, y = 0 - 63
// -----------------------------------------------------------------------
void GRAPHIC_DISPLAY::update_position(long position)
{
  // it is ok to comment this code out
  //int x = 48;
  //int y = 0;

  //if( ( _state == V_RUNNING) && (ControllerData->get_oledstate() == true) )
  //{
  //  char buff[12];
  //  snprintf(buff, sizeof(buff), "%ld", position);
  //  _display->drawString( 64, 28, buff);
  //  _display->display();
  //}
}

const char heartbeat[] = { '|', '/' , '-', '\\'};

void GRAPHIC_DISPLAY::draw_main_update()
{
  char buffer[80];

  if ( this->_state )
  {
    _display->clear();
    _display->setTextAlignment(TEXT_ALIGN_CENTER);
    _display->setFont(ArialMT_Plain_24);

    // check if there is a client connected
    if ( tcpipsrvr->get_clients() == false)
    {
      // no client connected
      _display->drawString(64, 28, F("offline"));
      _display->setFont(ArialMT_Plain_10);
      String top = "myF2ESP32:" + String(program_version);
      //_display->drawString(64, 0, ControllerData->get_brdname());
      _display->drawString(64, 0, top);
      snprintf(buffer, sizeof(buffer), "IP= %s", ipStr);
      _display->drawString(64, 14, buffer);
    }
    else
    {
      // tcpip client is connected
      char dir = (ControllerData->get_focuserdirection() == moving_in ) ? '<' : '>';
      snprintf(buffer, sizeof(buffer), "%ld:%i %c", driverboard->getposition(), (int)(driverboard->getposition() % ControllerData->get_brdstepmode()), dir);
      _display->drawString(64, 28, buffer);

      _display->setFont(ArialMT_Plain_10);
      snprintf(buffer, sizeof(buffer), "µSteps: %i MaxPos: %ld", ControllerData->get_brdstepmode(), ControllerData->get_maxstep());
      _display->drawString(64, 0, buffer);
      snprintf(buffer, sizeof(buffer), "TargetPos:  %ld", ftargetPosition);
      _display->drawString(64, 12, buffer);
    }

    _display->setTextAlignment(TEXT_ALIGN_LEFT);

    if ( tempprobe->get_state() == true)
    {
      snprintf(buffer, sizeof(buffer), "TEMP: %.2f C", temp);
      _display->drawString(54, 54, buffer);
    }
    else
    {
      snprintf(buffer, sizeof(buffer), "TEMP: %.2f C", 20.0);
    }

    snprintf(buffer, sizeof(buffer), "BL: %i", ControllerData->get_backlashsteps_out());
    _display->drawString(0, 54, buffer);

    snprintf(buffer, sizeof(buffer), "%c", heartbeat[++count_hb % 4]);
    _display->drawString(8, 14, buffer);

    _display->display();
  }
}

// TODO
// icons are available!
// reboot icon in images.h i_reboot[]
// temperature icon in images.h i_temp[]
// TODO add motor moving icon - when a move occurs, show target position and motor icon with heartbeat
// enum logo_num { nwifi, ntemp, nreboot };

// -----------------------------------------------------------------------
// DISPLAY DRAW XBM ICON
// void display_draw_xbm(logo_num, int16_t, int16_t);
// write bitmap id:num at pos x, pos y
// logonum is the ID of a logo from the images.h file
// -----------------------------------------------------------------------
void GRAPHIC_DISPLAY::display_draw_xbm(logo_num num, int16_t x, int16_t y)
{
  if ( this->_state )
  {
    // make all icons 40 pixels high
    // x becomes right aligned, passed value of x is ignored
    // y becomes MAX_HEIGHT - image height - 1 = 63 - 40 = 23 - 1 = 22
    // ((MAX_HEIGHT - wifi_height) - 1)
    switch ( num )
    {
      case nwifi:
        _display->drawXbm(((MAX_WIDTH - wifi_width) - 2), ((MAX_HEIGHT - wifi_height) - 1), wifi_width, wifi_height, i_wifi);
        break;
      // temperature is bottom left aligned
      case ntemp:
        _display->drawXbm(2, ((MAX_HEIGHT - temp_height) - 1), temp_width, temp_height, i_temp); // draw temperature
        break;
      // reboot is centered
      case nreboot:
        // x pos is (max width / 2) - (reboot_width / 2)
        // y pos is (max height / 2) - (reboot_height / 2)
        _display->drawXbm( ((MAX_WIDTH / 2) - ((reboot_width / 2) - 1)), ((MAX_HEIGHT / 2) - ((reboot_height / 2) - 1)), reboot_width, reboot_height, i_reboot ); // draw reboot icon
        break;
    }
    _display->display();
  }
}

// show reboot message
void GRAPHIC_DISPLAY::reboot(void)
{
  // ignore oledstate because this is a reboot
  if ( this->_state )
  {
    _display->clear();
    _display->setTextAlignment(TEXT_ALIGN_CENTER);
    _display->setFont(ArialMT_Plain_24);
    _display->drawString(64, 28, "REBOOT");
    _display->display();
  }
}

#endif // #if defined(ENABLE_GRAPHICDISPLAY)
