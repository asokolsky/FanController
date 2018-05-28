# FanController
Arduino controller for PC fans.

## Features
 - Upon start up spins up fan from StartPWM to MaxPWM to MinPWM.
 - Measures ambient temperature using LM35 sensor.
 - Spins the fan according to the temperature measured.  Starts spinning the fan (at 30%) when temperature is TempMin (25C) and at TempMax (35C) spin the fan at 100%.  Relevant: https://en.wikipedia.org/wiki/PID_controller
 - Periodically (every 30s) prints statistics, e.g. set points for TempMin ad TempMax and observed TempMin and TempMax.

## Hardware

For schematics and PCB see:
https://easyeda.com/asokolsky/Fan_Controller-0c20aa3afe5045e5a980d684715a4248

## TODO

The next big thing is to open up the control loop and offer e.g. a serial port for control of the fans.  Thus, e.g. a NAS box could use internal CPU and/or HDs temperature sensors to decide how fast to spin the fans.

 - Serial commands
 - Control multiple fans

On Li/Unix you can read HD tempslike this:
'''
smartctl -a /dev/ada0 | grep Temperature_Celsius | awk '{print $10}'
'''
or CPU temperature like this:
'''
sysctl -n dev.cpu.0.temperature
'''
