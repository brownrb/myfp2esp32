// ----------------------------------------------------------------------
// myFP2ESP32 FOCUSER CONFIGURATION CLASS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// controller_data.cpp
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------
#include <Arduino.h>
#include "controller_config.h"                // includes boarddefs.h and controller_defines.h
#include <ArduinoJson.h>
#include "SPIFFS.h"

// DEFAULT CONFIGURATION
// ControllerData
// Controller Persistant Data  cntlr_config.jsn
// Controller Variable Data    cntlr_var.jsn
// Controller Board Data       board_config.jsn


// -----------------------------------------------------------------------
// DEBUGGING
// -----------------------------------------------------------------------
// DO NOT ENABLE DEBUGGING INFORMATION.

// Remove comment to enable messages to Serial port
//#define CNTLRDATA_PRINT       1

// -----------------------------------------------------------------------
// DO NOT CHANGE
// -----------------------------------------------------------------------
#ifdef  CNTLRDATA_PRINT
#define CNTLRDATA_print(...)   Serial.print(__VA_ARGS__)
#define CNTLRDATA_println(...) Serial.println(__VA_ARGS__)
#else
#define CNTLRDATA_print(...)
#define CNTLRDATA_println(...)
#endif


// ----------------------------------------------------------------------
// CONTROLLER CONFIG DATA
// ----------------------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;


// ----------------------------------------------------------------------
// DRIVER BOARD DATA
// ----------------------------------------------------------------------
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;


// ----------------------------------------------------------------------
// OTA DATA
// ----------------------------------------------------------------------
extern const char *OTAName;                       // the username and password for the ElegantOTA service
extern const char *OTAPassword;
extern const char *OTAID;
#include "defines/otaupdates_defines.h"


// ----------------------------------------------------------------------
// TMC STEPPER DEFINES
// ----------------------------------------------------------------------
#include "defines/tmcstepper_defines.h"


// ----------------------------------------------------------------------
// EXTERNS
// ----------------------------------------------------------------------
extern int  myboardnumber;
extern int  myfixedstepmode;
extern int  mystepsperrev;
extern char duckdnsdomain[];
extern char duckdnstoken[];
extern char devicename[];
extern enum Display_Types displaytype;
extern bool isMoving;
extern bool filesystemloaded;                     // flag indicator for file usage, rather than use SPIFFS.begin() test

// task timer
extern volatile int save_var_flag;
extern volatile int save_board_flag;
extern volatile int save_cntlr_flag;
extern portMUX_TYPE varMux;
extern portMUX_TYPE boardMux;
extern portMUX_TYPE cntlrMux;

extern unsigned int display_maxcount;
extern portMUX_TYPE displaytimeMux;


// ----------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------
#define DEFAULT_0           0
#define DEFAULT_1           1
#define DEFAULTOFF          0
#define DEFAULTON           1
#define DEFAULTCELSIUS      1
#define DEFAULTFAHREN       0
#define DEFAULTDOCSIZE      2200                  // Arduino JSON assistant Total (recommended)  1536 Deserialize 2048
#define DEFAULTVARDOCSIZE   64
#define DEFAULTBOARDSIZE    2000
#define DEFAULTPOSITION     5000L
#define DEFAULTMAXSTEPS     80000L


// ----------------------------------------------------------------------
// CONTROLLER_DATA CLASS
// ----------------------------------------------------------------------
CONTROLLER_DATA::CONTROLLER_DATA(void)
{
  CNTLRDATA_println("CONTROLLER_DATA::CONTROLLER_DATA");
  // task timer has not started yet, so this is safe
  save_var_flag   = -1;
  save_board_flag = -1;
  save_cntlr_flag = -1;

  // mount SPIFFS
  if (!SPIFFS.begin())
  {
    ERROR_println("cd: FS start error, formatting..");
    SPIFFS.format();
    filesystemloaded = false;
  }
  else
  {
    CNTLRDATA_println("cd: FS start mounted");
    filesystemloaded = true;
  }
  LoadConfiguration();
};

