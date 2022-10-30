// ----------------------------------------------------------------------
// myFP2ESP32 FOCUSER CONFIGURATION CLASS DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// controller_data.h
// ----------------------------------------------------------------------
#include <Arduino.h>
#include "controller_defines.h"
#include "boarddefs.h"
#include "controller_config.h"


// ----------------------------------------------------------------------
// Not supported by ESP32
// ----------------------------------------------------------------------
// DelayedDisplayUpdate
// JoggingState
// JoggingDirection
// StepperPower


// ----------------------------------------------------------------------
// Controller_Data Class
// ----------------------------------------------------------------------
class CONTROLLER_DATA
{
  public:
    CONTROLLER_DATA(void);
    bool LoadConfiguration(void);
    bool LoadBrdConfigStart(String);          // attempt to load a board config file [DRVBRD] immediately after a firmware reprogram
    void LoadDefaultBoardData(void);

    bool SaveConfiguration(long, byte);
    bool SaveConfiguration(void);
    bool SaveVariableConfiguration(void);
    bool SavePersitantConfiguration(void);
    bool SaveBoardConfiguration(void);
    bool SaveNow(long, bool);

    void set_var_flags(void);
    void set_cntlr_flags(void);
    void set_board_flags(void);

    void SetFocuserDefaults(void);

    bool CreateBoardConfigfromjson(String);   // create a board config from a json string - used by Management Server

    long get_fposition(void);
    long get_maxstep(void);
    long get_focuserpreset(byte);
    byte get_focuserdirection(void);

    // display is a special case, it can be enabled, but the write to the display can be true or false
    // so it has both an enable state and a status. We only need to have enable here, status is a runtime
    // flag
    byte get_display_enable(void);            // enable/disable, start stop, optional
    byte get_ascomsrvr_enable(void);
    byte get_duckdns_enable(void);
    byte get_tempprobe_enable(void);
    byte get_mngsrvr_enable(void);
    byte get_tcpipsrvr_enable(void);
    byte get_websrvr_enable(void);

    byte get_backlash_in_enable(void);
    byte get_backlash_out_enable(void);
    byte get_coilpower_enable(void);          // enabled = ON, disabled = OFF
    byte get_delayaftermove_enable(void);
    byte get_hpswmsg_enable(void);
    byte get_hpswitch_enable(void);           // requires init
    byte get_inoutled_enable(void);           // requires init
    byte get_park_enable(void);
    byte get_pushbutton_enable(void);         // requires init and load
    byte get_joystick1_enable(void);
    byte get_joystick2_enable(void);
    byte get_reverse_enable(void);
    byte get_stepsize_enable(void);
    byte get_tempcomp_enable(void);

    // Server ports
    unsigned long get_ascomsrvr_port(void);
    unsigned long get_mngsrvr_port(void);
    unsigned long get_tcpipsrvr_port(void);
    unsigned long get_websrvr_port(void);

    // names, pwd's
    String get_duckdns_domain(void);
    String get_duckdns_token(void);
    String get_ota_name(void);
    String get_ota_password(void);
    String get_ota_id(void);

    byte get_backlashsteps_in(void);
    byte get_backlashsteps_out(void);
    byte get_delayaftermove_time(void);
    String get_devicename(void);
    int  get_displaypagetime(void);
    String get_displaypageoption(void);
    byte get_displayupdateonmove(void);       // also serves as DelayedDisplayUpdate (myFP2)

    unsigned int get_duckdns_refreshtime(void);
    byte get_inoutledmode(void);
    byte get_motorspeed(void);
    int  get_parktime(void);
    int  get_pushbutton_steps(void);

    float get_stepsize(void);
    byte get_tempmode(void);                  // C or F
    int  get_tempcoefficient(void);
    byte get_tempresolution(void);
    byte get_tcdirection(void);
    byte get_tcavailable(void);

    tmc2209stallguard get_stallguard_state(void);
    byte get_stallguard_value(void);
    int  get_tmc2225current(void);
    int  get_tmc2209current(void);

    byte get_filelistformat(void);

    String get_wp_backcolor(void);
    String get_wp_textcolor(void);
    String get_wp_headercolor(void);
    String get_wp_titlecolor(void);

    void set_fposition(long);
    void set_maxstep(long);
    void set_focuserpreset(byte, long);
    void set_focuserdirection(byte);

    void set_duckdns_domain(String);
    void set_duckdns_token(String);
    void set_ota_name(String);
    void set_ota_password(String);
    void set_ota_id(String);

