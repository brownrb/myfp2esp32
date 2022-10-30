// ----------------------------------------------------------------------
// myFP2ESP32 WEB SERVER CLASS DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// web_server.h
// ----------------------------------------------------------------------

#if !defined(_web_server_h_)
#define _web_server_h_


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "WebServer.h"


// ----------------------------------------------------------------------
// SUPPORT FUNCTIONS
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// CLASS
// ----------------------------------------------------------------------
class WEB_SERVER
{
  public:
    WEB_SERVER();
    bool start(unsigned long);
    void stop(void);
    void loop(bool);
    void get_notfound(void);

    void send_redirect(String);
    void get_presets(void);
    void post_presets(void);
    void get_index(void);
    void get_move(void);
    void post_move(void);
    void reload_webpages(void);
        
    // xhtml
    void get_position(void);
    void get_ismoving(void);
    void get_targetposition(void);
    void get_temperature(void);
    void get_park(void);
    void get_coilpower(void);
    
  private:
    void file_sys_error(void);
    void send_myheader(void);
    void send_mycontent(String);
    void send_json(String);
    void send_xhtml(String);
    void send_ACAOheader(void);
    String get_contenttype(String filename);
    bool is_hexdigit(char);
    bool loadpages(void);

    WebServer *_web_server;
    unsigned long int _port = WEBSERVERPORT;
    bool _loaded = false;
    bool _parked = true;
    bool _state = false;
    String _indexpg;
    String _movepg;
    String _presetspg;
    String _notfoundpg;
    
};



#endif // #if !defined(_web_server_h_)
