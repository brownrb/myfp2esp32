/*

Please do not delete this file
This file is maintained using Sublime Text

---------------------------------------------------------------------
MYFP2ESP32-firmware-changes.txt
---------------------------------------------------------------------
myFP2ESP32 Focus Controller
© Copyright Robert Brown 2014-2022. All Rights Reserved.
© Copyright Holger M, 2019-2021. All Rights Reserved.
© Copyright Paul P, 2021-2022. All Rights Reserved.
23 November 2022
---------------------------------------------------------------------


---------------------------------------------------------------------
KNOWN ISSUES
---------------------------------------------------------------------



---------------------------------------------------------------------
304 23-11-2022
---------------------------------------------------------------------
Prevent WiFi sleep mode
Remove duplicated code when loading admin/web pages
Fix temperature resolution not set correctly
Fix for titlecolor being overwritten by textcolor


---------------------------------------------------------------------
303 31-10-2022
---------------------------------------------------------------------
Add sub-title color to html pages (Management Server, Controller_Data)
Add check to stop display if display is running and user disables display
When display is disabled without first stopping, clear display
Remove Serial.prints


---------------------------------------------------------------------
302 31-10-2022
---------------------------------------------------------------------
fix for display update_position not showing focuser position correctly
fix for display not starting correctly


---------------------------------------------------------------------
301 30-10-2022
---------------------------------------------------------------------
fix for temperature probe


---------------------------------------------------------------------
300 19-09-2022
---------------------------------------------------------------------
Add extra calls to TCP/IP server. myFP2 Comms Protocol v250.pdf 
	Enable/Disable, Start/Stop ASCOM ALPACA Server, Management Server, Web Server
Add 10s refresh page to H_ISMOVINGPG, Management Server
Add new Focuser State, State_EndMove to handle cleanup when move has ended
Add code to release Web Server if caching of web pages fails
Add set coilpower state (0,1) to TCP/IP Server
Add coilpowerstate, On or Off, to Management Server and Web Server pages
Add /cp XHTML to Web Server, coil power state refreshes automatically
Add /pa XHTML to Web Server, park state refreshed automatically
Add Park state Enabled / Disable
Add Park status
Add Park enable state and Park status to Management Server and Web tcpip_server
Move define for OTAID to defines/otaupdates_define.h 
Rewrite Pushbutton, Joystick1, Joystick2 enable code (mutually exclusive)
Rewrite Park code
Rewrite Park and Oled_State, moved to ControllerData as dynamic values only
Rewrite Web Server pages using combined post/get handler
Fix tasktimer error following a motor move action 
Fix for Controller saving of config files
Fix for Coilpower release when Park time interval expires
Fix for Display on-off when Park time interval expires

---------------------------------------------------------------------
250-19
---------------------------------------------------------------------
Redesign Management Server admin pages including navigation buttons
Fix error in start/stop servers (management server) 
Fix devicename not showing for Web page notfound.html
Fix page display option :93 tcpip_server.cpp, controller_data.cpp
Fix parktime value not set correctly (tcpip_server.cpp)
Fix for Joystick2 Sw state (driver_board.cpp)

---------------------------------------------------------------------
250-18
---------------------------------------------------------------------
Add Park option to Web server home page
Add reload pages from SPIFFS to local storage for Web Server
Edit webserver pages to use DeviceName in place of myFP2ESP32
Rewrite ControllerData saves of config files, now managed by the task timer
Move save config files to options round robin queue
Fix for DelayAfterMove being applied at end of move, even when disabled
Fix for Home Position Switch messages not generated when enabled


---------------------------------------------------------------------
250-17
---------------------------------------------------------------------
Delete enable motor from movemotor() 
Move Web Server pages into memory when Web Server starts
Remove most of debug code, flag only errors and warnings
Fix Changing stepmode then coil power disabled (Pierre)
Fix Names in Management server (Eric)
Fix Display not updating after boot is finished (Eric)
Fix Page Display option in TCPIP server
Fix Display pos when moving cannot enable (Management Server DISPLAY)
Fix Controller crashing when using Web Server to move the focuser (Eric)


---------------------------------------------------------------------
250-16		21-Aug-2022
---------------------------------------------------------------------
Change TMC2209 support so that options can be changed on the fly
Fix error in case Option_WiFi


---------------------------------------------------------------------
250-15		15-Aug-2022
---------------------------------------------------------------------
Add code (driver_board.cpp) to get_joystick2_swstate() to reset the switch state
Add HPSW settings and Stall Guard settings for Management Server page /admin7
Add WiFi reconnect if connection is lost (station mode only)
Move TMC define settings to /defines/tmcstepper_defines.h
Remove constants for joystick pins from driver_board.cpp (get from ControllerData)
Fix Stepping error (some boards - moves completed in virtually 0 time)
Fix Error returning incorrect Backlash Out Enable state (tcpip_server.cpp)
Fix Backlash being applied when TMC2209 and Stall Guard is activated


---------------------------------------------------------------------
250-14		06 August 2022
---------------------------------------------------------------------
Transistion code to classes
main .ino file	
	Implement two state machines, 1 handles Options, other handles focuser states
	Add cached values to speed up management/web server page builds
	Add dedicated task timer for handling of option states (display, temp etc)
	Move option checks out of focuser states and place in new Options handler
	Add helper functions for classes that require to be defined in controller_config.h
	Move server checks from loop() into check_servers();
	Move options checks from loop() into check_options()
	Move Spiffs update to be scheduled under Option States
	Significant reduction in use of conditional defines, now mainly in helper functions
	Reduce text defines
	Deprecate SERIAL support
	Deprecate Moonlite protocol support

DEFINES
	New defines folder, Move servers/devices settings into a defines.h file

ASCOM ALPACA SERVER
	Improve gui for index and setup pages
	Add step size enable/disable and step size value to setup page
	Fix when enabled in ControllerData, does not load at boot
	Fix for ASCOM ALPACA server setup, buttons do not work
	Fix for Ascom ALPACA server failing Conform

CONTROLLERDATA CLASS
	Add DuckDNS, Pushbuttons, Joysticks, Park, parktime
	Add devicename, ota-name, ota-password, ota ID
	Clean up orphaned board configuration code
	Remove sda / sck from Board configs (now in controller_defines.h)
	Fix config files not being saved
	Fix for duplicate motorspeed setting in configuration file

DISPLAY CLASS
	Define shared interface for Text/Grahic/Future displays
	Iext_display and Graphic_display in separate classes (same interface)

ELEGANT OTA
	Add Elegant OTA to Management Server
	Add OTA name, password, ID to Management Server and ControllerData class

DRIVER BOARD CLASS
	Add LEDs option move or pulse
	Add Push Buttons steps (the step count value defines the number of steps
			to move for 1 button press, limit of 1/2 the step size value)
	Add admin pgs (Management server) to manage, select, edit and show board 
			configs
	Add hpsw/stall guard choices for TMC2209, via Management Server
	Move Joysticks to Driver Board class, included by default
	Move Pushbuttons to Driver Board class, included by default
	Remove all SDA/SCK info from board config files
	Fix board config errors: maxstepmode, stepsperrev, fixedstepmode
	Fix for PRO2ESP32ST6128 motor moving issue 

DUCKDNS CLASS
	Convert DUCKDNS to a class
	Add helper functions for duckdns, start(), refresh time()
	Add DUCKDNS domain, token to Management server and ControllerData

I2C
	I2CDATAPIN, I2CCLKPIN defined in controller_defines.h

MANAGEMENT SERVER CLASS
	Add admin pgs to manage, select, edit and show board configs
	Add authentication to all admin pages
	Add ElegantOTA service
	Add devicename, OTA-name, OTA-password, OTA ID to /admin6
	Add and expand get/set interface
	Add display enable/disable, display start/stop to json get/set handlers 
	Add duckdns domain, token, refresh time to /admin5
	Add fail page
	Add file list formats (short/long)
	Add management of Park state/time, InOut LEDs state/mode
		Park can be enabled/disabled, and duration set by Park Time
	Add Pushbuttons state/steps and Joysticks
	Add Home Position Switch state/msgs
	Add Display settings (checkboxes for page options)
	Add stepsize enable/stepsize to /admin4
	Add temperature page to handle all temperature settings
	Add IN-OUT LED mode, pulse (every step) or move (stays on when moving)
	Change to ftargetposition, maxsteps, presets etc - now all long 
		( changed from unsigned long )
	Improve GUI layout - format all pages using tables, button sizes
	Include OTA/DuckDNS status to /admin1
	Increase arbitary focuser limit
	Rename Navigation buttons at bottom of admin pages
	Rename Page titles, can be customised
	Fix errors in /get? and /set? JSON interfaces

MDNS
	Deprecated

TASKTIMER
	Add task timer to handle state machines for options/devices/sensors

TEMP PROBE CLASS
	All settings grouped on single Management Server page

TCPIP SERVER CLASS
	Replaces old comms
	Add support calls for LEDs state, mode (pulse or move)
	Add support calls for Park state, time
	Add support for Pushbuttons state, steps
	Add support for DelayAfterMove state
	Fix bug in additional chars being received from port 2020
	Fix bug in 05 goto position command
	Fix for Case 4 not sending correct response

WEB SERVER CLASS
	Improve GUI for Index, Move and Presets pages
	Add halt button to move page
	Fix move page errors
	Fix presets page errors, values not saved, buttons not working
	Fix issue of webserver being able to view admin files
	Fix start error when enabled and controller reboots
	Fix for not found error
	Fix for temp resolution not updating
	Fix for temp mode not changing between C/F
	Fix for step mode not saving
	Fix for maxsteps not saving
	For for values not updating on presets page when a Goto preset occurs
	For for values not updating on home page when focuser is moving
	Fix for coilpower/reverse/motorspeed unable to change state
	Fix for Goto and Set position buttons not always working


---------------------------------------------------------------------
Branch 231 and below deprecated
---------------------------------------------------------------------
