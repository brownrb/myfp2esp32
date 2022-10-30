// ----------------------------------------------------------------------
// myFP2ESP32 TCP/IP SERVER ROUTINES AND DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// tcpipserver.cpp
// Default Configuration
// For ASCOM client via myFP2ESPASOM driver
// For Windows and Linux applications
// For INDI clients via myFP2 INDI driver using TCPIP (no serial support)
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include "SPIFFS.h"
#include <SPI.h>

#include "controller_defines.h"
#include "controller_config.h"


// -----------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------
// DO NOT ENABLE DEBUGGING INFORMATION.

// Remove comment to enable messages to Serial port
//#define TCPSRVR_PRINT       1

// -----------------------------------------------------------------------
// DO NOT CHANGE
// -----------------------------------------------------------------------
#ifdef  TCPSRVR_PRINT
#define TCPSRVR_print(...)   Serial.print(__VA_ARGS__)
#define TCPSRVR_println(...) Serial.println(__VA_ARGS__)
#else
#define TCPSRVR_print(...)
#define TCPSRVR_println(...)
#endif


// ----------------------------------------------------------------------
// EXTERNS
// ----------------------------------------------------------------------
// ControllerData
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

// Driver board
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;

// temp probe
#include "temp_probe.h"
extern TEMP_PROBE *tempprobe;

// ASCOM server
#include "ascom_server.h"
extern ASCOM_SERVER *ascomsrvr;

// MANAGEMENT server
#include "management_server.h"
extern MANAGEMENT_SERVER *mngsrvr;

// web server
#include "web_server.h"
extern WEB_SERVER *websrvr;

#include "tcpip_server.h"

extern byte ascomsrvr_status;
extern byte mngsrvr_status;
extern byte websrvr_status;
extern volatile unsigned int park_maxcount;
extern volatile unsigned int display_maxcount;
extern portMUX_TYPE parkMux;
extern portMUX_TYPE displaytimeMux;


// ----------------------------------------------------------------------
// EXTERN HELPER FUNCTIONS
// ----------------------------------------------------------------------
extern byte display_status;
extern bool display_on(void);
extern bool display_off(void);
extern void reboot_esp32(int);
extern long getrssi(void);


// ----------------------------------------------------------------------
// EXTERNS VARS
// ----------------------------------------------------------------------
extern volatile bool halt_alert;
extern portMUX_TYPE  halt_alertMux;

extern char ipStr[];
extern char mySSID[];

extern long ftargetPosition;                    // target position
extern bool isMoving;
extern bool filesystemloaded;                   // flag indicator for webserver usage, rather than use SPIFFS.begin() test
extern float temp;


// ----------------------------------------------------------------------
// CLASS: TCPIP Server
// ----------------------------------------------------------------------
TCPIP_SERVER::TCPIP_SERVER()
{

}

// ----------------------------------------------------------------------
// bool start(void);
// Create and start the TCP/IP Server
// ----------------------------------------------------------------------
bool TCPIP_SERVER::start(unsigned long port)
{
  // if the server is not enabled then return
  if ( ControllerData->get_tcpipsrvr_enable() == V_NOTENABLED)
  {
    TCPSRVR_println("tcp: error: start failed: tcpip server not enabled:");
    return false;
  }

  // prevent any attempt to start if already started
  if ( this->_loaded == true )
  {
    return true;
  }

  this->_port = port;

  // check if port has changed
  if ( this->_port != ControllerData->get_tcpipsrvr_port() )
  {
    ControllerData->set_tcpipsrvr_port(this->_port);
  }

  // cache the presets
  cachepresets();

  //-------------------------------------------------
  // CREATE TCP/IP SERVER CONNECTION MANAGEMENT
  //-------------------------------------------------
  for (int lp = 0; lp < MAXCONNECTIONS; lp++)     // clear lists for client connections
  {
    _myclientsfreeslot[lp] = false;
  }
  _totalclients = 0;

  // check if server already created, if not, create one
  if ( this->_loaded == false )
  {
    _myserver = new WiFiServer(_port);
  }

  _myserver->begin( this->_port );
  this->_loaded = true;
  this->_state = V_RUNNING;
  return this->_loaded;
}

// ----------------------------------------------------------------------
// void stop(void);
// Stop the TCPIP SERVER
// This will stop and delete _myserver
// This must be done because start() creates _myserver
// ----------------------------------------------------------------------
void TCPIP_SERVER::stop(void)
{
  // can only stop a server that is this->_loaded
  if ( this->_loaded == true )
  {
    _myserver->stop();
    TCPSRVR_println("tcp: stop");
    delete _myserver;
    this->_loaded = false;
    this->_state = V_STOPPED;
  }
}

