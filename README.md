# EECE-321-Smart-Stoplight
Smart Stop Light Implementation for Tiva C Microcontroller

This is Chris Erb and Conor Johnson's Embedded Systems Design Project. We are using the Tiva C microcontroller to make a more efficient, smart stoplight that utilizes motion sensors, the onboard temperature sensor, and a moisture detector to determine the length of the stoplight cycle.

This repository contains all of the system's source code.<br />

## Assembled Unit
<img src="https://github.com/chris-erb/EECE-321-Smart-Stoplight/assets/145140858/c0ab629a-b003-4bf3-acf4-45e82b925aa4" width="700" height="700"/> <br />

### Demonstration Video: <br />
https://youtu.be/4dspw7MxHMw](https://www.youtube.com/watch?v=4dspw7MxHMw&ab_channel=ChrisErb)https://www.youtube.com/watch?v=4dspw7MxHMw&ab_channel=ChrisErb

## Demonstration Hardware
* Board: Tiva C Microcontroller
* Sensors: Onboard temperature sensor
  - 1x LM393 moisture sensor
  - 2x HC-SR501 PIR motion sensors
* 5x LED Output (2 green, 2 red, 1 yellow)
* 3x 1kΩ resistor
* 3x DIP slide switch <br />
### Reference Images of Hardware: <br />
<img src="https://github.com/chris-erb/EECE-321-Smart-Stoplight/assets/145140858/572a7251-0c13-42c0-93f5-986c042336bb" width="200" height="200"/> &emsp;&emsp; <img src="https://github.com/chris-erb/EECE-321-Smart-Stoplight/assets/145140858/5a23814e-a905-4764-8663-d75ed5b6c618" width="200" height="200"/> &emsp;&emsp; <img src="https://github.com/chris-erb/EECE-321-Smart-Stoplight/assets/145140858/0ba817b8-4eb0-4582-b1a1-b41aee0eb055" width="200" height="200"/>

&emsp; LM393 moisture sensor &emsp;&emsp;&emsp;    HC-SR501 PIR motion sensor &emsp;&emsp;&emsp;&emsp;    DIP slide switch <br />

## Set Stoplight Delay Times <br />
  Within the provided main file, the stop light is optimized for a half cycle of 5 seconds, a regular cycle of 10 seconds (including the time to detect whether motion occurred), and a double cycle of 20 seconds. When calling the functions, it is possible to maintain the current optimized values but multiply them by an integer to increase the duration; in the case of the regular cycle, the motion detection time must also be multiplied by the integer. Additionally, the call of the yellowLightOn functions can be modified to have more or less green-red flashes.<br />
  '''
  void regularCycle1(){ <br />
    //Cycle 1 <br />
    greenLightOn2(); <br />
    redLightOn1(80000); // integer value of 2, for example, would double the original delay of 10 seconds by using 2 * 80000 = 160000 (plus the 2 * motion check duration) <br />
    yellowLightOn2(0, 8); // the 8 denotes 4 red light flashes going: off-on-off-on-off-on-off-on <br />
    redLightOff1(); <br />
} <br />
''' <br />
