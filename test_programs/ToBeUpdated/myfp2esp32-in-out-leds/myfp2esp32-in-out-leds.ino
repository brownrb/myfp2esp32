// myFP2ESP32 FIRMWARE OFFICIAL RELEASE 300A
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// © Copyright Pieter P, SPIFFs examples found online
// © Copyright Paul P, 2021-2022. All Rights Reserved. TMC22xx code
//   myfp2esp32-firmware.ino
//   version: 300
//   date:    19-09-2022
// ----------------------------------------------------------------------

// Test InOit LEDs


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>





  // In Out LEDS
  if ( set_leds(ControllerData->get_inoutled_enable()) == true )
  {
    DRVBRD_println("drvbrd: inout led enable ok");
  }

  // ----------------------------------------------------------------------
// SET LED STATE
// if( driverboard->set_leds(true) == true)
// return true if the led state was set correctly
// ----------------------------------------------------------------------
bool DRIVER_BOARD::set_leds(bool state)
{
  if ( state == true )
  {
    // check if already enabled
    if ( this->_leds_loaded == V_ENABLED )
    {
      DRVBRD_println("drvbrd: leds enabled");
      return true;
    }
    // leds are not enabled, so enable them
    // check if they are permitted for this board
    if ( (ControllerData->get_brdinledpin() == -1) || (ControllerData->get_brdoutledpin() == -1) )
    {
      ERROR_println("drvbrd: leds not available for this board");
      this->_leds_loaded = V_NOTENABLED;
      ControllerData->set_inoutled_enable(V_NOTENABLED);
      return false;
    }
    ControllerData->set_inoutled_enable(V_ENABLED);
    pinMode(ControllerData->get_brdinledpin(), OUTPUT);
    pinMode(ControllerData->get_brdoutledpin(), OUTPUT);
    this->_leds_loaded = V_ENABLED;
    return true;
  }
  else
  {
    // state is false; disable leds
    // no need to check if it is already stopped/notenabled
    this->_leds_loaded = V_NOTENABLED;
    ControllerData->set_inoutled_enable(V_NOTENABLED);
    return true;
  }
  return this->_leds_loaded;
}

// ----------------------------------------------------------------------
// IS LEDS LOADED
// if( driverboard->get_leds_loaded() == true)
// ----------------------------------------------------------------------
bool DRIVER_BOARD::get_leds_loaded(void)
{
  return this->_leds_loaded;
}


  // Basic assumption rule: If associated pin is -1 then cannot set enable
  // turn on leds
  if (  (this->_leds_loaded == V_ENABLED) &&  (this->_ledmode == LEDPULSE) )
  {
    ( stepdir == moving_in ) ? digitalWrite(ControllerData->get_brdinledpin(), 1) : digitalWrite(ControllerData->get_brdoutledpin(), 1);
  }

  // turn off leds
  if (  (this->_leds_loaded == V_ENABLED) &&  (this->_ledmode == LEDPULSE))
  {
    ( stepdir == moving_in ) ? digitalWrite(ControllerData->get_brdinledpin(), 0) : digitalWrite(ControllerData->get_brdoutledpin(), 0);
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

  DEBUG_println("Serial started");



  //-------------------------------------------------
  // SETUP IN-OUT LEDS
  // Included by default
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // Now part of DriverBoard, and initialised/enabled/stopped there



  DEBUG_println("Setup done, controller is ready");
}


void loop()
{



} // end Loop()
