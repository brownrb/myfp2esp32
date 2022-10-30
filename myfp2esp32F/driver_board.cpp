// ----------------------------------------------------------------------
// myFP2ESP32 BOARD DRIVER CLASS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// © Copyright Paul P, 2021-2022. All Rights Reserved. TMC22xx code
// driver_board.cpp
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Impending changes
// hpsw_init and hps_alert will be changed
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Rules
// ----------------------------------------------------------------------
// setstepmode()
// ----------------------------------------------------------------------
// Basic rule for setting stepmode
// Set DRIVER_BOARD->setstepmode(xx);                         // this sets the physical pins
// and this also saves ControllerData->set_brdstepmode(xx);   // this saves config setting

// setstallguardvalue()
// Write
// to change a stall-guard value, code must call DRIVER_BOARD->setstallguard(sgval)
// which also updates ControllerData->set_stallguard_value(smgval);
// Read
// code calls ControllerData->get_stallguard_value();

// settmc2209current()
// Write
// to change tmc2209 current mA value, code must call DRIVER_BOARD->settmc2209current(val)
// which also updates ControllerData->set_tmc2209current(val);
// Read
// code calls ControllerData->get_tmc2209current();

// settmc2225current()
// Write
// to change tmc2225 current mA value, code must call DRIVER_BOARD->settmc2225current(val)
// which also updates ControllerData->set_tmc2225current(val);
// Read
// code calls ControllerData->get_tmc2225current();


// ----------------------------------------------------------------------
// NOTE
// ----------------------------------------------------------------------
// Consider this
//     BOARD_println("DrvBrd:start: INOUT LEDs enabled");
// The text is only ever included IF the BOARD_println is defined (default is NOT defined)
// This reduces program size
// Case 1: BOARD_print is defined
// The text is placed into program memory space by the compiler, it does not require (F(text))
// Case 2: Convert text to char *
// If the text is made a char *, then it is included every single time (program size increases)
// Conclusion: As the BOARD_println() is normally undefined then it is best to leave it as is


// ----------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------
#include <Arduino.h>
#include "controller_config.h"                // includes boarddefs.h and controller_defines.h


// -----------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------
// DO NOT ENABLE DEBUGGING INFORMATION.

// Remove comment to enable messages to Serial port
//#define DRVBRD_PRINT       1

// -----------------------------------------------------------------------
// DO NOT CHANGE
// -----------------------------------------------------------------------
#ifdef  DRVBRD_PRINT
#define DRVBRD_print(...)   Serial.print(__VA_ARGS__)
#define DRVBRD_println(...) Serial.println(__VA_ARGS__)
#else
#define DRVBRD_print(...)
#define DRVBRD_println(...)
#endif


// DEFAULT CONFIGURATION
// Driverboard
// In-Out LEDs
// Pushbuttons
// Joystick1 and Joystick2
// Home Position Switch
// Step Mode


// ----------------------------------------------------------------------
// CONTROLLER CONFIG DATA
// ----------------------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

// DRIVER BOARD
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;

// import defines for stall guard and current settings for TMC driver boards
#include "defines/tmcstepper_defines.h"


// ----------------------------------------------------------------------
// Externs
// ----------------------------------------------------------------------
extern volatile bool timerSemaphore;
extern volatile uint32_t stepcount;           // number of steps to move in timer interrupt service routine
extern portMUX_TYPE timerSemaphoreMux;
extern portMUX_TYPE stepcountMux;
extern bool filesystemloaded;                 // flag indicator for file access, rather than use SPIFFS.begin() test
extern long ftargetPosition;


// ----------------------------------------------------------------------
// JOYSTICK DEFINITIONS
// ----------------------------------------------------------------------
// joystick pin definitions come from the Board config

// other joystick settings
#define JZEROPOINT    1837                    // value read when joystick is centered
#define JTHRESHOLD    300                     // margin of error around center position
#define JMAXVALUE     4095                    // maximum value reading of joystick
#define JMINVALUE     0                       // minimum value reading of joystick


// ----------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// Data
// ----------------------------------------------------------------------
// shared between interrupt handler and driverboard class
bool stepdir;                                 // direction of steps to move


// ----------------------------------------------------------------------
// timer Interrupt
// ----------------------------------------------------------------------
#include "esp32-hal-cpu.h"                    // so we can get CPU frequency
hw_timer_t * movetimer = NULL;                // use a unique name for the timer

/*
  if (stepcount  && !(driverboard->hpsw_alert() && stepdir == moving_in))

  stepcount   stepdir        hpsw_alert     action
  ----------------------------------------------------
    0           x             x             stop
    >0          moving_out    x             step
    >0          moving_in     False         step
    >0          moving_in     True          stop
*/

inline void asm2uS()  __attribute__((always_inline));

// On esp32 with 240mHz clock a nop takes ? 1/240000000 second or 0.000000004166 of a second
// To get to 2us for ESP32 we will need 240 nop instructions

inline void asm1uS()                          // 1/3uS on ESP32
{
  asm volatile (
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    ::
  );
}

// timer ISR  Interrupt Service Routine
void IRAM_ATTR onTimer()
{
  static bool mjob = false;                   // motor job is running or not
  if (stepcount  && !(driverboard->hpsw_alert() && stepdir == moving_in))
  {
    driverboard->movemotor(stepdir, true);
    portENTER_CRITICAL(&stepcountMux);
    stepcount--;
    portEXIT_CRITICAL(&stepcountMux);
    mjob = true;                              // mark a running job
  }
  else
  {
    if (mjob == true)
    {
      portENTER_CRITICAL(&stepcountMux);
      stepcount = 0;                          // just in case hps_alert was fired up
      portEXIT_CRITICAL(&stepcountMux);
      mjob = false;                           // wait, and do nothing
      portENTER_CRITICAL(&timerSemaphoreMux);
      timerSemaphore = true;
      portEXIT_CRITICAL(&timerSemaphoreMux);
    }
  }
}

