# FanController
Arduino controller for PC fans.

## Features
 - Upon start up spins up fan from StartPWM to MaxPWM to MinPWM.
 - Measures ambient temperature using LM35 sensor.
 - Spins the fan according to the temperature measured.  Relevant: https://en.wikipedia.org/wiki/PID_controller


## TODO
 - Serial commands
 - Control multiple fans
 - Collect stats - runtime, min max avg temps

