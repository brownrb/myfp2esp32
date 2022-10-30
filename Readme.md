# myFP2ESP32 Firmware for ESP32 Focus Controllers
This is the firmware for the myFocuserPro2ESP32 focus controller projects on Sourceforge.  

(c) Robert Brown 2019-2022  
(c) Holger Manz 2020-2021  
(c) Paul Porters 2021-2022  
All rights reserved.  


# DOCUMENTATION
To get a clear understanding of myFP2ESP32 features and options, it is essential to read the PDF.  
https://sourceforge.net/projects/myfocuserpro2-esp32/files/Documentation/ 


# QUICK NOTE
The firmware generates a Default Configuration based on the DRVBRD and controller_config.h settings. The default configuration is uploaded to the controller when you "program" the controller using the Arduino IDE, followed by uploading the data sketch files. On the reboot following the upload of data sketch files, the board configuration is created.  


# Default Configuration
The devices/options/servers _included_ (and thus are not enabled in the controller_config.h file) are 
- ASCOM ALPACA Server
- Management Server  (enabled and running)
- TCPIP Server  (enabled and running)
- WEB Server
- Push Buttons
- Joysticks
- Temperature probe

Most of these will be _disabled_ or _stopped_ on the first reboot following an upload of the firmware and data sketch files upload. Use the Management Server to enable or start the devices/servers that you need. Once enabled, the state will be saved, and on subsequent reboots, you do not need to enable them again.


# APPLICATIONS
The Windows applications are found on Sourceforge.  
https://sourceforge.net/projects/myfocuserpro2-esp32/files/Windows%20Application/  

The Linux application and source code is found on Sourceforge.  
https://sourceforge.net/projects/myfocuserpro2-esp32/files/Linux%20Application/  

The ASCOM drivers are found on Sourceforge.  
https://sourceforge.net/projects/myfocuserpro2-esp32/files/ASCOM%20Drivers/  


# TESTED WITH
- myFP2ESP32W Windows application 1_2_5_2
- myFP2ESP32ASCOM_300 ASCOM driver
- myFP2 INDI driver *using Network* 
- myFP2ESP32 Linux application 1.1.10.1
- Web Browser on multiple hosts, Android tablet/phone
- Android myFP2E application 2019
- ASCOM Remote client to ASCOM ALPACA Server running on myFP2ESP32 controller


# PCB GERBER FILES  
The Gerbers files for the PCB are found on Sourceforge.  
https://sourceforge.net/projects/myfocuserpro2-esp32/files/PCB%20Gerber%20Files/  


# Compiling for ESP32
Compiling the source code requires Arduino IDE **v1.8.19** with the ExpressIF ESP32 Arduino extensions (Core 2.0.4). You will need to add the JSON file for the ESP32 library by using the File->Preferences menu of the 
Arduino IDE and add the location for the library into the board manager
https://dl.espressif.com/dl/package_esp32_index.json  

Once specified, open the board manager, scroll down to ESP32 and install the latest version 2.0.4 (at the time of writing). Once the esp32 core is added, you can specify the target board as **ESP32 Dev** with Flash Size set to 4M (1MB SPIFFS) and upload speed of 115200.


# Arduino Compiler Option  
In File Preferences, set the Compiler Warnings to **Default**


# Libraries
To compile you will also need to import these libraries in the folder libraries-files into the Arduino IDE environment using the Menu - Sketch - Include Library - Add .Zip file

* myOLED
* myHalfStepperESP32
* myDallasTemperature
* myfp2eIRremoteESP8266  

Do not edit or replace any of these library files with others.


# Additional Libraries which must be downloaded and installed
To compile this firmware you need to ensure that you have installed a number of "library" files into the Arduino environment. For each of the links below, you need to DOWNLOAD the ZIP file first.

OneWire
https://github.com/PaulStoffregen/OneWire  

Arduino JSON
https://github.com/bblanchon/ArduinoJson 

DuckDNS
https://github.com/ayushsharma82/EasyDDNS  

ElegantOTA
https://github.com/ayushsharma82/ElegantOTA

ESP32 Sketch Data uploader
https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/  

Graphics OLED
https://github.com/ThingPulse/esp8266-oled-ssd1306  

TMC2209-TMC2225 requires Library TMCStepper 
https://github.com/teemuatlut/TMCStepper

