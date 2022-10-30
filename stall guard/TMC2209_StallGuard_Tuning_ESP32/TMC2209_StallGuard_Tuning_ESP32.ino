/**
   Author Teemu Mäntykallio

   Plot TMC2130 or TMC2660 motor load using the stallGuard value.
   You can finetune the reading by changing the STALL_VALUE.
   This will let you control at which load the value will read 0
   and the stall flag will be triggered. This will also set pin DIAG1 high.
   A higher STALL_VALUE will make the reading less sensitive and
   a lower STALL_VALUE will make it more sensitive.

   You can control the rotation speed with
   0 Stop
   1 Resume
   + Speed up
   - Slow down
*/
#include <TMCStepper.h>

#define MIN_SPEED      1000

#define STALL_VALUE     50 // [0..255]

#define EN_PIN           14 // Enable
#define DIR_PIN          32 // Direction
#define STEP_PIN         33 // Step
#define DIAG_PIN          4 // DIAG (hpsw)
#define SERIAL_PORT Serial2 // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2

#define R_SENSE 0.11f // Match to your driver
// SilentStepStick series use 0.11
// UltiMachine Einsy and Archim2 boards use 0.2
// Panucatt BSD2660 uses 0.1
// Watterott TMC5160 uses 0.075

//#define STOP_AT_STALL     1     // uncomment if you want to stop the motor for 3 seconds when a stall is detected

// Select your stepper driver type
TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

hw_timer_t * timer1 = NULL;

using namespace TMC2208_n;

int32_t speed = 5000;
int newSGTHRS = STALL_VALUE;

void IRAM_ATTR onTimer() {

  digitalWrite(STEP_PIN, !digitalRead(STEP_PIN));
}

void setup() {
  Serial.begin(250000);         // Init serial port and set baudrate
  while (!Serial);              // Wait for serial port to connect
  Serial.println("\nStart...");

  SERIAL_PORT.begin(115200);

  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(DIAG_PIN, INPUT_PULLUP);
  digitalWrite(EN_PIN, LOW);

  driver.begin();
  driver.toff(4);
  driver.blank_time(24);
  driver.rms_current(400); // mA
  driver.microsteps(16);
  driver.TCOOLTHRS(0xFFFFF); // 20bit max
  driver.semin(5);
  driver.semax(2);
  driver.sedn(0b01);
  driver.SGTHRS(STALL_VALUE);

  Serial.println("OK");
  Serial.print("Motor is "); Serial.println(digitalRead(EN_PIN) ? "DISABLED" : "ENABLED");
  Serial.print("stepMode is "); Serial.println(driver.microsteps());

  clearError();
  activateInterrupt();
}

void loop() {
  static uint32_t last_time = 0;
  uint32_t ms = millis();

  while (Serial.available() > 0) {
    int8_t read_byte = Serial.read();
    if (read_byte == '0')      {
      digitalWrite( EN_PIN, HIGH );
    }
    else if (read_byte == '1') {
      digitalWrite( EN_PIN,  LOW );
    }
    else if (read_byte == '+') {
      speed += 1000;
      driver.VACTUAL(speed);
    }
    else if (read_byte == '-') {
      speed -= 1000;
      driver.VACTUAL(speed);
    }
    else if (read_byte == 'i') {
      newSGTHRS += 5;         // increase threshold by 5
      driver.SGTHRS(newSGTHRS);
    }
    else if (read_byte == 'd') {
      newSGTHRS -= 5;         // increase threshold by 5
      driver.SGTHRS(newSGTHRS);
    }
  }


  if ((ms - last_time) > 100) { //run every 0.1s
    last_time = ms;

    Serial.print("0 ");
    Serial.print(newSGTHRS, DEC);
    Serial.print(" ");
    Serial.print(driver.SG_RESULT(), DEC);
    Serial.print(" ");
    Serial.print(digitalRead(DIAG_PIN)*100, DEC);
    Serial.print(" ");
    //Serial.print((driver.SG_RESULT() < STALL_VALUE)*50, DEC);
    //Serial.print(" ");
    Serial.println(driver.cs2rms(driver.cs_actual()), DEC);
  }


#if defined(STOP_AT_STALL)
  if ( digitalRead(DIAG_PIN) ) {
    digitalWrite(EN_PIN, HIGH);
    delay(3000);
    digitalWrite(EN_PIN, LOW);
  }
#endif
}

void activateInterrupt() {
  {
    cli();                                              // stop interrupts
    timer1 = timerBegin(3, 8, true);                    // Initialize timer 4. 
                                                        // prescaler of 8, and true is a flag indicating whether the interrupt is performed on edge or level
    timerAttachInterrupt(timer1, &onTimer, true);       // link interrupt with function onTimer
    timerAlarmWrite(timer1, 8000, true);                // this function defines the counter value at which the timer interrupt is generated.
    timerAlarmEnable(timer1);                           // Enable timer
    sei();                                              // allow interrupts
  }
}

void clearError() {
  digitalWrite(EN_PIN, HIGH);
  delay(250);
  digitalWrite(EN_PIN, LOW);
  delay(250);
  digitalWrite(EN_PIN, HIGH);
  delay(250);
  digitalWrite(EN_PIN, LOW);
}
