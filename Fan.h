
/**
 * PWM-controled fan connected to an output  pin
 * Fan sensor may be connected to another input pin
 */
class Fan
{ 
  /** PWM pin controlling the fan's speed */
  short int m_pinFan;
  /** input pin attached to fan's sensor */
  short int m_pinSensor;

  /** last PWM value we sent to the fan */
  unsigned short m_pwm = 0;
  
  public:
  
  /** min fan PWM value to use */
  static const unsigned short pwmMin = 16;
  /** min fan PWM value at which a fan can start  */
  static const unsigned short pwmStart = 30;
  /** max fan PWM value to use */
  static const unsigned short pwmMax = 255;
  
  /**
   * 
   */
  Fan(short int pinFan, short int pinSensor) :
    m_pinFan(pinFan), m_pinSensor(pinSensor)
  {
  }
  
  bool isSpinning()
  {
    return (m_pwm != 0);
  }
  unsigned short getPWM()
  {
    return m_pwm;
  }

  void start()
  {
    DEBUG_PRINTLN("Starting fan.. ");
    spin(pwmStart);
  }
  void stop()
  {
    DEBUG_PRINTLN("Stopping fan...");
    spin(LOW);
  }
  
  void spin(unsigned short pwm)
  {
    analogWrite(m_pinFan, pwm);
    m_pwm = pwm;
    DEBUG_PRINT("analogWrite(pinFan) "); DEBUG_PRNTLN(pwm);    
  }
  void setup()
  {
    pinMode(m_pinFan, OUTPUT);
    pinMode(m_pinSensor, INPUT);
  }


  /**
   * utility to find the min PWM at which this fan starts
   */
  void test()
  {
    Serial.println("Starting a test to find a min pwm at which this fan can be started...");
    Serial.println("Note the value when we first FAIL to start the fan");
    for(unsigned short int pwm = 90; pwm > 10; pwm = pwm - 8)
    {
      stop();
      delay(4*1000);
      // try to start the fan
      spin(pwm);
      delay(4*1000);
    }    
  }
  
};


