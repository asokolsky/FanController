/**
 *  FAN temperature controller operational modes
 */
#include <Arduino.h>
#include "Trace.h"
#include "Fan.h"
#include "SerialCommand.h"
#include "Led.h"
#include "LM35.h"
#include "OperationalMode.h"

ManualTemperatureSettingMode g_theManualTemperatureSettingMode;
InternallyMeasuredTemperatureMode g_theInternallyMeasuredTemperatureMode;
ExternalyMeasuredTemperatureMode g_theExternalyMeasuredTemperatureMode;
DirectInternalFanControlMode g_theDirectInternalFanControlMode;
DirectExternalFanControlMode g_theDirectExternalFanControlMode;
/** default op mode */
OpMode *g_pOpMode = &g_theManualTemperatureSettingMode;

/**
 * most basic behaviour is response to serial commands
 */
bool OpMode::loop()
{
  if(g_sc.available())
  {
    g_sc.readAndDispatch();
    return true;
  }
  return false;
}

bool OpMode::onCommandGetTemp()
{
  unsigned short int temp = g_lm35.read();
  Serial.println(temp);  
}
bool OpMode::onCommandSetTemp(unsigned short int temp)
{
  DEBUG_PRINTLN("Can't set temp in this mode");
  return false; 
}

bool OpMode::onCommandSetFan(unsigned short int pwm)
{
  DEBUG_PRINTLN("Can't set fan pwm in this mode");
  return false; 
}
bool OpMode::onCommandSetOpMode(unsigned short int mode)
{
  switch(mode)
  {
    case opModeManualTemperatureSetting:
      g_pOpMode = &g_theManualTemperatureSettingMode;
      break;
    case opModeInternallyMeasuredTemperature:
      g_pOpMode = &g_theInternallyMeasuredTemperatureMode;
      break;
    case opModeExternalyMeasuredTemperature:
      g_pOpMode = &g_theExternalyMeasuredTemperatureMode;
      break;
    case opModeDirectInternalFanControl:
      g_pOpMode = &g_theDirectInternalFanControlMode;
      break;
    case opModeDirectExternalFanControl:
      g_pOpMode = &g_theDirectExternalFanControlMode;
      break;
    default:
      DEBUG_PRINT("Can't set mode to "); DEBUG_PRNTLN(mode);
      return false;
  }
  return true; 
}

/**
 * Given this temperature in C (internally or externally measured),
 * set fans PWMs
 */
void OpMode::onTemperature(unsigned short int temp)
{
  if(temp < tempMin) 
  {
    fansStop();
    g_led.off();
  }  
  else if(temp < tempMax)
  {
    unsigned int pwmFan = map(temp, tempMin, tempMax, Fan::pwmMin, Fan::pwmMax);
    unsigned short pwmNow = fansGetPWM();
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
    fansSpin(pwmFan);
    g_led.off();
  }
  else
  {
    fansSpin(Fan::pwmMax);
    g_led.on();
  }    
}

/**
- potentiometer is used to simulate input temperature;
- Firmware logic derives target fan PWM based on this temperature;
- Controller PWM fan driver deliveres desired PWM to the fan.
*/
bool ManualTemperatureSettingMode::loop()
{
  if(OpMode::loop())
    return true;
  unsigned int reading = g_pot.read();
  unsigned int temp = map(reading, 0, 1024, 0, 100);
  onTemperature(temp);
  return false;
}

/**
- Internal temperature sensor measures ambient temperature;
- Firmware logic derives target fan PWM based on this temperature;
- Controller PWM fan driver deliveres desired PWM to the fan.
*/
bool InternallyMeasuredTemperatureMode::loop()
{
  if(OpMode::loop())
    return true;
  onTemperature(g_lm35.read());
  return false;
}

/**
- External software measures temperature, e.g. that of a CPU or hard drive;
- The temperature is supplied to the controller via serial port;
- Firmware logic derives target fan PWM based on this temperature;
- Controller PWM fan driver deliveres desired PWM to the fan.
*/
bool ExternalyMeasuredTemperatureMode::loop()
{
  if(OpMode::loop())
    return true;
  onTemperature(m_uTemp);
  return false;
}
bool ExternalyMeasuredTemperatureMode::onCommandGetTemp()
{
  Serial.println(m_uTemp);  
}
bool ExternalyMeasuredTemperatureMode::onCommandSetTemp(unsigned short int temp)
{
  m_uTemp = temp;
}

/**
- Potentiometer is used to define fan PWM;
- Controller PWM fan driver deliveres desired PWM to the fan.
*/
bool DirectInternalFanControlMode::loop()
{
  // respond to serial commands
  if(OpMode::loop())
    return true;
  // read potentiometer
  unsigned int uReading = g_pot.read();
  // map it into pwm
  unsigned int pwm = map(uReading, 0, 1024, 0, Fan::pwmMax);
  // and set the fan pwm
  fansSpin(pwm);  
  return true;
}

/**
- external software determines desired fan pwm;
- desired pwm is supplied to the controller via serial port;
- Controller PWM fan driver deliveres desired PWM to the fan.
*/
/*bool DirectExternalFanControlMode::loop()
{
  if(OpMode::loop())
    return true;
  return false;
}*/

bool DirectExternalFanControlMode::onCommandSetFan(unsigned short int pwm)
{
  fansSpin(pwm);  
}

