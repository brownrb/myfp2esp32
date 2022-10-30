// ----------------------------------------------------------------------
// myFP2ESP32 DUCKDNS CLASS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// duckdns.cpp
// Optional
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// EXTERNALS
// ----------------------------------------------------------------------

#include <Arduino.h>
#include "controller_config.h"                    // includes boarddefs.h and controller_defines.h

#if defined(ENABLE_DUCKDNS)


// -----------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------
// DO NOT ENABLE DEBUGGING INFORMATION.

// Remove comment to enable messages to Serial port
//#define DUCKDNS_PRINT       1

// -----------------------------------------------------------------------
// DO NOT CHANGE
// -----------------------------------------------------------------------
#ifdef  DUCKDNS_PRINT
#define DUCKDNS_print(...)   Serial.print(__VA_ARGS__)
#define DUCKDNS_println(...) Serial.println(__VA_ARGS__)
#else
#define DUCKDNS_print(...)
#define DUCKDNS_println(...)
#endif


// ----------------------------------------------------------------------
// CONTROLLER CONFIG DATA
// ----------------------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;


// ----------------------------------------------------------------------
// EXTERNS
// ----------------------------------------------------------------------
extern char duckdnsdomain[];
extern char duckdnstoken[];


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <EasyDDNS.h>         // https://github.com/ayushsharma82/EasyDDNS
#include "duckdns.h"


// ----------------------------------------------------------------------
// DUCKDNS Class
// THERE IS ONLY STATUS SHOWN ON SERVER PAGE /admin1
// DUCKDNS CANNOT BE STARTED/STOPPED
// REFRESH IS HANDLED BY THE CODE - JUST SET THE REFRESH VALUE
// ----------------------------------------------------------------------
DUCK_DNS::DUCK_DNS()
{
  this->_loaded = false;
}

bool DUCK_DNS::start(void)
{
  if (this->_loaded == V_RUNNING )
  {
    return true;
  }

  if ( ControllerData->get_duckdns_enable() != V_ENABLED)
  {
    DUCKDNS_println("DuckDNS not enabled");
    return false;
  }
  else
  {
    EasyDDNS.service("duckdns");                  // Set DDNS Service Nameto "duckdns"

    String domain = ControllerData->get_duckdns_domain();
    int domainlen = domain.length() + 1;
    String token = ControllerData->get_duckdns_token();
    int tokenlen = token.length() + 1;
    char ddd[domainlen];
    char ddt[tokenlen];
    domain.toCharArray(ddd, domainlen);
    token.toCharArray(ddt, tokenlen);
    EasyDDNS.client(ddd, ddt); // Enter ddns Domain & Token | Example - "esp.duckdns.org","1234567"

    EasyDDNS.update(ControllerData->get_duckdns_refreshtime());

    this->_loaded = true;
    return true;
  }
}

void DUCK_DNS::set_refresh(unsigned int ti)
{
  // save new refresh value
  if ( ti != ControllerData->get_duckdns_refreshtime())
  {
    ControllerData->set_duckdns_refreshtime(ti);
  }

  // apply new refresh time if duckdns is running
  if ( this->_loaded == V_RUNNING )
  {
    DUCKDNS_print("Duckdns refresh time ");
    DUCKDNS_println(ti);
    EasyDDNS.update(ti);
  }
  else
  {
    ERROR_print("Duckdns refresh time: error: not running");
  }
}

#endif // #ifdef ENABLE_DUCKDNS