// ----------------------------------------------------------------------
// JOYSTICK2
// Keyes KY-023 PS2 style 2-Axis Joystick
// ----------------------------------------------------------------------
// This must be outside of class
void IRAM_ATTR joystick2sw_isr()
{
  // The pin is an input, with the internal pullup holding the pin state high.
  // When the switch is pressed, the pin is taken to ground (hence falling).
  // The pin is monitored by an interrupt which looks for a falling pulse.
  // When a fall is detected, the ISR sets the switch state by using the
  // pointer to the driverboard class
  // NOTE: true means switch has been pressed (and not the logic state of the pin)
  driverboard->set_joystick2_swstate(true);
}

// ----------------------------------------------------------------------
// DRIVER_BOARD CLASS
// ----------------------------------------------------------------------
DRIVER_BOARD::DRIVER_BOARD()
{

}

void DRIVER_BOARD::start(long startposition)
{
  do {
    _clock_frequency = ESP.getCpuFreqMHz();       // returns the CPU frequency in MHz as an unsigned 8-bit integer

    portENTER_CRITICAL(&timerSemaphoreMux);       // make sure timersemaphore is false when DRIVER_BOARD created
    timerSemaphore = false;
    portEXIT_CRITICAL(&timerSemaphoreMux);
    portENTER_CRITICAL(&stepcountMux);            // make sure stepcount is 0 when DRIVER_BOARD created
    stepcount = 0;
    portEXIT_CRITICAL(&stepcountMux);

    // get board number
    this->_boardnum = ControllerData->get_brdnumber();  // get board number and cache it locally here
    // setup board
    if ( this->_boardnum == PRO2ESP32R3WEMOS )
    {
      pinMode(ControllerData->get_brdenablepin(), OUTPUT);
      pinMode(ControllerData->get_brddirpin(), OUTPUT);
      pinMode(ControllerData->get_brdsteppin(), OUTPUT);
      digitalWrite(ControllerData->get_brdenablepin(), 1);
      // fixed step mode
    }
    else if ( this->_boardnum == PRO2ESP32DRV8825 )
    {
      pinMode(ControllerData->get_brdenablepin(), OUTPUT);
      pinMode(ControllerData->get_brddirpin(), OUTPUT);
      pinMode(ControllerData->get_brdsteppin(), OUTPUT);
      digitalWrite(ControllerData->get_brdenablepin(), 1);
      digitalWrite(ControllerData->get_brdsteppin(), 0);
      pinMode(ControllerData->get_brdboardpins(0), OUTPUT);
      pinMode(ControllerData->get_brdboardpins(1), OUTPUT);
      pinMode(ControllerData->get_brdboardpins(2), OUTPUT);
      // restore step mode
      setstepmode( ControllerData->get_brdstepmode() );
    }
    else if ( this->_boardnum == PRO2ESP32ULN2003 )
    {
      // IN1, IN2, IN3, IN4
      this->_inputpins[0] = ControllerData->get_brdboardpins(0);
      this->_inputpins[1] = ControllerData->get_brdboardpins(1);
      this->_inputpins[2] = ControllerData->get_brdboardpins(2);
      this->_inputpins[3] = ControllerData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->_inputpins[i], OUTPUT);
      }
      myhstepper = new HalfStepper(ControllerData->get_brdstepsperrev(), this->_inputpins[0], this->_inputpins[1], this->_inputpins[2], this->_inputpins[3]);  // ok
      // restore step mode
      setstepmode( ControllerData->get_brdstepmode() );
    }
    else if ( this->_boardnum == PRO2ESP32L298N )
    {
      // IN1, IN2, IN3, IN4
      this->_inputpins[0] = ControllerData->get_brdboardpins(0);
      this->_inputpins[1] = ControllerData->get_brdboardpins(1);
      this->_inputpins[2] = ControllerData->get_brdboardpins(2);
      this->_inputpins[3] = ControllerData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->_inputpins[i], OUTPUT);
      }
      myhstepper = new HalfStepper(ControllerData->get_brdstepsperrev(), this->_inputpins[0], this->_inputpins[1], this->_inputpins[2], this->_inputpins[3]);  // ok
      // restore step mode
      setstepmode( ControllerData->get_brdstepmode() );
    }
    else if ( this->_boardnum == PRO2ESP32L293DMINI )
    {
      // IN1, IN2, IN3, IN4
      this->_inputpins[0] = ControllerData->get_brdboardpins(0);
      this->_inputpins[1] = ControllerData->get_brdboardpins(1);
      this->_inputpins[2] = ControllerData->get_brdboardpins(2);
      this->_inputpins[3] = ControllerData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->_inputpins[i], OUTPUT);
      }
      myhstepper = new HalfStepper(ControllerData->get_brdstepsperrev(), this->_inputpins[0], this->_inputpins[1], this->_inputpins[2], this->_inputpins[3]);  // ok
      // restore step mode
      setstepmode( ControllerData->get_brdstepmode() );
    }
    else if ( this->_boardnum == PRO2ESP32L9110S )
    {
      // IN1, IN2, IN3, IN4
      this->_inputpins[0] = ControllerData->get_brdboardpins(0);
      this->_inputpins[1] = ControllerData->get_brdboardpins(1);
      this->_inputpins[2] = ControllerData->get_brdboardpins(2);
      this->_inputpins[3] = ControllerData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++)
      {
        pinMode(this->_inputpins[i], OUTPUT);
      }
      myhstepper = new HalfStepper(ControllerData->get_brdstepsperrev(), this->_inputpins[0], this->_inputpins[1], this->_inputpins[2], this->_inputpins[3]);  // ok
      // restore step mode
      setstepmode( ControllerData->get_brdstepmode() );
    }
    else if ( this->_boardnum == PRO2ESP32TMC2225 )
    {
      // init tmc2225
      pinMode(ControllerData->get_brdenablepin(), OUTPUT);
      pinMode(ControllerData->get_brddirpin(), OUTPUT);
      pinMode(ControllerData->get_brdsteppin(), OUTPUT);
      digitalWrite(ControllerData->get_brdenablepin(), 1);      // high disables the driver chip
      digitalWrite(ControllerData->get_brdsteppin(), 0);
      pinMode(ControllerData->get_brdboardpins(0), OUTPUT);     // ms1
      pinMode(ControllerData->get_brdboardpins(1), OUTPUT);     // ms2
      init_tmc2225();                                     // set step mode handled by init_tmc2225
    }
    else if ( this->_boardnum == PRO2ESP32TMC2209 || this->_boardnum == PRO2ESP32TMC2209P )
    {
      // init tmc2209
      pinMode(ControllerData->get_brdenablepin(), OUTPUT);
      pinMode(ControllerData->get_brddirpin(), OUTPUT);
      pinMode(ControllerData->get_brdsteppin(), OUTPUT);
      digitalWrite(ControllerData->get_brdenablepin(), 1);      // high disables the driver chip
      digitalWrite(ControllerData->get_brdsteppin(), 0);
      pinMode(ControllerData->get_brdboardpins(0), OUTPUT);     // ms1
      pinMode(ControllerData->get_brdboardpins(1), OUTPUT);     // ms2
      init_tmc2209();                                     // set step mode handled by init_tmc2209()
    }
  } while (0);

  // For all boards do the following
  this->_focuserposition = startposition;                       // set default focuser position to same as mySetupData
  if ( init_hpsw() == true )                                    // initialize home position switch
  {
    DRVBRD_println("drvbrd: init hpsw ok");
  }

  // In Out LEDS
  if ( set_leds(ControllerData->get_inoutled_enable()) == true )
  {
    DRVBRD_println("drvbrd: inout led enable ok");
  }

  // only 1 can be in an enabled state,
  if ( set_pushbuttons(ControllerData->get_pushbutton_enable()) == true )  // Pushbuttons
  {
    DRVBRD_println("drvbrd: pushbutton enable ok");
  }
  else if (set_joystick1(ControllerData->get_joystick1_enable()) == true )  // joystick1
  {
    DRVBRD_println("drvbrd: joystick1 enable ok");
  }
  else if (set_joystick2(ControllerData->get_joystick2_enable()) == true )  // joystick2
  {
    DRVBRD_println("drvbrd: joystick2 enable ok");
  }
  else
  {
    DRVBRD_println("drvbrd: set_pushbuttons: option not found");
  }
}

