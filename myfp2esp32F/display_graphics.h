// ----------------------------------------------------------------------
// myFP2ESP32 DISPLAY CLASS (GRAPHICS) DEFINITIONS
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2021. All Rights Reserved.
// displays.h
// ----------------------------------------------------------------------

#if !defined(_display_graphic_h_)
#define _display_graphic_h_

// requires esp8266-oled-ssd1306 library
// https://github.com/ThingPulse/esp8266-oled-ssd1306

// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>

#if defined(ENABLE_GRAPHICDISPLAY)

// check for SSD1306 display
#if defined(USE_SSD1306)                                  // for the OLED 128x64 0.96" display using the SSD1306 driver
#include <SSD1306Wire.h>                                  // requires esp8266-oled-ssd1306 library
#else
// assume SSH1106 display
#include <SH1106Wire.h>                                   // for the OLED 128x64 1.3" display using the SSH1106 driver                     
#endif // #if defined(USE_SSH1106)


// ----------------------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------------------
#define SCREEN_WIDTH          128           // OLED display width, in pixels
#define SCREEN_HEIGHT         64            // OLED display height, in pixels
// OLED_ADDR found in controller_defines.h


// Note: TEXT/GRAPHICS use the exact same class definition, but
// private members can be different.
// ----------------------------------------------------------------------
// CLASS
// ----------------------------------------------------------------------
class GRAPHIC_DISPLAY
{
  public:
    GRAPHIC_DISPLAY(uint8_t addr);

    bool start(void);
    void stop(void);  
    void update(void);
    void update_position(long);
    
    void clear(void);
    void on(void);
    void off(void); 

    void reboot(void);

  private:
    void draw_main_update(void);
    void display_draw_xbm(logo_num, int16_t, int16_t);
        
    uint8_t _addr;              // I2C address of display
    bool    _found = false;
    bool    _state = false;    
    byte    count_hb = 0;       // heart beat counter

#if defined(USE_SSD1306)
    SSD1306Wire *_display;
#else // Assume USE_SSH1106
    SH1106Wire *_display;
#endif
};



#endif // #if defined(GRAPHICDISPLAY)


#endif // #if !defined(_display_graphic_h_)