// ----------------------------------------------------------------------
// Loads the configuration from files (cntlr, board, var)
// If configuration files are not found, or cannot be deserialised, then
// create Default Configurations for each
// ----------------------------------------------------------------------
bool CONTROLLER_DATA::LoadConfiguration()
{
  CNTLRDATA_println("cd: LoadConfiguration() start");
  // LOAD CONTROLLER PERSISTANT DATA
  CNTLRDATA_print("cd: LoadConfiguration: CONTROLLER: ");
  CNTLRDATA_println(file_cntlr_config);

  // Focuser persistant data - Open cntlr_config.jsn file for reading
  if ( SPIFFS.exists(file_cntlr_config) == false )
  {
    CNTLRDATA_println("cd: cntlr_config.jsn file not found, create default config file");
    LoadDefaultPersistantData();
  }
  else
  {
    // file exists so open it
    File cfile = SPIFFS.open(file_cntlr_config, "r");
    String cdata;                                 // cntlr_config.jsn controller persistant data
    cdata.reserve(2400);                          // Controller data, ArduinoJson Assistant 1699, 2048

    cdata = cfile.readString();
    cfile.close();
    CNTLRDATA_print("cd: data: ");
    CNTLRDATA_println(cdata);

    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc_per(DEFAULTDOCSIZE);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_per, cdata);
    if ( error )
    {
      ERROR_println("cd: deserialise error. Create default cntlr config");
      LoadDefaultPersistantData();
    }
    else
    {
      // maxstep
      this->maxstep = doc_per["maxstep"];
      // presets
      for (int i = 0; i < 10; i++)
      {
        this->focuserpreset[i] = doc_per["preset"][i];
      }
      // SERVERS - SERVICES
      this->ascomsrvr_enable  = doc_per["ascom_en"];
      this->ascomsrvr_port    = doc_per["ascom_port"];
      this->mngsrvr_enable    = doc_per["mngt_en"];
      this->mngsrvr_port      = doc_per["mngt_port"];
      this->tcpipsrvr_enable  = doc_per["tcp_en"];
      this->tcpipsrvr_port    = doc_per["tcp_port"];
      this->websrvr_enable    = doc_per["ws_en"];
      this->websrvr_port      = doc_per["ws_port"];
      this->duckdns_enable    = doc_per["ddns_en"];
      this->duckdns_domain    = doc_per["ddns_d"].as<const char*>();
      this->duckdns_token     = doc_per["ddns_t"].as<const char*>();
      this->duckdns_refreshtime = doc_per["ddns_r"];
      this->ota_name          = doc_per["ota_name"].as<const char*>();
      this->ota_password      = doc_per["ota_pwd"].as<const char*>();
      this->ota_id            = doc_per["ota_id"].as<const char*>();
      // DEVICES
      // display
      this->display_enable    = doc_per["d_en"];
      this->displaypagetime   = doc_per["d_pgtime"];
      this->displaypageoption = doc_per["d_pgopt"].as<const char*>();
      this->displayupdateonmove = doc_per["d_updmove"];          // update position on display when moving
      // hpsw
      this->hpswitch_enable   = doc_per["hpsw_en"];
      this->hpswmsg_enable    = doc_per["hpswmsg_en"];
      this->stallguard_state  = doc_per["stall_st"];
      this->stallguard_value  = doc_per["stall_val"];
      this->tmc2225current    = doc_per["tmc2225mA"];
      this->tmc2209current    = doc_per["tmc2209mA"];
      // leds
      this->inoutled_enable   = doc_per["led_en"];
      this->inoutledmode      = doc_per["led_mode"];
      // joysticks
      this->joystick1_enable  = doc_per["joy1_en"];
      this->joystick2_enable  = doc_per["joy2_en"];
      // pushbuttons
      this->pushbutton_enable = doc_per["pb_en"];
      this->pushbutton_steps  = doc_per["pb_steps"];
      // temperature probe
      this->tempprobe_enable  = doc_per["t_en"];
      this->tempcomp_enable   = doc_per["t_comp_en"];       // indicates if temperature compensation is enabled
      this->tempmode          = doc_per["t_mod"];           // temperature display mode, Celcius=1, Fahrenheit=0
      this->tempcoefficient   = doc_per["t_coe"];           // steps per degree temperature coefficient value
      this->tempresolution    = doc_per["t_res"];           // 9 - 12
      this->tcdirection       = doc_per["t_tcdir"];
      this->tcavailable       = doc_per["t_tcavail"];
      // backlash
      this->backlash_in_enable  = doc_per["blin_en"];
      this->backlash_out_enable = doc_per["blout_en"];
      this->backlashsteps_in  = doc_per["blin_steps"];        // number of backlash steps to apply for IN moves
      this->backlashsteps_out = doc_per["blout_steps"];
      // coil power
      this->coilpower_enable  = doc_per["cp_en"];
      // delay after move
      this->delayaftermove_enable = doc_per["dam-en"];
      this->delayaftermove_time = doc_per["dam_time"];
      // devicename
      this->devicename      = doc_per["devname"].as<const char*>();
      // file list format
      this->filelistformat  = doc_per["filelist"];
      // motorspeed
      this->motorspeed      = doc_per["mspeed"];              // motorspeed slow, med, fast
      // park
      this->park_enable     = doc_per["park_en"];
      this->park_time       = doc_per["park_time"];
      // reverse
      this->reverse_enable  = doc_per["rdir_en"];
      // stepsize
      this->stepsize_enable = doc_per["ss_en"];               // if 1, controller returns step size
      this->stepsize        = doc_per["ss_val"];              // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
      // web page colors
      this->titlecolor  = doc_per["ticol"].as<const char*>();
      this->subtitlecolor = doc_per["scol"].as<const char*>();
      this->headercolor = doc_per["hcol"].as<const char*>();
      this->textcolor   = doc_per["tcol"].as<const char*>();
      this->backcolor   = doc_per["bcol"].as<const char*>();

      CNTLRDATA_println("cd: cntlr_config.jsn loaded OK");
    }
  }

  // LOAD CONTROLLER BOARD DATA
  CNTLRDATA_println("cd: LoadConfiguration(): BOARD");
  if ( SPIFFS.exists(file_board_config) == false )
  {
    CNTLRDATA_println("cd: board_config.jsn file not found, create default board config file");
    LoadDefaultBoardData();
  }
  else
  {
    // file exists so open it
    File bfile = SPIFFS.open(file_board_config, "r");
    String bdata;                                 // board_config.jsn board data
    bdata.reserve(1024);                          // Board data, ArduinoJson Assistant 384

    bdata = bfile.readString();
    bfile.close();
    CNTLRDATA_print("cd: data: ");
    CNTLRDATA_println(bdata);

    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc_brd(DEFAULTBOARDSIZE);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_brd, bdata);
    if ( error )
    {
      ERROR_println("cd: deserialise error, create default board file");
      LoadDefaultBoardData();
    }
    else
    {
      /*
        { "board":"PRO2ESP32DRV8825","maxstepmode":32,"stepmode":1,"enpin":14,"steppin":33,
        "dirpin":32,"temppin":13,"hpswpin":4,"inledpin":18,"outledpin":19,"pb1pin":34,"pb2pin":35,"irpin":15,
        "brdnum":60, "stepsrev":-1,"fixedsmode":-1,"brdpins":[27,26,25,-1],"msdelay":4000 }
      */
      this->board         = doc_brd["board"].as<const char*>();
      this->maxstepmode   = doc_brd["maxstepmode"];
      this->stepmode      = doc_brd["stepmode"];
      this->enablepin     = doc_brd["enpin"];
      this->steppin       = doc_brd["steppin"];
      this->dirpin        = doc_brd["dirpin"];
      this->temppin       = doc_brd["temppin"];
      this->hpswpin       = doc_brd["hpswpin"];
      this->inledpin      = doc_brd["inledpin"];
      this->outledpin     = doc_brd["outledpin"];
      this->pb1pin        = doc_brd["pb1pin"];
      this->pb2pin        = doc_brd["pb2pin"];
      this->irpin         = doc_brd["irpin"];
      this->boardnumber   = doc_brd["brdnum"];
      this->stepsperrev   = doc_brd["stepsrev"];
      this->fixedstepmode = doc_brd["fixedsmode"];
      for (int i = 0; i < 4; i++)
      {
        this->boardpins[i] = doc_brd["brdpins"][i];
      }
      this->msdelay = doc_brd["msdelay"];                    // motor speed delay - do not confuse with motorspeed
      CNTLRDATA_println("cd: board_config.jsn loaded OK");
    }
  }

  // LOAD CONTROLLER VAR DATA : POSITION : DIRECTION
  // this uses stepmode which is in boardconfig file so this must come after loading the board config
  CNTLRDATA_println("cd: LoadConfiguration(): VAR");
  if ( SPIFFS.exists(file_cntlr_var) == false )
  {
    CNTLRDATA_println("cd: cntlr_var.jsn file not found, create default var file");
    LoadDefaultVariableData();
  }
  else
  {
    // file exists so open it
    File vfile = SPIFFS.open(file_cntlr_var, "r");
    String vdata;                                 // controller variable data (position, direction)
    vdata.reserve(512);                           // Position and Direction data, ArduinoJson Assistant 32

    vdata = vfile.readString();
    vfile.close();
    CNTLRDATA_print("cd: data: ");
    CNTLRDATA_println(vdata);

    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc_var(DEFAULTVARDOCSIZE);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_var, vdata);
    if ( error )
    {
      ERROR_println("cd: deserialise error, create default var file");
      LoadDefaultVariableData();
    }
    else
    {
      this->fposition = doc_var["fpos"];          // last focuser position
      this->focuserdirection = doc_var["fdir"];   // keeps track of last focuser move direction

      if ( displaytype == Type_Graphic )
      {
        // round position to fullstep motor position, holgers code
        // only applicable if using a GRAPHICS Display
        this->fposition = (this->fposition + this->stepmode / 2) / this->stepmode * this->stepmode;
      }
      CNTLRDATA_println("cd: cntlr_var.jsn loaded OK");
    }
  }
  return true;
}


// ----------------------------------------------------------------------
// Load Focuser Variable Data - Position and Direction
// ----------------------------------------------------------------------
void CONTROLLER_DATA::LoadDefaultVariableData()
{
  CNTLRDATA_println("cd: LoadDefaultVariableData(): Create a default cntlr_var file");
  this->fposition = DEFAULTPOSITION;              // last focuser position
  this->focuserdirection = moving_in;             // keeps track of last focuser move direction
  SaveVariableConfiguration();
}


