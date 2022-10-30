// ----------------------------------------------------------------------
// myFP2ESP32 TEMPERATURE PROBE CLASS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger Manz, 2020-2021. All Rights Reserved.
// temp_probe.cpp
// Default Configuration
// Temperature Probe, Resolution, Handling of temperature compensation
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>
#include "controller_defines.h"   


// -----------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------
// DO NOT ENABLE DEBUGGING INFORMATION.

// Remove comment to enable messages to Serial port
//#define TPROBE_PRINT       1

// -----------------------------------------------------------------------
// DO NOT CHANGE
// -----------------------------------------------------------------------
#ifdef  TPROBE_PRINT
#define TPROBE_print(...)   Serial.print(__VA_ARGS__)
#define TPROBE_println(...) Serial.println(__VA_ARGS__)
#else
#define TPROBE_print(...)
#define TPROBE_println(...)
#endif


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
// get access to class
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;           // controller data

#include <OneWire.h>                              // https://github.com/PaulStoffregen/OneWire
#include <myDallasTemperature.h>

// get access to class definition
#include "temp_probe.h"


// ----------------------------------------------------------------------
// EXTERNALS
// ----------------------------------------------------------------------
extern unsigned long ftargetPosition;             // target position


// ----------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------
// default temperature value in C
#define V_DEFAULTTEMP     20.0
// temperature compensation DO NOT CHANGE
#define TEMP_FALLING      1
#define TEMP_RISING       0


// ----------------------------------------------------------------------
// CLASS
// Pass ControllerData->get_brdtemppin() as the pin number
// ----------------------------------------------------------------------
TEMP_PROBE::TEMP_PROBE( int pin ) : _pin(pin)
{
  //_oneWire = new OneWire(pin);
  //_tpsensor = new DallasTemperature(_oneWire);
  //_oneWire = new OneWire(pin);
  //_tpsensor(&_oneWire);
}


// ----------------------------------------------------------------------
// bool init(void);
// Init the temp probe, return false=error, true=ok
// ----------------------------------------------------------------------
bool TEMP_PROBE::init()
{
  this->_loaded   = false;
  this->_found    = false;
  this->_lasttemp = V_DEFAULTTEMP;
  this->_state    = false;

  // check if valid pin is defined for board
  if ( ControllerData->get_brdtemppin() == -1 )
  {
    ERROR_println("temp: board does not support temp");
    return false;
  }

  // check if valid pin is defined for board
  if ( ControllerData->get_brdtemppin() == -1 )
  {
    ERROR_println("temp: board does not support temp");    
    this->_loaded   = false;
    this->_found    = false;
    this->_lasttemp = V_DEFAULTTEMP;
    this->_state    = false;
    ControllerData->set_tcavailable(V_NOTENABLED);
    return false;
  }

  _tpOneWire = new OneWire();
  _tpOneWire->begin(this->_pin);
  _tpsensor = new DallasTemperature(_tpOneWire);
  _tpsensor->begin();

  if ( _tpsensor->getDeviceCount() != 0 )
  {
    if (_tpsensor->getAddress(_tpAddress, 0) == true)      // get the probe address ID so we can set the probe resolution
    {
      TPROBE_println("temp: probe found");
      this->_found = true;
      return true;
    }
  }
  // no probe was found
  return false;
}


// ----------------------------------------------------------------------
// start the temp probe
// bool start(void);
// ----------------------------------------------------------------------
bool TEMP_PROBE::start()
{
  // enabled?
  if( ControllerData->get_tempprobe_enable() == V_NOTENABLED )
  {
    ERROR_println("temp: temp probe is not enabled");    
    this->_loaded   = false;
    this->_lasttemp = V_DEFAULTTEMP;
    this->_state    = false;
    ControllerData->set_tcavailable(V_NOTENABLED);
    return false;
  }

  // search for a sensor
  if ( _tpsensor->getDeviceCount() != 0 )
  {
    if (_tpsensor->getAddress(_tpAddress, 0) == true)      // get the address so we can set the probe resolution
    {
      TPROBE_println("temp: probe found");
      this->_found = true;
      // set probe resolution
      _tpsensor->setResolution(_tpAddress, ControllerData->get_tempresolution());
      // request the sensor to begin a temperature reading
      _tpsensor->requestTemperatures();
      this->_found  = true;
      this->_loaded = true;
      this->_state  = true;
      ControllerData->set_tcavailable(V_ENABLED);
      TPROBE_println("temp: probe running");     
      return true;
    }
    else
    {
      // could not get sensor address info
      ERROR_println("tempprobe: start: error: sensor address not found");
      this->_loaded = false;
      this->_state  = false;
      ControllerData->set_tcavailable(V_ENABLED);
    }
  }
  else
  {
    // sensor not found
    ERROR_println("tempprobe: start: error: sensor not found");
    this->_loaded = false;
    this->_state = false;
    ControllerData->set_tcavailable(V_NOTENABLED);
  }
  return false;
}


