// ----------------------------------------------------------------------
// myFP2ESP32 WEB SERVER CLASS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// web_server.cpp
// Default Configuration
// Index, Move and Preset forms
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------
#include <Arduino.h>
#include "controller_defines.h"
#include "controller_config.h"                          // includes boarddefs.h and controller_defines.h

#include <WiFi.h>
#include "SPIFFS.h"
#include <SPI.h>
#include <WebServer.h>


// -----------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------
// DO NOT ENABLE DEBUGGING INFORMATION.

// Remove comment to enable messages to Serial port
//#define WEBSRVR_PRINT       1

// -----------------------------------------------------------------------
// DO NOT CHANGE
// -----------------------------------------------------------------------
#ifdef  WEBSRVR_PRINT
#define WEBSRVR_print(...)   Serial.print(__VA_ARGS__)
#define WEBSRVR_println(...) Serial.println(__VA_ARGS__)
#else
#define WEBSRVR_print(...)
#define WEBSRVR_println(...)
#endif


// -----------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------
#include "web_server.h"
extern WEB_SERVER *websrvr;

// ControllerData
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

// Driver board
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;

// temp probe
#include "temp_probe.h"
extern TEMP_PROBE *tempprobe;


// ----------------------------------------------------------------------
// EXTERNS
// ----------------------------------------------------------------------
extern char ipStr[];
extern char mySSID[];
extern char systemuptime[12];
extern long ftargetPosition;                    // target position
extern bool isMoving;                           // is the motor currently moving
extern bool filesystemloaded;                   // flag indicator for _webserver usage, rather than use SPIFFS.begin() test

extern float temp;

extern volatile bool halt_alert;
extern portMUX_TYPE halt_alertMux;

// display is optional so use helpers to access display if required
extern bool display_on(void);
extern bool display_off(void);

extern void get_systemuptime(void);

//cached vars
// cached vars
extern char devicename[32];
extern char titlecolor[8];
extern char subtitlecolor[8];
extern char headercolor[8];
extern char textcolor[8];
extern char backcolor[8];


// ----------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------
#define H_NOTFOUND  "<html><head><title>Web Server</title></head><body><p>URL not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>"
#define T_INDEX     "index.html "
#define T_MOVE      "move.html "
#define T_PRESETS   "presets.html "
#define T_NOTFOUND  "notfound.html "

// index HTML page
// position
#define H_FPSTART "<form action=\"/\" method =\"post\">"
#define H_PINPUT  "<input type=\"text\" style=\"height: 1.6em; width: 4.5em\" name=\"pos\" size=\"6\" value=\"\">"
#define H_BPSET   "<input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" name=\"setpos\" value=\"Set\">"
#define H_BPGO    "<input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" name=\"gotopos\" value=\"Goto\">"
#define H_FPEND   "</form>"
// maxsteps
#define H_MAXFS   "<form action=\"/\" method=\"post\">" // %MAXFS%
#define H_MAXVAL  "<input type=\"text\" style=\"height: 1.6em; width: 4.5em\" name=\"max\" size=\"6\" value=\"%mnum%\">" // %MAXVAL%
#define H_BMAXFS  "<input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" name=\"setmax\" value=\"Set\"></form>"  // %BMAXFS%
// Halt Button
#define H_HALTBUTTON      "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"ha\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Halt\"></form>"
// Temp Unit C or F
#define H_TEMPCELSIUS     "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"tem\" value=\"ce\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"C\"></form>"
#define H_TEMPFAHRENHEIT  "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"tem\" value=\"fa\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"F\"></form>"
// Temp Resolution
#define H_TEMPRESOLUTION  "<form action=\"/\" method=\"post\"><input type=\"text\" style=\"height: 1.6em; width: 4.5em\" name=\"tr\" size =\"6\" value=\"%trnum%\">"
#define H_TEMPRESBTN      "<input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Set\"></form>"
// Stepmode html to create radio buttons on web page
#define H_SM_FORM         "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"ssm\" value=\"true\">"
#define H_SM_FORMBUTTON   "<input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Set\"></form>"
#define H_SM1CHECKED      "<input type=\"radio\" name=\"sm\" value=\"1\" Checked> 1"
#define H_SM1UNCHECKED    "<input type=\"radio\" name=\"sm\" value=\"1\"> 1"
#define H_SM2CHECKED      "<input type=\"radio\" name=\"sm\" value=\"2\" Checked> 2"
#define H_SM2UNCHECKED    "<input type=\"radio\" name=\"sm\" value=\"2\"> 2"
#define H_SM4CHECKED      "<input type=\"radio\" name=\"sm\" value=\"4\" Checked> 4"
#define H_SM4UNCHECKED    "<input type=\"radio\" name=\"sm\" value=\"4\"> 4"
#define H_SM8CHECKED      "<input type=\"radio\" name=\"sm\" value=\"8\" Checked> 8"
#define H_SM8UNCHECKED    "<input type=\"radio\" name=\"sm\" value=\"8\"> 8"
#define H_SM16CHECKED     "<input type=\"radio\" name=\"sm\" value=\"16\" Checked> 16"
#define H_SM16UNCHECKED   "<input type=\"radio\" name=\"sm\" value=\"16\"> 16"
#define H_SM32CHECKED     "<input type=\"radio\" name=\"sm\" value=\"32\" Checked> 32"
#define H_SM32UNCHECKED   "<input type=\"radio\" name=\"sm\" value=\"32\"> 32"
// Motor speed html to create radio buttons on web page
#define H_MS_FORM         "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"msd\" value=\"true\">"
#define H_MS_FORM_BUTTON  "<input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Set\"> </form>"
#define H_MSSLOWCHECKED   "<input type=\"radio\" name=\"ms\" value=\"0\" Checked> S"
#define H_MSSLOWUNCHECKED "<input type=\"radio\" name=\"ms\" value=\"0\"> S"
#define H_MSMEDCHECKED    "<input type=\"radio\" name=\"ms\" value=\"1\" Checked> M"
#define H_MSMEDUNCHECKED  "<input type=\"radio\" name=\"ms\" value=\"1\"> M"
#define H_MSFASTCHECKED   "<input type=\"radio\" name=\"ms\" value=\"2\" Checked> F"
#define H_MSFASTUNCHECKED "<input type=\"radio\" name=\"ms\" value=\"2\"> F"
// Coil Power
#define H_CPENABLE  "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"cpr\" value=\"on\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Enable\"> </form>"
#define H_CPDISABLE "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"cpr\" value=\"off\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Disable\"> </form>"
// Reverse Direction
#define H_RDENABLE    "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"rd\" value=\"on\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Enable\"> </form>"
#define H_RDDISABLE   "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"rd\" value=\"off\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Disable\"> </form>"
// Park State %PTS% Enable/Disable %BPTS%
#define H_ENABLEPARK  "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"park\" value=\"on\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Enable\"></form>"
#define H_DISABLEPARK "<form action=\"/\" method=\"post\"><input type=\"hidden\" name=\"park\" value=\"off\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Disable\"></form>"