// ----------------------------------------------------------------------
// Load Default Focuser Persistant Data Settings (when no config file exists eg; after an upload
// Creates a default config setting file and saves it to spiffs
// ----------------------------------------------------------------------
void CONTROLLER_DATA::LoadDefaultPersistantData()
{
  CNTLRDATA_println("cd: LoadDefaultPersistantData: Create a default cntlr_config file");
  this->maxstep = DEFAULTMAXSTEPS;
  for (int i = 0; i < 10; i++)
  {
    this->focuserpreset[i] = 0;
  }

  // Set the initial defaults for a controller
  // SERVERS - SERVICES
  this->ascomsrvr_enable    = V_NOTENABLED;
  this->ascomsrvr_port      = ASCOMSERVERPORT;        // 4040
  this->mngsrvr_enable      = V_ENABLED;
  this->mngsrvr_port        = MNGSERVERPORT;          // 6060
  this->tcpipsrvr_enable    = V_ENABLED;
  this->tcpipsrvr_port      = TCPIPSERVERPORT;        // 2020
  this->websrvr_enable      = V_NOTENABLED;
  this->websrvr_port        = WEBSERVERPORT;          // 80
  this->duckdns_enable      = V_NOTENABLED;
  this->duckdns_domain      = String(duckdnsdomain);  // from /defines/duckdns_defines.h
  this->duckdns_token       = String(duckdnstoken);
  this->duckdns_refreshtime = DUCKDNS_REFRESHRATE;    // 120s
  this->ota_name            = String(OTAName);        // from /defines/otaupdates_defines.h
  this->ota_password        = String(OTAPassword);
  this->ota_id              = String(OTAID);

  // DEVICES
  // display
  this->display_enable      = V_NOTENABLED;
  this->displaypagetime     = DEFAULTDISPLAYPAGETIMEMIN;   // integer 2, 3 -- 10
  this->displaypageoption   = "11111111";
  this->displayupdateonmove = V_ENABLED;
  // hpsw
  this->hpswitch_enable     = V_NOTENABLED;           // this should be default OFF
  this->hpswmsg_enable      = V_NOTENABLED;           // this should be default OFF
  this->stallguard_state    = Use_None;
  this->stallguard_value    = STALL_VALUE;            // from boarddefs.h
  this->tmc2225current      = TMC2225CURRENT;         // from boarddefs.h
  this->tmc2209current      = TMC2209CURRENT;         // from boarddefs.h
  // leds
  this->inoutled_enable     = V_NOTENABLED;           // this should be default OFF - if HW not fitted could crash
  this->inoutledmode        = LEDPULSE;
  // joysticks
  this->joystick1_enable    = V_NOTENABLED;           // this should be default OFF
  this->joystick2_enable    = V_NOTENABLED;           // this should be default OFF
  // pushbuttons
  this->pushbutton_enable   = V_NOTENABLED;           // this should be default OFF
  this->pushbutton_steps    = PUSHBUTTON_STEPS;
  // temperature probe
  this->tempprobe_enable    = V_NOTENABLED;
  this->tempcomp_enable     = V_NOTENABLED;           // temperature compensation disabled
  this->tempmode            = V_CELSIUS;              // default is celsius
  this->tempcoefficient     = DEFAULT_0;
  this->tempresolution      = DEFAULTTEMPRESOLUTION;  // 0.25 degrees
  this->tcdirection         = TC_DIRECTION_IN;        // temperature compensation direction
  this->tcavailable         = V_NOTENABLED;
  // backlash
  this->backlash_in_enable  = V_NOTENABLED;
  this->backlash_out_enable = V_NOTENABLED;
  this->backlashsteps_in    = DEFAULT_0;
  this->backlashsteps_out   = DEFAULT_0;
  // coil power
  this->coilpower_enable    = V_NOTENABLED;
  // delay after move
  this->delayaftermove_enable = V_NOTENABLED;
  this->delayaftermove_time = 25;                     // milliseconds to wait after move done
  // devicename
  this->devicename          = "myFP2ESP32 ";
  // file list format
  this->filelistformat      = LISTLONG;
  // motorspeed
  this->motorspeed          = FAST;
  // park
  this->park_enable         = V_NOTENABLED;
  this->park_time           = DEFAULTPARKTIME;
  // reverse
  this->reverse_enable      = V_NOTENABLED;
  // stepsize
  this->stepsize_enable     = V_NOTENABLED;
  this->stepsize            = DEFAULTSTEPSIZE;
  // web page colors
  this->titlecolor          = DEFAULTTITLECOLOR;
  this->subtitlecolor       = DEFAULTSUBTITLECOLOR;
  this->headercolor         = DEFAULTHEADERCOLOR;
  this->textcolor           = DEFAULTTEXTCOLLOR;
  this->backcolor           = DEFAULTBACKCOLOR;

  SavePersitantConfiguration();               // write default values to SPIFFS
}


// ----------------------------------------------------------------------
// Load Default Board Data Settings
// ----------------------------------------------------------------------
void CONTROLLER_DATA::LoadDefaultBoardData()
{
  CNTLRDATA_println("cd: LoadDefaultBoardData: Create a default board_config.jsn file");
  // we are here because board_config.jsn was not found
  // we can load the default board configuration from DRVBRD defined - DefaultBoardName in .ino file
  // Driver board data - Open the specific board config .jsn file for reading

  // cannot use this->boardnumber because the value has not been set yet
  // Load the board file from /boards, make up filename first
  String brdfile = "/boards/" + String(myboardnumber) + ".jsn";
  CNTLRDATA_print("cd: LoadDefaultBoardData() : loading file ");
  CNTLRDATA_println(brdfile);

  // attempt to load the specified board config file from /boards
  if ( LoadBrdConfigStart(brdfile) == true )
  {
    CNTLRDATA_println("cd: LoadDefaultBoardData() : brdfile loaded");
  }
  else
  {
    // a board config file could not be loaded, so create a dummy one
    CNTLRDATA_println("cd: LoadDefaultBoardData() : error, brdfile not loaded, create default board");
    this->board         = "Unknown";
    this->maxstepmode   = -1;
    this->stepmode      =  1;                     // full step
    this->enablepin     = -1;
    this->steppin       = -1;
    this->dirpin        = -1;
    this->temppin       = -1;
    this->hpswpin       = -1;
    this->inledpin      = -1;
    this->outledpin     = -1;
    this->pb1pin        = -1;
    this->pb1pin        = -1;
    this->irpin         = -1;
    this->boardnumber   = myboardnumber;          // captured from controller_config.h
    this->fixedstepmode = myfixedstepmode;
    this->stepsperrev   = mystepsperrev;
    for (int i = 0; i < 4; i++)
    {
      this->boardpins[i] = -1;
    }
    this->msdelay = 8000;
  }
  SaveBoardConfiguration();
}


// ----------------------------------------------------------------------
// Reset focuser settings to defaults : tcpip_server.cpp case 42:
// ----------------------------------------------------------------------
void CONTROLLER_DATA::SetFocuserDefaults(void)
{
  CNTLRDATA_println("cd: SetFocuserDefaults(): delete existing config files");
  if ( SPIFFS.exists(file_cntlr_config))
  {
    SPIFFS.remove(file_cntlr_config);
  }

  if ( SPIFFS.exists(file_board_config))
  {
    SPIFFS.remove(file_board_config);
  }

  if ( SPIFFS.exists(file_cntlr_var))
  {
    SPIFFS.remove(file_cntlr_var);
  }
  CNTLRDATA_println("cd: SetFocuserDefaults(): load default config files");
  LoadDefaultPersistantData();
  LoadDefaultBoardData();
  LoadDefaultVariableData();
}


// ----------------------------------------------------------------------
// Saves the configurations to files
// ----------------------------------------------------------------------
bool CONTROLLER_DATA::SaveConfiguration(long currentPosition, byte DirOfTravel)
{
  bool state = false;

  if (this->fposition != currentPosition || this->focuserdirection != DirOfTravel)  // last focuser position
  {
    CNTLRDATA_println("SaveConfiguration: update fpos and dir");
    this->fposition = currentPosition;
    this->focuserdirection = DirOfTravel;
    // set flag to start 30s counter for saving var file
    portENTER_CRITICAL(&varMux);
    save_var_flag = 0;
    portEXIT_CRITICAL(&varMux);;
    CNTLRDATA_println("++ request to save cntlr_var.jsn, save_var_flag = 0");
  }

  // check the flags to determine what needs to be saved

  // only if the focuser is not moving
  if ( isMoving == true )
  {
    CNTLRDATA_println("cd: SaveConfiguration: ismoving=true, delaying save");
    state = false;
  }
  else
  {
    int t_mux;

    // not moving, safe to save files

    // check var flag, if saving of file cntlr_var.jsn is required
    portENTER_CRITICAL(&varMux);
    t_mux = save_var_flag;
    portEXIT_CRITICAL(&varMux);
    if ( t_mux == 1 )
    {
      CNTLRDATA_println("++ request to save cntlr_var.jsn, save_var_flag = 1, save NOW");
      portENTER_CRITICAL(&varMux);
      save_var_flag = -1;
      portEXIT_CRITICAL(&varMux);
      if ( ControllerData->SaveVariableConfiguration() == false )
      {
        ERROR_println("++ save cntlr_var.jsn ERROR");
        state = false;
      }
      else
      {
        CNTLRDATA_println("++ save cntlr_var.jsn DONE");
        state = true;
      }
    }

    // check cntlr flag
    portENTER_CRITICAL(&cntlrMux);
    t_mux = save_cntlr_flag;
    portEXIT_CRITICAL(&cntlrMux);
    if ( t_mux == 1 )
    {
      CNTLRDATA_println("++ request to save cntlr_config.jsn, save_cntlr_flag = 1, save cntlr_config.jsn NOW");
      portENTER_CRITICAL(&cntlrMux);
      save_cntlr_flag = -1;
      portEXIT_CRITICAL(&cntlrMux);
      if ( ControllerData->SavePersitantConfiguration() == false)
      {
        ERROR_println("++ save cntlr_config.jsn error");
        state = false;
      }
      else
      {
        CNTLRDATA_println("++ save cntlr_config.jsn DONE");
        state = true;
      }
    }

    // check board flag
    portENTER_CRITICAL(&boardMux);
    t_mux = save_board_flag;
    portEXIT_CRITICAL(&boardMux);
    if ( t_mux == 1 )
    {
      CNTLRDATA_println("++ request to save board_config.jsn, save_board_flag = 1, save board_config.jsn NOW");
      portENTER_CRITICAL(&boardMux);
      save_board_flag = -1;
      portEXIT_CRITICAL(&boardMux);
      if ( ControllerData->SaveBoardConfiguration() == false )
      {
        ERROR_println("++ save board_config.jsn error");
        state = false;
      }
      else
      {
        CNTLRDATA_println("++ save board_config.jsn DONE");
        state = true;
      }
    }
  }
  CNTLRDATA_print("SaveConfiguration: return val: ");
  CNTLRDATA_println(state);
  return state;
}


