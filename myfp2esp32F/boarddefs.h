// ----------------------------------------------------------------------
// myFP2ESP32 DEFINES FOR BOARD TYPES
// © Copyright Robert Brown 2014-2022. All Rights Reserved.
// © Copyright Holger M, 2019-2021. All Rights Reserved.
// boarddefs.h
// ----------------------------------------------------------------------

#ifndef boarddefs_h
#define boarddefs_h

// ---------------------------------------------------------------------------
// BOARD TYPE DEFINES
// ---------------------------------------------------------------------------      

// ESP32 boards only
#define PRO2ESP32DRV8825      44
#define PRO2ESP32ULN2003      45
#define PRO2ESP32L298N        46
#define PRO2ESP32L293DMINI    47          // uses PCB layout for ULN2003
#define PRO2ESP32L9110S       48          // uses PCB layout for ULN2003
#define PRO2ESP32R3WEMOS      49          // https://www.ebay.com/itm/R3-Wemos-UNO-D1-R32-ESP32-WIFI-Bluetooth-CH340-Development-Board-For-Arduino/264166013552 
#define PRO2ESP32TMC2225      56
#define PRO2ESP32TMC2209      57
#define PRO2ESP32TMC2209P     58          // This is board for Paul using TMC2209 for testing
#define PRO2ESP32ST6128       59          // This is board for CLOSED LOOP ST6128 driver

#define CUSTOMBRD             99          // For a user custom board see 99.jsn in /data/boards folder

// ---------------------------------------------------------------------------
// 2: STEP MODE DEFINES
// ---------------------------------------------------------------------------            
#ifndef STEP1
#define STEP1                 1           // stepmodes
#endif
#ifndef STEP2
#define STEP2                 2
#endif
#ifndef STEP4
#define STEP4                 4
#endif
#ifndef STEP8
#define STEP8                 8
#endif
#ifndef STEP16
#define STEP16                16
#endif
#ifndef STEP32
#define STEP32                32
#endif
#ifndef STEP64
#define STEP64                64
#endif
#ifndef STEP128
#define STEP128               128
#endif
#ifndef STEP256
#define STEP256               256
#endif

// ---------------------------------------------------------------------------
// 3: SPEED DEFINES
// ---------------------------------------------------------------------------    
#ifndef SLOW
#define SLOW                  0             // motorspeeds
#endif
#ifndef MED
#define MED                   1
#endif
#ifndef FAST
#define FAST                  2
#endif


#endif
