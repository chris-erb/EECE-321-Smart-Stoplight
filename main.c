// Final Project - Smart Stop Light //

#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"

void delay(uint32_t milliseconds){
    // Configure Timer0
    SYSCTL_RCGCTIMER_R |= 0x01; // Enable clock for Timer0
    TIMER0_CTL_R = 0; // Disable Timer0 during setup
    TIMER0_CFG_R = 0x04; // 16-bit mode
    TIMER0_TAMR_R = 0x02; // Periodic mode
    TIMER0_TAILR_R = 4000 - 1; // Load value for 1ms (4MHz clock)
    TIMER0_ICR_R = 0x01; // Clear the Timer0 timeout flag

    // Enable Timer0
    TIMER0_CTL_R |= 0x01;

    // Delay for the specified number of milliseconds
    uint32_t i;
    for (i = 0; i < milliseconds; i++){
        while ((TIMER0_RIS_R & 0x01) == 0); // Wait until Timer0 times out
        TIMER0_ICR_R = 0x01; // Clear the Timer0 timeout flag
    }

    // Disable Timer0
    TIMER0_CTL_R &= ~0x01;
}

void greenLightOff1(){
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);
}

void greenLightOff2(){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 0);
}

void redLightOff1(){
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);
}

void redLightOff2(){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0);
}

void greenLightOn1(){
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
}

void greenLightOn2(){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4);
}

void yellowLightOn1(int x, int y){ // Left yellow light
    while(x != y){
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, ~GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0));
        SysCtlDelay(SysCtlClockGet() / 6);
        x++;
    }
    greenLightOff1();
    redLightOff1();
}

void yellowLightOn2(int x, int y){ // Right yellow light
    while(x != y){
        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, ~GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5));
        SysCtlDelay(SysCtlClockGet() / 6);
        x++;
    }
    greenLightOff2();
    redLightOff2();
}

void redLightOn1(uint32_t x){
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);
    delay(x);
}

void redLightOn2(uint32_t x){
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);
    delay(x);
}

int motionCheckCycle1(int x, int y){
    int out = 0;
    greenLightOn2();
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);

    while(x != y){

            if(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3) == GPIO_PIN_3){ // Check if pin is HIGH
                out++;
            }
            SysCtlDelay(SysCtlClockGet() / 6);
            x++;
        }
    return out;
}

int motionCheckCycle2(int x, int y){
    int out = 0;
    greenLightOn1();
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);

    while(x != y){
            if(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2) == GPIO_PIN_2){ // Check if pin is HIGH
                out++;
            }
            SysCtlDelay(SysCtlClockGet() / 6);
            x++;
        }
    return out;
}

void regularCycle1(){
    //Cycle 1
    greenLightOn2();
    redLightOn1(80000); // Value accounts for software delay from motion sensor
    yellowLightOn2(0, 8);
    redLightOff1();
}

void regularCycle2(){
    // Cycle 2
    greenLightOn1();
    redLightOn2(80000); // Value accounts for software delay from motion sensor
    yellowLightOn1(0, 8);
    redLightOff2();
}

void noMotionCycle1(){
    greenLightOn2();
    redLightOn1(5);
    yellowLightOn2(0, 8);
    redLightOff1();
}

void noMotionCycle2(){
    greenLightOn1();
    redLightOn2(5);
    yellowLightOn1(0, 8);
    redLightOff2();
}

void halfCycle(){
    // Half Cycle 1
    greenLightOn2();
    redLightOn1(70000);
    yellowLightOn2(0, 8);
    redLightOff1();
    // Half Cycle 2
    greenLightOn1();
    redLightOn2(70000);
    yellowLightOn1(0, 8);
    redLightOff2();
}

void doubleCycle(){
    // Double Cycle 1
    greenLightOn2();
    redLightOn1(245000);
    yellowLightOn2(0, 8);
    redLightOff1();
    // Double Cycle 2
    greenLightOn1();
    redLightOn2(245000);
    yellowLightOn1(0, 8);
    redLightOff2();
}