// ----------------------------------------------------------------------
// Save configuration files immediately (like in the case for reboot()
// ----------------------------------------------------------------------
bool CONTROLLER_DATA::SaveNow(long focuser_position, bool focuser_direction)
{
  SaveBoardConfiguration();
  SaveVariableConfiguration();
  return SavePersitantConfiguration();
}


// ----------------------------------------------------------------------
// Save variable data (position, dir travel) settings to cntlr_var.jsn
// ----------------------------------------------------------------------
bool CONTROLLER_DATA::SaveVariableConfiguration()
{
  CNTLRDATA_println("cd: SaveVariableConfiguration() NOW");

  // Delete existing file
  if ( SPIFFS.exists(file_cntlr_var))
  {
    CNTLRDATA_println("cd: deleted existing cntlr_var.jsn");
    SPIFFS.remove(file_cntlr_var);
  }

  // Open file for writing
  File vfile = SPIFFS.open(this->file_cntlr_var, "w");
  if (!vfile)
  {
    ERROR_println("cd: Could not open ntlr_var.jsn for W mode");
    return false;
  }
  CNTLRDATA_println("cd: opened cntlr_var.jsn file for W mode");
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<DEFAULTVARDOCSIZE> doc;

  // Set the values in the document
  doc["fpos"] = this->fposition;                  // last focuser position
  doc["fdir"] = this->focuserdirection;           // keeps track of last focuser move direction

  // save settings to file
  if (serializeJson(doc, vfile) == 0)             // Serialize JSON to file
  {
    ERROR_println("cd: SaveVariableConfiguration() serialise error, cntlr_var.jsn file not saved");
    vfile.close();
    return false;
  }
  else
  {
    CNTLRDATA_println("cd: SaveVariableConfiguration: cntlr_var.jsn file written");
    vfile.close();
    return true;
  }
  return false;
}


// ----------------------------------------------------------------------
// Save Focuser Controller (persistent) Data to file cntlr_config.jsn
// ----------------------------------------------------------------------
bool CONTROLLER_DATA::SavePersitantConfiguration()
{
  CNTLRDATA_println("cd: SavePersitantConfiguration()");
  if ( SPIFFS.exists(file_cntlr_config))
  {
    CNTLRDATA_println("cd: SavePersitantConfiguration: cntlr_config.jsn found: deleting");
    SPIFFS.remove(file_cntlr_config);
  }
  else
  {
    CNTLRDATA_println("cd: SavePersitantConfiguration() : file not found");
  }

  CNTLRDATA_println("cd: SavePersitantConfiguration: cntlr_config.jsn open file for write");
  File file = SPIFFS.open(file_cntlr_config, "w");         // Open file for writing
  if (!file)
  {
    ERROR_println("cd: SavePersitantConfiguration() : file open for write error");
    return false;
  }
  CNTLRDATA_println("cd: SavePersitantConfiguration: prepare to write config file");
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  CNTLRDATA_println("cd: SavePersitantConfiguration: serialize data");
  // 303 - 1170, Size 1536
  StaticJsonDocument<DEFAULTDOCSIZE> doc;

  doc["maxstep"]      = this->maxstep;
  for (int i = 0; i < 10; i++)
  {
    doc["preset"][i]  = this->focuserpreset[i];   // array for presets
  };
  // SERVERS - SERVICES
  doc["ascom_en"]   = this->ascomsrvr_enable;
  doc["ascom_port"] = this->ascomsrvr_port;
  doc["mngt_en"]    = this->mngsrvr_enable;
  doc["mngt_port"]  = this->mngsrvr_port;
  doc["tcp_en"]     = this->tcpipsrvr_enable;
  doc["tcp_port"]   = this->tcpipsrvr_port;
  doc["ws_en"]      = this->websrvr_enable;
  doc["ws_port"]    = this->websrvr_port;
  doc["ddns_en"]    = this->duckdns_enable;
  doc["ddns_d"]     = this->duckdns_domain;
  doc["ddns_t"]     = this->duckdns_token;
  doc["ddns_r"]     = this->duckdns_refreshtime;
  doc["ota_name"]   = this->ota_name;
  doc["ota_pwd"]    = this->ota_password;
  doc["ota_id"]     = this->ota_id;
  // DEVICES
  // display
  doc["d_en"]       = this->display_enable;
  doc["d_pgtime"]   = this->displaypagetime;      // interval between oled pages display time
  doc["d_pgopt"]    = this->displaypageoption;
  doc["d_updmove"]  = this->displayupdateonmove;  // update position on oled when moving
  // hpsw
  doc["hpsw_en"]    = this->hpswmsg_enable;
  doc["hpswmsg_en"] = this->hpswitch_enable;
  doc["stall_st"]   = this->stallguard_state;
  doc["stall_val"]  = this->stallguard_value;
  doc["tmc2225mA"]  = this->tmc2225current;
  doc["tmc2209mA"]  = this->tmc2209current;
  // leds
  doc["led_en"]     = this->inoutled_enable;
  doc["led_mode"]   = this->inoutledmode;
  // joysticks
  doc["joy1_en"]    = this->joystick1_enable;
  doc["joy2_en"]    = this->joystick2_enable;
  // pushbuttons
  doc["pb_en"]      = this->pushbutton_enable;
  doc["pb_steps"]   = this->pushbutton_steps;
  // temperature probe
  doc["t_en"]       = this->tempprobe_enable;
  doc["t_comp_en"]  = this->tempcomp_enable;
  doc["t_mod"]      = this->tempmode;
  doc["t_coe"]      = this->tempcoefficient;      // steps per degree temperature coefficient value
  doc["t_res"]      = this->tempresolution;
  doc["t_tcdir"]    = this->tcdirection;
  doc["t_tcavail"]  = this->tcavailable;
  // backlash
  doc["blin_en"]    = this->backlash_in_enable;
  doc["blout_en"]   = this->backlash_out_enable;
  doc["blin_steps"] = this->backlashsteps_in;   // number of backlash steps to apply for IN moves
  doc["blout_steps"] = this->backlashsteps_out;
  // coil power
  doc["cp_en"]      = this->coilpower_enable;
  // delay after move
  doc["dam_en"]     = this->delayaftermove_enable;
  doc["dam_time"]   = this->delayaftermove_time;
  // devicename
  doc["devname"]    = this->devicename;
  // file list format
  doc["filelist"]   = this->filelistformat;
  // motorspeed
  doc["mspeed"]     = this->motorspeed;
  // park
  doc["park_en"]    = this->park_enable;
  doc["park_time"]  = this->park_time;
  // reverse
  doc["rdir_en"]    = this->reverse_enable;
  // stepsize
  doc["ss_en"]      = this->stepsize_enable;      // if 1, controller can return step size
  doc["ss_val"]     = this->stepsize;
  // web page colors
  doc["ticol"]      = this->titlecolor;
  doc["scol"]       = this->subtitlecolor;
  doc["hcol"]       = this->headercolor;
  doc["tcol"]       = this->textcolor;
  doc["bcol"]       = this->backcolor;
  // Serialize JSON to file
  if (serializeJson(doc, file) == 0)
  {
    ERROR_println("cd: SavePersitantConfiguration() serialise error");
    file.close();
    return false;
  }
  else
  {
    CNTLRDATA_println("cd: SavePersitantConfiguration: cntlr_config.jsn written");
    file.close();
    return true;
  }
}