After downloading the zip files, you then need to install these into the Arduino IDE environment. To do that, start the Arduino IDE, select Sketch, Include Library, Add .Zip library, then navigate to where the ZIP files are stored and select them (you can only load 1 zip file at a time so please repeat this for all ZIP files).

The ESP32 Sketch Data uploader require a different method for installing. The install instructions is found in the respective download pagesand require copying the files into a special folder of the Arduino program installation. After copying the uploader files, close the Arduino IDE (if open) and then restart of the Arduino IDE for the changes to take effect.

Once you have done this, you can start programming the controller.


# controller_config.h
Configuration information about the controller is specified in the *controller_config.h* file. This config information, when combined with the DRVBRD selection, is used to create a "default" configuration. The Management Server provides the ability to change, enable/disable devices and servers once the controller has been programmed.


# controller_defines.h
This file contains general definitions for all controllers, such as motor speed, step modes, focuser limits etc. DO NOT MODIFY THESE FILES.  


# Controller Modes
The controller supports the modes ACCESSPOINT or STATIONMODE. The serial connection is not needed, and is used for diagnostic messages (when enabled).  


# ControllerData.cpp and .h files
This class handles focuser data, saving, restoring and updating of controller settings.  


# DRIVER BOARDS
There is one constructor for this class which accepts the boardtype, the user should never have to change the main code within the firmware file which sets up the driver boards. All pins mappings for the controller chip, hardware options and the driver board are specified here. DO NOT MODIFY THESE FILES.  

The user must set the **DRVBRD** at the beginning of the file [controller_config.h] to the correct driver board.  


## myHalfStepperESP32 Library
This library manages the stepping modes for the ULN2003, L298N, L293DMINI and L9110S driver boards. It is a modified version of the HalfStepper library from Tom Biuso. DO NOT MODIFY THESE FILES. It has been modified to work with ESP32 chips.


## driver_board.cpp and .h files
This driver board class supports the following driver boards DRV8825, ULN2003, L298N, L293D-Mini, L9110S, ESP32R3WEMOS, TMC2209, TMC2205, ST6128. DO NOT MODIFY THESE FILES.    


## ULN2003-L298D-L293DMini-L9110S boards  
Require the myHalfStepperESP32 custom Library (provided).  


# Input Devices
Most devices can be enabled/disabled using the Management server (and do not appear in the controller_config.h file). Some devices cannot be enabled if a shared pin is used by another device that is already enabled. An example is Push buttons, which cannot be enabled if a joystick is enabled. The Management server is used to enable/disable devices, and will prevent a change if any shared pins are referenced.

The following devices are supported  
## Push buttons
## Joysticks
## Infra-red remote controller (requires provided library myfp2eIRremoteESP8266)
## Temperature probe (requires provided library myDallasTemperature)  

The myFP2ESP32 controller supports temperature compensation. The libray for the temperature probe is included (myDallasTemperature) as part of the default configuration. The probe can be enabled/disabled using the Management Server.  
 

# Displays
There are two types of displays (TEXT and GRAPHICS). OLED's can also have two different Driver Chip types (SSD1306 or SSH1106).  


## TEXT display requires myOLED library  
This is a special cut down version of the SSD1306AsciiWire library by Bill Greiman. The library has been reduced in size and a number of additional features added. DO NOT MODIFY THESE FILES.  

## Graphic Display  
Requires 3rd party library  4.3.0 https://github.com/ThingPulse/esp8266-oled-ssd1306  


# COMPILE ENVIRONMENT : Tested with 
* Arduino IDE 2.0.0
* ESP32 Core 2.0.5
Libraries  
* Arduino JSON 6.19.4 https://github.com/bblanchon/ArduinoJson.git  
* myOLED for text display as in Library Files
* OLED for graphics display 4.3.0 https://github.com/ThingPulse/esp8266-oled-ssd1306
* myHalfStepperESP32 as in Library Files
* myDallas Temperature as in Library Files
* Wire (as installed with Arduino 1.8.13)
* OneWire 2.3.7 http://www.pjrc.com/teensy/td_libs_OneWire.html  
* EasyDDNS 1.8.0 (supports DuckDNS) https://github.com/ayushsharma82/EasyDDNS
* ElegantOTA 2.2.9 https://github.com/ayushsharma82/ElegantOTA


# Notes:  
You may need to turn 12V off to reprogram chip. Speed is 115200. Sometimes you might need to remove the chip from PCB before re-programming new firmware. 
