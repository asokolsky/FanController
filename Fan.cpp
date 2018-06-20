#include <Arduino.h>
#include "Trace.h"
#include "Fan.h"
#include "pcb.h"

/** These are the fans we control */
Fan g_fan[] = {
  {pinFan1pwm, pinFan1sen},
  {pinFan2pwm, pinFan2sen},
  {pinFan3pwm, pinFan3sen}
};
/** # of fans we control */
const short int iFans = sizeof(g_fan) / sizeof(g_fan[0]);

/**
 * Fan startup sequence
 */
void fansSetup()
{
  // For Arduino Uno, Nano, Micro Magician, Mini Driver, Lilly Pad and any other board using ATmega 8, 168 or 328**
  // The Arduino uses Timer 0 internally for the millis() and delay() functions
  //---------------------------------------------- Set PWM frequency for D5 & D6 -------------------------------
  //TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   976.56 Hz (The DEFAULT)
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   244.14 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz

  //---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------
  //TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz

  //---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
  //TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz
  
  for(short int i = 0; i < iFans; i++)
    g_fan[i].setup();
  //attachInterrupt(0, onHallSensor, RISING);
  //g_fan[0].test();

  //
  // stop the fan
  //
  DEBUG_PRINTLN("Stopping fans...");
  fansStop();
  delay(4*1000);
  //
  // spin the fan at start PWM
  //
  DEBUG_PRINTLN("Spinning fans at start PWM..."); 
  for(short int i = 0; i < iFans; i++)
    g_fan[i].start();
  delay(4*1000);
  //
  // spin the fan at max RPM
  //
  DEBUG_PRINTLN("Spinning fans at max PWM...");
  fansSpin(Fan::pwmMax);
  delay(4*1000);
  //
  // spin the fan at min RPM
  //
  DEBUG_PRINTLN("Spinning fans at min PWM...");
  fansSpin(Fan::pwmMin);
  delay(4*1000);
}


void fansStop()
{
  for(short int i = 0; i < iFans; i++)
    g_fan[i].stop();
}

void fansSpin(unsigned short pwm)
{
  DEBUG_PRINT("fansSpin "); DEBUG_PRNTLN(pwm);
  for(short int i = 0; i < iFans; i++)
    g_fan[i].spin(pwm);
}

void fansDumpStats(char buf[])
{
  unsigned short pwm = g_fan[0].getPWM();
  sprintf(buf, "Fan: pwm=%d", (int)pwm);
  Serial.println(buf);
  sprintf(buf, "Now: %lu ms", millis());
  Serial.println(buf);
}


