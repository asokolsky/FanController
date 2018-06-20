# FanController

A universal low cost arduino controller for 12V fans.

## Features

- Upon start up spins up fan from StartPWM to MaxPWM to MinPWM to verify fan functionality;
- Measures ambient temperature using LM35 sensor;
- Spins the fans according to the temperature measured or potentiometer position or command received over serial port;
- Starts spinning the fan (at 30%) when temperature is TempMin (25C) and at TempMax (35C) spin the fan at 100%.  
Relevant: https://en.wikipedia.org/wiki/PID_controller
- Periodically (every 30s) prints statistics, e.g. set points for TempMin and TempMax and observed min and max temps.

## Hardware

For schematics and PCB see:
https://easyeda.com/asokolsky/Fan_Controller-0c20aa3afe5045e5a980d684715a4248

### Main Hardware Components

- internal trimmer/potentiometer;
- internal LM35 temperature sensor;
- built-in LED on Arduino Nano pin 13;
- serial interface to ingest commands, e.g. externally measured temperature;
- firmware logic to derive target fan PWM from internally or externally measured temperature;
- controller PWM fan driver

## Operational Logic

Controller can be in one of the following operational modes.  Serial port commands can be used to switch between the modes.

### 1. Manual Temperature Setting Mode

In this case:

- potentiometer is used to simulate input temperature;
- Firmware logic derives target fan PWM based on this temperature;
- Controller PWM fan driver deliveres desired PWM to the fan.

### 2. Internally Measured Temperature Mode

In this case:

- LM35 temperature sensor measures ambient temperature;
- Firmware logic derives target fan PWM based on this temperature;
- Controller PWM fan driver deliveres desired PWM to the fan.

This is the default opmode.

### 3. Externaly Measured Temperature Mode

In this case:

- External software measures temperature, e.g. that of a CPU or hard drive;
- The temperature is supplied to the controller via serial port;
- Firmware logic derives target fan PWM based on this temperature;
- Controller PWM fan driver deliveres desired PWM to the fan.

### 4. Direct Internal Fan Control Mode

- Potentiometer is used to define fan PWM;
- Controller PWM fan driver deliveres desired PWM to the fan.

### 5. Direct External Fan Control Mode

- external software determines desired fan pwm;
- desired pwm is supplied to the controller via serial port;
- Controller PWM fan driver deliveres desired PWM to the fan.

## TODO

??


## External Software to Communicate with the Controller

On Li/Unix you can read HD temperatures like this:
```
smartctl -a /dev/ada0 | grep Temperature_Celsius | awk '{print $10}'
```
or CPU temperature like this:
```
sysctl -n dev.cpu.0.temperature
```
