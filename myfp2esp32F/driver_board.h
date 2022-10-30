// ----------------------------------------------------------------------
// myFP2ESP32 BOARD DRIVER CLASS DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// © Copyright Paul P, 2021-2022. All Rights Reserved. TMC22xx code
// driver_board.h
// ----------------------------------------------------------------------
#ifndef _driver_board_h
#define _driver_board_h

// required for DRVBRD
#include "controller_config.h"     // includes boarddefs.h and controller_defines.h

#include <myHalfStepperESP32.h>    // includes myStepperESP32.h

// Changes by Paul P, 15-08-2022
#if (DRVBRD == PRO2ESP32TMC2225) || (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P)
#define SERIAL_PORT2  Serial2       // TMCxxxx HardwareSerial port
#include <TMCStepper.h>             // tmc library https://github.com/teemuatlut/TMCStepper
#endif


// ----------------------------------------------------------------------
// JOYSTICK1
// 2-AXIS Analog Thumb Joystick for Arduino
// ----------------------------------------------------------------------
// https://www.ebay.com/itm/1PCS-New-PSP-2-Axis-Analog-Thumb-GAME-Joystick-Module-3V-5V-For-arduino-PSP/401236361097
// https://www.ebay.com/itm/1PCS-New-PSP-2-Axis-Analog-Thumb-GAME-Joystick-Module-3V-5V-For-arduino-PSP/232426858990
//
// On ESP32 analog input is 0-4095. GND=GND, VCC=3.3V
// ADC2 pins cannot be used when WiFi is being used
// ADC2 [GPIO4/GPIO2/GPIO15/GPIO13/GPIO12/GPIO14/GPIO27/GPIO26/GPIO25]
// If using WiFi use ADC1 pins
// ADC1 [GPIO33/GPIO32/GPIO35/GPIO34/GPIO39/GPIO36]
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// JOYSTICK2
// Keyes KY-023 PS2 style 2-Axis Joystick with Switch
// ----------------------------------------------------------------------
// https://www.ebay.com/sch/i.html?_from=R40&_trksid=p2510209.m570.l1313&_nkw=Keyes+KY-023+PS2+style+2-Axis+Joystick+with+Switch&_sacat=0
//
// On ESP32 analog input is 0-4095. GND=GND, VCC=3.3V
// ADC2 pins cannot be used when WiFi is being used
// ADC2 [GPIO4/GPIO2/GPIO15/GPIO13/GPIO12/GPIO14/GPIO27/GPIO26/GPIO25]
// If using WiFi use ADC1 pins
// ADC1 [GPIO33/GPIO32/GPIO35/GPIO34/GPIO39/GPIO36]

// If using JOYSTICK TYPE2 WITH SWITCH
// Wire SW to J15-y HEADER ON PCB, and install jumper on J16-PB0EN
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// DRIVER BOARD CLASS : DO NOT CHANGE
// ----------------------------------------------------------------------
class DRIVER_BOARD
{
  public:
    DRIVER_BOARD();                               // constructor
    ~DRIVER_BOARD(void);                          // destructor
    void start(long);    
    void initmove(bool, long);                    // prepare to move
    void movemotor(byte, bool);                   // move the motor
    bool init_hpsw(void);                         // initialize home position switch
    void init_tmc2209(void);
    void init_tmc2225(void);
    bool hpsw_alert(void);                        // check for HPSW, and for TMC2209 stall guard or physical switch
    void end_move(void);                          // end a move

    
    bool set_leds(bool);
    bool get_leds_loaded(void);
    // no need for leds_enable because it is in ControllerData

    bool set_pushbuttons(bool);
    bool get_pushbuttons_loaded(void);
    void update_pushbuttons(void);
    // no need for pushbuttons_enable because it is in ControllerData

    bool set_joystick1(bool);
    bool get_joystick1_loaded(void);
    void update_joystick1(void);
    // no need for joystick1_enable because it is in ControllerData

    bool set_joystick2(bool);
    bool get_joystick2_loaded(void);
    void update_joystick2(void);
    bool get_joystick2_swstate(void);
    void set_joystick2_swstate(bool);     // needed by ISR to set the switch flag, joystick2    

    // no need for joystick2_enable because it is in ControllerData

    // get
    long getposition(void);
    byte getstallguardvalue(void);
    bool getdirection(void);
    
    // set
    void enablemotor(void);
    void releasemotor(void);
    void setposition(long);
    void setstepmode(int);
    
    void setstallguardvalue(byte);     // value
    void settmc2225current(int);
    void settmc2209current(int);

  private:
    HalfStepper* myhstepper;
    Stepper*     mystepper;
#if (DRVBRD == PRO2ESP32TMC2225 )
    // protection around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
    TMC2208Stepper* mytmcstepper;
#endif // #if (DRVBRD == PRO2ESP32TMC2225)
#if (DRVBRD == PRO2ESP32TMC2209 || DRVBRD == PRO2ESP32TMC2209P )
    // protection around mytmcstepper - it is not defined if not using tmc2209 or tmc2225
    TMC2209Stepper* mytmcstepper;
#endif // DRVBRD == PRO2ESP32TMC2209  || DRVBRD == PRO2ESP32TMC2209P 

    unsigned int _clock_frequency;  // clock frequency used to generate 2us delay for ESP32 160Mhz/240Mhz
    long _focuserposition;          // current focuser position
    int  _inputpins[4];             // input pins for driving stepper boards
    int  _boardnum;                 // get the board number from mySetupData
    bool _leds_loaded = false;
    byte _ledmode = LEDPULSE;       // cached from ControllerData for faster access when moving
    bool _pushbuttons_loaded = false;
    bool _joystick1_loaded  = false;
    bool _joystick2_loaded  = false;
    bool _joystick2_swstate = false;   
};



#endif // _driverboards_h
