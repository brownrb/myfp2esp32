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
#include "controller_config.h"

#if defined(ENABLE_TEXTDISPLAY)

// ----------------------------------------------------------------------
// EXTERN CLASSES
// ----------------------------------------------------------------------
// get access to class
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

#include "driver_board.h"
extern DRIVER_BOARD *driverboard;

// ----------------------------------------------------------------------
// TEMPERATURE PROBE
// ----------------------------------------------------------------------
#include "temp_probe.h"
extern TEMP_PROBE *tempprobe;

// For an optional class should not have access to the update flags etc
// For an optional class, keep serial messages to a minimum


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <mySSD1306Ascii.h>
#include <mySSD1306AsciiWire.h>
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
extern char  mySSID[];
extern int   myfp2esp32mode;                // controllermode, ACCESSPOINTMODE=1, STATIONMODE=2
extern bool  isMoving;
extern long  ftargetPosition;
extern float temp;

extern byte ota_status;
extern byte ascomsrvr_status;
extern byte mngsrvr_status;
extern byte tcpipsrvr_status;
extern byte websrvr_status;


// ----------------------------------------------------------------------
// DATA
// ----------------------------------------------------------------------
// Display Text DT_
// pg1
const char* DT_POSITION         = "Pos   : ";
const char* DT_TARGETPOSITION   = "Target: ";
const char* DT_ISMOVING         = "Moving: ";
const char* DT_TEMPERATURE      = "Temp  : ";
const char* DT_MAXSTEPS         = "MaxPos: ";
const char* DT_MOTORSPEED       = "Speed : ";
const char* DT_STEPMODE         = "SMode : ";
const char motor_speed[][5]     = { "Slow", "Med ", "Fast" };
// pg2
const char* DT_COILPOWER        = "Coil pwr : ";
const char* DT_REVERSE          = "Reverse  : ";
const char* DT_BACKLASHIN       = "BLI en   : ";
const char* DT_BACKLASHOUT      = "BLO en   : ";
const char* DT_BACKLASHINSTEPS  = "BLI step#: ";
const char* DT_BACKLASHOUTSTEPS = "BLO step#: ";
// pg3
const char* DT_STEPSIZE         = "Stepsz # : ";
const char* DT_STEPSIZESTATE    = "Stepsz en: ";
const char* DT_HPSWSTATE        = "hpsw en  : ";
const char *DT_INOUTLEDSTATE    = "LED  en  : ";
const char *DT_INOUTLEDMODE     = "LED Mode : ";
const char* DT_PBSTATE          = "PB en    : ";
const char* DT_PULSEMODE        = "Pul";
const char* DT_MOVEMODE         = "Mov";

//pg4
const char* DT_TEMPMODE           = "Temp C/F : ";
const char* DT_TEMPPROBESTATE     = "TProbe en: ";
const char* DT_TEMPCOMPSTATE      = "TComp en : ";
const char* DT_TEMPCOMPSTEPS      = "TComp #  : ";
const char* DT_TEMPCOMPDIRECTION  = "TComp Dir: ";
const char  DT_TEMPCELSIUS        = 'C';
const char  DT_TEMPFAHRENHEIT     = 'F';
const char* DT_ON                 = "On";
const char* DT_OFF                = "Off";
// pg5
const char* DT_PAGETIME         = "Pg time: ";
const char* DT_SHOWPOSONMOVE    = "ShowPos: ";
const char* DT_OTAUPDATE        = "OTA    : ";
const char* DT_DRIVERBOARD      = "DrvBrd : ";
const char* DT_FIRMWAREVERSION  = "Firm # : ";
// pg6
const char* DT_PRESETS          = "Presets";
const char* DT_PRESET1          = "1: ";
const char* DT_PRESET2          = "2: ";
const char* DT_PRESET3          = "3: ";
const char* DT_PRESET4          = "4: ";
const char* DT_PRESET5          = "5: ";
const char* DT_PRESET6          = "6: ";
// pg7
const char* DT_SSID             = "SSID:";
const char* DT_IP               = "IP  :";
const char* DT_ACCESSPOINT      = "Access Point";
const char* DT_STATIONMODE      = "Station Mode";
const char* DT_ASCOMSRVRSTATE   = "ASCOM Srvr:";
const char* DT_SRVRPORT         = "Port      :";
const char* DT_MANAGEMENTSRVRSTATE = "Mngnt Srvr:";
const char* DT_SRVRSTATERUN     = "Run";
const char* DT_SRVRSTATESTOP    = "Stop";
// pg8
const char* DT_TCPIPSRVRSTATE   = "TCPIP Srvr:";
const char* DT_WEBSRVRSTATE     = "Web Srvr  :";
const char* DT_PAGENUMBER       = "Pg ";


