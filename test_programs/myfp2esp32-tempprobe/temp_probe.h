// ----------------------------------------------------------------------
// myFP2ESP32 support for DS18B20 Temperature Sensor
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger Manz, 2020-2021. All Rights Reserved.
// temp_probe.h
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------
#if !defined(_temp_probe_h_)
#define _temp_probe_h_

#include <OneWire.h>
#include <myDallasTemperature.h>


// ----------------------------------------------------------------------
// TEMPERATURE Class
// ----------------------------------------------------------------------
class TEMP_PROBE
{
  public:
    TEMP_PROBE(int);
    bool init(void);
    bool start(void);  
    void stop(void);
    void set_resolution(byte);
    byte get_probefound(void);

    float update(void);
    float read(void);    
    void  request(void);
    bool  get_loaded();
    byte  get_state();
        
  private:
    byte    _loaded = false;
    byte    _state = V_STOPPED;
    uint8_t _pin;
    float   _lasttemp = 20.0f;
    byte    _found = V_NOTFOUND;
    
    //OneWire           _tpOneWire(12);
    OneWire           * _tpOneWire;
    DallasTemperature * _tpsensor;
    DeviceAddress       _tpAddress;                 // holds the temperature probe address
};  


#endif // #if !defined(_temp_probe_h_)
