// ----------------------------------------------------------------------
// myFP2ESP32 ASCOM ALPACA SERVER CLASS DEFINITION
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// ascom_server.h
// ----------------------------------------------------------------------

#ifndef _ascom_server_h
#define _ascom_server_h

#include <WiFiUdp.h>                                // Implementation ASCOM ALPACA DISCOVERY PROTOCOL
#include <WebServer.h>  


// ----------------------------------------------------------------------
// Class
// ----------------------------------------------------------------------
class ASCOM_SERVER
{
  public:
    ASCOM_SERVER(void);

    bool start(void);
    void stop(void);
    void loop(void);
    byte get_state(void);
    byte get_loaded(void);    

    void get_setup(void);
    void get_notfound(void);
    void check_alpaca(void);
    void file_sys_error(void);

    void sendmyheader(void);
    void sendmycontent(String);
    void checkASCOMALPACADiscovery(void);
    void sendreply(int, String, String);
    void getURLParameters(void);
    String addclientinfo(String);  
    void send_setup(void);

    // management api
    void get_focusersetup(void);
    void set_focusersetup(void);
    void get_man_versions(void);
    void get_man_description(void);
    void get_man_configureddevices(void);
    void get_interfaceversion(void);
    void set_connected(void);
    void get_connected(void);
    void get_name(void);
    void get_description(void);
    void get_driverinfo(void);
    void get_driverversion(void);
    void get_absolute(void);
    void get_maxstep(void);
    void get_maxincrement(void);
    void get_temperature(void);
    void get_position(void);
    void set_halt(void);
    void get_ismoving(void);
    void get_stepsize(void);
    void get_tempcomp(void);
    void set_tempcomp(void);
    void get_tempcompavailable(void);
    void set_move(void);
    void get_supportedactions(void);
       
  private:
    void notloaded(void);   

    byte          _state = V_STOPPED;
    bool          _loaded = false;
    WebServer     *_ascomserver;
    bool          _ascomdiscovery = false;
    //WiFiUDP       _ASCOMDISCOVERYUdp;
    char          _packetBuffer[255] = {0};
    bool          _discoverystate = false;

    unsigned int  _ASCOMClientID = 0;;
    unsigned int  _ASCOMClientTransactionID = 0;
    unsigned int  _ASCOMServerTransactionID = 0;
    int           _ASCOMErrorNumber = 0;
    String        _ASCOMErrorMessage = "";
    long          _ASCOMpos = 0L;
    byte          _ASCOMTempCompState = 0;
    byte          _ASCOMConnectedState = 0;
};

#endif // ifndef _ascom_server_h
