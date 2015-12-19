/**
 *  FAN temperature controller
 *  Use builtin LED to indicate overheating
 *  
 */

#define DEBUG 1

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTDEC(x) Serial.print(x, DEC)
  #define DEBUG_PRINTHEX(x) Serial.print(x, HEX)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTHEX(x)
  #define DEBUG_PRINTLN(x)
#endif 

 
/** LM35 temperature sensor is connected to this pin */
const int pinTemp = A1;
/** the PWM pin where fan is */
const int pinFan = 3;
/** the sensor input pin */
const int pinFanSensor = 2;
/** the overheating led pin - use a built-in one */
const int pinLed = 13;

/** the temperature in C to start the fan */
const unsigned short int tempMin = 26; // 30;
/** the maximum temperature in C when fan is at 100% */
const unsigned short int tempMax = 35; // 70;

/** Min observed temperature in C */
unsigned short int g_tempMin = 0;
/** Max observed temperature in C */
unsigned short int g_tempMax = 0;

/** min fan PWM value to use */
const int pwmMin = 16;
/** min fan PWM value at which a fan can start  */
const int pwmStart = 30;
/** max fan PWM value to use */
const int pwmMax = 255;

//
// nothing to customize below
//

/** last PWM value we sent to the fan */
static unsigned short int g_pwmFan = 0;

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
  return (unsigned short)tempC;
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
  if((unsigned long)(now - g_timeLastStatsDump) > timeStatsDumpPeriod) 
  {
    g_timeLastStatsDump = now;
    char buf[256];
    sprintf(buf, "Settings: tempMin=%d, tempMax=%d,", (int)tempMin, (int)tempMax);
    Serial.println(buf);
    sprintf(buf, "Observed: g_tempMin=%d, g_tempMax=%d", (int)g_tempMin, (int)g_tempMax);
    Serial.println(buf);
  }
}

/**
 * utility to find the min PWM at which this fan starts
 */
void fanTest()
{  
  Serial.println("Starting a test to find a min pwm at which this fan can be started...");
  Serial.println("Note the value when we first FAIL to start the fan");
  for(unsigned short int pwm = 90; pwm > 10; pwm = pwm - 8)
  {
    //
    // stop the fan
    //
    analogWrite(pinFan, LOW);
    DEBUG_PRINT("Stopping fan.. analogWrite(pinFan) ");
    DEBUG_PRINTLN(LOW);
    delay(4*1000);
    // try to start the fan
    analogWrite(pinFan, pwm);
    DEBUG_PRINT("Starting fan.. analogWrite(pinFan) ");
    DEBUG_PRINTLN(pwm);
    delay(4*1000);
  }
}

/**
 * Fan startup sequence
 */
void fanSetup()
{
  //
  // stop the fan
  //
  analogWrite(pinFan, LOW);
  DEBUG_PRINT("Stopping fan.. analogWrite(pinFan) ");
  DEBUG_PRINTLN(LOW);
  delay(4*1000);
  //
  // spin the fan at start PWM
  //
  analogWrite(pinFan, pwmStart);
  DEBUG_PRINT("Starting fan.. analogWrite(pinFan) ");
  DEBUG_PRINTLN(pwmStart);
  delay(4*1000);
  //
  // spin the fan at max RPM
  //
  analogWrite(pinFan, pwmMax);
  DEBUG_PRINT("Max fan spin... analogWrite(pinFan) ");
  DEBUG_PRINTLN(pwmMax);
  delay(4*1000);
  //
  // spin the fan at min RPM
  //
  analogWrite(pinFan, pwmMin);
  DEBUG_PRINT("Min fan spin... analogWrite(pinFan) ");
  DEBUG_PRINTLN(pwmMin);
  delay(4*1000);
}

void setup() 
{
  Serial.begin(115200);

  // LM35 is not going to provide more than 1V output and that @100C
  // switch to internal 1.1V reference
  analogReference(INTERNAL);

  pinMode(pinFan, OUTPUT);
  pinMode(pinFanSensor, INPUT);
  pinMode(pinLed, OUTPUT);
  pinMode(pinTemp, INPUT);

  //---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
  TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz

  //attachInterrupt(0, onHallSensor, RISING);
  g_tempMin = g_tempMax = readTemperature();

  //fanTest();
  fanSetup();
}

void loop() 
{
  // temp in degrees C
  unsigned short int temp = readTemperature();
  if(temp < g_tempMin)
    g_tempMin = temp;
  else if(temp > g_tempMax)
    g_tempMax = temp;
  
  Serial.print("readTemperature() => ");
  Serial.println(temp);

  if(temp < tempMin) 
  {
    digitalWrite(pinFan, LOW);
    digitalWrite(pinLed, LOW);
    g_pwmFan = 0;
  }  
  else if(temp < tempMax)
  {
    unsigned int pwmFan = map(temp, tempMin, tempMax, pwmMin, pwmMax);
    if(g_pwmFan == 0) {
      if(pwmFan < pwmStart) 
        pwmFan = pwmStart;
    }
    else
    {
      // instead of just spinning at target pwm, lets get there gradually.
      pwmFan = (pwmFan + g_pwmFan)/2;
    }
    analogWrite(pinFan, pwmFan);
    DEBUG_PRINT("analogWrite(pinFan) ");
    DEBUG_PRINTLN(pwmFan);
    digitalWrite(pinLed, LOW);
    g_pwmFan = pwmFan;
  }
  else
  {
    analogWrite(pinFan, pwmMax);
    digitalWrite(pinLed, HIGH);  // turn overheating led ON
    g_pwmFan = pwmMax;
  }
  //DEBUG_PRINTLN(g_uiCounter);
  //g_uiCounter = 0;

  dumpStats(millis());
  
  delay(4000);
}