// Move HTML page
#define H_MOVL500 "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"mvl500\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 3.5em\" value=\"-500\"></form>"
#define H_MOVL100 "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"mvl100\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 3.5em\" value=\"-100\"></form>"
#define H_MOVL10  "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"mvl10\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 3.5em\" value=\"-10\"></form>"
#define H_MOVL1   "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"mvl1\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 3.5em\" value=\"-1\"></form>"
#define H_MOVP1   "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"mvp1\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 3.5em\" value=\"1\"></form>"
#define H_MOVP10  "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"mvp10\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 3.5em\" value=\"10\"></form>"
#define H_MOVP100 "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"mvp100\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 3.5em\" value=\"100\"></form>"
#define H_MOVP500 "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"mvp500\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 3.5em\" value=\"500\"></form>"
#define H_HALTBTNMOVE  "<form action=\"/move\" method=\"post\"><input type=\"hidden\" name=\"ha\" value=\"true\"><input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"Halt\"></form>"
#define H_MPI  "<form action=\"/move\" method=\"post\"><input type=\"text\" name=\"pos\" size=\"10\" value=\"\">"
#define H_MBP  "<input type=\"submit\" style=\"height: 1.6em; width: 4.5em\" value=\"GOTO\"></form>"

// Presets HTML page
#define P0F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p0\" size=\"10\" value=\"%p0num%\">"
#define P1F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p1\" size=\"10\" value=\"%p1num%\">"
#define P2F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p2\" size=\"10\" value=\"%p2num%\">"
#define P3F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p3\" size=\"10\" value=\"%p3num%\">"
#define P4F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p4\" size=\"10\" value=\"%p4num%\">"
#define P5F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p5\" size=\"10\" value=\"%p5num%\">"
#define P6F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p6\" size=\"10\" value=\"%p6num%\">"
#define P7F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p7\" size=\"10\" value=\"%p7num%\">"
#define P8F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p8\" size=\"10\" value=\"%p8num%\">"
#define P9F   "<form action=\"/presets\" method=\"post\"><input type=\"text\" name=\"p9\" size=\"10\" value=\"%p9num%\">"
#define GeT   true
#define PosT  false
bool index_type;
bool move_type;
bool presets_type;


// ----------------------------------------------------------------------
// Web Page Handlers
// ----------------------------------------------------------------------
void wsget_index(void)
{
  index_type = GeT;
  websrvr->get_index();
}

void wspost_index(void)
{
  index_type = PosT;
  websrvr->get_index();
}

void wsget_move(void)
{
  move_type = GeT;
  websrvr->get_move();
}

void wspost_move(void)
{
  move_type = PosT;
  websrvr->get_move();

}

void wsget_presets(void)
{
  presets_type = GeT;
  websrvr->get_presets();
}

void wspost_presets(void)
{
  presets_type = PosT;
  websrvr->get_presets();
}

void wsget_notfound(void)
{
  websrvr->get_notfound();
}

void wsget_pages(void)
{
  websrvr->reload_webpages();
}

// ----------------------------------------------------------------------
// XHTML
// ----------------------------------------------------------------------
void wsget_position(void)
{
  websrvr->get_position();
}

void wsget_ismoving(void)
{
  websrvr->get_ismoving();
}

void wsget_targetposition(void)
{
  websrvr->get_targetposition();
}

void wsget_temperature(void)
{
  websrvr->get_temperature();
}

void wsget_park(void)
{
  websrvr->get_park();
}

void wsget_coilpower(void)
{
  websrvr->get_coilpower();
}


// ----------------------------------------------------------------------
// CLASS
// ----------------------------------------------------------------------
WEB_SERVER::WEB_SERVER()
{
  this->_loaded = false;
  this->_state = false;
}

// ----------------------------------------------------------------------
// bool start(unsigned long);
// Create and start the _webserver with the passed port number
// ----------------------------------------------------------------------
bool WEB_SERVER::start(unsigned long port)
{
  // if server is not enabled then return
  if ( ControllerData->get_websrvr_enable() == V_NOTENABLED)
  {
    WEBSRVR_println("ws: not enabled");
    return false;
  }

  // prevent any attempt to start if server is already started
  if ( this->_loaded == true )
  {
    return true;
  }

  this->_port = port;
  this->_indexpg.reserve(6000);
  this->_movepg.reserve(4500);
  this->_presetspg.reserve(6000);
  this->_notfoundpg.reserve(1200);

  // save port if not same as last used port
  if ( port != ControllerData->get_websrvr_port() )
  {
    ControllerData->set_websrvr_port(this->_port);
  }

  if ( this->_loaded == false )
  {
    // create the web server
    _web_server = new WebServer(this->_port);
  }
  // Web pages
  _web_server->on("/",        HTTP_GET,  wsget_index);
  _web_server->on("/",        HTTP_POST, wspost_index);
  _web_server->on("/move",    HTTP_GET,  wsget_move);
  _web_server->on("/move",    HTTP_POST, wspost_move);
  _web_server->on("/presets", HTTP_GET,  wsget_presets);
  _web_server->on("/presets", HTTP_POST, wspost_presets);
  _web_server->on("/load",    HTTP_GET,  wsget_pages);
  _web_server->on("/load",    HTTP_POST, wsget_pages);

  // XHTML
  _web_server->on("/po", wsget_position);
  _web_server->on("/im", wsget_ismoving);
  _web_server->on("/ta", wsget_targetposition);
  _web_server->on("/tm", wsget_temperature);
  _web_server->on("/pa", wsget_park);
  _web_server->on("/cp", wsget_coilpower);

  _web_server->onNotFound([]()
  {
    wsget_notfound();
  });

  if ( loadpages() == false )
  {
    // error could not load pages
    // loading pages should come before _web_serv->begin()
    ERROR_println("ws: loadpages(): error");
    stop();
    return false;
  }
  _web_server->begin();
  this->_loaded = true;
  this->_state = true;
  return true;
}

