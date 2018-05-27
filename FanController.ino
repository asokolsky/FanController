/**
 *  FAN temperature controller
 *  Uses builtin LED to indicate overheating
 *  
 */
#include <Arduino.h>
#include "Trace.h"
#include "Fan.h"
#include "SerialCommand.h"
#include "Led.h"

 
/** LM35 temperature sensor is connected to this pin */
const int pinTemp = A1;

/** These are the fans we control */
static Fan g_fan[] = {
  {/** the PWM pin where fan is */ 3, /** the sensor input pin */ 2}
};
/** # of fans we control */
const short int iFans = sizeof(g_fan) / sizeof(g_fan[0]);


/** the overheating (builtin) led is on pin 13 */
Led g_led(13);

/** the temperature in C to start the fan */
const unsigned short int tempMin = 26;
/** the maximum temperature in C when fan is at 100% */
const unsigned short int tempMax = 35;

/////////////////////////////////////////////////////////////////////////////////////
// nothing to customize below
//

/** Min observed temperature in C */
unsigned short int g_tempMin = 0;
/** Max observed temperature in C */
unsigned short int g_tempMax = 0;

/** serial command handler */
SerialCommand g_sc;

/** Counter for sensor fan feedback */
//volatile unsigned long int g_uiCounter = 0;

/** called because fan hall sensor generates interupts */
/*void onHallSensor()
{
  g_uiCounter++;
}*/

/**  
 * get the temperature and convert it to Celsius 
 * read analog LM35 sensor, 
 * presumes you did analogReference(INTERNAL); - more precise but smaller range
 */
unsigned short int readTemperature() 
{
  // first sample seems to fluctuate a lot. Disregard it
  {
    unsigned int intmp = analogRead(pinTemp);
    //Serial.println(intmp);
    delay(60);
  }

  // according to http://www.atmel.com/dyn/resources/prod_documents/doc8003.pdf
  // 11 bit virtual resolution arduino ADC is 10 bit real resolution
  // for 12 bit resolution 16 samples and >> 4 is needed
  unsigned int reading = 0; // accumulate samples here
  for(int i=0; i<=3; i++)
  {
    unsigned int intmp = analogRead(pinTemp);
    reading = reading + intmp;
    //Serial.println(intmp);
    delay(60);
  }
  reading = reading>>2; // averaged over 4 samples
  /*
  unsigned int reading = analogRead(pinTemp);
  Serial.print("analogRead() => ");
  Serial.println(reading);
  */
  // 110 mV is mapped into 1024 steps.  analogReference(INTERNAL) needed
  float tempC = (float)reading * 110 / 1024;
  unsigned short temp = (unsigned short)tempC;
  if(temp < g_tempMin)
    g_tempMin = temp;
  else if(temp > g_tempMax)
    g_tempMax = temp;
  return temp;
}
/**
 * Dump some statistics so that we can see how the controller and environment are doing...
 */
void dumpStats()
{
  char buf[80];
  sprintf(buf, "Settings: tempMin=%d, tempMax=%d,", (int)tempMin, (int)tempMax);
  Serial.println(buf);
  int temp = (int)readTemperature();
  sprintf(buf, "Observed: g_tempMin=%d, g_tempMax=%d, temp=%d", (int)g_tempMin, (int)g_tempMax, temp);
  Serial.println(buf);
  unsigned short pwm = g_fan[0].getPWM();
  sprintf(buf, "Fan: pwm=%d", (int)pwm);
  Serial.println(buf);
  sprintf(buf, "Now: %lu ms", millis());
  Serial.println(buf);
}

/**
 * Dump some statistics so that we can see how the controller and environment are doing...
 */
void dumpStats(unsigned long now)
{
  /** how often to dump stats */
  const unsigned long timeStatsDumpPeriod = 30000;
  /** when we dumped stats last */
  static unsigned long g_timeLastStatsDump = 0;
  // the following will handle rollover just fine!
  if(now < (g_timeLastStatsDump + timeStatsDumpPeriod))
    return;
  g_timeLastStatsDump = now;
  dumpStats();
}

/**
 * Fan startup sequence
 */
void fanSetup()
{
  //
  // stop the fan
  //
  for(short int i = 0; i < iFans; i++)
    g_fan[i].stop();
  delay(4*1000);
  //
  // spin the fan at start PWM
  //
  for(short int i = 0; i < iFans; i++)
    g_fan[i].start();
  delay(4*1000);
  //
  // spin the fan at max RPM
  //
  for(short int i = 0; i < iFans; i++)
    g_fan[i].spin(Fan::pwmMax);
  delay(4*1000);
  //
  // spin the fan at min RPM
  //
  for(short int i = 0; i < iFans; i++)
    g_fan[i].spin(Fan::pwmMin);
  delay(4*1000);
}

void onCommandGet() 
{
  char *arg = g_sc.next();
  if(arg == 0)
    return;
  //aNumber = atoi(arg);    // Converts a char string to an integer
  //Serial.print("get arg: "); Serial.println(arg);  
  if(arg[0] == 'F')
  {
    // GET FAN handler
    unsigned short pwmNow = g_fan[0].getPWM();
    Serial.println(pwmNow);
  } 
  else if(arg[0] == 'T')
  {
    // GET TEMP handler
    unsigned short int temp = readTemperature();  
    Serial.println(temp);
  }
  else if(arg[0] == 'S')
  {
    // GET STATS handler
    dumpStats();
  }
  else
  {
    onCommandUnrecognized(0);
  }
}
void onCommandSet() 
{
   char *arg = g_sc.next();
}
void onCommandStats()
{
  dumpStats();  
}
void onCommandUnrecognized(const char *command)
{
  // 
}

void setup() 
{
  Serial.begin(115200);
  // LM35 is not going to provide more than 1V output and that @100C
  // switch to internal 1.1V reference
  analogReference(INTERNAL);  
  pinMode(pinTemp, INPUT);
  g_tempMin = g_tempMax = readTemperature();

  g_led.setup();

  //---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
  TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
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
  fanSetup();

  // Setup callbacks for SerialCommand commands
  g_sc.addCommand("GET", onCommandGet);
  g_sc.addCommand("SET", onCommandSet);
  g_sc.addCommand("STATS", onCommandStats);
  g_sc.addDefaultHandler(onCommandUnrecognized); 
}

void loop() 
{
  if(g_sc.available())
  {
    g_sc.readAndDispatch();
    return;
  }
  // temp in degrees C
  unsigned short int temp = readTemperature();  
  Serial.print("readTemperature() => "); Serial.println(temp);

  if(temp < tempMin) 
  {
    for(short int i = 0; i < iFans; i++)
      g_fan[i].stop();
    g_led.off();
  }  
  else if(temp < tempMax)
  {
    unsigned int pwmFan = map(temp, tempMin, tempMax, Fan::pwmMin, Fan::pwmMax);
    unsigned short pwmNow = g_fan[0].getPWM();
    if(pwmNow == 0) 
    {
      if(pwmFan < Fan::pwmStart) 
        pwmFan = Fan::pwmStart;
    }
    else
    {
      // instead of just spinning at target pwm, lets get there gradually.
      pwmFan = (pwmFan + pwmNow)/2;
    }
    for(short int i = 0; i < iFans; i++)
      g_fan[i].spin(pwmFan);
    g_led.off();
  }
  else
  {
    for(short int i = 0; i < iFans; i++)
      g_fan[i].spin(Fan::pwmMax);
    g_led.on();
  }
  //DEBUG_PRINTLN(g_uiCounter);
  //g_uiCounter = 0;

  dumpStats(millis());  
  delay(4000);
}


