/**
 * LED connected to an output pin
 */
class Led
{ 
public:
  Led(short int pin) :
    m_pin(pin)
  {
  }   

  void setup()
  {
    pinMode(m_pin, OUTPUT);
  }
  void off()
  {
    digitalWrite(m_pin, LOW);
  }
  void on()
  {
    digitalWrite(m_pin, HIGH);
  }

private:
  /** pin controlling the led */
  short int m_pin;
};

/** the overheating (builtin) led is on pin 13 */
extern Led g_led;