// ----------------------------------------------------------------------
// Save Board Data to file board_config.jsn
// ----------------------------------------------------------------------
bool CONTROLLER_DATA::SaveBoardConfiguration()
{
  CNTLRDATA_println("cd: SaveBoardConfiguration() NOW");
  if ( SPIFFS.exists(file_board_config))
  {
    CNTLRDATA_println("cd: SaveBoardConfiguration(): board_config.jsn found: deleting");
    SPIFFS.remove(file_board_config);
  }
  CNTLRDATA_println("cd: SaveBoardConfiguration(): board_config.jsn preparing to write");
  File bfile = SPIFFS.open(file_board_config, "w");         // Open file for writing
  if (!bfile)
  {
    CNTLRDATA_println("cd: SaveBoardConfiguration() : write board_config.jsn error");
    return false;
  }
  else
  {
    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    StaticJsonDocument<DEFAULTBOARDSIZE> doc_brd;
    CNTLRDATA_println("cd: SaveBoardConfiguration(): prepare to write board_config.jsn file");
    // Set the values in the document
    doc_brd["board"]        = this->board;
    doc_brd["maxstepmode"]  = this->maxstepmode;
    doc_brd["stepmode"]     = this->stepmode;
    doc_brd["enpin"]        = this->enablepin;
    doc_brd["steppin"]      = this->steppin;
    doc_brd["dirpin"]       = this->dirpin;
    doc_brd["temppin"]      = this->temppin;
    doc_brd["hpswpin"]      = this->hpswpin;
    doc_brd["inledpin"]     = this->inledpin;
    doc_brd["outledpin"]    = this->outledpin;
    doc_brd["pb1pin"]       = this->pb1pin;
    doc_brd["pb2pin"]       = this->pb2pin;
    doc_brd["irpin"]        = this->irpin;
    doc_brd["brdnum"]       = this->boardnumber;
    doc_brd["stepsrev"]     = this->stepsperrev;
    doc_brd["fixedsmode"]   = this->fixedstepmode;
    for (int i = 0; i < 4; i++)
    {
      doc_brd["brdpins"][i] = this->boardpins[i];
    }
    doc_brd["msdelay"]      = this->msdelay;

    // Serialize JSON to file
    CNTLRDATA_println("cd: SaveBoardConfiguration(): serialise data");
    if (serializeJson(doc_brd, bfile) == 0)
    {
      ERROR_println("cd: SaveBoardConfiguration(): serialise error");
      bfile.close();
      return false;
    }
    else
    {
      CNTLRDATA_println("cd: SaveBoardConfiguration(): file written");
      bfile.close();
    }
  }
  return true;
}


// ----------------------------------------------------------------------
// Controller_Data : get()
// ----------------------------------------------------------------------

long CONTROLLER_DATA::get_fposition()
{
  return this->fposition;                               // last focuser position
}

long CONTROLLER_DATA::get_maxstep()
{
  return this->maxstep;                                 // max steps
}

long CONTROLLER_DATA::get_focuserpreset(byte idx)
{
  return this->focuserpreset[idx % 10];                 // the focuser position for each preset
}

byte CONTROLLER_DATA::get_focuserdirection()
{
  return this->focuserdirection;                        // keeps track of last focuser move direction
}

byte CONTROLLER_DATA::get_display_enable(void)
{
  return this->display_enable;
}

byte CONTROLLER_DATA::get_duckdns_enable(void)
{
  return this->duckdns_enable;
}

byte CONTROLLER_DATA::get_tempprobe_enable(void)
{
  return this->tempprobe_enable;
}

byte CONTROLLER_DATA::get_ascomsrvr_enable(void)
{
  return this->ascomsrvr_enable;
}

byte CONTROLLER_DATA::get_mngsrvr_enable(void)
{
  return this->mngsrvr_enable;
}

byte CONTROLLER_DATA::get_tcpipsrvr_enable(void)
{
  return this->tcpipsrvr_enable;
}

byte CONTROLLER_DATA::get_websrvr_enable(void)
{
  return this->websrvr_enable;
}

unsigned long CONTROLLER_DATA::get_ascomsrvr_port(void)
{
  return this->ascomsrvr_port;                         // the port number used by the ALPACA ASCOM Remote server
}

unsigned long CONTROLLER_DATA::get_mngsrvr_port(void)
{
  return this->mngsrvr_port;
}

unsigned long CONTROLLER_DATA::get_tcpipsrvr_port(void)
{
  return this->tcpipsrvr_port;
}

unsigned long CONTROLLER_DATA::get_websrvr_port(void)
{
  return this->websrvr_port;                           // the port number of the webserver
}

String CONTROLLER_DATA::get_duckdns_domain(void)
{
  return this->duckdns_domain;
}

String CONTROLLER_DATA::get_duckdns_token(void)
{
  return this->duckdns_token;
}

String CONTROLLER_DATA::get_ota_name(void)
{
  return this->ota_name;
}

String CONTROLLER_DATA::get_ota_password(void)
{
  return this->ota_password;
}

String CONTROLLER_DATA::get_ota_id(void)
{
  return this->ota_id;
}

byte CONTROLLER_DATA::get_backlashsteps_in(void)
{
  return this->backlashsteps_in;                        // number of backlash steps to apply for IN moves
}

byte CONTROLLER_DATA::get_backlashsteps_out(void)
{
  return this->backlashsteps_out;                       // number of backlash steps to apply for OUT moves
}

byte CONTROLLER_DATA::get_delayaftermove_time(void)
{
  return this->delayaftermove_time;
}

String CONTROLLER_DATA::get_devicename(void)
{
  return this->devicename;
}

int CONTROLLER_DATA::get_displaypagetime(void)
{
  return this->displaypagetime;                            // the length of time the page is displayed for, int 2-10
}

String CONTROLLER_DATA::get_displaypageoption(void)
{
  String tmp = this->displaypageoption;
  while ( tmp.length() < 8 )
  {
    tmp = tmp + "0";
  }
  tmp = tmp + "";
  this->displaypageoption = tmp;
  return this->displaypageoption;
}

byte CONTROLLER_DATA::get_displayupdateonmove(void)
{
  return this->displayupdateonmove;                        // update position on oled when moving
}

unsigned int CONTROLLER_DATA::get_duckdns_refreshtime(void)
{
  return this->duckdns_refreshtime;
}

byte CONTROLLER_DATA::get_inoutledmode(void)
{
  return this->inoutledmode;
}

byte CONTROLLER_DATA::get_motorspeed(void)
{
  return this->motorspeed;                              // the stepper motor speed, slow, medium, fast
}

int CONTROLLER_DATA::get_parktime(void)
{
  return this->park_time;
}

int CONTROLLER_DATA::get_pushbutton_steps(void)
{
  return this->pushbutton_steps;
}

float CONTROLLER_DATA::get_stepsize(void)
{
  return this->stepsize;                                // the step size in microns
  // this is the actual measured focuser stepsize in microns amd is reported to ASCOM, so must be valid
  // the amount in microns that the focuser tube moves in one step of the motor
}

byte CONTROLLER_DATA::get_tempmode(void)
{
  return this->tempmode;                                // temperature display mode, Celcius=1, Fahrenheit=0
}

int CONTROLLER_DATA::get_tempcoefficient(void)
{
  return this->tempcoefficient;                         // steps per degree temperature coefficient value (maxval=256)
}

byte CONTROLLER_DATA::get_tempresolution(void)
{
  return this->tempresolution;                          // resolution of temperature measurement 9-12
}

byte CONTROLLER_DATA::get_tcdirection(void)
{
  return this->tcdirection;                             // indicates the direction in which temperature compensation is applied
}

byte CONTROLLER_DATA::get_tcavailable(void)
{
  return this->tcavailable;
}

tmc2209stallguard CONTROLLER_DATA::get_stallguard_state(void)
{
  return this->stallguard_state;
}

byte CONTROLLER_DATA::get_stallguard_value(void)
{
  return this->stallguard_value;
}

int CONTROLLER_DATA::get_tmc2225current(void)
{
  return this->tmc2225current;
}

int CONTROLLER_DATA::get_tmc2209current(void)
{
  return this->tmc2209current;
}

byte CONTROLLER_DATA::get_backlash_in_enable(void)
{
  return this->backlash_in_enable;                       // apply backlash when moving in [0=!enabled, 1=enabled]
}

byte CONTROLLER_DATA::get_backlash_out_enable(void)
{
  return this->backlash_out_enable;                      // apply backlash when moving out [0=!enabled, 1=enabled]
}

byte CONTROLLER_DATA::get_coilpower_enable(void)
{
  return this->coilpower_enable;                               // state of coil power, 0 = !enabled, 1= enabled
}

byte CONTROLLER_DATA::get_delayaftermove_enable(void)
{
  return this->delayaftermove_enable;
}

byte CONTROLLER_DATA::get_hpswmsg_enable(void)
{
  return this->hpswmsg_enable;
}

byte CONTROLLER_DATA::get_hpswitch_enable(void)
{
  return this->hpswitch_enable;
}

byte CONTROLLER_DATA::get_inoutled_enable(void)
{
  return this->inoutled_enable;
}

byte CONTROLLER_DATA::get_park_enable(void)
{
  return this->park_enable;
}