// ----------------------------------------------------------------------
// void stop(void);
// Stop the web server
// This will stop and delete the _webserver
// This must be done because start() creates _webserver
// ----------------------------------------------------------------------
void WEB_SERVER::stop(void)
{
  if ( this->_loaded == true )
  {
    _web_server->stop();
    delete _web_server;
    this->_loaded = false;
    this->_state = V_STOPPED;
  }
  else
  {
    if ( this->_state == V_RUNNING )
    {
      _web_server->stop();
    }
    delete _web_server;
    this->_loaded = false;
    this->_state = V_STOPPED;
  }
}

// ----------------------------------------------------------------------
// void loop(bool);
// Checks for any new clients or existing client requests
// ----------------------------------------------------------------------
void WEB_SERVER::loop(bool parked)
{
  // avoid a crash
  if ( this->_loaded == false )
  {
    return;
  }
  _parked = parked;
  _web_server->handleClient();
}

// ----------------------------------------------------------------------
// bool loadpages(void);
// Load webserver pages into memory
// ----------------------------------------------------------------------
bool WEB_SERVER::loadpages(void)
{
  if ( !filesystemloaded )
  {
    ERROR_println("ws: loadpages: FS error");
    file_sys_error();
    return false;
  }

  if ( SPIFFS.exists("/index.html"))
  {
    File file = SPIFFS.open("/index.html", "r");
    this->_indexpg = file.readString() + "";
    file.close();
  }
  else
  {
    ERROR_println("ws: Unable to load index.html");
    this->_indexpg = H_FILENOTFOUNDSTR;
    return false;
  }

  if ( SPIFFS.exists("/move.html"))
  {
    File file = SPIFFS.open("/move.html", "r");
    this->_movepg = file.readString() + "";
    file.close();
  }
  else
  {
    ERROR_println("ws: Unable to load move.html");
    this->_movepg = H_FILENOTFOUNDSTR;
    return false;
  }

  if ( SPIFFS.exists("/presets.html"))
  {
    File file = SPIFFS.open("/presets.html", "r");
    this->_presetspg = file.readString() + "";
    file.close();
  }
  else
  {
    ERROR_println("ws: Unable to load presets.html");
    this->_presetspg = H_FILENOTFOUNDSTR;
    return false;
  }

  if ( SPIFFS.exists("/notfound.html"))
  {
    File file = SPIFFS.open("/notfound.html", "r");
    this->_notfoundpg = file.readString() + "";
    file.close();
  }
  else
  {
    ERROR_println("ws: Unable to load notfound.html");
    this->_notfoundpg = H_FILENOTFOUNDSTR;
    return false;
  }
  // all pages loaded
  return true;
}


// ----------------------------------------------------------------------
// File System Not Loaded
// ----------------------------------------------------------------------
void WEB_SERVER::file_sys_error(void)
{
  // SPIFFS file system not loaded
  ERROR_println("ws: file system error");
  this->send_myheader();
  this->send_mycontent(H_FSNOTLOADEDSTR);
}

// ----------------------------------------------------------------------
// Send a redirect pg to client
// ----------------------------------------------------------------------
void WEB_SERVER::send_redirect(String pg)
{
  _web_server->sendHeader("Location", pg);
  _web_server->send(301);
}

