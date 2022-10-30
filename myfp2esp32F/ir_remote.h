// ----------------------------------------------------------------------
// myFP2ESP32 IR-REMOTE CLASS DEFINITION
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// ir_remote.h
// Library  myfp2eIRremoteESP8266.zip
// Optional
// ----------------------------------------------------------------------

#ifndef _ir_remote_h
#define _ir_remote_h


// ----------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// IRREMOTE Class
// ----------------------------------------------------------------------
class IR_REMOTE
{
  public:
    IR_REMOTE();
    bool start();
    void update(void);
    
  private:
    int _pin;
    bool _loaded;
};



#endif