// destructor
DRIVER_BOARD::~DRIVER_BOARD()
{
  if ( this->_boardnum == PRO2ESP32ULN2003 || this->_boardnum == PRO2ESP32L298N || this->_boardnum == PRO2ESP32L293DMINI || this->_boardnum == PRO2ESP32L9110S)
  {
    delete myhstepper;
  }
  else if  (this->_boardnum == PRO2ESP32TMC2225 || this->_boardnum == PRO2ESP32TMC2209 )
  {
#if DRVBRD == PRO2ESP32TMC2225 || DRVBRD == PRO2ESP32TMC2209
    //delete mytmcstepper;  // tmc2209Stepper has no destructor
#endif
  }
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

// ----------------------------------------------------------------------
// SET PUSHBUTTONS STATE
// if( driverboard->set_pushbuttons(bool) == true)
// return true if the pushbuttons state was set correctly
// ----------------------------------------------------------------------
bool DRIVER_BOARD::set_pushbuttons(bool state)
{
  if ( state == true )
  {
    DRVBRD_println("drvbrd: set_pushbuttons(true), start");
    // check if already enabled
    if ( this->_pushbuttons_loaded == V_ENABLED )
    {
      DRVBRD_println("drvbrd: set_pushbuttons(true), already started");
    }
    else
    {
      // pushbuttons are not enabled
      // check if joystick1 is enabled
      if ( this->_joystick1_loaded == V_ENABLED )
      {
        ERROR_println("drvbrd: joystick1 is loaded");
        this->_pushbuttons_loaded = V_NOTENABLED;
        ControllerData->set_pushbutton_enable(V_NOTENABLED);
      }
      else
      {
        // check if joystick2 is enabled
        if ( this->_joystick2_loaded == V_ENABLED )
        {
          ERROR_println("drvbrd: joystick2 is loaded");
          this->_pushbuttons_loaded = V_NOTENABLED;
          ControllerData->set_pushbutton_enable(V_NOTENABLED);
        }
        else
        {
          // push buttons are not enabled, so enable them
          // check if they are permitted for this board
          if ( (ControllerData->get_brdpb1pin() == -1) || (ControllerData->get_brdpb2pin() == -1) )
          {
            ERROR_println("drvbrd: set_pushbuttons(true), not supported");
            ControllerData->set_pushbutton_enable(V_NOTENABLED);
            this->_pushbuttons_loaded = V_NOTENABLED;
          }
          else
          {
            // pushbuttons is permitted
            // enable pushbuttons
            DRVBRD_println("drvbrd: set_pushbuttons(true), enable pb now");
            ControllerData->set_pushbutton_enable(V_ENABLED);
            pinMode(ControllerData->get_brdpb2pin(), INPUT);
            pinMode(ControllerData->get_brdpb1pin(), INPUT);
            this->_pushbuttons_loaded = V_ENABLED;
          }
        }
      }
    }
  }
  else
  {
    // no need to check if it is already stopped/notenabled
    DRVBRD_println("drvbrd: set_pushbuttons(false), disable cd->pb enable()");
    ControllerData->set_pushbutton_enable(V_NOTENABLED);
    this->_pushbuttons_loaded = V_NOTENABLED;
  }
  return this->_pushbuttons_loaded;
}

// ----------------------------------------------------------------------
// IS PUSHBUTTONS LOADED
// if( driverboard->get_pushbuttons_loaded() == true)
// ----------------------------------------------------------------------
bool DRIVER_BOARD::get_pushbuttons_loaded(void)
{
  return this->_pushbuttons_loaded;
}

// ----------------------------------------------------------------------
// UPDATE PUSHBUTTONS
// driverboard->update_pushbuttons()
// ----------------------------------------------------------------------
void DRIVER_BOARD::update_pushbuttons(void)
{
  if ( this->_pushbuttons_loaded == true )
  {
    static long newpos;

    // PB are active high - pins are low by virtue of pull down resistors through J16 and J17 jumpers
    // read from the board pin number, and compare the return pin value - if 1 then button is pressed
    if ( digitalRead(ControllerData->get_brdpb1pin()) == 1 )
    {
      newpos = ftargetPosition - ControllerData->get_pushbutton_steps();
      newpos = (newpos < 0 ) ? 0 : newpos;
      ftargetPosition = newpos;
    }
    if ( digitalRead(ControllerData->get_brdpb2pin()) == 1 )
    {
      newpos = ftargetPosition + ControllerData->get_pushbutton_steps();
      // an unsigned long range is 0 to 4,294,967,295
      // when an unsigned long decrements from 0-1 it goes to largest +ve value, ie 4,294,967,295
      // which would in likely be much much greater than maxstep
      newpos = (newpos > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : newpos;
      ftargetPosition = newpos;
    }
  }
}

// ----------------------------------------------------------------------
// SET JOYSTICK1 STATE
// if( driverboard->set_joystick1() == true)
// ----------------------------------------------------------------------
bool DRIVER_BOARD::set_joystick1(bool state)
{
  if ( state == true )
  {
    DRVBRD_println("drvbrd: set_joystick1(true), start");
    // check if already enabled
    if ( this->_joystick1_loaded == V_ENABLED )
    {
      DRVBRD_println("drvbrd: set_joystick1(true), already started");
    }
    else
    {
      // joystick1 is not enabled
      // check if push buttons is enabled
      if ( this->_pushbuttons_loaded == V_ENABLED )
      {
        ERROR_println("drvbrd: pushbuttons is loaded");
        this->_joystick1_loaded = V_NOTENABLED;
        ControllerData->set_joystick1_enable(V_NOTENABLED);
      }
      else
      {
        // check if joystick2 is enabled
        if ( this->_joystick2_loaded == V_ENABLED )
        {
          ERROR_println("drvbrd: joystick2 is loaded");
          this->_joystick1_loaded = V_NOTENABLED;
          ControllerData->set_joystick1_enable(V_NOTENABLED);
        }
        else
        {
          // pb and joy2 are not enabled, so try enable joy1
          // check if joystick is permitted for this board
          if ( (ControllerData->get_brdpb1pin() == -1) || (ControllerData->get_brdpb2pin() == -1) )
          {
            ERROR_println("drvbrd: joystick1 not allowed for this board");
            this->_joystick1_loaded = V_NOTENABLED;
            ControllerData->set_joystick1_enable(V_NOTENABLED);
          }
          else
          {
            // joystick1 is permitted
            // enable joystick1
            DRVBRD_println("drvbrd: set_joystick1_enable(true), enable joystick1 now");
            // enable joystick1, joystick 1 does not use brdpb2pin
            pinMode(ControllerData->get_brdpb2pin(), INPUT);
            pinMode(ControllerData->get_brdpb1pin(), INPUT);
            ControllerData->set_joystick1_enable(V_ENABLED);
            this->_joystick1_loaded = V_ENABLED;
          }
        }
      }
    }
  }
  else // if ( state == true )
  {
    // state is false
    DRVBRD_println("drvbrd: set_joystick1_enable(false)");
    // disable joystick1
    ControllerData->set_joystick1_enable(V_NOTENABLED);
    this->_joystick1_loaded = V_NOTENABLED;
  }
  return this->_joystick1_loaded;
}

// ----------------------------------------------------------------------
// IS JOYSTICK1 LOADED
// if( driverboard->get_joystick1_loaded() == true)
// ----------------------------------------------------------------------
bool DRIVER_BOARD::get_joystick1_loaded(void)
{
  return this->_joystick1_loaded;
}

// ----------------------------------------------------------------------
// UPDATE JOYSTICK1
// driverboard->update_joystick1()
// ----------------------------------------------------------------------
void DRIVER_BOARD::update_joystick1(void)
{
  if ( this->_joystick1_loaded == true )
  {
    static int joyval;
    static long newpos;

    joyval = analogRead(ControllerData->get_brdpb1pin());
    DRVBRD_print("drvbrd: Raw joyval:");
    DRVBRD_println(joyval);
    if ( joyval < (JZEROPOINT - JTHRESHOLD) )
    {
      newpos = ftargetPosition - 1;
      newpos = (newpos < 0 ) ? 0 : newpos;
      ftargetPosition = newpos;
      DRVBRD_print("drvbrd: X IN joyval:");
      DRVBRD_println(joyval);
    }
    else if ( joyval > (JZEROPOINT + JTHRESHOLD) )
    {
      newpos = ftargetPosition + 1;
      // an unsigned long range is 0 to 4,294,967,295
      // when an unsigned long decrements from 0-1 it goes to largest +ve value, ie 4,294,967,295
      // which would in likely be much much greater than maxstep
      newpos = (newpos > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : newpos;
      ftargetPosition = newpos;
      DRVBRD_print("drvbrd: X OUT joyval:");
      DRVBRD_println(joyval);
    }
  }
  else
  {
    DRVBRD_println("drvbrd: joystick1 not loaded");
  }
}

// ----------------------------------------------------------------------
// SET JOYSTICK2 ENABLED STATE
// if( driverboard->set_joystick2() == true)
// ----------------------------------------------------------------------
bool DRIVER_BOARD::set_joystick2(bool state)
{
  if ( state == true )
  {
    DRVBRD_println("drvbrd: set_joystick2(true), start");
    // check if already enabled
    if ( this->_joystick2_loaded == V_ENABLED )
    {
      DRVBRD_println("drvbrd: set_joystick2(true), already started");
    }
    else
    {
      // joystick2 is not enabled
      // check if push buttons is enabled
      if ( this->_pushbuttons_loaded == V_ENABLED )
      {
        ERROR_println("drvbrd: pushbuttons is loaded");
        this->_joystick2_loaded = V_NOTENABLED;
        ControllerData->set_joystick2_enable(V_NOTENABLED);
      }
      else
      {
        // check if joystick1 is enabled
        if ( this->_joystick1_loaded == V_ENABLED )
        {
          ERROR_println("drvbrd: joystick1 is loaded");
          this->_joystick2_loaded = V_NOTENABLED;
          ControllerData->set_joystick2_enable(V_NOTENABLED);
        }
        else
        {
          // pb and joy1 are not enabled, so try enable joy2
          // check if joystick is permitted for this board
          if ( (ControllerData->get_brdpb1pin() == -1) || (ControllerData->get_brdpb2pin() == -1) )
          {
            ERROR_println("drvbrd: joystick2 not permitted with this board");
            this->_joystick2_loaded = V_NOTENABLED;
            ControllerData->set_joystick2_enable(V_NOTENABLED);
          }
          else
          {
            // joystick2 is permitted
            // enable joystick2
            DRVBRD_println("drvbrd: set_joystick2_enable(true), enable joystick2 now");
            ControllerData->set_joystick2_enable(V_ENABLED);
            this->_joystick2_swstate = false;
            pinMode(ControllerData->get_brdpb2pin(), INPUT);
            pinMode(ControllerData->get_brdpb1pin(), INPUT);
            // setup interrupt, falling edge, pin state = HIGH and falls to GND (0) when pressed
            attachInterrupt(ControllerData->get_brdpb2pin(), joystick2sw_isr, FALLING);
            this->_joystick2_loaded = V_ENABLED;
          }
        }
      }
    }
  }
  else // if ( state == true )
  {
    // state is false
    DRVBRD_println("drvbrd: set_joystick2_enable(false)");
    // disable joystick2
    if ( this->_joystick2_loaded == V_ENABLED )
    {
      // detach interrupt
      detachInterrupt(ControllerData->get_brdpb2pin());
    }
    this->_joystick2_swstate = false;
    this->_joystick2_loaded = false;
    ControllerData->set_joystick2_enable(V_NOTENABLED);
  }
  return this->_joystick2_loaded;
}

// ----------------------------------------------------------------------
// IS JOYSTICK2 LOADED
// if( driverboard->get_joystick2_loaded() == true)
// ----------------------------------------------------------------------
bool DRIVER_BOARD::get_joystick2_loaded(void)
{
  return this->_joystick2_loaded;
}

// ----------------------------------------------------------------------
// UPDATE JOYSTICK2
// driverboard->update_joystick2()
// ----------------------------------------------------------------------
void DRIVER_BOARD::update_joystick2(void)
{
  if ( _joystick2_loaded == true )
  {
    static int joyval;
    static long newpos;

    analogRead(ControllerData->get_brdpb1pin());
    DRVBRD_print("drvbrd: Raw joyval:");
    DRVBRD_println(joyval);
    if ( joyval < (JZEROPOINT - JTHRESHOLD) )
    {
      newpos = ftargetPosition - 1;
      newpos = (newpos < 0 ) ? 0 : newpos;
      ftargetPosition = newpos;
      DRVBRD_print("drvbrd: X IN joyval:");
      DRVBRD_println(joyval);
    }
    else if ( joyval > (JZEROPOINT + JTHRESHOLD) )
    {
      newpos = ftargetPosition + 1;
      // an unsigned long range is 0 to 4,294,967,295
      // when an unsigned long decrements from 0-1 it goes to largest +ve value, ie 4,294,967,295
      // which would in likely be much much greater than maxstep
      newpos = (newpos > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : newpos;
      ftargetPosition = newpos;
      DRVBRD_print("drvbrd: X OUT joyval:");
      DRVBRD_println(joyval);
    }

    // handle switch
    if ( this->_joystick2_swstate == true)                    // switch is pressed
    {
      // user defined code here to handle switch pressed down
      // could be a halt
      // could be a home
      // could be a preset
      // insert code here

      // OR

      // use driverboard->get_joystick2_swstate()

      // finally reset joystick switch state to allow more pushes
      this->_joystick2_swstate = false;
    }
  }
  else
  {
    ERROR_println("drvbrd: joystick2 not loaded");
  }
}

// ----------------------------------------------------------------------
// bool driverboard->get_joystick2_swstate();
// returns true if the switch was pressed
// reading the switch state clears the switch state to false, allowing
// further button presses.
// ----------------------------------------------------------------------
bool DRIVER_BOARD::get_joystick2_swstate(void)
{
  bool _state = this->_joystick2_swstate;
  this->_joystick2_swstate = false;
  return _state;
}

// ----------------------------------------------------------------------
// void driverboard->set_joystick2_swstate(bool);
// Used by the ISR routine to set the switch state
// ----------------------------------------------------------------------
void DRIVER_BOARD::set_joystick2_swstate(bool state)
{
  this->_joystick2_swstate = state;
  update_joystick2();
}


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
}

// ----------------------------------------------------------------------
// INITIALISE TMC2209 STEPPER DRIVER
// driverboard->init_tmc2209()
// ----------------------------------------------------------------------
void DRIVER_BOARD::init_tmc2209(void)
{
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
  // Specify the serial2 interface to the tmc
  mytmcstepper = new TMC2209Stepper(&SERIAL_PORT2, R_SENSE, DRIVER_ADDRESS);
  Serial2.begin(TMC2209SPEED);
  mytmcstepper->begin();
  mytmcstepper->pdn_disable(1);                                 // Use PDN/UART pin for communication
  mytmcstepper->mstep_reg_select(1);                            // Adjust stepMode from the registers
  mytmcstepper->I_scale_analog(0);                              // Adjust current from the registers
  mytmcstepper->toff(TOFF_VALUE);                               // use TMC22xx Calculations sheet to get these
  mytmcstepper->tbl(2);
  mytmcstepper->rms_current(ControllerData->get_tmc2209current()); // set driver current mA
  // step mode
  int sm = ControllerData->get_brdstepmode(); // stepmode set according to ControllerData->get_brdstepmode()
  sm = (sm == STEP1) ? 0 : sm;                                  // handle full steps
  DRVBRD_print("drvbrd: init_tmc2209() set microsteps: ");
  DRVBRD_println(sm);
  mytmcstepper->microsteps(sm);

  // stall guard settings
  mytmcstepper->semin(0);
  // lower threshold velocity for switching on smart energy CoolStep and StallGuard to DIAG output
  mytmcstepper->TCOOLTHRS(0xFFFFF);                             // 20bit max
  mytmcstepper->hysteresis_end(0);                              // use TMC22xx Calculations sheet to get these
  mytmcstepper->hysteresis_start(0);                            // use TMC22xx Calculations sheet to get these

  // setup of stall guard moved to init_hpsw()

  DRVBRD_print("drvbrd: TMC2209 Status: ");
  DRVBRD_println( driver.test_connection() == 0 ? "OK" : "NOT OK" );
  DRVBRD_print("drvbrd: Motor is ");
  DRVBRD_println(digitalRead(ControllerData->get_brdenablepin()) ? "DISABLED" : "ENABLED");
  DRVBRD_print("drvbrd: stepMode is ");
  DRVBRD_println(driver.microsteps());
#endif // #if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
}

// ----------------------------------------------------------------------
// INITIALISE TMC2225 STEPPER DRIVER
// driverboard->init_tmc2225()
// ----------------------------------------------------------------------
void DRIVER_BOARD::init_tmc2225(void)
{
#if (DRVBRD == PRO2ESP32TMC2225)
  // protection around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
  mytmcstepper = new TMC2208Stepper(&SERIAL_PORT2);             // specify the serial2 interface to the tmc2225
  Serial2.begin(TMC2225SPEED);
  mytmcstepper.begin();
  mytmcstepper->pdn_disable(1);                                 // use PDN/UART pin for communication
  mytmcstepper->mstep_reg_select(true);
  mytmcstepper->I_scale_analog(0);                              // adjust current from the registers
  mytmcstepper->rms_current(ControllerData->get_tmc2225current());   // set driver current [recommended NEMA = 400mA, set to 300mA]
  mytmcstepper->toff(2);                                        // enable driver
  unsigned short sm = (unsigned short) ControllerData->get_brdstepmode(); // stepmode set according to ControllerData->get_brdstepmode();
  sm = (sm == STEP1) ? 0 : sm;                                  // handle full steps
  mytmcstepper->microsteps(sm);
  // step mode = 1/4 - default specified in boardfile.jsn
  mytmcstepper->hysteresis_end(0);
  mytmcstepper->hysteresis_start(0);
  DRVBRD_print("drvbrd: TMC2225 Status: ");
  DRVBRD_println( driver.test_connection() == 0 ? "OK" : "NOT OK" );
  DRVBRD_print("drvbrd: Motor is ");
  DRVBRD_println(digitalRead(ControllerData->get_brdenablepin()) ? "DISABLED" : "ENABLED");
  DRVBRD_print("drvbrd: stepMode is ");
  DRVBRD_println(driver.microsteps());
#endif // #if (DRVBRD == PRO2ESP32TMC2225)
}

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

// ----------------------------------------------------------------------
// Basic rule for setting stepmode in this order
// Set DRIVER_BOARD->setstepmode(xx);                        // this sets the physical pins
// driver->setstepmode() also writes to ControllerData->set_brdstepmode(xx);
// ----------------------------------------------------------------------
void DRIVER_BOARD::setstepmode(int smode)
{
  do {
    if (  (this->_boardnum == PRO2ESP32R3WEMOS) ||  (this->_boardnum == PRO2ESP32ST6128) )
    {
      // stepmode is set in hardware jumpers, cannot set by software
      // ControllerData->set_brdstepmode(ControllerData->get_brdfixedstepmode());
      // ignore request
    }
    else if  (this->_boardnum == PRO2ESP32DRV8825 )
    {
      switch (smode)
      {
        case STEP1:
          digitalWrite(ControllerData->get_brdboardpins(0), 0);
          digitalWrite(ControllerData->get_brdboardpins(1), 0);
          digitalWrite(ControllerData->get_brdboardpins(2), 0);
          break;
        case STEP2:
          digitalWrite(ControllerData->get_brdboardpins(0), 1);
          digitalWrite(ControllerData->get_brdboardpins(1), 0);
          digitalWrite(ControllerData->get_brdboardpins(2), 0);
          break;
        case STEP4:
          digitalWrite(ControllerData->get_brdboardpins(0), 0);
          digitalWrite(ControllerData->get_brdboardpins(1), 1);
          digitalWrite(ControllerData->get_brdboardpins(2), 0);
          break;
        case STEP8:
          digitalWrite(ControllerData->get_brdboardpins(0), 1);
          digitalWrite(ControllerData->get_brdboardpins(1), 1);
          digitalWrite(ControllerData->get_brdboardpins(2), 0);
          break;
        case STEP16:
          digitalWrite(ControllerData->get_brdboardpins(0), 0);
          digitalWrite(ControllerData->get_brdboardpins(1), 0);
          digitalWrite(ControllerData->get_brdboardpins(2), 1);
          break;
        case STEP32:
          digitalWrite(ControllerData->get_brdboardpins(0), 1);
          digitalWrite(ControllerData->get_brdboardpins(1), 0);
          digitalWrite(ControllerData->get_brdboardpins(2), 1);
          break;
        default:
          digitalWrite(ControllerData->get_brdboardpins(0), 0);
          digitalWrite(ControllerData->get_brdboardpins(1), 0);
          digitalWrite(ControllerData->get_brdboardpins(2), 0);
          smode = STEP1;
          break;
      }
      // update boardconfig.jsn
      ControllerData->set_brdstepmode(smode);
    }
    else if  (this->_boardnum == PRO2ESP32ULN2003 || this->_boardnum == PRO2ESP32L298N || this->_boardnum == PRO2ESP32L9110S || this->_boardnum == PRO2ESP32L293DMINI )
    {
      switch ( smode )
      {
        case STEP1:
          myhstepper->SetSteppingMode(SteppingMode::FULL);
          break;
        case STEP2:
          myhstepper->SetSteppingMode(SteppingMode::HALF);
          break;
        default:
          smode = STEP1;
          myhstepper->SetSteppingMode(SteppingMode::FULL);
          break;
      }
      // update boardconfig.jsn
      ControllerData->set_brdstepmode(smode);
    }
    else if ( this->_boardnum == PRO2ESP32TMC2225 || this->_boardnum == PRO2ESP32TMC2209 || this->_boardnum == PRO2ESP32TMC2209P )
    {
      smode = (smode < STEP1)   ? STEP1   : smode;
      smode = (smode > STEP256) ? STEP256 : smode;
      ControllerData->set_brdstepmode( smode );
      // handle full stepmode
      smode = (smode == STEP1) ? 0 : smode;     // tmc uses 0 as full step mode
#if (DRVBRD == PRO2ESP32TMC2225 || DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
      // protection around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
      mytmcstepper->microsteps(smode);
#endif // #if (DRVBRD == PRO2ESP32TMC2225 || DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
      if (smode == 0)                           // controller uses 1 as full step mode
      {
        smode = STEP1;
      }
      // update boardconfig.jsn
      ControllerData->set_brdstepmode(smode);
    }
  } while (0);
}

// ----------------------------------------------------------------------
// ENABLE MOTOR
// driverboard->enablemotor()
// Must be done whenever the motor is required to step
// ----------------------------------------------------------------------
void DRIVER_BOARD::enablemotor(void)
{
  if  (this->_boardnum == PRO2ESP32DRV8825 || this->_boardnum == PRO2ESP32R3WEMOS || this->_boardnum == PRO2ESP32TMC2225 \
       || this->_boardnum == PRO2ESP32TMC2209 || this->_boardnum == PRO2ESP32TMC2209P || this->_boardnum == PRO2ESP32ST6128 )
  {
    digitalWrite(ControllerData->get_brdenablepin(), 0);
    delay(1);                                                   // boards require 1ms before stepping can occur
  }
}

// ----------------------------------------------------------------------
// RELEASE MOTOR
// driverboard->releasemotor()
// Turns off coil power current to the motor.
// ----------------------------------------------------------------------
void DRIVER_BOARD::releasemotor(void)
{
  if  (this->_boardnum == PRO2ESP32DRV8825 || this->_boardnum == PRO2ESP32R3WEMOS || this->_boardnum == PRO2ESP32TMC2225 \
       || this->_boardnum == PRO2ESP32TMC2209 || this->_boardnum == PRO2ESP32TMC2209P || this->_boardnum == PRO2ESP32ST6128 )
  {
    digitalWrite(ControllerData->get_brdenablepin(), 1);
  }
  else if  (this->_boardnum == PRO2ESP32ULN2003 || this->_boardnum == PRO2ESP32L298N || this->_boardnum == PRO2ESP32L293DMINI || this->_boardnum == PRO2ESP32L9110S)
  {
    digitalWrite(this->_inputpins[0], 0 );
    digitalWrite(this->_inputpins[1], 0 );
    digitalWrite(this->_inputpins[2], 0 );
    digitalWrite(this->_inputpins[3], 0 );
  }
}

// ----------------------------------------------------------------------
// MOVE MOTOR
// driverboard->movemotor(byte direction, bool updatefocuser position when moving)
// ----------------------------------------------------------------------
void DRIVER_BOARD::movemotor(byte ddir, bool updatefpos)
{
  // the fixed step mode board does not have any move associated with them in driver_board.cpp
  // only ESP32 boards have in out leds
  stepdir = ddir;

  // Basic assumption rule: If associated pin is -1 then cannot set enable
  // turn on leds
  if (  (this->_leds_loaded == V_ENABLED) &&  (this->_ledmode == LEDPULSE) )
  {
    ( stepdir == moving_in ) ? digitalWrite(ControllerData->get_brdinledpin(), 1) : digitalWrite(ControllerData->get_brdoutledpin(), 1);
  }

  // do direction, enable and step motor
  if  (this->_boardnum == PRO2ESP32DRV8825 || this->_boardnum == PRO2ESP32R3WEMOS || this->_boardnum == PRO2ESP32TMC2225 \
       || this->_boardnum == PRO2ESP32TMC2209 || this->_boardnum == PRO2ESP32TMC2209P || this->_boardnum == PRO2ESP32ST6128 )
  {
    if ( ControllerData->get_reverse_enable() == V_ENABLED )
    {
      digitalWrite(ControllerData->get_brddirpin(), !stepdir);  // set Direction of travel
    }
    else
    {
      digitalWrite(ControllerData->get_brddirpin(), stepdir);   // set Direction of travel
    }
    // board is enabled by init_motor() before timer starts, so not required here
    digitalWrite(ControllerData->get_brdsteppin(), 1);          // Step pin on
    // assume clock frequency is 240mHz
    asm1uS();                                                   // ESP32 must be 2uS delay for DRV8825 chip
    asm1uS();
    asm1uS();
    asm1uS();
    asm1uS();
    asm1uS();
    digitalWrite(ControllerData->get_brdsteppin(), 0);          // Step pin off
  }
  else if ( this->_boardnum == PRO2ESP32ULN2003 || this->_boardnum == PRO2ESP32L298N || this->_boardnum == PRO2ESP32L293DMINI || this->_boardnum == PRO2ESP32L9110S )
  {
    if ( stepdir == moving_in )
    {
      if ( ControllerData->get_reverse_enable() == V_ENABLED )
      {
        myhstepper->step(1);
      }
      else
      {
        myhstepper->step(-1);
      }
    }
    else
    {
      if ( ControllerData->get_reverse_enable() == V_ENABLED )
      {
        myhstepper->step(-1);
      }
      else
      {
        myhstepper->step(1);
      }
    }
    asm1uS();
    asm1uS();
  }

  // turn off leds
  if (  (this->_leds_loaded == V_ENABLED) &&  (this->_ledmode == LEDPULSE))
  {
    ( stepdir == moving_in ) ? digitalWrite(ControllerData->get_brdinledpin(), 0) : digitalWrite(ControllerData->get_brdoutledpin(), 0);
  }

  // update focuser position
  if ( updatefpos )
  {
    ( stepdir == moving_in ) ? this->_focuserposition-- : this->_focuserposition++;
  }
}

// ----------------------------------------------------------------------
// INIT MOVE
// driverboard->initmove(direction, steps to move)
// This enables the move timer and sets the leds for the required mode
// ----------------------------------------------------------------------
void DRIVER_BOARD::initmove(bool mdir, long steps)
{
  stepdir = mdir;
  portENTER_CRITICAL(&stepcountMux);                            // make sure stepcount is 0 when DRIVER_BOARD created
  stepcount = steps;
  portEXIT_CRITICAL(&stepcountMux);
  DRIVER_BOARD::enablemotor();
  portENTER_CRITICAL(&timerSemaphoreMux);
  timerSemaphore = false;
  portEXIT_CRITICAL(&timerSemaphoreMux);

  DRVBRD_print("drvbrd: init_move(), steps: ");
  DRVBRD_println(steps);

  // cache led mode at start of move because it may have changed
  this->_ledmode = ControllerData->get_inoutledmode();

  // if ledmode is ledmove then turn on leds now
  if ( this->_ledmode == LEDMOVE )
  {
    ( stepdir == moving_in ) ? digitalWrite(ControllerData->get_brdinledpin(), 1) : digitalWrite(ControllerData->get_brdoutledpin(), 1);
  }

  unsigned long curspd = ControllerData->get_brdmsdelay();      // get current board speed delay value

  // handle the board step delays for TMC22xx steppers differently
  if ( this->_boardnum == PRO2ESP32TMC2225 || this->_boardnum == PRO2ESP32TMC2209 || this->_boardnum == PRO2ESP32TMC2209P)
  {
    switch ( ControllerData->get_brdstepmode() )
    {
      case STEP1:
        //curspd = curspd;
        break;
      case STEP2:
        curspd = curspd / 2;
        break;
      case STEP4:
        curspd = curspd / 4;
        break;
      case STEP8:
        curspd = curspd / 8;
        break;
      case STEP16:
        curspd = curspd / 16;
        break;
      case STEP32:
        curspd = curspd / 32;
        break;
      case STEP64:
        curspd = curspd / 64;
        break;
      case STEP128:
        curspd = curspd / 128;
        break;
      case STEP256:
        curspd = curspd / 256;
        break;
      default:
        curspd = curspd / 4;
        break;
    }
  } // if( this->_boardnum == PRO2ESP32TMC2225 || this->_boardnum == PRO2ESP32TMC2209 || this->_boardnum == PRO2ESP32TMC2209P)

  DRVBRD_print("drvbrd:initmove: speed-delay: ");
  DRVBRD_println(curspd);

  // for TMC2209 stall guard, setting varies with speed setting so we need to adjust sgval for best results
  // handle different motor peeds
  byte sgval = ControllerData->get_stallguard_value();
  switch ( ControllerData->get_motorspeed() )
  {
    case 0: // slow, 1/3rd the speed
      curspd *= 3;
      // no need to change stall guard
      break;
    case 1: // med, 1/2 the speed
      curspd *= 2;
      sgval = sgval / 2;
      break;
    case 2: // fast, 1/1 the speed
      //curspd *= 1;                                           // obviously not needed
      sgval = sgval / 6;
      break;
  }
  DRVBRD_print("drvbrd: SG value to write: ");
  DRVBRD_println(sgval);
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
  // protection required around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
  // don't change the value in ControllerData : this is for a speed calculation
  mytmcstepper->SGTHRS(sgval);
#endif

  movetimer = timerBegin(1, 80, true);                         // timer-number, prescaler, count up (true) or down (false)
  timerAttachInterrupt(movetimer, &onTimer, true);             // our handler name, address of function int handler, edge=true
  // Set alarm to call onTimer function every interval value curspd (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(movetimer, curspd, true);                    // timer for ISR, interval time, reload=true
  timerAlarmEnable(movetimer);
}

// ----------------------------------------------------------------------
// END MOVE
// driverboard->end_move()
// ----------------------------------------------------------------------
// when a move has completed [or halted], we need to detach/disable movetimer
void DRIVER_BOARD::end_move(void)
{
  DRVBRD_println("drvbrd: end_move()");

  // stop the timer
  timerStop(movetimer);
  timerAlarmDisable(movetimer);       // stop alarm
  timerDetachInterrupt(movetimer);

  // if using led move mode then turn off leds at end of move
  if (  (this->_leds_loaded == V_ENABLED) &&  (this->_ledmode == LEDMOVE) )
  {
    digitalWrite(ControllerData->get_brdinledpin(), 0);
    digitalWrite(ControllerData->get_brdoutledpin(), 0);
  }
}

bool DRIVER_BOARD::getdirection(void)
{
  return stepdir;
}

long DRIVER_BOARD::getposition(void)
{
  return this->_focuserposition;
}

void DRIVER_BOARD::setposition(long newpos)
{
  this->_focuserposition = newpos;
}

byte DRIVER_BOARD::getstallguardvalue(void)
{
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
  // protection required around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
  byte sgval = mytmcstepper->SGTHRS();   // read sgthreshold
  ControllerData->set_stallguard_value(sgval);
#endif
  return ControllerData->get_stallguard_value();
}

void DRIVER_BOARD::setstallguardvalue(byte newval)
{
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
  // protection required around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
  mytmcstepper->SGTHRS(newval); // write sgthreshold
#endif
  ControllerData->set_stallguard_value(newval);
}

void DRIVER_BOARD::settmc2209current(int newval)
{
  ControllerData->set_tmc2209current(newval);
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
  // protection required around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
  mytmcstepper->rms_current(ControllerData->get_tmc2209current());     // Set driver current
#endif
}

void DRIVER_BOARD::settmc2225current(int newval)
{
  ControllerData->set_tmc2225current(newval);
#if (DRVBRD == PRO2ESP32TMC2225)
  // protection around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
  mytmcstepper->rms_current(ControllerData->get_tmc2225current());     // Set driver current
#endif
}
