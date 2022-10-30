// ----------------------------------------------------------------------
// myFP2ESP32
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// about.h
// ----------------------------------------------------------------------
// There are a large number of extra features that are not listed in 
// controller_config.h file
//
// Most of those features are disabled. To enable them requires using the
// Management Server interface.
//
// This file is maintained using Sublime Text
//
// ----------------------------------------------------------------------
// COMPILE ENVIROMENT
// ----------------------------------------------------------------------
// Firmware 300
// 
// Arduino IDE          1.8.19
// Arduino Core ESP32   2.0.4
// ESP32FS              1.0     https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/  
// IRRemote             myfp2eIRremoteESP8266 
// Stepper Motor        myHalfStepperESP32
// Temperature Probe    myDallasTemperature
// Text Oled Display    myOLED

// Other libraries
// ArduinoJSON  		6.19.4	// https://github.com/bblanchon/ArduinoJson
// DuckDNS      		1.8.0	// https://github.com/ayushsharma82/EasyDDNS
// ElegantOTA   		2.2.9	// https://github.com/ayushsharma82/ElegantOTA
// Graphics Oled 		4.3.0	// https://github.com/ThingPulse/esp8266-oled-ssd1306  
// OneWire      		2.3.7	// https://github.com/PaulStoffregen/OneWire
// TMCStepper 			0.7.3   // https://github.com/teemuatlut/TMCStepper


// ----------------------------------------------------------------------
// IMPORTANT
// ----------------------------------------------------------------------
// Set the Preferences of Arduino IDE, for Compiler Warnings: to Default


// ----------------------------------------------------------------------
// DEFAULT CONFIGURATION
// ----------------------------------------------------------------------
// The following are NOT defined in firmware. When the controller firmware 
// is generated (by the Arduino IDE) and uploaded to the focus controller, 
// most of these default devices/services/servers are in a disabled/not 
// started state.
//
// To use them you need to login to the Management Server and enable/start
// ----------------------------------------------------------------------
// ASCOM Remote Server    [Port 4040]	[Off] [Stopped]
// Management Server      [Port 6060]	[On ] [Running]
// TCPIP Server           [Port 2020]	[On ] [Running]
// Web Server             [Port 80]		[Off] [Stopped]
// Joysticks							[Off]
// In-Out LED's           [Off]
// Push Buttons           [Off]
// Temperature probe      [Off]


// ----------------------------------------------------------------------
// Extra features not listed in controller_config.h
// Accessible from Management Server
// ----------------------------------------------------------------------
// Backlash, In and Out, State=disabled, Steps=0
// Board Configuration, specify pins, change pins etc [Severity Warning: MASSIVE]
// Coil Power, State=disabled, Timeout=120s
// DelayAfterMove, State=disabled, Time interval=25ms
// Display, State=disabled, Text Display Page Time=2, Page Options=all, Show pos when moving=enabled
// DuckDNS, refresh time=120s
// Focuser Presets, values=0
// Home Position Switch, State=disabled, Show HPSW messages=disabled
// In-Out LEDs, State=disabled, Mode=Pulse
// Motor Speed=fast, Delay [values for fast, medium and slow]
// Reverse Direction, State=disabled
// Step mode [Full]
// Stepsize, State=disabled, Step size in microns=50.0
// Temperature Compensation, State=disabled, Direction, Coefficient=0
// Temperature probe, State=disabled, Resolution-0.25c, Mode (C or F)
// TMC2209 Current [600] and TMC2205 Current [300]
// TMC2209 Stall guard [100]
// TMC2209 and TMC2225 Speed [57600]
// Web Page Colors, Background, text, header, title


// ----------------------------------------------------------------------
// TASK TIMER
// ----------------------------------------------------------------------
// The task timer runs the state engines for the focuser, with a resolution
// of 100ms (1 time-slice). Delays are expressed as a number of time-slices
// and each device/option has its own time-slice value. Some time-slices are
// dynamic and can be changed when the controller is running.
// 
// The task timer is responsible for scheduling the reading/servicing of 
// push buttons, joysticks, ir-remote, display, temp probe, duckdns, saving
// of config files, monitoring of WiFi connection state, and Park (which also
// handles oled state and coilpower off).
//
// Flags are used for sychronisation
//		-1	ignore flag
//		0   starts task timer counter, adds 1 for each time slice
//		1   task counter reached maximum value
//
// How it all works
// The device flag is initialised to -1 (stops the task timer from counting for that flag)
// The device sets the flag to 0 when the device is successfully started
// The task timer updates the counter value for the device, adding 1 for each time slice
// The device monitors (polls periodically) the flag for a change to 1
// The task timer sets the flag to 1 when the device counter reaches the max count value
// The device, when flag = 1
//		Resets the flag to 0 to begin the next cycle
//		Services/Updates the device
// Flags are protected by mutex's
//
// When a device is stopped or disabled, the flag is set to -1 which prevents the task 
// timer from counting time slices for that device.

// ----------------------------------------------------------------------
// Using the Board Config within Management Server
// Strongly suggest you do NOT do this.
// ----------------------------------------------------------------------
// Pin designations, including I2C pins
// Step Mode and Fixed Step Mode
// Steps per revolution
// Not for the faint of heart
// Modifying the board config can render your controller non-operational


// ----------------------------------------------------------------------
// I programmed the controller but the ASCOM ALPACA SERVER is not running
// ----------------------------------------------------------------------
// As stated above the default state is OFF
//
// To start the ASCOM server, open a web browser and access the Management 
// server on port 4040
//
// On the Servers page, it will show the ASCOM Server as stopped.
// Click the Start button to start the server. The state is saved in 
// the controllers settings, so next time the controller reboots, the
// ASCOM server will be still enabled, thus the controller will start
// the server at boot time.

// Same applies for
// Web Server
// In-Out LEDs
// Push Buttons
// Joysticks
// Temperature Probe
// DuckDNS


//---------------------------------------------------------------------
//Nomenclature
//---------------------------------------------------------------------
// V_xxxxx    Variable
// T_xxxxx    Text String
// H_xxxxx    HTML code


//---------------------------------------------------------------------
// Helper Functions
//---------------------------------------------------------------------
// These are found in the main .ino file and provide a means to start/stop
// a class and access its members 
//
// Issue: 
// As some classes are related to defines in Controller_Config.h they 
// would only exist if the class was enabled.
// 
// An example is the Display, which is only included IF defined. Another 
// class trying to access the Display need not know if the display was 
// enabled, loaded and available.
//
// The helper function hides the details from the calling class, and only
// lets the call proceed if the display class is available. So we can
// hide all the details and use of #if defined() statements inside the
// helper function, removing the need for that from elsewhere in the code
// or other classes.
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// DEBUG SUPPORT
//---------------------------------------------------------------------
// Localised for each class
// Init, Info, Warning, Error groups inserted into code
// In general, warnings = recoverable
// All messages are based on codes, WSV001, 
//      where WSV is Web Server, 001=create class