    void set_display_enable(byte);
    void set_ascomsrvr_enable(byte);
    void set_duckdns_enable(byte);
    void set_mngsrvr_enable(byte);
    void set_tcpipsrvr_enable(byte);
    void set_tempprobe_enable(byte);
    void set_websrvr_enable(byte);

    void set_backlash_in_enable(byte);
    void set_backlash_out_enable(byte);
    void set_coilpower_enable(byte);
    void set_delayaftermove_enable(byte);
    void set_hpswmsg_enable(byte);
    void set_hpswitch_enable(byte);
    void set_inoutled_enable(byte);
    void set_park_enable(byte);
    void set_pushbutton_enable(byte);
    void set_joystick1_enable(byte);
    void set_joystick2_enable(byte);
    void set_reverse_enable(byte);
    void set_stepsize_enable(byte);
    void set_tempcomp_enable(byte);

    void set_wp_backcolor(String);
    void set_wp_textcolor(String);
    void set_wp_headercolor(String);
    void set_wp_titlecolor(String);

    void set_ascomsrvr_port(unsigned long);
    void set_mngsrvr_port(unsigned long);
    void set_tcpipsrvr_port(unsigned long);
    void set_websrvr_port(unsigned long);

    // Setting values
    void set_backlashsteps_in(byte);
    void set_backlashsteps_out(byte);
    void set_delayaftermove_time(byte);
    void set_devicename(String);
    void set_displaypagetime(int);
    void set_displaypageoption(String);
    void set_displayupdateonmove(byte);
    void set_duckdns_refreshtime(unsigned int);
    void set_inoutledmode(byte);
    void set_motorspeed(byte);
    void set_parktime(int);
    void set_pushbutton_steps(int);
    void set_stepsize(float);
    void set_tempmode(byte);
    void set_tempcoefficient(int);
    void set_tempresolution(byte);
    void set_tcdirection(byte);
    void set_tcavailable(byte);
    void set_filelistformat(byte);

    void set_stallguard_state(tmc2209stallguard);
    void set_stallguard_value(byte);
    void set_tmc2225current(int);
    void set_tmc2209current(int);

    // get boardconfig
    String get_brdname(void);
    int get_brdmaxstepmode(void);
    int get_brdstepmode(void);
    int get_brdenablepin(void);
    int get_brdsteppin(void);
    int get_brddirpin(void);
    int get_brdtemppin(void);
    int get_brdhpswpin(void);
    int get_brdinledpin(void);
    int get_brdoutledpin(void);
    int get_brdirpin(void);
    int get_brdboardpins(int);
    int get_brdstepsperrev(void);
    int get_brdfixedstepmode(void);
    int get_brdpb1pin(void);
    int get_brdpb2pin(void);
    unsigned long get_brdmsdelay(void);
    int get_brdnumber(void);
    int get_fixedstepmode(void);
    int get_stepsperrev(void);

    // set boardconfig
    void set_brdname(String);
    void set_brdmaxstepmode(int);
    void set_brdstepmode(int);
    void set_brdenablepin(int);
    void set_brdsteppin(int);
    void set_brddirpin(int);
    void set_brdtemppin(int);
    void set_brdhpswpin(int);
    void set_brdinledpin(int);
    void set_brdoutledpin(int);
    void set_brdirpin(int);
    void set_brdboardpins(int);
    void set_brdstepsperrev(int);
    void set_brdfixedstepmode(int);
    void set_brdpb1pin(int);
    void set_brdpb2pin(int);
    void set_brdmsdelay(unsigned long);
    void set_brdnumber(int);
    void set_fixedstepmode(int);
    void set_stepsperrev(int);

  private:
    void LoadDefaultPersistantData(void);
    void LoadDefaultVariableData(void);
    void LoadBoardConfiguration(void);
    void SetDefaultBoardData(void);

    void StartDelayedUpdate(unsigned long &, unsigned long);
    void StartDelayedUpdate(long &, long);
    void StartDelayedUpdate(float &, float);
    void StartDelayedUpdate(byte &, byte);
    void StartDelayedUpdate(int &, int);
    void StartDelayedUpdate(tmc2209stallguard &, tmc2209stallguard);
    void StartDelayedUpdate(unsigned int &, unsigned int);
    void StartDelayedUpdate(String &, String);

    void StartBoardDelayedUpdate(unsigned long &, unsigned long);
    void StartBoardDelayedUpdate(float &, float);
    void StartBoardDelayedUpdate(byte &, byte);
    void StartBoardDelayedUpdate(int &, int);
    void StartBoardDelayedUpdate(String &, String);

