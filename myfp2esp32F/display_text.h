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

#if defined(ENABLE_TEXTDISPLAY)

#include <mySSD1306Ascii.h>
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
    void update();
    void update_position(long);

    void clear(void);
    void on(void);
    void off(void);

    void reboot(void);

  private:
    void draw_main_update(void);
    void printpagenumber(byte);
    void page1(void);
    void page2(void);
    void page3(void);
    void page4(void);
    void page5(void);
    void page6(void);
    void page7(void);
    void page8(void);
    void bright(void);
    void dim(void);
    void normal(void);

    uint8_t _addr;              // I2C address of display
    bool    _found;
    bool    _state;
    String _images;
    SSD1306AsciiWire *_display;
};



#endif // #if defined(TEXTDISPLAY)

#endif // _text_display_h_
