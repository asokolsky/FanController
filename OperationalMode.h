
const short int opModeInvalid = 0;

const short int opModeManualTemperatureSetting = 1;
const short int opModeInternallyMeasuredTemperature = 2;
const short int opModeExternalyMeasuredTemperature = 3;
const short int opModeDirectInternalFanControl = 4;
const short int opModeDirectExternalFanControl = 5;

const short int opModeFirst = opModeManualTemperatureSetting;
const short int opModeLast = opModeDirectExternalFanControl;

/**
 * Abstract class with basic functionality
 */
class OpMode
{
public:
    /** the temperature in C to start the fan */
    static const unsigned short int tempMin = 26;
    /** the maximum temperature in C when fan is at 100% */
    static const unsigned short int tempMax = 35;


    OpMode() : m_opMode(opModeInvalid)
    {
    }
    /** 
     * Basic functionality includes responding to serial commands.
     * Specifically this means setting/getting attributes, but not something like fan spinning 
     */
    virtual bool loop();
    virtual bool onCommandGetTemp();
    virtual bool onCommandSetFan(unsigned short int pwm);
    virtual bool onCommandSetOpMode(unsigned short int mode);
    virtual bool onCommandSetTemp(unsigned short int temp);

    /** accessor */
    short int getOpMode()
    {
      return m_opMode;
    }
protected:
    /** spins the fans according to this temperature */
    void onTemperature(unsigned short int temp);
    /** just to keep track of where we are. */
    short int m_opMode;    
};

extern OpMode *g_pOpMode;

/**
 *
 */
class ManualTemperatureSettingMode : public OpMode
{
public:    
    ManualTemperatureSettingMode()
    {
      m_opMode = opModeManualTemperatureSetting;
    }
    /** respond to potentiometer position as if it is temperature */
    bool loop();
};

extern ManualTemperatureSettingMode g_theManualTemperatureSettingMode;

/**
 *
 */
class InternallyMeasuredTemperatureMode : public OpMode
{
public:
    InternallyMeasuredTemperatureMode()
    {
      m_opMode = opModeInternallyMeasuredTemperature;
    }
    /** respond to LM35 temp measurement */
    bool loop();
};
extern InternallyMeasuredTemperatureMode g_theInternallyMeasuredTemperatureMode;

/**
 * Respond to temps measured externally and supplied via serial port
 */ 
class ExternalyMeasuredTemperatureMode : public OpMode
{
public:
  ExternalyMeasuredTemperatureMode()
  {
    m_opMode = opModeExternalyMeasuredTemperature;
  }
  /** respond to externally measured temp */
  bool loop();
  bool onCommandGetTemp();
  bool onCommandSetTemp(unsigned short int temp);
protected:
  unsigned short m_uTemp = 0;
};
extern ExternalyMeasuredTemperatureMode g_theExternalyMeasuredTemperatureMode;

/**
 * Potentiometer directly sets PWM
 */ 
class DirectInternalFanControlMode : public OpMode
{
public:
    DirectInternalFanControlMode()
    {
      m_opMode = opModeDirectInternalFanControl;
    }
    /** respond to potentiomer setting PWM */
    bool loop();
};
extern DirectInternalFanControlMode g_theDirectInternalFanControlMode;

/**
 * Serial command sets PWM
 */ 
class DirectExternalFanControlMode : public OpMode
{
public:
    DirectExternalFanControlMode()
    {
      m_opMode = opModeDirectExternalFanControl;
    }
    //bool loop();
    /** respond to potentiomer setting PWM */
    bool onCommandSetFan(unsigned short int pwm);

};
extern DirectExternalFanControlMode g_theDirectExternalFanControlMode;