byte CONTROLLER_DATA::get_pushbutton_enable(void)
{
  return this->pushbutton_enable;
}

byte CONTROLLER_DATA::get_joystick1_enable(void)
{
  return this->joystick1_enable;
}

byte CONTROLLER_DATA::get_joystick2_enable(void)
{
  return this->joystick2_enable;
}

byte CONTROLLER_DATA::get_reverse_enable(void)
{
  return this->reverse_enable;                        // state for reverse direction, 0 = !enabled, 1= enabled
}

byte CONTROLLER_DATA::get_stepsize_enable(void)
{
  return this->stepsize_enable;                         // if 1, controller returns step size
}

byte CONTROLLER_DATA::get_tempcomp_enable(void)
{
  return this->tempcomp_enable;                          // indicates if temperature compensation is enabled
}

byte CONTROLLER_DATA::get_filelistformat(void)
{
  return this->filelistformat;
}

String CONTROLLER_DATA::get_wp_backcolor(void)
{
  return this->backcolor;
}

String CONTROLLER_DATA::get_wp_textcolor(void)
{
  return this->textcolor;
}

String CONTROLLER_DATA::get_wp_headercolor(void)
{
  return this->headercolor;
}

String CONTROLLER_DATA::get_wp_titlecolor(void)
{
  return this->titlecolor;
}

String CONTROLLER_DATA::get_wp_subtitlecolor(void)
{
  return this->subtitlecolor;
}

// ----------------------------------------------------------------------
// Controller_Data : set()
// ----------------------------------------------------------------------

void CONTROLLER_DATA::set_fposition(long fposition)
{
  this->fposition = fposition;                          // last focuser position
  portENTER_CRITICAL(&varMux);
  save_var_flag = 0;
  portEXIT_CRITICAL(&varMux);
  CNTLRDATA_println("cd: set_fposition: Set var flag: var flag and count to 0, start count");
}

void CONTROLLER_DATA::set_focuserdirection(byte newdir)
{
  this->focuserdirection = newdir;                      // keeps track of last focuser move direction
}

void CONTROLLER_DATA::set_maxstep(long newval)
{
  this->StartDelayedUpdate(this->maxstep, newval);      // max steps
}

void CONTROLLER_DATA::set_focuserpreset(byte idx, long pos)
{
  this->StartDelayedUpdate(this->focuserpreset[idx % 10], pos);
}

void CONTROLLER_DATA::set_display_enable(byte newstate)
{
  this->StartDelayedUpdate(this->display_enable, newstate);
}

void CONTROLLER_DATA::set_duckdns_enable(byte newstate)
{
  this->StartDelayedUpdate(this->duckdns_enable, newstate);
}

void CONTROLLER_DATA::set_tempprobe_enable(byte newstate)
{
  this->StartDelayedUpdate(this->tempprobe_enable, newstate);
}

void CONTROLLER_DATA::set_ascomsrvr_enable(byte newstate)
{
  this->StartDelayedUpdate(this->ascomsrvr_enable, newstate);
}

void CONTROLLER_DATA::set_mngsrvr_enable(byte newstate)
{
  this->StartDelayedUpdate(this->mngsrvr_enable, newstate);
}

void CONTROLLER_DATA::set_tcpipsrvr_enable(byte newstate)
{
  this->StartDelayedUpdate(this->tcpipsrvr_enable, newstate);
}

void CONTROLLER_DATA::set_websrvr_enable(byte newstate)
{
  this->StartDelayedUpdate(this->websrvr_enable, newstate);
}

void CONTROLLER_DATA::set_backlash_in_enable(byte newstate)
{
  this->StartDelayedUpdate(this->backlash_in_enable, newstate);
}

void CONTROLLER_DATA::set_backlash_out_enable(byte newstate)
{
  this->StartDelayedUpdate(this->backlash_out_enable, newstate);
}

void CONTROLLER_DATA::set_coilpower_enable(byte newstate)
{
  this->StartDelayedUpdate(this->coilpower_enable, newstate);
}

void CONTROLLER_DATA::set_delayaftermove_enable(byte newstate)
{
  this->StartDelayedUpdate(this->delayaftermove_enable, newstate);
}

void CONTROLLER_DATA::set_hpswmsg_enable(byte newstate)
{
  this->StartDelayedUpdate(this->hpswmsg_enable, newstate);
}

void CONTROLLER_DATA::set_hpswitch_enable(byte newstate)
{
  this->StartDelayedUpdate(this->hpswitch_enable, newstate);
}

void CONTROLLER_DATA::set_inoutled_enable(byte newstate)
{
  this->StartDelayedUpdate(this->inoutled_enable, newstate);
}

void CONTROLLER_DATA::set_park_enable(byte newstate)
{
  this->StartDelayedUpdate(this->park_enable, newstate);
}

void CONTROLLER_DATA::set_pushbutton_enable(byte newstate)
{
  this->StartDelayedUpdate(this->pushbutton_enable, newstate);
}

void CONTROLLER_DATA::set_joystick1_enable(byte newstate)
{
  this->StartDelayedUpdate(this->joystick1_enable, newstate);
}

void CONTROLLER_DATA::set_joystick2_enable(byte newstate)
{
  this->StartDelayedUpdate(this->joystick2_enable, newstate);
}

void CONTROLLER_DATA::set_reverse_enable(byte newstate)
{
  this->StartDelayedUpdate(this->reverse_enable, newstate);
}

void CONTROLLER_DATA::set_stepsize_enable(byte newstate)
{
  this->StartDelayedUpdate(this->stepsize_enable, newstate); // if 1, controller returns step size
}

void CONTROLLER_DATA::set_tempcomp_enable(byte newstate)
{
  this->StartDelayedUpdate(this->tempcomp_enable, newstate); // indicates if temperature compensation is enabled
}

void CONTROLLER_DATA::set_ascomsrvr_port(unsigned long newport)
{
  this->StartDelayedUpdate(this->ascomsrvr_port, newport);
}

void CONTROLLER_DATA::set_mngsrvr_port(unsigned long newport)
{
  this->StartDelayedUpdate(this->mngsrvr_port, newport);
}

void CONTROLLER_DATA::set_tcpipsrvr_port(unsigned long newport)
{
  this->StartDelayedUpdate(this->tcpipsrvr_port, newport);
}

void CONTROLLER_DATA::set_websrvr_port(unsigned long newport)
{
  this->StartDelayedUpdate(this->websrvr_port, newport);
}

void CONTROLLER_DATA::set_duckdns_domain(String newdomain)
{
  this->StartDelayedUpdate(this->duckdns_domain, newdomain);
}

void CONTROLLER_DATA::set_duckdns_token(String newtoken)
{
  this->StartDelayedUpdate(this->duckdns_token, newtoken);
}

void CONTROLLER_DATA::set_ota_name(String newname)
{
  this->StartDelayedUpdate(this->ota_name, newname);
}

void CONTROLLER_DATA::set_ota_password(String newpwd)
{
  this->StartDelayedUpdate(this->ota_password, newpwd);
}

void CONTROLLER_DATA::set_ota_id(String newid)
{
  this->StartDelayedUpdate(this->ota_id, newid);
}

void CONTROLLER_DATA::set_backlashsteps_in(byte newval)
{
  this->StartDelayedUpdate(this->backlashsteps_in, newval); // number of backlash steps to apply for IN moves
}

void CONTROLLER_DATA::set_backlashsteps_out(byte newval)
{
  this->StartDelayedUpdate(this->backlashsteps_out, newval); // number of backlash steps to apply for OUT moves
}

void CONTROLLER_DATA::set_delayaftermove_time(byte newtime)
{
  this->StartDelayedUpdate(this->delayaftermove_time, newtime);
}

void CONTROLLER_DATA::set_devicename(String newname)
{
  this->StartDelayedUpdate(this->devicename, newname);
}

void CONTROLLER_DATA::set_displaypagetime(int newtime)
{
  portENTER_CRITICAL(&displaytimeMux);
  display_maxcount = newtime * 10;                         // convert to timeslices
  portEXIT_CRITICAL(&displaytimeMux);
  this->StartDelayedUpdate(this->displaypagetime, newtime);
}

void CONTROLLER_DATA::set_displaypageoption(String newoption)
{
  String tmp = newoption;
  while ( tmp.length() < 8 )
  {
    tmp = tmp + "0";
  }
  tmp = tmp + "";
  this->displaypageoption = tmp;
  this->StartDelayedUpdate(this->displaypageoption, newoption);
}

