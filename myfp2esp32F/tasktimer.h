// -----------------------------------------------------------------------
// myFP2ESP32 TASK TIMER
// © Copyright Robert Brown 2020-2022.
// All Rights Reserved.
// tasktimer.h
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Uses hw timer 2
// -----------------------------------------------------------------------

#ifndef mytasktimer_h
#define mytasktimer_h


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>


// ----------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------
#define TASKTIMERINITSTR      "task timer init"


// ----------------------------------------------------------------------
// Flag states
// ----------------------------------------------------------------------
// -1 the flag is disabled, task timer ignores any flag set to -1
//
// 0 set by a device so task timer can count the elapsed time
//
// 1 set by task timer when count is reached
// When flag is 1, the device that set the flag to 0 should set the flag
// back to 0 and service the device (update temp, update display etc)
// ----------------------------------------------------------------------

volatile int update_temp_flag;            // if 1 a new temp reading will occur
volatile int update_display_flag;         // if one change the display page
volatile int update_park_flag;            // if 1, park time is expired and park the focuser

volatile int save_board_flag;            // if 1, board data is ready to be saved to SPIFFS
volatile int save_cntlr_flag;             // if 1, controller data is ready to be saved to SPIFFS
volatile int save_var_flag;               // if 1, position and dir data is ready to be saved to SPIFFS
volatile int update_wifi_flag;

// ----------------------------------------------------------------------
// MUTEX'S
// ----------------------------------------------------------------------
portMUX_TYPE displayMux     = portMUX_INITIALIZER_UNLOCKED;     // for the flag, now its time to update page on display
portMUX_TYPE displaytimeMux = portMUX_INITIALIZER_UNLOCKED;     // for updating the actual time delay value display_maxcount
portMUX_TYPE parkMux        = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE tempMux        = portMUX_INITIALIZER_UNLOCKED;

portMUX_TYPE boardMux       = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE cntlrMux       = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE varMux         = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE wifiMux        = portMUX_INITIALIZER_UNLOCKED;

// ----------------------------------------------------------------------
// CONST
// ----------------------------------------------------------------------
volatile unsigned int display_maxcount  = 40;     // 4s     subject to ControllerData->get_displaypagetime(), default 4s
volatile unsigned int park_maxcount     = 1200;   // 120s   subject to ControllerData->get_parktime(), default 120s
volatile unsigned int temp_maxcount     = 35;     // 3.5s   refresh temp rate = 30 timeslices (30*100 milliseconds)

// DEFAULTSAVETIME
unsigned int const save_board_maxcount  = 600;    // wait 60s before saving board data to SPIFFS
unsigned int const save_cntlr_maxcount  = 600;    // wait 60s before saving controller data to SPIFFS
unsigned int const save_var_maxcount    = 600;    // wait 60s before saving position and dir data to SPIFFS
unsigned int const wifi_maxcount        = 1200;   // wait 120s before checking if WiFi connection 


// ----------------------------------------------------------------------
// PRIVATE
// ----------------------------------------------------------------------
// hardware timer to manage task execution
hw_timer_t * task_timer = NULL;

unsigned int counter_display;
unsigned int counter_park;
unsigned int counter_temp;
unsigned int counter_board;
unsigned int counter_cntlr;
unsigned int counter_var;
unsigned int counter_wifi;

// -----------------------------------------------------------------------
// task_100MilliTimer();
// Function: handle all time intervals associated with sensors, services and the file system
// Uses: Timer 2
// Resolution: 100mS
// -----------------------------------------------------------------------
void IRAM_ATTR task_100MillisecondTimer()
{
  // park
  portENTER_CRITICAL(&parkMux);
  if ( update_park_flag == 0 )
  {
    counter_park = counter_park + 1;
    if ( counter_park > park_maxcount)
    {
      update_park_flag = 1;
      counter_park = 0;
    }
  }
  portEXIT_CRITICAL(&parkMux);

  // temp sensor
  portENTER_CRITICAL(&tempMux);
  if ( update_temp_flag == 0 )
  {
    counter_temp = counter_temp + 1;
    if ( counter_temp > temp_maxcount)
    {
      update_temp_flag = 1;
      counter_temp = 0;
    }
  }
  portEXIT_CRITICAL(&tempMux);

  // display
  portENTER_CRITICAL(&displayMux);
  if ( update_display_flag == 0 )
  {
    counter_display = counter_display + 1;
    // displaypagetime in seconds (4), so * 10 to get to time slices
    portENTER_CRITICAL(&displaytimeMux);
    if ( counter_display > display_maxcount)
    {
      update_display_flag = 1;
      counter_display = 0;
    }
    portEXIT_CRITICAL(&displaytimeMux);
  }
  portEXIT_CRITICAL(&displayMux);

  // save position/direction to file
  portENTER_CRITICAL(&varMux);
  if ( save_var_flag == 0 )
  {
    counter_var = counter_var + 1;
    if ( counter_var > save_var_maxcount)
    {
      save_var_flag = 1;
      counter_var = 1;
    }
  }
  portEXIT_CRITICAL(&varMux);

  // save controller data to file
  portENTER_CRITICAL(&cntlrMux);
  if ( save_cntlr_flag == 0 )
  {
    counter_cntlr = counter_cntlr + 1;
    if ( counter_cntlr > save_cntlr_maxcount )
    {
      save_cntlr_flag = 1;
      counter_cntlr = 1;
    }
  }
  portEXIT_CRITICAL(&cntlrMux);

  // save board data to file
  portENTER_CRITICAL(&boardMux);
  if ( save_board_flag == false )
  {
    counter_board = counter_board + 1;
    if ( counter_board > save_board_maxcount )
    {
      save_board_flag = true;
      counter_board = 1;
    }
  }
  portEXIT_CRITICAL(&boardMux);

  // monitor wifi connection
  portENTER_CRITICAL(&wifiMux);
  if ( update_wifi_flag == 0 )
  {
    counter_wifi = counter_wifi + 1;
    if ( counter_wifi > wifi_maxcount )
    {
      update_wifi_flag = 1;
      counter_wifi = 0;
    }
  }
  portEXIT_CRITICAL(&wifiMux);
}

void init_task_counters()
{
  counter_temp    = 0;
  counter_display = 0;
  counter_park    = 0;
  counter_board   = 0;
  counter_cntlr   = 0;
  counter_var     = 0;
  counter_wifi    = 0;

  // These are dynamic and can change at any time, *10 because there are 10 slices within 1s
  // display page time in seconds, adjusted to time slices
  display_maxcount = ControllerData->get_displaypagetime() * 10;
  // park time max count in seconds, adjusted to time slices
  park_maxcount = ControllerData->get_parktime() * 10;
}

void init_task_timer()
{
  DEBUG_println("Tasktimer started");
  init_task_counters();
  // using a define in place of 2, the task timer did not start
  task_timer = timerBegin(2, 80, true);
  timerAttachInterrupt(task_timer, &task_100MillisecondTimer, true);
  // param 2 is interval in microseconds, 100ms = 100000 microseconds
  timerAlarmWrite(task_timer, 100000, true);
  timerAlarmEnable(task_timer);
}


#endif // mytasktimer_h