// OLED_ADDR is defined in controller_defines.h
// sda, scl are in Controller->get_brdsda() and ControllerData->get_brdsck()


// ----------------------------------------------------------------------
// CLASS
// ----------------------------------------------------------------------
TEXT_DISPLAY::TEXT_DISPLAY(uint8_t addr) : _addr(addr)
{
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
  if ( this->_state )
  {
    return true;
  }

  // if disabled then do not start
  if ( ControllerData->get_display_enable() == V_NOTENABLED)
  {
    DISPLAY_println("TEXT_DISPLAY::start(), display not enabled");
    return false;
  }

  // set defaults, if display is found then these will change
  this->_found = false;

  // check if display is present
  Wire.beginTransmission( _addr );
  if ( Wire.endTransmission() != 0 )
  {
    ERROR_println("TEXT_DISPLAY::start(), display not found");
    this->_found = false;
    return false;
  }
  else
  {
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
    _display->set400kHz();
    _display->setFont(Adafruit5x7);
    _display->clear();                                    // clear also sets cursor at 0,0
    _display->Display_Normal();                           // black on white
    _display->Display_On();                               // display ON
    _display->Display_Rotate(0);                          // portrait, not rotated
    _display->Display_Bright();
    _display->set2X();
    _display->println("myFP2ESP");
    _display->set1X();
    _display->println("booting");
    DISPLAY_println("TEXT_DISPLAY::start(), display found, running");
  }
  return true;
}

// -----------------------------------------------------------------------
// STOP DISPLAY
// void stop(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::stop(void)
{
  delete _display;
  this->_state = false;
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
  if ( this->_state )
  {
    draw_main_update();
  }
}

// -----------------------------------------------------------------------
// UPDATE POSITION
// -----------------------------------------------------------------------
void TEXT_DISPLAY::update_position(long position)
{
  if ( this->_state )
  {
    _display->setCursor(0, 0);
    _display->print(DT_POSITION);
    _display->print(position);
    _display->clearToEOL();
    _display->println();

    _display->print(DT_TARGETPOSITION);
    _display->print(ftargetPosition);
    _display->clearToEOL();
    _display->println();
  }
}

// -----------------------------------------------------------------------
// UPDATE DISPLAY PAGE
// void draw_main_update(void);
// controlled by task timer
// -----------------------------------------------------------------------
void TEXT_DISPLAY::draw_main_update()
{

  if ( this->_state )
  {
    static int displaybitmask = 1;

    _display->clear();                                      // clrscr OLED
    int page = 0;
    String mypage = ControllerData->get_displaypageoption();
    for (int i = 0; i < mypage.length(); i++)
    {
      page *= 2;
      if (mypage[i] == '1')
      {
        page++;
      }
    }

    // find the next page to display
    while ( (page & displaybitmask) == 0 )              // mask off one bit at a time, skip page if not enabled
    {
      displaybitmask *= 2;
      if ( displaybitmask > 255 )                       // 255 = B11111111
      {
        break;
      }
    } // while ( (page & displaybitmask) == 0 )

    // bitmask has stopped at current page indicating 1 in page option string
    // ensure that mask does not exceed page value
    if ( displaybitmask > 255 )                         // 255 = B111111111
    {
      displaybitmask = 1;                               // reset bitmask
    }

    // displaybitmask is now the page to display, 1=pg1, 2=pg2, 4=pg3, 8=pg4 etc
    switch (displaybitmask)
    {
      case 1:
        page1();
        break;
      case 2:
        page2();
        break;
      case 4:
        page3();
        break;
      case 8:
        page4();
        break;
      case 16:
        page5();
        break;
      case 32:
        page6();
        break;
      case 64:
        page7();
        break;
      case 128:
        page8();
        break;
      default:
        page1();
        break;
    } // switch (displaybitmask)
    displaybitmask *= 2;                                // next page
  } // for (int i = 0; i < mypage.length(); i++)
} //   if ( (_found == true) && (_loaded == true) )