// ----------------------------------------------------------------------
// handler for /index
// ----------------------------------------------------------------------
void WEB_SERVER::get_index(void)
{
  String WSpg;
  String tmp;

  WSpg.reserve(6400);                           // 300: 6159

  if ( this->_loaded == false )
  {
    ERROR_println("Unable to load web pages");
    return;
  }

  if ( index_type == PosT )
  {
    // if set focuser position
    tmp = _web_server->arg("setpos");
    if ( tmp != "" )
    {
      // get new position value from text field pos
      // set new position only: this is NOT a move
      String fp = _web_server->arg("pos");
      if ( fp != "" )
      {
        long tp = 0;
        long maxp = ControllerData->get_maxstep();
        tp = fp.toInt();
        // range check the new position
        tp = (tp < 0) ? 0 : tp;
        ftargetPosition = ( tp > maxp) ? maxp : tp;
        driverboard->setposition(ftargetPosition);
        ControllerData->set_fposition(ftargetPosition);
      }
    }

    // if goto focuser position
    tmp = _web_server->arg("gotopos");
    if ( tmp != "" )
    {
      String fp = _web_server->arg("pos");
      if ( fp != "" )
      {
        long targp = 0;
        long maxp = ControllerData->get_maxstep();
        targp = fp.toInt();
        targp = (targp < 0)    ? 0        : targp;
        targp = (targp > maxp) ? maxp - 1 : targp;
        ftargetPosition = targp;
      }
    }

    // if update of maxsteps
    tmp = _web_server->arg("setmax");
    if ( tmp != "" )
    {
      String newmaxpos = _web_server->arg("max");
      if ( newmaxpos != "" )
      {
        long tp = 0;
        long cpos = driverboard->getposition();
        long maxp = ControllerData->get_maxstep();
        tp = newmaxpos.toInt();
        tp = (tp < cpos) ? cpos + 1 : tp;
        tp = (tp < FOCUSERLOWERLIMIT) ? FOCUSERLOWERLIMIT : tp;
        tp = (tp > maxp) ? maxp : tp;
        ControllerData->set_maxstep(tp);
      }
    }

    // if a HALT request
    tmp = _web_server->arg("ha");
    if ( tmp != "" )
    {
      portENTER_CRITICAL(&halt_alertMux);
      halt_alert = true;
      portEXIT_CRITICAL(&halt_alertMux);
    }

    // if update Temperature Unit C/F
    tmp = _web_server->arg("tem");
    if ( tmp != "" )
    {
      if ( tmp == "ce")
      {
        ControllerData->set_tempmode(V_CELSIUS);
      }
      else if ( tmp == "fa" )
      {
        ControllerData->set_tempmode(V_FAHRENHEIT);
      }
    }

    // if update temperature resolution
    tmp = _web_server->arg("tr");
    if ( tmp != "" )
    {
      int tr = tmp.toInt();
      tr = ( tr < 9 ) ? 9  : tr;
      tr = (tr > 12 ) ? 12 : tr;
      // tr = ( tr < 9 ) ? 9 : (tr > 12 ) ? 12 : tr;
      ControllerData->set_tempresolution(tr);
      tempprobe->set_resolution((byte) tr );           // set resolution
    }

    // if update coilpower
    tmp = _web_server->arg("cpr");
    if ( tmp != "" )
    {
      if ( tmp == "on" )
      {
        ControllerData->set_coilpower_enable(true);
        driverboard->enablemotor();
      }
      else if ( tmp == "off" )
      {
        ControllerData->set_coilpower_enable(false);
        driverboard->releasemotor();
      }
    }

    // if update motorspeed
    tmp = _web_server->arg("ms");
    if ( tmp != "" )
    {
      int mspd = 0;
      mspd = tmp.toInt();
      mspd = (mspd < SLOW) ? SLOW : mspd;
      mspd = (mspd > FAST) ? FAST : mspd;
      ControllerData->set_motorspeed(mspd);
    }

    // park enable on, off
    tmp = _web_server->arg("park");
    if ( tmp != "" )
    {
      if ( tmp == "on" )
      {
        ControllerData->set_park_enable(V_ENABLED);
      }
      else if ( tmp = "off")
      {
        ControllerData->set_park_enable(V_NOTENABLED);
      }
    }

    // if update reverse direction
    tmp = _web_server->arg("rd");
    if ( tmp != "" )
    {
      if ( tmp == "on" )
      {
        ControllerData->set_reverse_enable(V_ENABLED);
      }
      else if ( tmp == "off" )
      {
        ControllerData->set_reverse_enable(V_NOTENABLED);
      }
    }

    // Basic rule for setting stepmode
    // Set DRIVER_BOARD->setstepmode(xx);                         // this sets the physical pins
    // and this also saves ControllerData->set_brdstepmode(xx);   // this saves config setting

    // if update stepmode
    // (1=Full, 2=Half, 4=1/4, 8=1/8, 16=1/16, 32=1/32, 64=1/64, 128=1/128, 256=1/256)
    tmp = _web_server->arg("sm");
    if ( tmp != "" )
    {
      int stepm = tmp.toInt();
      int smtmp = ControllerData->get_brdmaxstepmode();
      stepm = (stepm < STEP1) ? STEP1 : stepm;
      stepm = (stepm > smtmp) ? smtmp   : stepm;
      driverboard->setstepmode(stepm);
      //send_redirect("/");
      //return;
    }
    // end of post
  }

  // make sure not to change the original template
  WSpg = this->_indexpg;

  WSpg.replace("%PGT%", devicename);
  // Web page colors
  WSpg.replace("%TIC%", titlecolor);
  WSpg.replace("%HEC%", headercolor);
  WSpg.replace("%TXC%", textcolor);
  WSpg.replace("%BKC%", backcolor);  

  // First cache the current position as it will be used multiple times
  String pos_c = String(driverboard->getposition());
  // Insert start of form
  WSpg.replace("%FPOS%", H_FPSTART);

  // Current Position
  WSpg.replace("%CPO%", pos_c);
  // Text field
  tmp = H_PINPUT;
  WSpg.replace("%POSI%", tmp);
  // Set position button
  WSpg.replace("%BPSET%",  H_BPSET);

  // Target is a special case, we need to fill this in,
  // When a user clicks goto, then the index page is displayed
  // again and we need to pick up the target position and use
  // XHTML to update both current and target positions on the page
  // Check if prior command was a GOTO command
  tmp = _web_server->arg("gotopos");
  if ( tmp != "" )
  {
    WSpg.replace("%TAR%", String(ftargetPosition));
  }
  else
  {
    WSpg.replace("%TAR%", String(driverboard->getposition()));
  }

  // Goto position button
  WSpg.replace("%BPGO%", H_BPGO);
  // Position form end
  WSpg.replace("%BFPOS%", H_FPEND);

  // max steps  value = %MAX%  inout field %MAXVAL% button %BMAXFS%
  // form start
  WSpg.replace("%MAXFS%", H_MAXFS);
  String str = String(ControllerData->get_maxstep());
  // [ maxstep value ]
  WSpg.replace("%MAX%", str);
  tmp = H_MAXVAL;
  // input text field
  tmp.replace("%mnum%", str);
  WSpg.replace("%MAXVAL%", tmp);
  // submit button
  WSpg.replace("%BMAXFS%", H_BMAXFS);

  // isMoving
  if ( isMoving == true )
  {
    WSpg.replace("%MOV%", "True");
  }
  else
  {
    WSpg.replace("%MOV%", "False");
  }

  // Halt button
  WSpg.replace("%BHA%", H_HALTBUTTON);

  // temperature mode, celsius or fahrenheit
  if ( ControllerData->get_tempmode() == V_CELSIUS)
  {
    String tpstr = String(temp, 2);
    WSpg.replace("%TEM%", tpstr);
    WSpg.replace("%TUN%", "C");
    WSpg.replace("%BTUN%", H_TEMPFAHRENHEIT);
  }
  else
  {
    float ft = temp;
    ft = (ft * 1.8) + 32;
    String tpstr = String(ft, 2);
    WSpg.replace("%TEM%", tpstr);
    WSpg.replace("%TUN%", "F");
    WSpg.replace("%BTUN%", H_TEMPCELSIUS);
  }

  // show temperature resolution, 9=0.5 %TPR%  %BTPR%
  String str1 = String(ControllerData->get_tempresolution());
  WSpg.replace("%TPR%", str1);
  tmp = H_TEMPRESOLUTION;
  tmp.replace("%trnum%", str1);
  WSpg.replace("%TRI%", tmp);
  WSpg.replace("%BTPR%", H_TEMPRESBTN);

  // coil power enabled %CPS%  button %BCPS%
  if ( ControllerData->get_coilpower_enable() == false )
  {
    // state = Off
    WSpg.replace("%CPS%",  T_DISABLED);
    WSpg.replace("%BCPS%", H_CPENABLE);
  }
  else
  {
    // state = On
    WSpg.replace("%CPS%",  T_ENABLED);
    WSpg.replace("%BCPS%", H_CPDISABLE);
  }

  // motorspeed
  String msbuffer = String(ControllerData->get_motorspeed());
  switch ( ControllerData->get_motorspeed() )
  {
    case 0:
      msbuffer = H_MSSLOWCHECKED;
      msbuffer = msbuffer + H_MSMEDUNCHECKED;
      msbuffer = msbuffer + H_MSFASTUNCHECKED;
      break;
    case 1:
      msbuffer = H_MSSLOWUNCHECKED;
      msbuffer = msbuffer + H_MSMEDCHECKED;
      msbuffer = msbuffer + H_MSFASTUNCHECKED;
      break;
    case 2:
      msbuffer = H_MSSLOWUNCHECKED;
      msbuffer = msbuffer + H_MSMEDUNCHECKED;
      msbuffer = msbuffer + H_MSFASTCHECKED;
      break;
    default:
      msbuffer = H_MSSLOWUNCHECKED;
      msbuffer = msbuffer + H_MSMEDUNCHECKED;
      msbuffer = msbuffer + H_MSFASTCHECKED;
      break;
  }
  WSpg.replace("%MSF%", H_MS_FORM );
  WSpg.replace("%MS%", msbuffer);
  WSpg.replace("%BMSF%", H_MS_FORM_BUTTON );

  // Park, State %PTS%, button %BPTS%
  if ( ControllerData->get_park_enable() == V_ENABLED)
  {
    WSpg.replace("%BPTS%", H_DISABLEPARK);       // button
    WSpg.replace("%PTS%", T_ENABLED);            // state
  }
  else
  {
    WSpg.replace("%BPTS%", H_ENABLEPARK);
    WSpg.replace("%PTS%", T_DISABLED);
  }

  // Park status %PAS%
  if ( this->_parked == false )
  {
    WSpg.replace("%PAS%", "Not parked");
  }
  else
  {
    WSpg.replace("%PAS%", "Parked");
  }

  // reverse direction  state %RDS%  button %BRDS%
  if ( ControllerData->get_reverse_enable() == 0 )
  {
    WSpg.replace("%BRDS%", H_RDENABLE);
    WSpg.replace("%RDS%", T_DISABLED);
  }
  else
  {
    WSpg.replace("%BRDS%", H_RDDISABLE);
    WSpg.replace("%RDS%", T_ENABLED);
  }

  // step mode
  String smbuffer;
  smbuffer.reserve(600);
  WSpg.replace("%SMF%", H_SM_FORM);
  switch ( ControllerData->get_brdstepmode() )
  {
    case 1:
      smbuffer = H_SM1CHECKED;
      smbuffer = smbuffer + H_SM2UNCHECKED;
      smbuffer = smbuffer + H_SM4UNCHECKED;
      smbuffer = smbuffer + H_SM8UNCHECKED;
      smbuffer = smbuffer + H_SM16UNCHECKED;
      smbuffer = smbuffer + H_SM32UNCHECKED;
      break;
    case 2 :
      smbuffer = H_SM1UNCHECKED;
      smbuffer = smbuffer + H_SM2CHECKED;
      smbuffer = smbuffer + H_SM4UNCHECKED;
      smbuffer = smbuffer + H_SM8UNCHECKED;
      smbuffer = smbuffer + H_SM16UNCHECKED;
      smbuffer = smbuffer + H_SM32UNCHECKED;
      break;
    case 4 :
      smbuffer = H_SM1UNCHECKED;
      smbuffer = smbuffer + H_SM2UNCHECKED;
      smbuffer = smbuffer + H_SM4CHECKED;
      smbuffer = smbuffer + H_SM8UNCHECKED;
      smbuffer = smbuffer + H_SM16UNCHECKED;
      smbuffer = smbuffer + H_SM32UNCHECKED;
      break;
    case 8 :
      smbuffer = H_SM1UNCHECKED;
      smbuffer = smbuffer + H_SM2UNCHECKED;
      smbuffer = smbuffer + H_SM4UNCHECKED;
      smbuffer = smbuffer + H_SM8CHECKED;
      smbuffer = smbuffer + H_SM16UNCHECKED;
      smbuffer = smbuffer + H_SM32UNCHECKED;
      break;
    case 16 :
      smbuffer = H_SM1UNCHECKED;
      smbuffer = smbuffer + H_SM2UNCHECKED;
      smbuffer = smbuffer + H_SM4UNCHECKED;
      smbuffer = smbuffer + H_SM8UNCHECKED;
      smbuffer = smbuffer + H_SM16CHECKED;
      smbuffer = smbuffer + H_SM32UNCHECKED;
      break;
    case 32 :
      smbuffer = H_SM1UNCHECKED;
      smbuffer = smbuffer + H_SM2UNCHECKED;
      smbuffer = smbuffer + H_SM4UNCHECKED;
      smbuffer = smbuffer + H_SM8UNCHECKED;
      smbuffer = smbuffer + H_SM16UNCHECKED;
      smbuffer = smbuffer + H_SM32CHECKED;
      break;
    default :
      smbuffer = H_SM1CHECKED;
      smbuffer = smbuffer + H_SM2UNCHECKED;
      smbuffer = smbuffer + H_SM4UNCHECKED;
      smbuffer = smbuffer + H_SM8UNCHECKED;
      smbuffer = smbuffer + H_SM16UNCHECKED;
      smbuffer = smbuffer + H_SM32UNCHECKED;
      break;
  }
  WSpg.replace("%SMB%", smbuffer);
  WSpg.replace("%BSMF%", H_SM_FORMBUTTON);

  // Board name
  WSpg.replace("%NAM%", ControllerData->get_brdname());
  // Firmware Version
  WSpg.replace("%VER%", String(program_version));
  // heap
  WSpg.replace("%HEA%", String(ESP.getFreeHeap()));
  // add system uptime
  get_systemuptime();
  WSpg.replace("%SUT%", systemuptime);

  WEBSRVR_print("/index ");
  WEBSRVR_println(WSpg.length());
  send_myheader();
  send_mycontent(WSpg);
}


