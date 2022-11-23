// ----------------------------------------------------------------------
// myFP2ESP32 GENERAL DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// controller_defines.h
// ----------------------------------------------------------------------

#include <Arduino.h>
#include "controller_defines.h"

// ----------------------------------------------------------------------
// NOTE
// ----------------------------------------------------------------------
// Consider this
//     BOARD_println("DrvBrd:start: INOUT LEDs enabled");
// The text is only ever included IF the BOARD_println is defined (default is NOT defined)
// This reduces program size as the text is not included

// Case 1: BOARD_print is defined
// The text is placed into program memory space by the compiler, it does not require (F(text))

// Case 2: Convert text to char *
// If the text is made a char *, then it is included every single time (program size increases)
// Conclusion: As the BOARD_println() is normally undefined then it is best to leave it as is

const char *filesysnotloadedstr = "filesystem not loaded";
const char *TEXTPAGETYPE        = "text/html";
const char *PLAINTEXTPAGETYPE   = "text/plain";
const char *JSONTEXTPAGETYPE    = "text/json";
const char *JSONPAGETYPE        = "application/json";

const char *H_FILENOTFOUNDSTR   = "<html><head><title>myFP2ESP32</title></head><body><p>myFP2ESP32</p><p>File not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>";
const char *H_FSNOTLOADEDSTR    = "<html><head><title>myFP2ESP32</title></head><body><p>myFP2ESP32</p><p>err: File-system not started.</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>";
const char *H_ISMOVINGPG		= "<html><meta http-equiv=refresh content=\"10; url=/\"><head><title>myFP2ESP32</title><body><p>myFP2ESP32</p><p>Focuser is moving, please wait. Will retry in 10s.</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>";

const char *DEFAULTTITLECOLOR    = "8E44AD";
const char *DEFAULTSUBTITLECOLOR = "3399FF";
const char *DEFAULTHEADERCOLOR   = "2B65EC";
const char *DEFAULTTEXTCOLLOR    = "5D6D7E";
const char *DEFAULTBACKCOLOR     = "333333";

const char *T_CONTROLLERMODE    = "Controller mode ";
const char *T_ACCESSPOINT       = "ACCESSPOINT";
const char *T_STATIONMODE       = "STATIONMODE";
const char *T_OFF               = "OFF";
const char *T_ON                = "ON";
const char *T_DISABLED          = "Disabled";
const char *T_ENABLED           = "Enabled";
const char *T_CELSIUS           = "Celsius";
const char *T_FAHRENHEIT        = "Fahrenheit";
const char *T_FOUND             = "found";
const char *T_NOTFOUND          = "err not found";
const char *T_NO                = "No";
const char *T_YES               = "Yes";
const char *T_STOPPED           = "Stopped";
const char *T_RUNNING           = "Running";
