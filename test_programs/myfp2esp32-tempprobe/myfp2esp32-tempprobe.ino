// myFP2ESP32 FIRMWARE OFFICIAL RELEASE 300
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// © Copyright Pieter P, SPIFFs examples found online
// © Copyright Paul P, 2021-2022. All Rights Reserved. TMC22xx code
//   myfp2esp32-firmware.ino
//   version: 300
//   date:    19-09-2022
// ----------------------------------------------------------------------
//
// TEST PROGRAM TEMP PROBE
// OUTPUT: Open serial port monitor, speed=115200
// Information messages are output to the serial port
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>
#include "controller_defines.h"
#include <Wire.h>


// ----------------------------------------------------------------------
// TEMPERATURE PROBE
// Library  myDallasTemperature
// Default Configuration: Included
// ----------------------------------------------------------------------
#include "temp_probe.h"
TEMP_PROBE *tempprobe;
byte tempprobe_status;                        // V_STOPPED
float temp;                                   // the last temperature read


//-------------------------------------------------
// void load_vars(void);
// Load cached vars, gives quicker access for web pages etc
// when any cached variable changes, it is reloaded
//-------------------------------------------------
void load_vars()
{
  // temperature probe
  temp = 20.0;
  tempprobe_status = V_STOPPED;
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

  delay(1000);
  load_vars();

  Serial.println("Starting test for temperature probe");


  //-------------------------------------------------
  // SETUP TEMPERATURE PROBE
  // Included by default
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
  // create the pointer to the class
  tempprobe = new TEMP_PROBE(TEMPPIN);   // expects a pin number
  // initialise the probe
  tempprobe_status = tempprobe->init();
  if ( tempprobe_status == true )
  {
    // check if temperature probe is loaded at boot time
    // probe is enabled, so start the probe
    Serial.println("Start temperature probe");
    tempprobe_status = tempprobe->start();
    if ( tempprobe_status == V_RUNNING )
    {
      Serial.println("Reading probe");
      temp = tempprobe->read();
      Serial.print("Temperature = ");
      Serial.println(temp);
    }
    else
    {
      Serial.println("start temp probe error");
    }
  }
}

void loop()
{
  delay(2000);
  temp = tempprobe->update();
  Serial.print("Temperature: ");
  Serial.println(temp);
}
