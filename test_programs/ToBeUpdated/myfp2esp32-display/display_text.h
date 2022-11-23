// ----------------------------------------------------------------------
// myFP2ESP32 DISPLAY CLASS (TEXT) DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2021. All Rights Reserved.
// displays.h
// ----------------------------------------------------------------------

#if !defined(_text_display_h_)
#define _text_display_h_


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>

// ** #include <mySSD1306Ascii.h>
#include <mySSD1306AsciiWire.h>


// Note: TEXT/GRAPHICS use the exact same class definition, but
// private members can be different.
// ----------------------------------------------------------------------
// CLASS
// ----------------------------------------------------------------------
class TEXT_DISPLAY
{
  public:
    TEXT_DISPLAY(uint8_t addr);
    
    bool start(void);
    void stop(void);
    void update(void);
    void clear(void);
    void on(void);
    void off(void);

  private:
    uint8_t _addr;              // I2C address of display
    bool    _found;
    bool    _state;
    String  _images;
    SSD1306AsciiWire *_display;
};


#endif // _text_display_h_
