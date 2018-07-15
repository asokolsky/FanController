#pragma once

/**
 * PWM-controled fan connected to an output pin
 * Fan sensor may be connected to another input pin
 */
class Fan
{ 
public:  
  /** min fan PWM at which the fan continues to spin */
  static const unsigned short pwmMin = 30; // 16;
  /** min fan PWM value at which a fan can start  */
  static const unsigned short pwmStart = 60; // 30;
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

  void start();
  void stop();
  /** spin the fan at this pwm */
  void spin(unsigned short pwm);
  /** 
   * Setup the fan
   */
  void setup();

protected:
  /** PWM output pin controlling the fan's speed */
  short int m_pinFan;
  /** input pin attached to fan's sensor */
  short int m_pinSensor;
  /** last PWM value we sent to the fan */
  short unsigned m_pwm = 255;
};

extern Fan g_fan[];
/** # of fans we control */
//const short int iFans = 3; //sizeof(g_fan) / sizeof(g_fan[0]);
//extern volatile unsigned long g_ulFanTick;

void fansSetup();
void fansStop();
void fansSpin(unsigned short pwm);
void fansDumpStats(char buf[]);

inline unsigned short fansGetPWM()
{
  return g_fan[0].getPWM();
}

extern void beginCalculateRPM();
extern unsigned long endCalculateRPM();

unsigned long nowMillis();
void myDelay(unsigned long ms);