// ----------------------------------------------------------------------
// void loop(void);
// Checks for any new clients or existing client requests
// ----------------------------------------------------------------------
void TCPIP_SERVER::loop(bool parkstate)
{
  // avoid a crash
  if ( this->_loaded == false )
  {
    return;
  }

  this->_parked = parkstate;
  WiFiClient newclient = _myserver->available();              // check if any new connections
  //TCPSRVR_println("tcp: check new clients");
  if ( newclient )                                            // if there is a new tcp/ip client
  {
    TCPSRVR_println("tcp: new client found");
    // search for free slot, return first free slot found
    int lp = 0;
    for ( lp = 0; lp < MAXCONNECTIONS; lp++ )
    {
      if ( _myclientsfreeslot[lp] == false )
      {
        break;
      }
    }
    if ( lp < MAXCONNECTIONS )                                // if there is a free slot
    {
      TCPSRVR_println("tcp: client connected");
      _myclients[lp] = new WiFiClient(newclient);             // save new client to client list
      _myclientsIPAddressList[lp] = newclient.remoteIP();     // myFP2 get IP of client
      _myclientsfreeslot[lp] = true;                          // indicate slot is in use
      _totalclients++;
      newclient.stop();                                       // newClient will dispose at end of loop()
      // TODO turn oled_state true to start display for this client?
    }
    else
    {
      ERROR_println("tcp: error, max connections");
      newclient.stop();
    }
  }

  // cycle through each tcp/ip client connection
  if ( _totalclients > 0 )                                    // faster to avoid for loop if there are no clients
  {
    for ( int lp = 0; lp < MAXCONNECTIONS; lp++ )             // check all wifi client slots for data
    {
      if ( _myclientsfreeslot[lp] == true )                   // if there is a client
      {
        if (_myclients[lp]->connected())                      // if client is connected
        {
          while (_myclients[lp]->available())                 // if client has send request
          {
            process_command(lp);                              // process request and send client number
          }
        }
        else
        {
          _myclients[lp]->stop();                             // not connected, stop client
          _myclientsfreeslot[lp] = false;                     // free client space
          delete _myclients[lp];                              // release client
          _myclients[lp] = NULL;
          _totalclients--;
          if ( this->_totalclients < 0 )
          {
            this->_totalclients = 0;
          }
        } // if (myclients[lp].connected())
      } // if ( myclientsfreeslot[lp] == true )
    } // for ( int lp = 0; lp < MAXCONNECTIONS; lp++ )
  } // if ( totalclients > 0 )
}

bool TCPIP_SERVER::get_clients(void)
{
  if ( this->_totalclients == 0 )
  {
    return false;
  }
  else
  {
    return true;
  }
}

// ----------------------------------------------------------------------
// Cache focuser presets for faster access
// ----------------------------------------------------------------------
void TCPIP_SERVER::cachepresets(void)
{
  for ( int lp = 0; lp < 10; lp++ )
  {
    _presets[lp] = ControllerData->get_focuserpreset(lp);
  }
}

// ----------------------------------------------------------------------
// Convert float to ascii string
// ----------------------------------------------------------------------
char * TCPIP_SERVER::ftoa(char *a, double f, int precision)
{
  const long p[] = {0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};

  char *ret = a;
  long heiltal = (long)f;
  itoa(heiltal, a, 10);
  while (*a != '\0') a++;
  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  itoa(desimal, a, 10);
  return ret;
}

void TCPIP_SERVER::send_reply(const char *str, int clientnum)
{
  if ( _myclients[clientnum]->connected() )                          // if client is still connected
  {
    _myclients[clientnum]->print(str);                               // send reply
  }
}

void TCPIP_SERVER::build_reply(const char token, const char *str, int clientnum)
{
  char buff[32];
  snprintf(buff, sizeof(buff), "%c%s%c", token, str, _EOFSTR);
  send_reply(buff, clientnum);
}

void TCPIP_SERVER::build_reply(const char token, unsigned char data_val, int clientnum)
{
  char buff[32];
  snprintf(buff, sizeof(buff), "%c%u%c", token, data_val, _EOFSTR);
  send_reply(buff, clientnum);
}

void TCPIP_SERVER::build_reply(const char token, float data_val, int i, int clientnum)    // i = decimal places
{
  char buff[32];
  char tmp[10];
  // Note Arduino snprintf does not support .2f
  ftoa(tmp, data_val, i);
  snprintf(buff, sizeof(buff), "%c%s%c", token, tmp, _EOFSTR);
  send_reply(buff, clientnum);
}

void TCPIP_SERVER::build_reply(const char token, int data_val, int clientnum)
{
  char buff[32];
  snprintf(buff, sizeof(buff), "%c%i%c", token, data_val, _EOFSTR);
  send_reply(buff, clientnum);
}

void TCPIP_SERVER::build_reply(const char token, String str, int clientnum)
{
  char buff[32];
  char tmp[30];
  str.toCharArray(tmp, str.length());
  snprintf(buff, str.length() + 1, "%c%s%c", token, tmp, _EOFSTR);
  send_reply(buff, clientnum);
}

void TCPIP_SERVER::build_reply(const char token, long data_val, int clientnum)
{
  char buff[32];
  snprintf(buff, sizeof(buff), "%c%ld%c", token, data_val, _EOFSTR);
  send_reply(buff, clientnum);
}

void TCPIP_SERVER::build_reply(const char token, unsigned long data_val, int clientnum)
{
  char buff[32];
  snprintf(buff, sizeof(buff), "%c%lu%c", token, data_val, _EOFSTR);
  send_reply(buff, clientnum);
}

void TCPIP_SERVER::nullarg( int cmdval )
{
  ERROR_print("tcp: cmd was null: error: cmd: ");
  ERROR_println(cmdval);
}

