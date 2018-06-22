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
#include "LM35.h"
#include "pcb.h"
#include "OperationalMode.h"


/** LM35 temperature sensor is connected to this pin */
LM35 g_lm35(pinLM35);

unsigned short int LM35::g_tempMin;
unsigned short int LM35::g_tempMax;

/** Potentiometer connected to this pin */
Potentiometer g_pot(pinPotentiometer);

/** the overheating (builtin) led is on pin 13 */
Led g_led(pinLed);

/** Counter for sensor fan feedback */
//volatile unsigned long int g_uiCounter = 0;

/** called because fan hall sensor generates interupts */
/*void onHallSensor()
{
  g_uiCounter++;
}*/

/**
 * analogRead wrapper with better precision
 */
unsigned int myAnalogRead(short int pin)
{
  // first sample seems to fluctuate a lot. Disregard it
  {
    unsigned int intmp = analogRead(pin);
    //Serial.println(intmp);
    delay(60);
  }
  // according to http://www.atmel.com/dyn/resources/prod_documents/doc8003.pdf
  // 11 bit virtual resolution arduino ADC is 10 bit real resolution
  // for 12 bit resolution 16 samples and >> 4 is needed
  unsigned int reading = 0; // accumulate samples here
  for(int i=0; i<=3; i++)
  {
    unsigned int intmp = analogRead(pin);
    reading = reading + intmp;
    //Serial.println(intmp);
    delay(60);
  }
  reading = reading>>2; // averaged over 4 samples
  /*
  unsigned int reading = analogRead(pin);
  Serial.print("analogRead() => ");
  Serial.println(reading);
  */
  return reading;
}

/**
 * This works on Arduinos with a 328 or 168 only
 * https://code.google.com/archive/p/tinkerit/wikis/SecretVoltmeter.wiki
 */
static long readVcc() 
{ 
  long result; 
  // Read 1.1V reference against AVcc 
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); 
  delay(2); 
  // Wait for Vref to settle 
  ADCSRA |= _BV(ADSC); 
  // Convert 
  while (bit_is_set(ADCSRA,ADSC))
    ; 
  result = ADCL; 
  result |= ADCH<<8; 
  result = 1126400L / result; 
  // Back-calculate AVcc in mV 
  return result; 
}

/**
 * This works on Arduinos with a 328 only
 * https://code.google.com/archive/p/tinkerit/wikis/SecretThermometer.wiki
 */
static long readTemp() 
{ 
  long result; 
  // Read temperature sensor against 1.1V reference 
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3); 
  delay(2); 
  // Wait for Vref to settle 
  ADCSRA |= _BV(ADSC); 
  // Convert 
  while (bit_is_set(ADCSRA,ADSC))
    ; 
  result = ADCL; 
  result |= ADCH<<8; 
  result = (result - 125) * 1075; 
  return result; 
}


/**
 * Dump some statistics so that we can see how the controller and environment are doing...
 */
void dumpStats()
{
  char buf[80];
  sprintf(buf, "Vcc=%ld mV, temp=%ld,", readVcc(), readTemp());
  Serial.println(buf);
  sprintf(buf, "Settings: tempMin=%d, tempMax=%d,", (int)OpMode::tempMin, (int)OpMode::tempMax);
  Serial.println(buf);
  int temp = (int)g_lm35.read();
  sprintf(buf, "Observed: g_tempMin=%d, g_tempMax=%d, temp=%d", (int)LM35::g_tempMin, (int)LM35::g_tempMax, temp);
  Serial.println(buf);
  fansDumpStats(buf);
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
 * gettable vars:
 *   OPMODE - current opmode
 *   FAN - fan speed in pwm
 *   TEMP - C reading of the internal temp sensor
 *   TEMP_SETPOINT_MIN - when to start fan
 *   TEMP_SETPOINT_MAX - when to blow fan at full speed
 */
void onCommandGet() 
{
  char *arg = g_sc.next();
  if(arg == 0)
    return;
  DEBUG_PRINT("onCommandGet "); DEBUG_PRNTLN(arg);
  if(arg[0] == 'F')
  {
    // GET FAN handler
    unsigned short pwmNow = fansGetPWM();
    Serial.println(pwmNow);
  } 
  else if(arg[0] == 'O')
  {
    // GET OPMODE handler
    unsigned short int opmode = g_pOpMode->getOpMode();  
    Serial.println(opmode);
  }
  else if(arg[0] == 'T')
  {
    // GET TEMP handler
    if(g_pOpMode != 0)
      g_pOpMode->onCommandGetTemp();
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

/**
 * settable vars:
 *   FAN - fan speed in pwm
 *   TEMP - C reading of the internal temp sensor
 *   TEMP_SETPOINT_MIN - when to start fan
 *   TEMP_SETPOINT_MAX - when to blow fan at full speed
 *   OPMODE
 * Argument is always numeric
 */
void onCommandSet() 
{
  char *arg = g_sc.next();
  if(arg == 0)
    return;
  char *arg1 = g_sc.next();
  if(arg1 == 0)
    return;
  int iArg = atoi(arg1);
  DEBUG_PRINT("onCommandSet "); DEBUG_PRNT(arg); DEBUG_PRINT(" "); DEBUG_PRNTLN(iArg);
  if(arg[0] == 'F')
  {
    // SET FAN handler
    if(g_pOpMode != 0)
      g_pOpMode->onCommandSetFan(iArg);
  } 
  else if(arg[0] == 'O')
  {
    // SET OpMode handler
    if(g_pOpMode != 0)
      g_pOpMode->onCommandSetOpMode(iArg);
  }
  else if(arg[0] == 'T')
  {
    // SET TEMP handler
    if(g_pOpMode != 0)
      g_pOpMode->onCommandSetTemp(iArg);
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
void onCommandStats()
{
  dumpStats();  
}
void onCommandUnrecognized(const char *command)
{
  //
  DEBUG_PRINT("Unrecognized command: ");
  if(command != 0)
  {
    DEBUG_PRNTLN(command);
  }
  else
  {
    DEBUG_PRINTLN("");
  }
}

void setup() 
{
  Serial.begin(115200);
  g_lm35.setup();
  g_lm35.read();
  g_led.setup();
  
  fansSetup();

  // Setup callbacks for SerialCommand commands
  g_sc.addCommand("GET", onCommandGet);
  g_sc.addCommand("SET", onCommandSet);
  g_sc.addCommand("STATS", onCommandStats);
  g_sc.addDefaultHandler(onCommandUnrecognized); 
}

void loop() 
{
  if(g_pOpMode != 0)
    g_pOpMode->loop();
  dumpStats(millis());  
  delay(1000);
}