// ----------------------------------------------------------------------
// handler for /move
// ----------------------------------------------------------------------
void WEB_SERVER::get_move(void)
{
  String WSpg;
  WSpg.reserve(4500);

  if ( this->_loaded == false )
  {
    ERROR_println("ws: pages not loaded");
    return;
  }

  if ( move_type == PosT )
  {
    // if a HALT request
    if ( _web_server->arg("ha") != "" )
    {
      portENTER_CRITICAL(&halt_alertMux);
      halt_alert = true;
      portEXIT_CRITICAL(&halt_alertMux);
      send_redirect("/move");
      return;
    }

    // check if focuser is moving, cannot move if focuser is alreadymoving
    if ( isMoving == false )
    {
      long pos = 0;

      // Check the move buttons
      String mv = _web_server->arg("mvl500");
      if ( _web_server->arg("mvl500") != "" )
      {
        pos = -500;
      }
      else if ( _web_server->arg("mvl100") != "" )
      {
        pos = -100;
      }
      else if ( _web_server->arg("mvl10") != "" )
      {
        pos = -10;
      }
      else if ( _web_server->arg("mvl1") != "" )
      {
        pos = -1;
      }
      else if ( _web_server->arg("mvp1") != "" )
      {
        pos = 1;
      }
      else if ( _web_server->arg("mvp10") != "" )
      {
        pos = 10;
      }
      else if ( _web_server->arg("mvp100") != "" )
      {
        pos = 100;
      }
      else if ( _web_server->arg("mvp500") != "" )
      {
        pos = 500;
      }
      if ( pos != 0 )
      {
        // a move button was pressed, so now process the move
        WEBSRVR_print("ws: move: ");
        WEBSRVR_println(pos);
        // get current focuser position
        long curpos = driverboard->getposition();     // cache current position
        // get maxsteps
        long maxpos = ControllerData->get_maxstep();  // cache max steps
        // calculate target
        long target = curpos + pos;
        // range check target position
        target = (target < 0) ? 0 : target;
        target = (target > maxpos) ? maxpos : target;
        WEBSRVR_print("ws: target: ");;
        WEBSRVR_println(target);
        // apply the move
        ftargetPosition = target;
      }

      // so it was not a move button. check for the goto button
      // handle goto
      String fp = _web_server->arg("pos");
      if ( fp != "" )
      {
        long tp = 0;
        long max = (long) ControllerData->get_maxstep();
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > max) ? max : tp;
        WEBSRVR_println("ws: goto: ");
        WEBSRVR_println(tp);
        // apply the move
        ftargetPosition = tp;
      }
    }
  }
  // end of move_post

  WSpg = this->_movepg;

  WSpg.replace("%PGT%", devicename);
  // Web page colors
  WSpg.replace("%TIC%", titlecolor);
  WSpg.replace("%HEC%", headercolor);
  WSpg.replace("%TXC%", textcolor);
  WSpg.replace("%BKC%", backcolor);  

  String pos = String(driverboard->getposition());

  //WSpg.replace("%CPO%", String(driverboard->getposition()));
  WSpg.replace("%CPO%", pos);
  WSpg.replace("%TPO%", String(ftargetPosition));
  if ( isMoving == true )
  {
    WSpg.replace("%MOV%", "True");
  }
  else
  {
    WSpg.replace("%MOV%", "False");
  }
  // Move form
  // now the buttons -500 to +500, each button is its own form
  WSpg.replace("%MOVL500%", H_MOVL500);
  WSpg.replace("%MOVL100%", H_MOVL100);
  WSpg.replace("%MOVL10%", H_MOVL10);
  WSpg.replace("%MOVL1%", H_MOVL1);
  WSpg.replace("%MOVP1%", H_MOVP1);
  WSpg.replace("%MOVP10%", H_MOVP10);
  WSpg.replace("%MOVP100%", H_MOVP100);
  WSpg.replace("%MOVP500%", H_MOVP500);

  // position and goto position button
  // Position [value span id POS2 %CP%] Input Field %PI% button %BP%
  WSpg.replace("%CP%", pos);
  WSpg.replace("%PI%", H_MPI);
  WSpg.replace("%BP%", H_MBP);

  // halt button
  // inline html

  // Boardname
  WSpg.replace("%NAM%", ControllerData->get_brdname());
  // Firmware Version
  WSpg.replace("%VER%", String(program_version));
  // heap
  WSpg.replace("%HEA%", String(ESP.getFreeHeap()));
  // add system uptime
  get_systemuptime();
  WSpg.replace("%SUT%", systemuptime);

  WEBSRVR_print("/move ");
  WEBSRVR_println(WSpg.length());
  send_myheader();
  send_mycontent(WSpg);
}

