// myFP2ESP32 FIRMWARE OFFICIAL RELEASE 300
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// © Copyright Pieter P, SPIFFs examples found online
// © Copyright Paul P, 2021-2022. All Rights Reserved. TMC22xx code
//   myfp2esp32-firmware.ino
//   version: 300
//   date:    19-09-2022
// ----------------------------------------------------------------------

// Test HPSW

// ----------------------------------------------------------------------
// DO NOT EDIT BELOW THIS LINE
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// CONTROLLER DEFINITIONS AND CONFIGURATION
// ----------------------------------------------------------------------
#include "controller_defines.h"


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>



// ----------------------------------------------------------------------
// INITIALISE HOME POSITION SWITCH
// if( driverboard->init_hpsw() == true)
// ----------------------------------------------------------------------
bool DRIVER_BOARD::init_hpsw(void)
{
  // If the designated pin for an option is -1 then the option is not supported
  // Check if this board supports HPSW option
  if ( ControllerData->get_brdhpswpin() == -1)
  {
    ERROR_println("drvbrd: hpsw not permitted on this board");
    return false;
  }

  // tmc2209 stall guard etc is a special case
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
  bool state = false;
  // determe what the hpsw setting is, stall guard, physical switch or none
  tmc2209stallguard sgstate = ControllerData->get_stallguard_state();
  switch ( sgstate )
  {
    case Use_Stallguard:
      // StallGuard4 threshold [0... 255] level for stall detection. It compensates for
      // motor specific characteristics and controls sensitivity. A higher value gives a higher
      // sensitivity. A higher value makes StallGuard4 more sensitive and requires less torque to
      // indicate a stall. The double of this value is compared to SG_RESULT.
      // The stall output becomes active if SG_RESULT falls below this value.
      pinMode(ControllerData->get_brdhpswpin(), INPUT_PULLUP);    // initialize the pin
      mytmcstepper->SGTHRS(ControllerData->get_stallguard_value());
      DRVBRD_println("drvbrd: init_hpsw: use stall guard");
      state = true;
      break;

    case Use_Physical_Switch:
      // if using a physical switch then hpsw in controllerdata must also be enabled
      pinMode(ControllerData->get_brdhpswpin(), INPUT_PULLUP);    // initialize the pin
      mytmcstepper->SGTHRS(0);
      DRVBRD_println("drvbrd: init_hpsw: use physical switch");
      state = true;
      break;

    case Use_None:
      mytmcstepper->SGTHRS(0);
      DRVBRD_println("drvbrd: init_hpsw: use none");
      state = true;
      break;
  }
  return state;
#endif
  // for all other boards
  if ( ControllerData->get_hpswitch_enable() == V_ENABLED )
  {
    pinMode(ControllerData->get_brdhpswpin(), INPUT_PULLUP);    // initialize the pin
    return true;
  }
  return false;


// ----------------------------------------------------------------------
// CHECK HOME POSITION SWITCH
// if( driverboard->hpsw_alert() == true )
// ----------------------------------------------------------------------
// check hpsw state - switch or stall guard for tmc2209 if enabled
// 1: physical switch uses internal pullup, if hpsw is closed = low, so we need to invert state
//    to return high when switch is closed
//
// 2: tmc2209 stall guard
//    if stall guard high then stall guard detected on DIAG pin - HIGH = activated
//    hps_alert must return high if hpsw or stall guard is activated
//
// Robert note to self : 07-Aug-2022 01:01am
// I believe there is a simpler, faster, more efficient way of handling HPSW
// I intend to drop that in for the next release

bool DRIVER_BOARD::hpsw_alert(void)
{
  bool state = false;
  // if moving out, return
  if ( stepdir == moving_out )
  {
    return state;
  }
  // if hpsw is not enabled then return false
  if ( ControllerData->get_hpswitch_enable() == V_NOTENABLED )
  {
    return state;
  }
  // hpsw is enabled so check it
  else
  {
    // check tmc2209 boards
    if ( this->_boardnum == PRO2ESP32TMC2209 ||  (this->_boardnum == PRO2ESP32TMC2209P) )
    {
      // yes checked both states
      if ( ControllerData->get_stallguard_state() == Use_Stallguard)
      {
        DRVBRD_println("drvbrd: hpsw_alert: use stall guard");
        // diag pin = HIGH if stall guard found, we return high if DIAG, low otherwise
        return (bool) digitalRead(ControllerData->get_brdhpswpin());
      }
      else if ( ControllerData->get_stallguard_state() == Use_Physical_Switch)
      {
        DRVBRD_println("drvbrd: hpsw_alert: use physical switch");
        // diag pin = HIGH if stall guard found, we return high if DIAG, low otherwise
        return !( (bool)digitalRead(ControllerData->get_brdhpswpin()) );
      }
      else
      {
        DRVBRD_println("drvbrd: hpsw_alert: use none");
        state = false;
      }
    }
    // for all other boards check the physical HOMEPOSITIONSWITCH
    else
    {
      // switch uses internal pullup, if hpsw is closed = low, so we need to invert state to return high when switch is closed
      return !( (bool)digitalRead(ControllerData->get_brdhpswpin()) );
    }
  }
  return state;
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


  DEBUG_println("Setup done, controller is ready");
}

void loop()
{
  
} // end Loop()