void CONTROLLER_DATA::set_displayupdateonmove(byte newstate)
{
  this->StartDelayedUpdate(this->displayupdateonmove, newstate); // update position on oled when moving
}

void CONTROLLER_DATA::set_duckdns_refreshtime(unsigned int newtime)
{
  this->StartDelayedUpdate(this->duckdns_refreshtime, newtime);
}

void CONTROLLER_DATA::set_inoutledmode(byte newmode)
{
  this->StartDelayedUpdate(this->inoutledmode, newmode);
}

void CONTROLLER_DATA::set_motorspeed(byte newval)
{
  this->StartDelayedUpdate(this->motorspeed, newval);
}

void CONTROLLER_DATA::set_parktime(int newtime)
{
  this->StartDelayedUpdate(this->park_time, newtime);
}

void CONTROLLER_DATA::set_pushbutton_steps(int newval)
{
  this->StartDelayedUpdate(this->pushbutton_steps, newval);
}

void CONTROLLER_DATA::set_stepsize(float newval)
{
  this->StartDelayedUpdate(this->stepsize, newval);   // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
}

void CONTROLLER_DATA::set_tempmode(byte newmode)
{
  this->StartDelayedUpdate(this->tempmode, newmode);   // temperature display mode, Celcius=1, Fahrenheit=0
}

void CONTROLLER_DATA::set_tempcoefficient(int newval)
{
  this->StartDelayedUpdate(this->tempcoefficient, newval); // steps per degree temperature coefficient value (maxval=256)
}

void CONTROLLER_DATA::set_tempresolution(byte newval)
{
  this->StartDelayedUpdate(this->tempresolution, newval);
}

void CONTROLLER_DATA::set_tcdirection(byte newdirection)
{
  this->StartDelayedUpdate(this->tcdirection, newdirection);
}

void CONTROLLER_DATA::set_tcavailable(byte newval)
{
  this->StartDelayedUpdate(this->tcavailable, newval);
}

void CONTROLLER_DATA::set_stallguard_state(tmc2209stallguard newstate)
{
  this->StartDelayedUpdate(this->stallguard_state, newstate);
}

void CONTROLLER_DATA::set_stallguard_value(byte newval)
{
  this->StartDelayedUpdate(this->stallguard_value, newval);
}

void CONTROLLER_DATA::set_tmc2225current(int newval)
{
  this->StartDelayedUpdate(this->tmc2225current, newval);
}

void CONTROLLER_DATA::set_tmc2209current(int newval)
{
  this->StartDelayedUpdate(this->tmc2209current, newval);
}

void CONTROLLER_DATA::set_filelistformat(byte newlistformat)
{
  this->StartDelayedUpdate(this->filelistformat, newlistformat);
}

void CONTROLLER_DATA::set_wp_backcolor(String newcolor)
{
  this->StartDelayedUpdate(this->backcolor, newcolor);
}

void CONTROLLER_DATA::set_wp_textcolor(String newcolor)
{
  this->StartDelayedUpdate(this->textcolor, newcolor);
}

void CONTROLLER_DATA::set_wp_headercolor(String newcolor)
{
  this->StartDelayedUpdate(this->headercolor, newcolor);
}

void CONTROLLER_DATA::set_wp_titlecolor(String newcolor)
{
  this->StartDelayedUpdate(this->titlecolor, newcolor);
}

void CONTROLLER_DATA::set_wp_subtitlecolor(String newcolor)
{
  this->StartDelayedUpdate(this->subtitlecolor, newcolor);
}


// ----------------------------------------------------------------------
// Delayed Write routines which update the focuser setting with the
// new value, then sets a flag for when the data should be written to file
// ----------------------------------------------------------------------
void CONTROLLER_DATA::StartDelayedUpdate(int & org_data, int new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_cntlr_flags();
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(tmc2209stallguard & org_data, tmc2209stallguard new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_cntlr_flags();
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(unsigned int & org_data, unsigned int new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_cntlr_flags();
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(long & org_data, long new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_cntlr_flags();
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(unsigned long & org_data, unsigned long new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_cntlr_flags();
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(float & org_data, float new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_cntlr_flags();
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(byte & org_data, byte new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_cntlr_flags();
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(String & org_data, String new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_cntlr_flags();
  }
}


// ----------------------------------------------------------------------
// Focuser Persistent Data: Driver Board
// ----------------------------------------------------------------------
bool CONTROLLER_DATA::LoadBrdConfigStart(String brdfile)
{
  File bfile = SPIFFS.open(brdfile, "r");                         // Open file for writing
  if (!bfile)
  {
    ERROR_println("cd: LoadBrdConfigStart() open file read error");
    return false;
  }
  else
  {
    // read file and deserialize
    String fdata = bfile.readString();                            // read content of the text file
    bfile.close();

    // Allocate a temporary JsonDocument
    DynamicJsonDocument doc_brd(DEFAULTBOARDSIZE);

    // Deserialize the JSON document
    DeserializationError jerror = deserializeJson(doc_brd, fdata);
    if ( jerror )
    {
      ERROR_println("cd: LoadBrdConfigStart() deserialise error");
      return false;
    }
    else
    {
      // save the brd_data just read from board config file (brdfile) into board_config.jsn
      // Set the board values from doc_brd
      this->board         = doc_brd["board"].as<const char*>();
      this->maxstepmode   = doc_brd["maxstepmode"];
      this->stepmode      = doc_brd["stepmode"];
      this->enablepin     = doc_brd["enpin"];
      this->steppin       = doc_brd["steppin"];
      this->dirpin        = doc_brd["dirpin"];
      this->temppin       = doc_brd["temppin"];
      this->hpswpin       = doc_brd["hpswpin"];
      this->inledpin      = doc_brd["inledpin"];
      this->outledpin     = doc_brd["outledpin"];
      this->pb1pin        = doc_brd["pb1pin"];
      this->pb2pin        = doc_brd["pb2pin"];
      this->irpin         = doc_brd["irpin"];
      this->boardnumber   = doc_brd["brdnum"];
      // brdstepsperrev comes from STEPSPERREVOLUTION and will be different so must override the default setting in the board files
      switch ( myboardnumber )
      {
        case PRO2ESP32ULN2003:
        case PRO2ESP32L298N:
        case PRO2ESP32L293DMINI:
        case PRO2ESP32L9110S:
          this->stepsperrev = mystepsperrev;         // override STEPSPERREVOLUTION from controller_config.h FIXEDSTEPMODE
          break;
        default:
          this->stepsperrev = doc_brd["stepsrev"];
          break;
      }
      // myfixedstepmode comes from FIXEDSTEPMODE and will be different so must override the default setting in the board files
      switch ( myboardnumber )
      {
        case PRO2ESP32R3WEMOS:
        case PRO2ESP32ST6128:
          this->fixedstepmode = myfixedstepmode;     // override fixedstepmode from controller_config.h FIXEDSTEPMODE
          break;
        default:
          this->fixedstepmode = doc_brd["fixedsmode"];
          break;
      }
      for (int i = 0; i < 4; i++)
      {
        this->boardpins[i] = doc_brd["brdpins"][i];
      }
      this->msdelay = doc_brd["msdelay"];      // motor speed delay - do not confuse with motorspeed
      return true;
    }
  }
  return false;
}

// legacy orphaned code
// kept here in case I change my mind about something
bool CONTROLLER_DATA::CreateBoardConfigfromjson(String jsonstr)
{
  // generate board configuration from json string

  CNTLRDATA_print("cd: CreateBoardConfigfromjson() : ");
  CNTLRDATA_println(jsonstr);
  // Allocate a temporary Json Document
  DynamicJsonDocument doc_brd(DEFAULTBOARDSIZE + 100);

  // Deserialize the JSON document
  DeserializationError jerror = deserializeJson(doc_brd, jsonstr);
  if ( jerror )
  {
    CNTLRDATA_println("cd: CreateBoardConfigfromjson() : deserialise error");
    LoadDefaultBoardData();
    return false;
  }
  else
  {
    /*
      { "board":"PRO2ESP32DRV8825","maxstepmode":32,"stepmode":1,"enpin":14,"steppin":33,
      "dirpin":32,"temppin":13,"hpswpin":4,"inledpin":18,"outledpin":19,"pb1pin":34,"pb2pin":35,"irpin":15,
      "brdnum":60,"stepsrev":-1,"fixedsmode":-1,"brdpins":[27,26,25,-1],"msdelay":4000 }
    */
    this->board         = doc_brd["board"].as<const char*>();
    this->maxstepmode   = doc_brd["maxstepmode"];
    this->stepmode      = doc_brd["stepmode"];
    this->enablepin     = doc_brd["enpin"];
    this->steppin       = doc_brd["steppin"];
    this->dirpin        = doc_brd["dirpin"];
    this->temppin       = doc_brd["temppin"];
    this->hpswpin       = doc_brd["hpswpin"];
    this->inledpin      = doc_brd["inledpin"];
    this->outledpin     = doc_brd["outledpin"];
    this->pb1pin        = doc_brd["pb1pin"];
    this->pb2pin        = doc_brd["pb2pin"];
    this->irpin         = doc_brd["irpin"];
    this->boardnumber   = doc_brd["brdnum"];
    // brdstepsperrev comes from STEPSPERREVOLUTION and will be different so must override the default setting in the board files
    switch ( myboardnumber )
    {
      case PRO2ESP32ULN2003:
      case PRO2ESP32L298N:
      case PRO2ESP32L293DMINI:
      case PRO2ESP32L9110S:
        this->stepsperrev = mystepsperrev;         // override STEPSPERREVOLUTION from controller_config.h FIXEDSTEPMODE
        break;
      default:
        this->stepsperrev = doc_brd["stepsrev"];
        break;
    }
    // myfixedstepmode comes from FIXEDSTEPMODE and will be different so must override the default setting in the board files
    switch ( myboardnumber )
    {
      case PRO2ESP32R3WEMOS:
      case PRO2ESP32ST6128:
        this->fixedstepmode = myfixedstepmode;     // override fixedstepmode from controller_config.h FIXEDSTEPMODE
        break;
      default:
        this->fixedstepmode = doc_brd["fixedsmode"];
        break;
    }
    for (int i = 0; i < 4; i++)
    {
      this->boardpins[i] = doc_brd["brdpins"][i];
    }
    this->msdelay = doc_brd["msdelay"];                    // motor speed delay - do not confuse with motorspeed
    SaveBoardConfiguration();
    return true;
  }
}

// get
String CONTROLLER_DATA::get_brdname()
{
  return this->board;
}

int CONTROLLER_DATA::get_brdmaxstepmode()
{
  return this->maxstepmode;
}

int CONTROLLER_DATA::get_brdstepmode()
{
  return this->stepmode;
}

int CONTROLLER_DATA::get_brdenablepin()
{
  return this->enablepin;
}

int CONTROLLER_DATA::get_brdsteppin()
{
  return this->steppin;
}

int CONTROLLER_DATA::get_brddirpin()
{
  return this->dirpin;
}

int CONTROLLER_DATA::get_brdtemppin()
{
  return this->temppin;
}

int CONTROLLER_DATA::get_brdhpswpin()
{
  return this->hpswpin;
}

int CONTROLLER_DATA::get_brdinledpin()
{
  return this->inledpin;
}

int CONTROLLER_DATA::get_brdoutledpin()
{
  return this->outledpin;
}

int CONTROLLER_DATA::get_brdirpin()
{
  return this->irpin;
}

int CONTROLLER_DATA::get_brdboardpins(int pinnum)
{
  return this->boardpins[pinnum];
}

int CONTROLLER_DATA::get_brdnumber()
{
  return this->boardnumber;
}

int CONTROLLER_DATA::get_brdstepsperrev()
{
  return this->stepsperrev;
}

int CONTROLLER_DATA::get_brdfixedstepmode()
{
  return this->fixedstepmode;
}

int CONTROLLER_DATA::get_brdpb1pin()
{
  return this->pb1pin;
}

int CONTROLLER_DATA::get_brdpb2pin()
{
  return this->pb2pin;
}

unsigned long CONTROLLER_DATA::get_brdmsdelay()
{
  return this->msdelay;
}

// set
void CONTROLLER_DATA::set_brdname(String newstr)
{
  this->StartBoardDelayedUpdate(this->board, newstr);
}

void CONTROLLER_DATA::set_brdmaxstepmode(int newval)
{
  this->StartBoardDelayedUpdate(this->maxstepmode, newval);
}

void CONTROLLER_DATA::set_brdstepmode(int newval)
{
  this->StartBoardDelayedUpdate(this->stepmode, newval);
}

void CONTROLLER_DATA::set_brdenablepin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->enablepin, pinnum);
}

void CONTROLLER_DATA::set_brdsteppin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->steppin, pinnum);
}

void CONTROLLER_DATA::set_brddirpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->dirpin, pinnum);
}