// -----------------------------------------------------------------------
// Print page number at bottom of display
// -----------------------------------------------------------------------
void TEXT_DISPLAY::printpagenumber(byte pagenum)
{
  if ( this->_state )
  {
    _display->setCursor(0, 7);                            // place page number on bottom line of display
    _display->print(DT_PAGENUMBER);
    _display->print(pagenum);
  }
}

// -----------------------------------------------------------------------
// void page1(long, float);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::page1()
{
  if ( this->_state )
  {
    char tempString[20];

    // Position
    _display->home();
    _display->print(DT_POSITION);
    _display->print(driverboard->getposition());
    _display->clearToEOL();

    // Target
    _display->println();
    _display->print(DT_TARGETPOSITION);
    _display->print(ftargetPosition);
    _display->clearToEOL();
    _display->println();

    // ismoving
    _display->print(DT_ISMOVING);
    if ( isMoving == true )
    {
      _display->print(T_YES);
    }
    else
    {
      _display->print(T_NO);
    }
    _display->clearToEOL();
    _display->println();

    // Temperature
    _display->print(DT_TEMPERATURE);
    _display->print(String(temp, 2));
    if ( ControllerData->get_tempmode() == V_CELSIUS)
    {
      _display->print(" c");
    }
    else
    {
      _display->print(" f");
    }
    _display->clearToEOL();
    _display->println();

    // MaxSteps
    _display->print(DT_MAXSTEPS);
    ltoa(ControllerData->get_maxstep(), tempString, 10);
    _display->print(tempString);
    _display->clearToEOL();
    _display->println();

    // Motor speed
    _display->print(DT_MOTORSPEED);
    _display->print(motor_speed[ControllerData->get_motorspeed()][0]);
    _display->clearToEOL();
    _display->println();

    // Step mode
    _display->print(DT_STEPMODE);
    _display->print(ControllerData->get_brdstepmode());
    _display->clearToEOL();
    _display->println();

    printpagenumber(1);
  }
}

// -----------------------------------------------------------------------
// void page2(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::page2(void)
{
  if ( this->_state )
  {
    // Coil power
    _display->print(DT_COILPOWER);
    if ( ControllerData->get_coilpower_enable() == V_ENABLED )
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // Reverse direction
    _display->print(DT_REVERSE);
    if ( ControllerData->get_reverse_enable() == V_ENABLED )
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // backlash in state
    _display->print(DT_BACKLASHIN);
    if ( ControllerData->get_backlash_in_enable() == V_ENABLED )
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // backlash out state
    _display->print(DT_BACKLASHOUT);
    if ( ControllerData->get_backlash_out_enable() == V_ENABLED )
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // backlash in steps
    _display->print(DT_BACKLASHINSTEPS);
    _display->print(ControllerData->get_backlashsteps_in());
    _display->clearToEOL();
    _display->println();

    // backlash out steps
    _display->print(DT_BACKLASHOUTSTEPS);
    _display->print(ControllerData->get_backlashsteps_out());
    _display->clearToEOL();
    _display->println();

    printpagenumber(2);
  }
}

