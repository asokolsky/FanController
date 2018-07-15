#include <Arduino.h>
#include "Trace.h"
#include "Fan.h"
#include "pcb.h"

/** These are the fans we control */
Fan g_fan[] = {
  {pinFan1pwm, pinFan1sen},
  //{pinFan2pwm, pinFan2sen},
  //{pinFan3pwm, pinFan3sen}
};
/** # of fans we control */
const short int iFans = sizeof(g_fan) / sizeof(g_fan[0]);

/**
 * fan sensor increments this
 */
static volatile unsigned long g_ulFanTick = 0;

/**
 * fan sense pin causes this interrupt
 */
static void fanISR()
{
  g_ulFanTick++;
}

/**
 * used by begin/end calculateRPM
 */
static unsigned long g_ulRPMcalcMillis = 0;

/**
 * starts calculation of fan RPM
 * Side effect - zeros g_ulFanTick
 */
void beginCalculateRPM() 
{
  noInterrupts();
  g_ulRPMcalcMillis = nowMillis();
  g_ulFanTick = 0;
  interrupts();
}

/**
 * calculates fan RPM
 * return RPM or 0 if beginCalculateRPM was not called before
 * Side effect - zeros g_ulFanTick
 */
unsigned long endCalculateRPM() 
{
  if(g_ulRPMcalcMillis == 0)
    return 0;
  noInterrupts();
  unsigned long revolutions = g_ulFanTick/2;
  interrupts();
  float elapsedS = (nowMillis() - g_ulRPMcalcMillis)/1000.0;
  g_ulRPMcalcMillis = 0;
  float revPerS = revolutions / elapsedS;
  unsigned long rpm = revPerS * 60.0;
  DEBUG_PRINT("RPM="); DEBUG_PRINTDEC(rpm); DEBUG_PRINTLN("");
  return rpm;
}

/**
 * We mess with timer 0 - see fanSetup() - so need this correction
 */
unsigned long nowMillis()
{
  return (millis() / 64);
}

void myDelay(unsigned long ms)
{
  delay(ms * 64);
}

/**
 * Fan startup sequence
 */
void fansSetup()
{
  // For Arduino Uno, Nano, Micro Magician, Mini Driver, Lilly Pad and any other board using ATmega 8, 168 or 328**
  // The Arduino uses Timer 0 internally for the millis() and delay() functions
  //---------------------------------------------- Set PWM frequency for D5 & D6 -------------------------------
  TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
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
  //g_fan[0].test();

  //
  // stop the fan
  //
  DEBUG_PRINTLN("Stopping fans...");
  fansStop();
  myDelay(4*1000);
  //
  // spin the fan at start PWM
  //
  DEBUG_PRINTLN("Spinning fans at start PWM...");
  for(short int i = 0; i < iFans; i++)
    g_fan[i].start();
  beginCalculateRPM();
  unsigned long ulFanTick = g_ulFanTick;  
  myDelay(4*1000);
  unsigned long rpm = endCalculateRPM();
  if(ulFanTick == g_ulFanTick)
  {
    DEBUG_PRINTLN("Fan seem to be absent or failed to start!");
  }
  else
  {
    DEBUG_PRINT("Fan ticks="); DEBUG_PRINTDEC(g_ulFanTick); DEBUG_PRINTLN("");
  }
  
  //
  // spin the fan at max RPM
  //
  DEBUG_PRINTLN("Spinning fans at max PWM...");
  fansSpin(Fan::pwmMax);
  beginCalculateRPM();
  myDelay(4*1000);
  rpm = endCalculateRPM();
  //
  // spin the fan at min RPM
  //
  DEBUG_PRINTLN("Spinning fans at min PWM...");
  fansSpin(Fan::pwmMin);
  beginCalculateRPM();
  myDelay(4*1000);
  rpm = endCalculateRPM();

  beginCalculateRPM();
}


void fansStop()
{
  for(short int i = 0; i < iFans; i++)
    g_fan[i].stop();
}

void fansSpin(unsigned short pwm)
{
  //DEBUG_PRINT("fansSpin "); DEBUG_PRNTLN(pwm);
  for(short int i = 0; i < iFans; i++)
    g_fan[i].spin(pwm);
}

void fansDumpStats(char buf[])
{
  unsigned short pwm = g_fan[0].getPWM();
  sprintf(buf, "Now=%lums, PWM=%d, FanTicks=%lu, ", nowMillis(), (int)pwm, g_ulFanTick);
  Serial.print(buf);
  endCalculateRPM();  // this will print "RPM=xxx\n"
  beginCalculateRPM();
}

/** 
 * Setup the fan
 */
void Fan::setup()
{
  pinMode(m_pinFan, OUTPUT);
  if(m_pinSensor > 0)
  {
    pinMode(m_pinSensor, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(m_pinSensor), fanISR, FALLING);
  }
}

/**
 * Start spinning the fan
 */
void Fan::start()
{
  DEBUG_PRINTLN("Starting fan.. ");
  spin(pwmStart);
}
/**
 * Stop the fan if it is spinning
 */
void Fan::stop()
{
  if(m_pwm == 0)
    return;
  DEBUG_PRINTLN("Stopping fan...");
  digitalWrite(m_pinFan, LOW);
  m_pwm = 0;
}
/** 
 * spin the fan at this pwm 
 */
void Fan::spin(unsigned short pwm)
{
  if(pwm == m_pwm)
    return;
  analogWrite(m_pinFan, pwm);
  m_pwm = pwm;
  DEBUG_PRINT("analogWrite("); DEBUG_PRNT(m_pinFan); DEBUG_PRINT(", "); DEBUG_PRNT(pwm); DEBUG_PRINTLN(")");
}

/**
 * utility to find the min PWM at which this fan starts
 */
/*void Fan::test()
{
  
  Serial.println("Starting a test to find a min pwm at which this fan can be started...");
  Serial.println("Note the value when we first FAIL to start the fan");
  for(unsigned short int pwm = 90; pwm > 10; pwm = pwm - 8)
  {
    stop();
    myDelay(4*1000);
    // try to start the fan
    spin(pwm);
    myDelay(4*1000);
  } 
     
}*/