    void ListDir(const char*, uint8_t);

    const String file_cntlr_config = "/cntlr_config.jsn";       // Controller JSON configuration
    const String file_cntlr_var    = "/cntlr_var.jsn";          // variable JSON setup data, position and direction
    const String file_board_config = "/board_config.jsn";       // board JSON configuration

    long fposition;                 // last focuser position
    long maxstep;                   // max steps
    long focuserpreset[10];         // focuser presets can be used with software or ir-remote controller
    byte focuserdirection;          // keeps track of last focuser move direction
    
    // Loaded at boot time, if enabled is 1 then an attempt will be made to "start" and "run"
    byte display_enable;
    byte duckdns_enable;
    byte tempprobe_enable;
    byte ascomsrvr_enable;
    byte mngsrvr_enable;
    byte tcpipsrvr_enable;
    byte websrvr_enable;

    byte backlash_in_enable;        // if 1, backlash is enabled for IN movements (lower or -ve moves)
    byte backlash_out_enable;       // if 1, backlash is enabled for OUT movements (higher or +ve moves)
    byte coilpower_enable;          // if 1, coil power is enabled. If 0 then coil power is off
    byte delayaftermove_enable;
    byte hpswmsg_enable;            // if 1, home position switch msg's show on display if enabled
    byte hpswitch_enable;
    byte inoutled_enable;           // if 1, in out leds are enabled [only if board supports them]
    byte park_enable;
    byte pushbutton_enable;
    byte joystick1_enable;
    byte joystick2_enable;
    byte reverse_enable;            // if 1, motor direction is reversed

    //byte stallguard_enable;         //
    tmc2209stallguard stallguard_state;

    byte stepsize_enable;           // if 1, controller returns step size
    byte tempcomp_enable;           // indicates if temperature compensation is enabled

    unsigned long ascomsrvr_port;
    unsigned long mngsrvr_port;
    unsigned long tcpipsrvr_port;
    unsigned long websrvr_port;

    // Settings values
    String duckdns_domain;
    String duckdns_token;
    String ota_name;
    String ota_password;
    String ota_id;

    byte backlashsteps_in;          // number of backlash steps to apply for IN moves
    byte backlashsteps_out;         // number of backlash steps to apply for OUT moves
    byte delayaftermove_time;       // number of milliseconds to wait after a move
    String devicename;
    int  displaypagetime;           // length of time in seconds that a display page is shown for, 2-10
    String displaypageoption;       // which display pages to show/hide
    byte displayupdateonmove;       // update position on display when moving
    unsigned int duckdns_refreshtime;  // default 60s

    byte inoutledmode;              // 0=blink every stepper pulse, 1=stay on whilst motor moving
    byte motorspeed;                // speed of motor, slow, medium or fast
    int  park_time;                 // time in seconds that elapses after the end of a move, used to put the display to sleep
    int  pushbutton_steps;
    byte stallguard_value;          // value for STALL_GUARD, tmc2209, in boardefs.h
    float stepsize;                 // the step size in microns, ie 7.2 - value * 10, so real stepsize = stepsize / 10 (maxval = 25.6)
    byte tempmode;                  // temperature display mode, Celcius=1, Fahrenheit=0
    int  tempcoefficient;           // steps per degree temperature coefficient value
    byte tempresolution;            // 9 - 12
    byte tcdirection;               // direction in which to apply temperature compensation
    byte tcavailable;               // temperature compensation available

    int  tmc2209current;
    int  tmc2225current;
    byte filelistformat;

    String backcolor;
    String textcolor;
    String headercolor;
    String titlecolor;

    // dataset board configuration
    String board;
    int maxstepmode;
    int stepmode;
    int enablepin;
    int steppin;
    int dirpin;
    int temppin;
    int hpswpin;
    int inledpin;
    int outledpin;
    int pb1pin;
    int pb2pin;
    int irpin;
    int boardnumber;
    int fixedstepmode;
    int stepsperrev;
    int boardpins[4];
    unsigned long msdelay;

    /*
      { "board":"PRO2ESP32DRV8825","brdnum":20,"maxstepmode":32,"stepmode":1,"enpin":14,"steppin":33,"dirpin":32,
      "temppin":13,"hpswpin":4,"inledpin":18,"outledpin":19,"pb1pin":34,"pb2pin":35,"irpin":15,"brdnum":60,"stepsrev":-1,
      "fixedsmode":-1,"brdpins":[27,26,25,-1],"mspeed":4000 }
    */
};