// ----------------------------------------------------------------------
// handler for /presets
// ----------------------------------------------------------------------
void WEB_SERVER::get_presets(void)
{
  String tmp;
  String WSpg;
  WSpg.reserve(6000);

  if ( this->_loaded == false )
  {
    ERROR_println("ws: pages not loaded");
    return;
  }

  if ( presets_type == PosT )
  {
    // if the root page was a HALT request via Submit button
    String halt_str = _web_server->arg("ha");
    if ( halt_str != "" )
    {
      portENTER_CRITICAL(&halt_alertMux);
      halt_alert = true;
      portEXIT_CRITICAL(&halt_alertMux);
    }

    // was not a halt so process any set or goto presets requested
    // if set focuser preset 0
    String fp_str = _web_server->arg("setp0");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p0");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(0, tp);
      }
    }

    // if goto focuser preset 0
    fp_str = _web_server->arg("gop0");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p0");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(0, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 1
    fp_str = _web_server->arg("setp1");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p1");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(1, tp);
      }
    }

    // if goto focuser preset 1
    fp_str = _web_server->arg("gop1");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p1");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(1, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 2
    fp_str = _web_server->arg("setp2");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p2");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(2, tp);
      }
    }

    // if goto focuser preset 2
    fp_str = _web_server->arg("gop2");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p2");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(2, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 3
    fp_str = _web_server->arg("setp3");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p3");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(3, tp);
      }
    }

    // if goto focuser preset 3
    fp_str = _web_server->arg("gop3");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p3");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(3, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 4
    fp_str = _web_server->arg("setp4");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p4");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(4, tp);
      }
    }

    // if goto focuser preset 4
    fp_str = _web_server->arg("gop4");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p4");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(4, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 5
    fp_str = _web_server->arg("setp5");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p5");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(5, tp);
      }
    }

    // if goto focuser preset 5
    fp_str = _web_server->arg("gop5");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p5");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(5, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 6
    fp_str = _web_server->arg("setp6");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p6");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(6, tp);
      }
    }

    // if goto focuser preset 6
    fp_str = _web_server->arg("gop6");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p6");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(6, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 7
    fp_str = _web_server->arg("setp7");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p7");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(7, tp);
      }
    }

    // if goto focuser preset 7
    fp_str = _web_server->arg("gop7");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p7");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(7, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 8
    fp_str = _web_server->arg("setp8");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p8");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(8, tp);
      }
    }

    // if goto focuser preset 8
    fp_str = _web_server->arg("gop8");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p8");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(8, tp);
        ftargetPosition = tp;
      }
    }

    // if set focuser preset 9
    fp_str = _web_server->arg("setp9");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p9");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(9, tp);
      }
    }

    // if goto focuser preset 9
    fp_str = _web_server->arg("gop9");
    if ( fp_str != "" )
    {
      String fp = _web_server->arg("p9");
      if ( fp != "" )
      {
        long tp = 0;
        tp = fp.toInt();
        tp = (tp < 0) ? 0 : tp;
        tp = ( tp > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : tp;
        ControllerData->set_focuserpreset(9, tp);
        ftargetPosition = tp;
      }
    }
  } // end of presets post

  WSpg = this->_presetspg;

  WSpg.replace("%PGT%", devicename);
  // Web page colors
  WSpg.replace("%TIC%", titlecolor);
  WSpg.replace("%HEC%", headercolor);
  WSpg.replace("%TXC%", textcolor);
  WSpg.replace("%BKC%", backcolor);  

  WSpg.replace("%CPO%", String(driverboard->getposition()));
  WSpg.replace("%TPO%", String(ftargetPosition));
  if ( isMoving == true )
  {
    WSpg.replace("%MOV%", "True");
  }
  else
  {
    WSpg.replace("%MOV%", "False");
  }

  tmp = P0F;
  tmp.replace("%p0num%", String(ControllerData->get_focuserpreset(0)));
  WSpg.replace("%P0F%", tmp);
  tmp = P1F;
  tmp.replace("%p1num%", String(ControllerData->get_focuserpreset(1)));
  WSpg.replace("%P1F%", tmp);
  tmp = P2F;
  tmp.replace("%p2num%", String(ControllerData->get_focuserpreset(2)));
  WSpg.replace("%P2F%", tmp);
  tmp = P3F;
  tmp.replace("%p3num%", String(ControllerData->get_focuserpreset(3)));
  WSpg.replace("%P3F%", tmp);
  tmp = P4F;
  tmp.replace("%p4num%", String(ControllerData->get_focuserpreset(4)));
  WSpg.replace("%P4F%", tmp);
  tmp = P5F;
  tmp.replace("%p5num%", String(ControllerData->get_focuserpreset(5)));
  WSpg.replace("%P5F%", tmp);
  tmp = P6F;
  tmp.replace("%p6num%", String(ControllerData->get_focuserpreset(6)));
  WSpg.replace("%P6F%", tmp);
  tmp = P7F;
  tmp.replace("%p7num%", String(ControllerData->get_focuserpreset(7)));
  WSpg.replace("%P7F%", tmp);
  tmp = P8F;
  tmp.replace("%p8num%", String(ControllerData->get_focuserpreset(8)));
  WSpg.replace("%P8F%", tmp);
  tmp = P9F;
  tmp.replace("%p9num%", String(ControllerData->get_focuserpreset(9)));
  WSpg.replace("%P9F%", tmp);
  // halt is inline html

  // Boardname
  WSpg.replace("%NAM%", ControllerData->get_brdname());
  // Firmware Version
  WSpg.replace("%VER%", String(program_version));
  // heap
  WSpg.replace("%HEA%", String(ESP.getFreeHeap()));
  // add system uptime
  get_systemuptime();
  WSpg.replace("%SUT%", systemuptime);

  WEBSRVR_print("/presets ");
  WEBSRVR_println(WSpg.length());
  send_myheader();
  send_mycontent(WSpg);
}


