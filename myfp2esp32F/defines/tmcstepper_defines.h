// -------------------------------------------------------------------------
// myFP2ESP32 TMC2208/2225 STEPPER DRIVER SETTINGS
// © Copyright Robert Brown 2020-2022.
// © Copyright Holger Manz 2020-2021 
// © Copyright Paul Porters 2021-2022.
// All Rights Reserved.
// -------------------------------------------------------------------------


// ----------------------------------------------------------------------
// TMCxxxx DEFINES
// ----------------------------------------------------------------------
// These values are held in ControllerData and are managed using /admin7 HPSW page
#define STALL_VALUE         100           // [0... 255]

#define TMC2209CURRENT      600           // 600mA for 8HS15-0604S NEMA8 stepper motor with tmc2209
#define TMC2225CURRENT      300           // 300mA for recommended stepper NEMA motor - you can change this - with tmc2225

// These values do not change
#define TMC2225SPEED        57600
#define TMC2209SPEED        57600

#define TOFF_VALUE          4             // [1... 15]
#define DRIVER_ADDRESS      0b00          // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE             0.11f         // Match to your driver

// SilentStepStick series use 0.11
// UltiMachine Einsy and Archim2 boards use 0.2
// Panucatt BSD2660 uses 0.1
// Watterott TMC5160 uses 0.075