void TCPIP_SERVER::process_command(int clientnum)
{
  static byte joggingstate = 0;               // myfp2 compatibility
  static byte joggingdirection = 0;           // myfp2 compatibility
  static byte delayeddisplayupdatestatus = 0; // myfp2 compatibility

  byte   cmdvalue;
  String receiveString = "";
  String WorkString = "";
  long   paramvalue = 0;

  String drvbrd = ControllerData->get_brdname();
  receiveString = _myclients[clientnum]->readStringUntil(_EOFSTR);  // read until terminator
  receiveString = receiveString + '#' + "";

  String cmdstr = receiveString.substring(1, 3);

  if ( cmdstr[0] == 'A' )
  {
    cmdvalue = 100 + (cmdstr[1] - '0');                               // can only use digits A0-A9
  }
  else if ( cmdstr[0] == 'B' )
  {
    cmdvalue = 110 + (cmdstr[1] - '0');                               // can only use digits B0-B9
  }
  else if ( cmdstr[0] == 'C' )
  {
    cmdvalue = 120 + (cmdstr[1] - '0');                               // can only use digits C0-C9
  }
  else
  {
    cmdvalue = cmdstr.toInt();
  }

  TCPSRVR_print("tcp: recstr=" + receiveString + "  ");
  TCPSRVR_println("tcp: cmdstr=" + cmdstr);
  switch (cmdvalue)
  {
    case 0: // myFP2 get focuser position
      build_reply('P', driverboard->getposition(), clientnum);
      break;
    case 1: // myFP2 ismoving
      build_reply('I', isMoving, clientnum);
      break;
    case 2: // myFP2 get controller status
      build_reply('E', "OK", clientnum);
      break;
    case 3: // myFP2 get firmware version
      build_reply('F', program_version, clientnum);
      break;
    case 4: // myFP2 get get_brdname + version number
      {
        char buff[32];
        char tempstr[20];
        String brdname = ControllerData->get_brdname();
        brdname.toCharArray(tempstr, brdname.length() + 1);
        snprintf(buff, sizeof(buff), "%s%c%c%s", tempstr, '\r', '\n', program_version );
        build_reply('F', buff, clientnum);
      }
      break;
    case 5: // myFP2 Set new target position to xxxxxx (and focuser initiates immediate move to xxxxxx)
      // only if not already moving
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        ftargetPosition = WorkString.toInt();
        ftargetPosition = (ftargetPosition < 0) ? 0 : ftargetPosition;
        ftargetPosition = (ftargetPosition > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : ftargetPosition;
      }
      break;
    case 6: // myFP2 get temperature
      build_reply('Z', temp, 3, clientnum);
      break;
    case 7: // myFP2 Set maxsteps
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        long tmppos = WorkString.toInt();

        // check to make sure not above largest value for maxstep
        tmppos = (tmppos > FOCUSERUPPERLIMIT) ? FOCUSERUPPERLIMIT : tmppos;
        // check if below lowest set valueue for maxstep
        tmppos = (tmppos < FOCUSERLOWERLIMIT) ? FOCUSERLOWERLIMIT : tmppos;
        // check to make sure its not less than current focuser position
        tmppos = (tmppos < driverboard->getposition()) ? driverboard->getposition() : tmppos;
        ControllerData->set_maxstep(tmppos);
      }
      break;
    case 8: // myFP2 get maxStep
      build_reply('M', ControllerData->get_maxstep(), clientnum);
      break;
    case 9: // myFP2ESP32 get _inoutledmode, pulse or move
      build_reply('$', ControllerData->get_inoutledmode(), clientnum);
      break;
    case 10: // myFP2 get maxIncrement
      build_reply('Y', ControllerData->get_maxstep(), clientnum);
      break;
    case 11: // myFP2 get coil power enable
      build_reply('O', ControllerData->get_coilpower_enable(), clientnum);
      break;
    case 12: // myFP2 set coil power enable
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt();
      // if 1, enable coilpower, set coilpowerstate true, enable motor
      // if 0, disable coilpower, set coilpowerstate false; release motor
      ( paramvalue == 1 ) ? driverboard->enablemotor() : driverboard->releasemotor();
      ( paramvalue == 1 ) ? ControllerData->set_coilpower_enable(V_ENABLED) : ControllerData->set_coilpower_enable(V_NOTENABLED);
      break;
    case 13: // myFP2 get reverse direction setting, 00 off, 01 on
      build_reply('R', ControllerData->get_reverse_enable(), clientnum);
      break;
    case 14: // myFP2 set reverse direction
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramvalue = (byte) WorkString.toInt();
        ( paramvalue == 1 ) ? ControllerData->set_reverse_enable(V_ENABLED) : ControllerData->set_reverse_enable(V_NOTENABLED);
      }
      break;
    case 15: // myFP2 set motor speed
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte)WorkString.toInt() & 3;
      ControllerData->set_motorspeed((byte) paramvalue);
      break;
    case 16: // myFP2 set temperature display setting to celsius
      ControllerData->set_tempmode(V_CELSIUS); // temperature display mode, Celsius=1, Fahrenheit=0
      break;
    case 17: // myFP2 set temperature display setting to fahrenheit
      ControllerData->set_tempmode(V_FAHRENHEIT); // temperature display mode, Celsius=1, Fahrenheit=0
      break;
    case 18: // myFP2 set Stepsize enable state
      // :180#    None    Set stepsize to be OFF - default
      // :181#    None    stepsize to be ON - reports what user specified as stepsize
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      ControllerData->set_stepsize_enable((byte) paramvalue);
      break;
    case 19: // myFP2 set the step size value - double type, eg 2.1
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        float tempstepsize = (float)WorkString.toFloat();
        tempstepsize = (tempstepsize < MINIMUMSTEPSIZE ) ? MINIMUMSTEPSIZE : tempstepsize;
        tempstepsize = (tempstepsize > MAXIMUMSTEPSIZE ) ? MAXIMUMSTEPSIZE : tempstepsize;
        ControllerData->set_stepsize(tempstepsize);
      }
      break;
    case 20: // myFP2 set the temperature resolution setting for the DS18B20 temperature probe
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = WorkString.toInt();
      paramvalue = (paramvalue <  9) ?  9 : paramvalue;
      paramvalue = (paramvalue > 12) ? 12 : paramvalue;
      ControllerData->set_tempresolution((byte) paramvalue);
      tempprobe->set_resolution((byte) paramvalue);             // myFP2 set probe resolution
      break;
    case 21: // myFP2 get temp probe resolution
      build_reply('Q', ControllerData->get_tempresolution(), clientnum);
      break;
    case 22: // myFP2 set temperature coefficient steps value to xxx
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = WorkString.toInt();
      ControllerData->set_tempcoefficient(paramvalue);
      break;
    case 23: // myFP2 set the temperature compensation ON (1) or OFF (0)
      if ( tempprobe->get_state() == V_RUNNING)
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramvalue = (byte)WorkString.toInt() & 0x01;
        ControllerData->set_tempcomp_enable((byte) paramvalue);
      }
      break;
    case 24: // myFP2 get status of temperature compensation (enabled | disabled)
      build_reply('1', ControllerData->get_tempcomp_enable(), clientnum);
      break;
    case 25: // myFP2 get temperature compensation available
      build_reply('A', ControllerData->get_tcavailable(), clientnum);
      break;
    case 26: // myFP2 get temperature coefficient steps/degree
      build_reply('B', ControllerData->get_tempcoefficient(), clientnum);
      break;
    case 27: // myFP2 stop a move - like a Halt
      portENTER_CRITICAL(&halt_alertMux);
      halt_alert = true;
      portEXIT_CRITICAL(&halt_alertMux);
      break;
    case 28: // myFP2 home the motor to position 0
      if ( isMoving == 0 )
      {
        ftargetPosition = 0; // if this is a home then set target to 0
      }
      break;
    case 29: // myFP2 get stepmode
      build_reply('S', ControllerData->get_brdstepmode(), clientnum);
      break;

    // ----------------------------------------------------------------------
    // Basic rule for setting stepmode
    // myFP2 set DRIVER_BOARD->setstepmode(xx);                         // this sets the physical pins
    // and this also saves ControllerData->set_brdstepmode(xx);   // this saves config setting
    case 30: // myFP2 set step mode
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramvalue = WorkString.toInt();
        int brdnum = ControllerData->get_brdnumber();
        if (brdnum == PRO2ESP32ULN2003 || brdnum == PRO2ESP32L298N || brdnum == PRO2ESP32L293DMINI || brdnum == PRO2ESP32L9110S)
        {
          paramvalue = (int)(paramvalue & 3);      // STEP1 - STEP2
        }
        else if (brdnum == PRO2ESP32DRV8825 || brdnum == PRO2ESP32R3WEMOS)
        {
          paramvalue = (paramvalue < STEP1 ) ? STEP1 : paramvalue;
          paramvalue = (paramvalue > STEP32) ? STEP32 : paramvalue;
        }
        else if (brdnum == PRO2ESP32TMC2225 || brdnum == PRO2ESP32TMC2209 || brdnum == PRO2ESP32TMC2209P )
        {
          paramvalue = (paramvalue < STEP1 )  ? STEP1   : paramvalue;
          paramvalue = (paramvalue > STEP256) ? STEP256 : paramvalue;
        }
        else
        {
          TCPSRVR_print("tcp: invalid brd: ");
          TCPSRVR_println(brdnum);
        }
      }
      ControllerData->set_brdstepmode((int)paramvalue);
      driverboard->setstepmode((int)paramvalue);
      break;
    case 31: // myFP2 set focuser position
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        {
          long tpos = (long)WorkString.toInt();
          tpos = (tpos < 0) ? 0 : tpos;
          tpos = (tpos > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tpos;
          ftargetPosition = tpos;
          driverboard->setposition(tpos);
          ControllerData->set_fposition(tpos);
        }
      }
      break;
    case 32: // myFP2 get if stepsize is enabled
      build_reply('U', ControllerData->get_stepsize_enable(), clientnum);
      break;
    case 33: // myFP2 get stepsize
      build_reply('T', ControllerData->get_stepsize(), 2, clientnum);
      break;
    case 34: // myFP2 get the time that a display page is shown for
      build_reply('X', ControllerData->get_displaypagetime(), clientnum);
      break;
    case 35: // myFP2 set the time a display page is displayed for in seconds, integer, 2-10
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = WorkString.toInt();
      paramvalue = ( paramvalue < V_DISPLAYPAGETIMEMIN ) ? V_DISPLAYPAGETIMEMIN : paramvalue;
      paramvalue = ( paramvalue > V_DISPLAYPAGETIMEMAX ) ? V_DISPLAYPAGETIMEMAX : paramvalue;
      ControllerData->set_displaypagetime(paramvalue);
      // update display_maxcount
      portENTER_CRITICAL(&displaytimeMux);
      display_maxcount = paramvalue * 10;                         // convert to timeslices
      portEXIT_CRITICAL(&displaytimeMux);
      break;
    case 36: // myFP2 set display writing state, 0 = write not allowed, 1 = write text allowed
      // :360#    None    Blank the Display
      // :361#    None    UnBlank the Display
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      (paramvalue == 1) ? display_on() : display_off();
      break;
    case 37: // myFP2 get display status (1=Running or 0=Stopped)
      build_reply('D', display_status, clientnum);
      break;
    case 38: // myFP2 get temperature mode 1=Celsius, 0=Fahrenheight
      build_reply('b', ControllerData->get_tempmode(), clientnum);
      break;
    case 39: // myFP2 get the new motor position (target) XXXXXX
      build_reply('N', ftargetPosition, clientnum);
      break;
    case 40: // myFP2 reboot controller with 2s delay
      reboot_esp32(2000);
      break;
    case 41: // myFP2ESP32 set in-out-led-mode (pulsed or move)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte)WorkString.toInt() & 0x01;
      ControllerData->set_inoutledmode((byte) paramvalue);
      break;
    case 42: // myFP2 reset focuser defaults
      if ( isMoving == 0 )
      {
        ControllerData->SetFocuserDefaults();
        ftargetPosition = ControllerData->get_fposition();
        driverboard->setposition(ftargetPosition);
        ControllerData->set_fposition(ftargetPosition);
      }
      break;
    case 43: // myFP2 get motorspeed
      build_reply('C', ControllerData->get_motorspeed(), clientnum);
      break;
    case 44: // myFP2ESP32 get park enable state
      build_reply( '$', ControllerData->get_park_enable(), clientnum);
      break;
    case 45: // myFP2ESP32 set park enable state
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = WorkString.toInt() & 0x01;
      ControllerData->set_park_enable((byte) paramvalue);
      break;
    case 46: // myFP2ESP32 get in-out led enable state
      build_reply( '$', ControllerData->get_inoutled_enable(), clientnum);
      break;
    case 47: // myFP2ESP32 set in-out led enable state
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = WorkString.toInt() & 0x01;
      ControllerData->set_inoutled_enable((byte) paramvalue);
      break;
    case 48: // save settings to file
      // do not do this if focuser is moving
      if ( isMoving == false)
      {
        // need to save position setting
        ControllerData->set_fposition(driverboard->getposition());
        // save the focuser settings immediately
        ControllerData->SaveNow(driverboard->getposition(), driverboard->getdirection());
      }
      break;
    case 49: // aXXXXX
      build_reply('a', "b552efd", clientnum);
      break;
    case 50: // myFP2 get if Home Position Switch enabled, 0 = no, 1 = yes
      build_reply('l', ControllerData->get_hpswitch_enable(), clientnum);
      break;
    case 51: // myFP2ESP32 get Wifi Controller IP Address
      build_reply('$', ipStr, clientnum);
      break;
    case 52: // myFP2ESP32 get park state
      if( this->_parked )
        build_reply('$', 1, clientnum);
      else
        build_reply('$', 0, clientnum);
      break;
    case 54: // myFP2ESP32 ESP32 Controller SSID
      build_reply('$', mySSID, clientnum);
      break;
    case 55: // myFP2 get motorspeed delay for current speed setting
      build_reply('0', ControllerData->get_brdmsdelay(), clientnum);
      break;
    case 56: // myFP2 set motorspeed delay for current speed setting
      {
        int newdelay = 1000;
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        newdelay = WorkString.toInt();
        newdelay = (newdelay < 1000) ? 1000 : newdelay;   // ensure it is not too low
        ControllerData->set_brdmsdelay(newdelay);
      }
      break;
    case 57: // myFP2ESP32 get pushbutton enable state
      build_reply( '$', driverboard->get_pushbuttons_loaded(), clientnum);
      break;
    case 58: // myFP2ESP32 set pushbutton enable state
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte)WorkString.toInt() & 0x01;
      driverboard->set_pushbuttons(paramvalue);
      break;
    case 59: // myFP2ESP32 get park time
      build_reply('$', ControllerData->get_parktime(), clientnum);
      break;
    case 60: // myFP2ESP32 set park time interval in seconds
      {
        // range check 30s to 300s (5m)
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramvalue = WorkString.toInt();
        paramvalue = (paramvalue < 30) ? 30 : paramvalue;
        paramvalue = (paramvalue > 300 ) ? 300 : paramvalue;
        ControllerData->set_parktime(paramvalue);
        // update park_maxcount
        portENTER_CRITICAL(&parkMux);
        park_maxcount = paramvalue * 10;                         // convert to timeslices
        portEXIT_CRITICAL(&parkMux);
      }
      break;
    case 61: // myFP2 set update of position on oled when moving (0=disable, 1=enable)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte)WorkString.toInt() & 0x01;
      ControllerData->set_displayupdateonmove((byte) paramvalue);
      break;
    case 62: // myFP2 get update of position on oled when moving (00=disable, 01=enable)
      build_reply('L', ControllerData->get_displayupdateonmove(), clientnum);
      break;
    case 63: // myFP2 get status of home position switch
      if ( ControllerData->get_hpswitch_enable() == V_RUNNING)  // if the hpsw is enabled
      {
        // myFP2 get state of hpsw, return 1 if closed, 0 if open
        // myFP2ESP32  (hpsw pin 1=open, 0=closed)
        // if( driverboard->hpsw_alert() == true )
        build_reply('H', driverboard->hpsw_alert(), clientnum);     
      }
      else
      {
        build_reply('H', 0, clientnum);
      }
      break;
    case 64: // myFP2 move a specified number of steps
      if ( isMoving == 0 )
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        long pos = WorkString.toInt() + driverboard->getposition();
        pos  = (pos < 0) ? 0 : pos;
        ftargetPosition = ( pos > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : pos;
      }
      break;
    case 65: // myFP2 set jogging state enable/disable
      WorkString = receiveString.substring(2, receiveString.length() );
      joggingstate = (byte) WorkString.toInt();
      break;
    case 66: // myFP2 get jogging state enabled/disabled
      build_reply('K', joggingstate, clientnum);
      break;
    case 67: // myfp2 set jogging direction, 0=IN, 1=OUT
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      joggingdirection = (byte)WorkString.toInt() & 0x01;
      break;
    case 68: // myfp2 get jogging direction, 0=IN, 1=OUT
      build_reply('V', joggingdirection, clientnum);
      break;
    case 69: // myfp2 get push button steps
      build_reply('?', ControllerData->get_pushbutton_steps(), clientnum);
      break;
    case 70: // myFP2 set push buttons steps [1-max] where max = stepsize / 2
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramvalue = WorkString.toInt();
        paramvalue = (paramvalue < 1) ?  1 : paramvalue;
        // myFP2 set maximum steps to be 1/2 the step size
        int sz = (int) ControllerData->get_stepsize() / 2;
        sz = (sz < 1) ? 1 : sz;
        paramvalue = (paramvalue > sz) ? sz : paramvalue;
        ControllerData->set_pushbutton_steps((byte) paramvalue);
      }
      break;
    case 71: // myFP2 set delayaftermove time value in milliseconds [0-250]
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = WorkString.toInt();
      paramvalue = (paramvalue < 0  ) ?   0 : paramvalue;
      paramvalue = (paramvalue > 250) ? 250 : paramvalue;
      ControllerData->set_delayaftermove_time((byte) paramvalue);
      break;
    case 72: // myFP2 get delayaftermove_state value in milliseconds
      build_reply('3', ControllerData->get_delayaftermove_time(), clientnum);
      break;
    case 73: // myFP2 set disable/enable backlash IN (going to lower focuser position)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      ControllerData->set_backlash_in_enable((byte) paramvalue);
      break;
    case 74: // myFP2 get backlash in enabled status
      build_reply('4', ControllerData->get_backlash_in_enable(), clientnum);
      break;
    case 75: // myFP2 set disable/enable backlash OUT (going to lower focuser position)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      ControllerData->set_backlash_in_enable((byte) paramvalue);
      break;
    case 76: // myFP2 get backlash OUT enabled status
      build_reply('5', ControllerData->get_backlash_out_enable(), clientnum);
      break;
    case 77: // myFP2 set backlash in steps [0-255]
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0xff;
      ControllerData->set_backlashsteps_in((byte) paramvalue);
      break;
    case 78: // myFP2 get backlash steps IN
      build_reply('6', ControllerData->get_backlashsteps_in(), clientnum);
      break;
    case 79: // myFP2 set backlash OUT steps
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0xff;
      ControllerData->set_backlashsteps_out((byte) paramvalue );
      break;
    case 80: // myFP2 get backlash steps OUT
      build_reply('7', ControllerData->get_backlashsteps_out(), clientnum);
      break;
    case 81: // myFP2 get STALL_VALUE (for TMC2209 stepper modules)
      build_reply('8', ControllerData->get_stallguard_value(), clientnum);
      break;
    case 82: // myFP2ESP32 set STALL_VALUE (for TMC2209 stepper modules)
      WorkString = receiveString.substring(2, receiveString.length() );
      driverboard->setstallguardvalue( (byte) WorkString.toInt() );
      break;
    case 83: // myFP2 get if there is a temperature probe
      if(  tempprobe->get_found() == false )
        build_reply('c', 0, clientnum);
      else
        build_reply('c', 1, clientnum);
      break;

    case 84: // myFP2N reserved
      break;

    case 85: // myFP2ESP32 get delay after move enable state
      build_reply('$', ControllerData->get_delayaftermove_enable(), clientnum);
      break;
    case 86: // myFP2ESP32 set delay after move enable state
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = WorkString.toInt() & 0x01;
      ControllerData->set_delayaftermove_enable((byte) paramvalue);
      break;
    case 87: // myFP2 get tc direction
      build_reply('k', ControllerData->get_tcdirection(), clientnum);
      break;
    case 88: // myFP2 set tc direction
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte)((WorkString.toInt()) & 0x01);
      ControllerData->set_tcdirection((byte) paramvalue);
      break;
    case 89: // myFP2 get stepper power (reads from A7) - only valid if hardware circuit is added (1=stepperpower ON)
      build_reply('9', 1, clientnum);
      break;
    case 90: // myFP2ESP32 set preset x [0-9] with position value yyyy [unsigned long]
      {
        byte preset = (byte) (receiveString[3] - '0');
        preset = (preset > 9) ? 9 : preset;
        WorkString = receiveString.substring(4, receiveString.length() - 1);
        long tmppos = WorkString.toInt();
        tmppos = (tmppos < 0) ? 0 : tmppos;
        tmppos = (tmppos > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tmppos;
        ControllerData->set_focuserpreset( preset, tmppos );
        // update cached copy
        _presets[preset] = tmppos;
      }
      break;
    case 91: // myFP2ESP32 get focuserpreset [0-9]
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        byte preset = (byte) WorkString.toInt();
        build_reply('$', _presets[preset], clientnum);
      }
      break;
    case 92: // myFP2 set display page display option (8 digits, index of 0-7)
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        // If empty (no args) - fill with default display string
        if ( WorkString == "" )
        {
          WorkString = "11111111";
        }

        // if display option length less than 8, pad with leading 0's
        if ( WorkString.length() < 8 )
        {
          while ( WorkString.length() < 8 )
          {
            WorkString = '0' + WorkString;
          }
        }

        // do not allow display strings that exceed length of buffer (0-7, 8 digits)
        if ( WorkString.length() > 8 )
        {
          WorkString[8] = 0x00;
        }
        ControllerData->set_displaypageoption(WorkString);
      }
      break;
    case 93: // myFP2 get display page option
      {
        // return as string of 01's
        char buff[10];
        memset(buff, 0, 10);
        String answer = ControllerData->get_displaypageoption();
        // should always be 8 digits (0-7) due to set command (:92)
        // copy to buff
        int i;
        for ( i = 0; i < answer.length(); i++ )
        {
          buff[i] = answer[i];
        }
        buff[i] = 0x00;
        build_reply('l', buff, clientnum);
      }
      break;
    case 94: // myfp2 - set DelayedDisplayUpdate (0=disabled, 1-enabled)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      delayeddisplayupdatestatus = (byte) WorkString.toInt();
      break;
    case 95: // myfp2 - get DelayedDisplayUpdate (0=disabled, 1-enabled)
      build_reply('n', delayeddisplayupdatestatus, clientnum);
      break;

    case 96: // not used
      break;

    case 97: // not used
      break;

    case 98: // myFP2ESP32 get network strength dbm
      {
        long rssi = getrssi();
        build_reply('$', rssi, clientnum);
      }
      break;
    case 99:  // myFP2ESP32 set home positon switch enable state, 0 or 1, disabled or enabled
      {
        WorkString = receiveString.substring(3, receiveString.length() - 1);
        paramvalue = WorkString.toInt() & 0x01;
        if ( ControllerData->get_brdhpswpin() == -1)
        {
          ERROR_println("tcp: hpswpin not supported on this board");
        }
        else
        {
          ControllerData->set_hpswitch_enable((byte) paramvalue);
          if ( paramvalue == 1 )
          {
            TCPSRVR_println("tcp: hpsw state: enabled");
            if ( driverboard->init_hpsw() == true)
            {
              TCPSRVR_println("tcp: hpsw init OK");
            }
            else
            {
              ERROR_println("tcp: hpsw init ERROR");
            }
          }
          else
          {
            TCPSRVR_println("tcp: hpsw state: disabled");
          }
        }
      }
      break;

    // :A0-A9
    case 100: // myFP2ESP32 get joystick1 enable state
      build_reply( '$', driverboard->get_joystick1_loaded(), clientnum);
      break;
    case 101: // myFP2ESP32 set joystick1 enable state (0=stopped, 1=started)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte)WorkString.toInt() & 0x01;
      driverboard->set_joystick1(paramvalue);
      break;
    case 102: // myFP2ESP32 get joystick2 enable state
      build_reply( '$', driverboard->get_joystick2_loaded(), clientnum);
      break;
    case 103: // myFP2ESP32 set joystick2 enable state (0=stopped, 1=started)
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte)WorkString.toInt() & 0x01;
      driverboard->set_joystick2(paramvalue);
      break;
    case 104: // myFP2ESP32 get temp probe enabled state
      build_reply( '$', ControllerData->get_tempprobe_enable(), clientnum );
      break;
    case 105: // myFP2ESP32 set temp probe enabled state
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      ControllerData->set_tempprobe_enable(paramvalue);
      break;
    case 106: // myFP2ESP32 get ASCOM ALPACA Server enabled state
      build_reply( '$', ControllerData->get_ascomsrvr_enable(), clientnum );
      break;
    case 107: // myFP2ESP32 set ASCOM ALPACA Server enabled state
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      if ( paramvalue == 1 )
      {
        if ( ControllerData->get_ascomsrvr_enable() != V_ENABLED)
        {
          // status cannot be running if server is not enabled
          // enable the server
          ControllerData->set_ascomsrvr_enable(V_ENABLED);
        }
      }
      else
      {
        // stop and disable
        if ( ascomsrvr_status == V_RUNNING )
        {
          ascomsrvr->stop();
          ascomsrvr_status = V_STOPPED;
        }
        ControllerData->set_ascomsrvr_enable(V_NOTENABLED);
      }
      break;
    case 108: // myFP2ESP32 get ASCOM ALPACA Server Start/Stop status
      build_reply( '$', ascomsrvr_status, clientnum );
      break;
    case 109: // myFP2ESP32 set ASCOM ALPACA Server Start/Stop - this will start or stop the ASCOM server
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      if ( paramvalue == 1 )
      {
        // start if enabled
        if ( ControllerData->get_ascomsrvr_enable() == V_ENABLED )
        {
          if ( ascomsrvr_status == V_STOPPED )
          {
            ascomsrvr_status = ascomsrvr->start();
            if ( ascomsrvr_status != V_RUNNING )
            {
              ERROR_println("ASCOM ALPACA Server start error");
            }
          }
        }
      }
      else
      {
        // stop
        if ( ascomsrvr_status == V_RUNNING )
        {
          ascomsrvr->stop();
          ascomsrvr_status = V_STOPPED;
        }
      }
      break;

    // :B0 to :B9
    case 110: // myFP2ESP32 get Web Server enabled state
      build_reply( '$', ControllerData->get_ascomsrvr_enable(), clientnum );
      break;
    case 111: // myFP2ESP32 set Web Server enabled state
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      if ( paramvalue == 1 )
      {
        // enable
        if ( ControllerData->get_websrvr_enable() != V_ENABLED)
        {
          // status cannot be running if server is not enabled
          // enable the server
          ControllerData->set_websrvr_enable(V_ENABLED);
        }
      }
      else
      {
        // stop and disable
        if ( websrvr_status == V_RUNNING )
        {
          websrvr->stop();
          websrvr_status = V_STOPPED;
        }
        ControllerData->set_websrvr_enable(V_NOTENABLED);
      }
      break;
    case 112: // myFP2ESP32 get Web Server Start/Stop status
      build_reply( '$', ascomsrvr_status, clientnum );
      break;
    case 113: // myFP2ESP32 set Web Server Start/Stop - this will start or stop the ASCOM server
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      if ( paramvalue == 1 )
      {
        // start if enabled
        if ( ControllerData->get_websrvr_enable() == V_ENABLED )
        {
          // enabled
          if ( websrvr_status == V_STOPPED )
          {
            websrvr_status = websrvr->start(ControllerData->get_websrvr_port());
            if ( websrvr_status != V_RUNNING )
            {
              ERROR_println("web server: start error");
            }
          }
        }
      }
      else
      {
        // stop
        if ( websrvr_status == V_RUNNING )
        {
          websrvr->stop();
          websrvr_status = V_STOPPED;
        }
      }
      break;

    case 114: // myFP2ESP32 get Management Server enabled state
      build_reply( '$', ControllerData->get_mngsrvr_enable(), clientnum );
      break;
    case 115: // myFP2ESP32 set Management Server enabled state
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      if ( paramvalue == 1 )
      {
        // enable
        if ( ControllerData->get_mngsrvr_enable() != V_ENABLED)
        {
          // status cannot be running if server is not enabled
          // enable the server
          ControllerData->set_mngsrvr_enable(V_ENABLED);
        }
      }
      else
      {
        // stop and disable
        if ( mngsrvr_status == V_RUNNING )
        {
          mngsrvr->stop();
          mngsrvr_status = V_STOPPED;
        }
        ControllerData->set_mngsrvr_enable(V_NOTENABLED);
      }
      break;
    case 116: // myFP2ESP32 get Management Server Start/Stop status
      build_reply( '$', mngsrvr_status, clientnum );
      break;
    case 117: // myFP2ESP32 set Management Server Start/Stop - this will start or stop the Management server
      WorkString = receiveString.substring(3, receiveString.length() - 1);
      paramvalue = (byte) WorkString.toInt() & 0x01;
      if ( paramvalue == 1 )
      {
        // start if enabled
        if ( ControllerData->get_mngsrvr_enable() == V_ENABLED )
        {
          // enabled
          if ( mngsrvr_status == V_STOPPED )
          {
            mngsrvr_status = mngsrvr->start(ControllerData->get_mngsrvr_port());
            if ( mngsrvr_status != V_RUNNING )
            {
              ERROR_println("start management server: start error");
            }
          }
        }
        else
        {
          ERROR_println("start management server: not enabled");
        }
      }
      else
      {
        // stop
        if ( mngsrvr_status == V_RUNNING )
        {
          mngsrvr->stop();
          mngsrvr_status = V_STOPPED;
        }
      }
      break;

    case 118: // myFP2ESP32 get cntlr_config.jsn
      {
        if ( SPIFFS.exists("/cntlr_config.jsn") == false )
        {
          send_reply("tcp: cntlr_config.jsn not found", clientnum);
        }
        else
        {
          // file exists so open it
          File dfile = SPIFFS.open("/cntlr_config.jsn", "r");
          if (!dfile)
          {
            send_reply("tcp: B8: cntlr_config.jsn not found", clientnum);
          }
          else
          {
            String cdata = dfile.readString();
            dfile.close();
            TCPSRVR_print("tcp: B8: cntlr_config = ");
            TCPSRVR_println(cdata);
            int len = cdata.length();
            char cd[len + 2];
            snprintf(cd, len + 2, "%c%s%c", '$', cdata, _EOFSTR);
            send_reply(cd, clientnum);
          }
        }
      }
      send_reply("tcp: B8: error", clientnum);
      break;

    case 119: // myFP2ESP32 get coil power state :B9#
      //deprecated
      break;
    case 120: // myFP2ESP32 set coil power state :C0# (change only the coilpowestate) :C0x#
      // deprecated
      break;

    default:
      TCPSRVR_print("tcp: invalid command: ");
      TCPSRVR_println(receiveString);
      break;
  }
}
