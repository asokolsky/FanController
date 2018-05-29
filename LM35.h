/**
 * Arduino wrapper for a simple analog temperature sensor - LM35
 */
class LM35
{
public:
  LM35(short int pin) :
    m_pin(pin)
  {

  }

  void setup()
  {
    // LM35 is not going to provide more than 1V output and that @100C
    // switch to internal 1.1V reference
    analogReference(INTERNAL);  
    pinMode(m_pin, INPUT);
  }

  /**  
  * get the temperature and convert it to Celsius 
  * read analog LM35 sensor, 
  * presumes you did analogReference(INTERNAL); - more precise but smaller range
  */
  unsigned short int read()
  {
    // first sample seems to fluctuate a lot. Disregard it
    {
      unsigned int intmp = analogRead(m_pin);
      //Serial.println(intmp);
      delay(60);
    }

    // according to http://www.atmel.com/dyn/resources/prod_documents/doc8003.pdf
    // 11 bit virtual resolution arduino ADC is 10 bit real resolution
    // for 12 bit resolution 16 samples and >> 4 is needed
    unsigned int reading = 0; // accumulate samples here
    for(int i=0; i<=3; i++)
    {
      unsigned int intmp = analogRead(m_pin);
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

private:
  short int m_pin;
};

