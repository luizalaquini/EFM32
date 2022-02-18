/** ***************************************************************************
 * @file    main.c
 * @brief   Simple LED Blink Demo for EFM32GG_STK3700
 * @version 1.0
******************************************************************************/

#include <stdint.h>
/*
 * Including this file, it is possible to define which processor using command line
 * E.g. -DEFM32GG995F1024
 * The alternative is to include the processor specific file directly
 * #include "efm32gg995f1024.h"
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "em_device.h"
#include "led.h"
#include "pwm.h"
#include "gpio.h"


/*****************************************************************************
 * @brief  Main function
 *
 * @note   Using default clock configuration
 *         HFCLK = HFRCO
 *         HFCORECLK = HFCLK
 *         HFPERCLK  = HFCLK
 */

void Delay(uint32_t delay) {
    volatile uint32_t counter;
    int i;

    for(i=0;i<delay;i++) {
        counter = 100000;
        while( counter ) counter--;
    }
}


int main(void) {
    /* Configure LEDs */
    LED_Init(LED1);
    //Configure inputs
    GPIO_Init(GPIOD, BIT(0)|BIT(1)|BIT(2)|BIT(3) , 0);

    // Configure LED PWM
    PWM_Init(TIMER3,PWM_LOC1,PWM_PARAMS_ENABLECHANNEL2);

    while(1){
        int value = 0;
        if((GPIO_ReadPins(GPIOD) & BIT(0))){ // Verdadeiro quando PD0 estiver em VCC
            value += 1<<0;
        } 

        if((GPIO_ReadPins(GPIOD) & BIT(1))){ // Verdadeiro quando PD1 estiver em VCC
             value += 1<<1;
        } 

        if((GPIO_ReadPins(GPIOD) & BIT(2))){ // Verdadeiro quando PD2 estiver em VCC
            value += 1<<2;
        } 

        if((GPIO_ReadPins(GPIOD) & BIT(3))){ // Verdadeiro quando PD3 estiver em VCC
            value += 1<<3;
        }
        //Value 0 - 15

        value *= 4369; // max dividido por 15

        //0 - 65535
        PWM_Write(TIMER3,2,value);

        Delay(2);
    }   
}