void CONTROLLER_DATA::set_brdtemppin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->temppin, pinnum);
}

void CONTROLLER_DATA::set_brdhpswpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->hpswpin, pinnum);
}

void CONTROLLER_DATA::set_brdinledpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->inledpin, pinnum);
}

void CONTROLLER_DATA::set_brdoutledpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->outledpin, pinnum);
}

void CONTROLLER_DATA::set_brdirpin(int pinnum)
{
  this->StartBoardDelayedUpdate(this->irpin, pinnum);
}

void CONTROLLER_DATA::set_brdboardpins(int pinnum)
{
  this->StartBoardDelayedUpdate(this->boardpins[pinnum], pinnum);
}

void CONTROLLER_DATA::set_brdnumber(int newval)
{
  this->StartBoardDelayedUpdate(this->boardnumber, newval);
}

void CONTROLLER_DATA::set_brdfixedstepmode(int newval)
{
  this->StartBoardDelayedUpdate(this->fixedstepmode, newval);
}

void CONTROLLER_DATA::set_brdstepsperrev(int stepsrev)
{
  this->StartBoardDelayedUpdate(this->stepsperrev, stepsrev);
}

void CONTROLLER_DATA::set_brdpb1pin(int newpin)
{
  this->StartBoardDelayedUpdate(this->pb1pin, newpin);
}

void CONTROLLER_DATA::set_brdpb2pin(int newpin)
{
  this->StartBoardDelayedUpdate(this->pb2pin, newpin);
}

void CONTROLLER_DATA::set_brdmsdelay(unsigned long newval)
{
  this->StartBoardDelayedUpdate(this->msdelay, newval);
}

void CONTROLLER_DATA::StartBoardDelayedUpdate(int & org_data, int new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_board_flags();
  }
}

void CONTROLLER_DATA::StartBoardDelayedUpdate(unsigned long & org_data, unsigned long new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_board_flags();
  }
}

void CONTROLLER_DATA::StartBoardDelayedUpdate(byte & org_data, byte new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_board_flags();
  }
}

void CONTROLLER_DATA::StartBoardDelayedUpdate(String & org_data, String new_data)
{
  if (org_data != new_data)
  {
    org_data = new_data;
    this->set_board_flags();
  }
}

void CONTROLLER_DATA::set_cntlr_flags(void)
{
  portENTER_CRITICAL(&cntlrMux);
  save_cntlr_flag = 0;
  portEXIT_CRITICAL(&cntlrMux);
  CNTLRDATA_println("++ request to save cntlr_config.jsn, save_cntlr_flag = 0");
}

void CONTROLLER_DATA::set_board_flags(void)
{
  portENTER_CRITICAL(&boardMux);
  save_board_flag = 0;
  portEXIT_CRITICAL(&boardMux);
  CNTLRDATA_println("++ request to save board_config.jsn, save_board_flag = 0");
}


// ----------------------------------------------------------------------
// Misc
// ----------------------------------------------------------------------
void CONTROLLER_DATA::ListDir(const char * dirname, uint8_t levels)
{
  File root = SPIFFS.open(dirname);
  CNTLRDATA_print("cd: Listing directory: {");
  if (!root)
  {
    CNTLRDATA_println(" - failed to open directory");
  }
  else
  {
    if (!root.isDirectory())
    {
      CNTLRDATA_println(" - not a directory");
    }
    else
    {
      File file = root.openNextFile();
      int i = 0;
      while (file)
      {
        if (file.isDirectory())
        {
          CNTLRDATA_print("  DIR : ");
          CNTLRDATA_println(file.name());
          if (levels)
          {
            this->ListDir(file.name(), levels - 1);
          }
        }
        else
        {
          CNTLRDATA_print(file.name());
          CNTLRDATA_print(":");
          CNTLRDATA_print(file.size());
          if ((++i % 6) == 0)
          {
            CNTLRDATA_println("");
          }
          else
          {
            CNTLRDATA_print("  ");
          }
        }
        file = root.openNextFile();
      }
      CNTLRDATA_println("}");
    }
  }
}