int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); // 4MHz clock speed for better responsiveness and throughput

    // Variable Declaration
    uint8_t switchState1;
    uint8_t switchState2;
    uint8_t switchState3;
    uint8_t moistureSensor;
    int check, check2;
    uint32_t ui32ADC0Value[1];
    double CelsiusTemperature;
    double FahrenheitTemperature;

    /*
     *  - - - - - - Temperature Sensor - - - - - -
     *  If the temperature reads below 0C, it will double the X second sequence
     *  switch time to minimize the start & stop of traffic
     *
     *  If the temperature reads above 27C, it will half the X second sequence
     *  switch time to decrease traffic build up at a particular light
     */

    // Temperature Code
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // ADC0 peripheral configuration
    ADCHardwareOversampleConfigure(ADC0_BASE, 64); // Enables hardware averaging
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);


    /*
     *  - - - - - - Breadboard Implementation - - - - - -
     */

    // Breadboard Code
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // Enable the peripheral for GPIO Port A
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // Enable the peripheral for GPIO Port B (5V out)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); // Enable the peripheral for GPIO Port C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); // Enable the peripheral for GPIO Port D
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // Enable the peripheral for GPIO Port E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Enable the peripheral for GPIO Port F

    // Outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5); // Configure PB5 as output (left red light)
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0); // Configure PB0 as output (left green light)
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4); // Configure PB5 as output (right red light)
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5); // Configure PB5 as output (right green light)
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4); // Switch 1-3
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // Half Cycle / Double Cycle indicator

    // Inputs
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_4); // Switch 1
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_5); // Switch 2
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6); // Switch 3
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_3); // Moisture sensor
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_4); // Motion sensor 1
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2); // Motion sensor 2

    // Power
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4); // Switch 1-3 power
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3); // Moisture sensor power

    // 5V for sensors
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2); // Configure PB2 as output (5V)
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_3); // Configure PB3 as output (5V)
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7); // Configure PB7 as output (5V)
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3); // 5V out for moisture sensor
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2); // 5V out for motion sensor 1
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_PIN_7); // 5V out for motion sensor 2

    while(1){
        // Switch states & Moisture check
       switchState1 = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_4); // On switch
       switchState2 = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_5); // Switch for double cycle
       switchState3 = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6); // Switch for half cycle
       moistureSensor = GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_4); // Read moisture

       // Obtain temperature
       ADCIntClear(ADC0_BASE, 3);
       ADCProcessorTrigger(ADC0_BASE, 3);

       while (!ADCIntStatus(ADC0_BASE, 3, false)) {}

       ADCSequenceDataGet(ADC0_BASE, 3, ui32ADC0Value);
       CelsiusTemperature = (1475 - ((2475 * ui32ADC0Value[0]) / 4096)) / 10.0;
       FahrenheitTemperature = (CelsiusTemperature * 9 / 5) + 32;

        // Traffic light implementation
        /*
           * - - - - - - Motion - - - - - -
           * Stop light will wait X seconds to switch between the light sequence
           *
           * If no motion occurs after X seconds, the light sequence will switch
           */

        if(switchState1 != 0){
            if((moistureSensor == GPIO_PIN_4) && (switchState2 == GPIO_PIN_5) &&  (switchState3 == GPIO_PIN_6) && (FahrenheitTemperature > 32) && (FahrenheitTemperature < 90)){
                check = motionCheckCycle1(0, 6);

                if(check != 0){
                    regularCycle1();
                } else{
                    noMotionCycle1();
                }

                check2 = motionCheckCycle2(0, 6);

               if(check2 != 0){
                   regularCycle2();
               } else{
                   noMotionCycle2();
               }

            } else if((moistureSensor == 0) || (switchState2 == 0) || (FahrenheitTemperature < 32)){ // Double cycle from water and/or freezing detection
                doubleCycle();

            } else if((switchState3 == 0) || (FahrenheitTemperature > 90)){ // Half cycle from clear conditions
                halfCycle();

           }
        }
    }
}