// ----------------------------------------------------------------------
// Stop probe
// ----------------------------------------------------------------------
void TEMP_PROBE::stop()
{
  // check if already stopped
  this->_state = false;
  this->_loaded = false;
  TPROBE_println("temp: stopped");
}

// ----------------------------------------------------------------------
// Get state
// ----------------------------------------------------------------------
bool TEMP_PROBE::get_state(void)
{
  return this->_state;
}

// ----------------------------------------------------------------------
// Get loaded
// ----------------------------------------------------------------------
bool TEMP_PROBE::get_loaded(void)
{
  return this->_loaded;
}

// ----------------------------------------------------------------------
// Return temp probe found
// ----------------------------------------------------------------------
bool TEMP_PROBE::get_found()
{
  return this->_found;
}

// ----------------------------------------------------------------------
// Start a temperature reading, can take up to 700mS, but we do an async read
// ----------------------------------------------------------------------
void TEMP_PROBE::request()
{
  if ( this->_loaded == false )
  {
    return;
  }
  _tpsensor->requestTemperatures();
}

// ----------------------------------------------------------------------
// Set probe resolution
// ----------------------------------------------------------------------
void TEMP_PROBE::set_resolution(byte tpr)
{
  if ( this->_loaded == true )
  {
    _tpsensor->setResolution(_tpAddress, tpr );
  }
}

// ----------------------------------------------------------------------
// read temp probe value
// ----------------------------------------------------------------------
float TEMP_PROBE::read(void)
{
  if ( this->_loaded == false )
  {
    return this->_lasttemp;
  }

  float result = _tpsensor->getTempCByIndex(0);        // get temperature, always in celsius
  if (result > -40.0 && result < 80.0)                // avoid erronous readings
  {
    // valid, save in _lasttemp
    this->_lasttemp = result;
  }
  else
  {
    // invalid, use the last valid temp reading
    result = this->_lasttemp;
  }
  return result;
}

// ----------------------------------------------------------------------
// update_temp probe
// read temperature
// check for temperature compensation and if so, apply tc rules
// ----------------------------------------------------------------------
float TEMP_PROBE::update(void)
{
  if ( this->_loaded == false )
  {
    ERROR_println("temp: update: probe not loaded");
    return this->_lasttemp;
  }

  static byte tcchanged = ControllerData->get_tempcomp_enable();  // track tempcompenabled changes
  static byte requesttempflag = 0;                      // start with a temp request
  static float tempval;
  static float starttemp;                               // start temperature to use when temperature compensation is enabled

  if ( tcchanged != ControllerData->get_tempcomp_enable() )
  {
    tcchanged = ControllerData->get_tempcomp_enable();
    if ( tcchanged == 1 )
    {
      starttemp = read();
    }
  }

  if (requesttempflag)
  {
    tempval = read();
  }
  else
  {
    _tpsensor->requestTemperatures();
  }

  requesttempflag ^= 1; // toggle flag

  if (ControllerData->get_tempcomp_enable() == V_ENABLED) // check for temperature compensation
  {
    if ((abs)(starttemp - tempval) >= 1)                  // calculate if temp has moved by more than 1 degree
    {
      unsigned long newPos;
      byte temperaturedirection;                          // did temperature falling (1) or rising (0)?
      temperaturedirection = (tempval < starttemp) ? TEMP_FALLING : TEMP_RISING;
      if (ControllerData->get_tcdirection() == TC_DIRECTION_IN)             // check if tc direction for compensation is inwards
      {
        // temperature compensation direction is inwards, if temperature falling then move in else move out
        if ( temperaturedirection == TEMP_FALLING )                         // check if temperature is falling
        {
          newPos = ftargetPosition - ControllerData->get_tempcoefficient(); // then move inwards
        }
        else
        {
          newPos = ftargetPosition + ControllerData->get_tempcoefficient(); // else move outwards
        }
      }
      else
      {
        // temperature compensation direction is out, if a fall then move out else move in
        if ( temperaturedirection == TEMP_FALLING )
        {
          newPos = ftargetPosition + ControllerData->get_tempcoefficient();
        }
        else
        {
          newPos = ftargetPosition - ControllerData->get_tempcoefficient();
        } // if ( temperaturedirection == 1 )
      } // if (ControllerData->get_tcdirection() == 0)
      newPos = (newPos > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : newPos;
      // newPos should be checked for < 0 but cannot due to unsigned
      // newPos = (newPos < 0 ) ? 0 : newPos;
      ftargetPosition = newPos;
      starttemp = tempval;                        // save this current temp point for future reference
    } // end of check for tempchange >=1
  } // end of check for tempcomp enabled
  return tempval;
}
