// -----------------------------------------------------------------------
// myFP2ESP32 TCPIP SERVER ROUTINES AND DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// tcpipserver.h
// -----------------------------------------------------------------------

#if !defined(_tcpip_server_h)
#define _tcpip_server_h

#include <WiFiServer.h>

#define MAXCONNECTIONS    4


// ----------------------------------------------------------------------
// CLASS
// ----------------------------------------------------------------------
class TCPIP_SERVER
{
  public:
    TCPIP_SERVER();
    void begin();

    bool start(unsigned long);                // start the tcp/ip server
    void stop(void);                          // top the tcp/ip server
    void loop(bool);                          // check for new client and manage existing clients

    bool get_clients();
    void not_loaded(void);
    void send_reply(const char *, int);
      
    void build_reply(const char, const char *, int);
    void build_reply(const char, unsigned char, int);
    void build_reply(const char, float, int, int);
    void build_reply(const char, int, int);
    void build_reply(const char, long, int);            
    void build_reply(const char, unsigned long, int);
    void build_reply(const char, String, int);
    void build_reply_board(const char,  const char *, int);    
    void build_reply_config(const char, const char *, int);
    void build_reply_string(const char, const char *, int);    

    char * ftoa(char *, double, int);
    void cachepresets(void);

  private:
    void nullarg(int);
    void process_command(int);

    WiFiServer *_myserver;
    WiFiClient *_myclients[MAXCONNECTIONS] = { NULL };  // 4 connections allowed
    IPAddress  _myclientsIPAddressList[MAXCONNECTIONS]; // IP Address for each connection
    bool _myclientsfreeslot[MAXCONNECTIONS];            // indicator for free connection slot
    int  _totalclients = 0;
    bool _clientstatus = false;
    bool _loaded = false;
    bool _parked = false;
    byte _state = V_STOPPED;
    unsigned long _port = TCPIPSERVERPORT;
    long _presets[10] = { 0 };                          // to cache presets for commands :90 and :91
    const char _EOFSTR = '#';                           // 0x23   '#'  end of command
};


#endif // #if !defined(_tcpip_server_h)