// ----------------------------------------------------------------------
// load web pages from SPIFFS to local storage
// ----------------------------------------------------------------------
void WEB_SERVER::reload_webpages()
{
  WEBSRVR_println("ws: reload_webpages start");
  if ( this->_loaded == false )
  {
    ERROR_println("ws: reload_webpages FS not loaded");
    return;
  }

  if ( loadpages() == false )
  {
    ERROR_println("ws: Web pages reloaded into cache: ERROR");
  }
  get_index();
}


// ----------------------------------------------------------------------
// get position and send to web client
// xhtml
// ----------------------------------------------------------------------
void WEB_SERVER::get_position()
{
  // Send position value only to client ajax request
  _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, String(driverboard->getposition()));
}

// ----------------------------------------------------------------------
// get ismoving and send to web client
// xhtml
// ----------------------------------------------------------------------
void WEB_SERVER::get_ismoving()
{
  if ( this->_loaded == false )
  {
    ERROR_println("ws: pages not loaded");
    return;
  }

  // Send isMoving value only to client ajax request
  if ( isMoving == true )
  {
    _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "True");
  }
  else
  {
    _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "False");
  }
}

// ----------------------------------------------------------------------
// get target position and send to web client
// xhtml
// ----------------------------------------------------------------------
void WEB_SERVER::get_targetposition()
{
  //Send targetPosition value only to client ajax request
  _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, String(ftargetPosition));
}

