// ----------------------------------------------------------------------
// myFP2ESP32 MANAGEMENT SERVER ROUTINES
// © Copyright Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// management_server.h
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------
#include <Arduino.h>
#include <ArduinoJson.h>                                // Benoit Blanchon https://github.com/bblanchon/ArduinoJson
#include "SPIFFS.h"
#include <WebServer.h>


// ----------------------------------------------------------------------
// MANAGEMENT SERVER CLASS
// ----------------------------------------------------------------------
class MANAGEMENT_SERVER
{
  public:
    MANAGEMENT_SERVER();
    bool start(unsigned long);
    void stop(void);
    void loop(bool);
    void not_loaded(void);
    void get_notfound(void);
    void handlenotfound(void);
    void success(void);
    void fail(void);
    void send_redirect(String);

    // admin
    void getadmin1(void);
    void getadmin2(void);
    void getadmin3(void);
    void getadmin4(void);
    void getadmin5(void);
    void getadmin6(void);
    void getadmin7(void);
    void getadmin8(void);
    void getadmin9(void);

    // file management
    void get_filelist_long(void);
    void get_filelist_short(void);
    void post_deletefile();
    void get_deletefile(void);
    void upload_file(void);
    void post_uploadstart(void);
    void post_uploadfile(void);
    bool handlefileread(String);
    String get_uri(void);

    // get set command interface
    void handlecmds(void);
    void handleget(void);
    void handleset(void);

    // board management
    void brdedit(void);
        
    void rssi(void);
    void reboot(void);
    void saveconfig(void);

  private:
    bool check_access(void);
    void checkreboot(void);
    void focuser_moving(void);    
    void file_sys_error(void);
    void send_myheader(void);
    void send_mycontent(String);
    void send_json(String);
    void send_ACAOheader(void);
    String get_contenttype(String);
    bool is_hexdigit(char);

    File   _fsUploadFile;
    WebServer *mserver;
    unsigned int _port = MNGSERVERPORT;
    bool _loaded = false;
    bool _parkstate = true;
    byte _state = V_STOPPED;
};