// -----------------------------------------------------------------------
// void page3(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::page3(void)
{
  if ( this->_state )
  {
    // Step Size
    _display->print(DT_STEPSIZE);
    _display->print(ControllerData->get_stepsize(), 2);
    _display->clearToEOL();
    _display->println();

    // Step Size state
    _display->print(DT_STEPSIZESTATE);
    if ( ControllerData->get_stepsize_enable() == V_ENABLED )
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // HPSW Enable State
    _display->print(DT_HPSWSTATE);
    if ( ControllerData->get_hpswitch_enable() == V_ENABLED )
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();


    // IN-OUT LED state
    _display->print(DT_INOUTLEDSTATE);
    if ( ControllerData->get_inoutled_enable() == V_ENABLED )
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // IN-OUT LED mode
    _display->print(DT_INOUTLEDMODE);
    if ( ControllerData->get_inoutledmode() == LEDPULSE )
    {
      _display->print(DT_PULSEMODE);
    }
    else
    {
      _display->print(DT_MOVEMODE);
    }
    _display->clearToEOL();
    _display->println();

    // Push Button state
    _display->print(DT_PBSTATE);
    if ( driverboard->get_pushbuttons_loaded() == V_ENABLED )
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();
    printpagenumber(3);
  }
}

// -----------------------------------------------------------------------
// void page4(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::page4(void)
{
  if ( this->_state )
  {
    //_display->setCursor(0,0);

    // temperature mode C/F
    _display->print(DT_TEMPMODE);
    if ( ControllerData->get_tempmode() == V_CELSIUS)
    {
      _display->print(DT_TEMPCELSIUS);
    }
    else
    {
      _display->print(DT_TEMPFAHRENHEIT);
    }
    _display->clearToEOL();
    _display->println();

    // Temp Probe State
    _display->print(DT_TEMPPROBESTATE);
    if ( tempprobe->get_state() == true)
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // temperature compenstation enable state
    _display->print(DT_TEMPCOMPSTATE);
    if ( ControllerData->get_tempcomp_enable() == V_ENABLED)
    {
      _display->print(DT_ON);
    }
    else
    {
      _display->print(DT_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // temperature compensation coefficient #steps
    _display->print(DT_TEMPCOMPSTEPS);
    _display->print(ControllerData->get_tempcoefficient());
    _display->clearToEOL();
    _display->println();

    // temperature compensation direction
    _display->print(DT_TEMPCOMPDIRECTION);
    _display->print(ControllerData->get_tcdirection());
    _display->clearToEOL();
    _display->println();

    printpagenumber(4);
  }
}

// -----------------------------------------------------------------------
// void page5(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::page5(void)
{
  if ( this->_state )
  {
    _display->print(DT_PAGETIME);
    _display->print(ControllerData->get_displaypagetime());
    _display->clearToEOL();
    _display->println();
    _display->print(DT_SHOWPOSONMOVE);        // update position when moving
    _display->print(ControllerData->get_displayupdateonmove() );
    _display->clearToEOL();
    _display->println();
    _display->print(DT_OTAUPDATE);
    _display->print(ota_status);
    _display->clearToEOL();
    _display->println();
    _display->print(DT_DRIVERBOARD);
    _display->print(DRVBRD);
    _display->clearToEOL();
    _display->println();
    _display->print(DT_FIRMWAREVERSION);
    _display->print(program_version);
    _display->clearToEOL();
    _display->println();

    printpagenumber(5);
  }
}

// -----------------------------------------------------------------------
// void page6(void);
// Focuser Presets
// -----------------------------------------------------------------------
void TEXT_DISPLAY::page6(void)
{
  if ( this->_state )
  {
    _display->print(DT_PRESETS);
    _display->clearToEOL();
    _display->println();
    _display->print(DT_PRESET1);
    _display->print(ControllerData->get_focuserpreset(0));
    _display->clearToEOL();
    _display->println();
    _display->print(DT_PRESET2);
    _display->print(ControllerData->get_focuserpreset(1));
    _display->clearToEOL();
    _display->println();
    _display->print(DT_PRESET3);
    _display->print(ControllerData->get_focuserpreset(2));
    _display->clearToEOL();
    _display->println();
    _display->print(DT_PRESET4);
    _display->print(ControllerData->get_focuserpreset(3));
    _display->clearToEOL();
    _display->println();
    _display->print(DT_PRESET5);
    _display->print(ControllerData->get_focuserpreset(4));
    _display->clearToEOL();
    _display->println();
    _display->print(DT_PRESET6);
    _display->print(ControllerData->get_focuserpreset(5));
    _display->clearToEOL();
    _display->println();

    printpagenumber(6);
  }
}

// -----------------------------------------------------------------------
// DISPLAY PAGE 7:
// void page7(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::page7(void)
{
  if ( this->_state )
  {
    _display->setCursor(0, 0);
    _display->print(DT_SSID);
    _display->println(mySSID);
    _display->clearToEOL();
    _display->println();
    _display->print(DT_IP);
    _display->print(ipStr);
    _display->clearToEOL();
    _display->println();
    if ( myfp2esp32mode == ACCESSPOINT)
    {
      _display->print(DT_ACCESSPOINT);
      _display->clearToEOL();
      _display->println();
    }
    else
    {
      _display->print(DT_STATIONMODE);
      _display->clearToEOL();
      _display->println();
    }

    _display->print(DT_ASCOMSRVRSTATE);
    (ascomsrvr_status == V_RUNNING) ? _display->print(DT_SRVRSTATERUN) : _display->print(DT_SRVRSTATESTOP);
    _display->clearToEOL();
    _display->println();
    _display->print(DT_SRVRPORT);
    _display->print(ControllerData->get_ascomsrvr_port());
    _display->clearToEOL();
    _display->println();
    _display->print(DT_MANAGEMENTSRVRSTATE);
    (mngsrvr_status == V_RUNNING) ? _display->print(DT_SRVRSTATERUN) : _display->print(DT_SRVRSTATESTOP);
    _display->clearToEOL();
    _display->println();
    _display->print(DT_SRVRPORT);
    _display->print(ControllerData->get_mngsrvr_port());
    _display->clearToEOL();
    _display->println();

    printpagenumber(7);
  }
}


// -----------------------------------------------------------------------
// DISPLAY PAGE 8: WiFi, Web server, ASCOM remote server settings
// void page8(void);
// -----------------------------------------------------------------------
void TEXT_DISPLAY::page8(void)
{
  if ( this->_state )
  {
    //_display->setCursor(0, 0);
    _display->print(DT_TCPIPSRVRSTATE);
    (tcpipsrvr_status == V_RUNNING) ? _display->print("R") : _display->print("S");
    _display->clearToEOL();
    _display->println();
    _display->print(DT_SRVRPORT);
    _display->print(ControllerData->get_tcpipsrvr_port());
    _display->clearToEOL();
    _display->println();

    _display->print(DT_WEBSRVRSTATE);
    (websrvr_status == V_RUNNING) ? _display->print("R") : _display->print("S");
    _display->clearToEOL();
    _display->println();
    _display->print(DT_SRVRPORT);
    _display->print(ControllerData->get_websrvr_port());
    _display->clearToEOL();
    _display->println();

    printpagenumber(8);
  }
}

void TEXT_DISPLAY::reboot( void )
{
  if ( this->_state )
  {
    _display->clear();
    _display->set2X();
    _display->println("REBOOT");
  }
}

void TEXT_DISPLAY::bright( void )
{
  if ( this->_state )
  {
    _display->Display_Bright();
  }
}

void TEXT_DISPLAY::dim( void )
{
  if ( this->_state )
  {
    _display->Display_Dim();
  }
}

void TEXT_DISPLAY::normal( void )
{
  if ( this->_state )
  {
    _display->Display_Normal();
  }
}

#endif // #if defined(ENABLE_TEXTDISPLAY)