// ----------------------------------------------------------------------
// get temperature and send to web client
// xhtml
// ----------------------------------------------------------------------
void WEB_SERVER::get_temperature()
{
  //Send temperature value only to client ajax request
  _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, String(temp, 2));
}

// ----------------------------------------------------------------------
// get park state and send to web client
// xhtml
// ----------------------------------------------------------------------
void WEB_SERVER::get_park()
{
  //Send park state to client ajax request
  if ( this->_parked == true)
  {
    _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Parked");
  }
  else
  {
    _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Not Parked");
  }
}

// ----------------------------------------------------------------------
// get coil power state and send to web client
// xhtml
// ----------------------------------------------------------------------
void WEB_SERVER::get_coilpower()
{
  // Send coil power enabled state to client ajax request
  if ( ControllerData->get_coilpower_enable() == true)
  {
    _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Enabled");
  }
  else
  {
    _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, "Disabled");
  }
}

// ----------------------------------------------------------------------
// get notfound and send to web client
// xhtml
// ----------------------------------------------------------------------
void WEB_SERVER::get_notfound(void)
{
  String WSpg;
  WSpg.reserve(1200);

  if ( this->_loaded == false )
  {
    WEBSRVR_println("ws: pages not loaded");
    return;
  }

  WSpg = this->_notfoundpg;

  // process for dynamic data
  WSpg.replace("%PGT%", devicename);
  WSpg.replace("%IP%", ipStr);
  WSpg.replace("%POR%", String(ControllerData->get_websrvr_port()));
  WSpg.replace("%VER%", String(program_version));
  WSpg.replace("%NAM%", ControllerData->get_brdname());

  String tmp = ControllerData->get_wp_titlecolor();
  WSpg.replace("%TIC%", tmp);
  tmp = ControllerData->get_wp_headercolor();
  WSpg.replace("%HEC%", tmp);
  tmp = ControllerData->get_wp_textcolor();
  WSpg.replace("%TXC%", tmp);
  tmp = ControllerData->get_wp_backcolor();
  WSpg.replace("%BKC%", tmp);

  // drive board name
  WSpg.replace("%NAM%", ControllerData->get_brdname());
  // Firmware Version
  WSpg.replace("%VER%", String(program_version));
  // heap
  WSpg.replace("%HEA%", String(ESP.getFreeHeap()));
  // add system uptime
  get_systemuptime();
  WSpg.replace("%SUT%", systemuptime);

  WEBSRVR_print("ws: /notfound: ");
  WEBSRVR_println(WSpg.length());
  send_myheader();
  send_mycontent(WSpg);
}

// ----------------------------------------------------------------------
// send header for XHTML to client
// ----------------------------------------------------------------------
void WEB_SERVER::send_ACAOheader(void)
{
  _web_server->sendHeader("Access-Control-Allow-Origin", "*");
}

// ----------------------------------------------------------------------
// send HTML header to client
// ----------------------------------------------------------------------
void WEB_SERVER::send_myheader(void)
{
  _web_server->client().println("HTTP/1.1 200 OK");
  _web_server->client().println("Content-type:text/html");
  _web_server->client().println();
}

// ----------------------------------------------------------------------
// XHTML Add AOC Header
// ----------------------------------------------------------------------
void WEB_SERVER::send_xhtml( String str )
{
  send_ACAOheader();
  _web_server->send(NORMALWEBPAGE, PLAINTEXTPAGETYPE, str);
}

// ----------------------------------------------------------------------
// send json string to client
// ----------------------------------------------------------------------
void WEB_SERVER::send_json(String str)
{
  send_ACAOheader();
  _web_server->send(NORMALWEBPAGE, JSONPAGETYPE, str );
}

// ----------------------------------------------------------------------
// send HTML page to client
// ----------------------------------------------------------------------
void WEB_SERVER::send_mycontent(String pg)
{
  _web_server->client().print(pg);
}

// ----------------------------------------------------------------------
// convert the file extension to the MIME type
// ----------------------------------------------------------------------
String WEB_SERVER::get_contenttype(String filename)
{
  String retval = "text/plain";
  if (filename.endsWith(".html"))
  {
    retval = "text/html";
  }
  else if (filename.endsWith(".css"))
  {
    retval = "text/css";
  }
  else if (filename.endsWith(".js"))
  {
    retval = "application/javascript";
  }
  else if (filename.endsWith(".ico"))
  {
    retval = "image/x-icon";
  }
  //retval = "application/octet-stream";
  return retval;
}

// ----------------------------------------------------------------------
// send the right file to the client (if it exists)
// ----------------------------------------------------------------------
bool WEB_SERVER::is_hexdigit( char c )
{
  if ( (c >= '0') && (c <= '9') )                       // is a digit
  {
    return true;
  }
  if ( (c >= 'a') && (c <= 'f') )                       // is a-f
  {
    return true;
  }
  if ( (c >= 'A') && (c <= 'F') )                       // is A-F
  {
    return true;
  }
  return false;
}
