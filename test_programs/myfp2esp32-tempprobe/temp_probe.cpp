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


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <OneWire.h>                              // https://github.com/PaulStoffregen/OneWire
#include <myDallasTemperature.h>

// get access to class definition
#include "temp_probe.h"


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
  this->_loaded = false;

  _tpOneWire = new OneWire();
  _tpOneWire->begin(TEMPPIN);
  _tpsensor = new DallasTemperature(_tpOneWire);
  _tpsensor->begin();

  if ( _tpsensor->getDeviceCount() != 0 )
  {
    if (_tpsensor->getAddress(_tpAddress, 0) == true)      // get the address so we can set the probe resolution
    {
      Serial.println("temp: probe found");
      this->_found = 1;
      return true;
    }
  }
  Serial.println("temp: probe not found");
  this->_found = 0;
  return false;
}


// ----------------------------------------------------------------------
// start the temp probe, return false=not found, true=found
// bool start(void);
// ----------------------------------------------------------------------
bool TEMP_PROBE::start()
{
  this->_lasttemp = V_DEFAULTTEMP;

  // prevent any attempt to start if already started
  if ( this->_loaded == true )
  {
    Serial.println("temp: probe already running");
    return true;
  }

  //_tpsensor = DallasTemperature(_tpOneWire);

  //_tpsensor->begin();                                   // start dallas temp probe sensor1
  int num = _tpsensor->getDeviceCount();                  // should return 1 if probe connected
  if ( num != 0 )
  {
    if (_tpsensor->getAddress(_tpAddress, 0) == true)  // get the address so we can set the probe resolution
    {
      // set probe resolution
      _tpsensor->setResolution(_tpAddress, 10);
      // request the sensor to begin a temperature reading
      _tpsensor->requestTemperatures();
      this->_found = 1;
      this->_loaded = true;
      this->_state = V_RUNNING;
      Serial.println("temp: probe running");
      return true;
    }
  }
  else
  {
    Serial.println("temp: probe not started");
    this->_found = 0;
    this->_loaded = false;
  }
  return false;
}


// ----------------------------------------------------------------------
// Stop probe
// ----------------------------------------------------------------------
void TEMP_PROBE::stop()
{
  // check if already stopped
  if ( this->_loaded == true )
  {
    this->_state = V_STOPPED;
  }
  this->_loaded = false;;
  Serial.println("temp: stopped");
}

// ----------------------------------------------------------------------
// Get state
// ----------------------------------------------------------------------
byte TEMP_PROBE::get_state(void)
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
byte TEMP_PROBE::get_probefound()
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
    return this->_lasttemp;
  }

  static byte requesttempflag = 0;                      // start with a temp request
  static float tempval;
  static float starttemp;                               // start temperature to use when temperature compensation is enabled

  if (requesttempflag)
  {
    tempval = read();
  }
  else
  {
    _tpsensor->requestTemperatures();
  }

  requesttempflag ^= 1; // toggle flag

  return tempval;
}
