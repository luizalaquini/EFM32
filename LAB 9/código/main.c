/** ***************************************************************************
 * @file    main.c
 * @brief   Simple UART Demo for EFM32GG_STK3700
 * @version 1.0
******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*
 * Including this file, it is possible to define which processor using command line
 * E.g. -DEFM32GG995F1024
 * The alternative is to include the processor specific file directly
 * #include "efm32gg995f1024.h"
 */
#include "em_device.h"
#include "clock_efm32gg2.h"

#include "gpio.h"
#include "led.h"
#include "uart.h"
#include "pwm.h"
#include "adc.h"


void execCommand(char *command);
void readPrintButtons();
void printAnalogRead();

/*****************************************************************************
 * @brief  Main function
 *
 * @note   Using external crystal oscillator
 *         HFCLK = HFXO
 *         HFCORECLK = HFCLK
 *         HFPERCLK  = HFCLK
 */

int main(void) {
    ClockConfiguration_t clockconf;
    int cntprint=0, iCommand = 0;
    unsigned ch;
    char command[100];

    /* Configure LEDs */
    LED_Init(LED1|LED2);

    // Set clock source to external crystal: 48 MHz
    (void) SystemCoreClockSet(CLOCK_HFXO,1,1);

    #if 1
    ClockGetConfiguration(&clockconf);
    #endif
    /* Turn on LEDs */
    LED_Write(0,LED2);

    /* Configure UART */
    UART_Init();

    // Configure LED PWM
    PWM_Init(TIMER3,PWM_LOC1,PWM_PARAMS_ENABLECHANNEL2);
    PWM_Write(TIMER3, 2, 0);

    //Configure ADC read
    ADC_Init(500000);
    ADC_ConfigChannel(ADC_CH0, 0); 

    /* Configure Pins in GPIOB for reading the buttons PB0 and PB1 */
    GPIO_Init(GPIOB, BIT(9)|BIT(10), 0);

    ch = '*';
    while (1) {

        if( (ch = UART_GetCharNoWait()) != 0 ) {    
            if( (ch == '\n') ){ //Término da entrada
                //Coloca \0 no fim da string
                command[iCommand] = '\0';
                iCommand = 0;
                //Chama tratador da string
                execCommand(command);
            } else if(ch != '\r') {
                //Preenche próximo caracter da string
                if(iCommand < 100){
                    command[iCommand] = ch;
                    iCommand++;                
                } else {
                    iCommand = 0;
                }
            }
        }

        if(cntprint >= 1000000){
            //Imprime os estados dos botões
            readPrintButtons();

            //Imprime a leitura analógica (porta ADC)
            printAnalogRead();

            cntprint=0;
        }
        cntprint++;
    }

}

void readPrintButtons(){
    int buttons = GPIO_ReadPins(GPIOB);

    UART_SendString("\n\n//////////\n");    

    if((buttons & BIT(9)) > 0){
        UART_SendString("Button PB0 is NOT pressed\n");    
    } else {
        UART_SendString("Button PB0 is pressed\n");    
    }

    if((buttons & BIT(10)) > 0){
        UART_SendString("Button PB1 is NOT pressed\n");    
    } else {
        UART_SendString("Button PB1 is pressed\n");    
    }
}

void printAnalogRead(){
    char buffer[100];
    int value = ADC_Read(ADC_CH0);

    value = 1000 * 3.3 * value / 4095.0; //Convert from 2^12 to volts

    UART_SendString("Sensor read is: ");
    UART_SendString(itoa(value, buffer, 10));
    UART_SendString(" mV\n");
}


void execCommand(char *command){
    UART_SendString(command);
    UART_SendChar('\n');

    if(strcmp(command, "LED1_ON") == 0){
        LED_On(LED2);
        return;
    }
    if(strcmp(command, "LED1_OFF") == 0){
        LED_Off(LED2);
        return;
    }
    
    if(strcmp(command, "LED0_0") == 0){
        //0 - 65535
        PWM_Write(TIMER3, 2, 0);
        return;
    }
    if(strcmp(command, "LED0_25") == 0){
        //0 - 65535
        PWM_Write(TIMER3, 2, 16384);
        return;
    }
    if(strcmp(command, "LED0_50") == 0){
        //0 - 65535
        PWM_Write(TIMER3, 2, 32767);
        return;
    }
    if(strcmp(command, "LED0_75") == 0){
        //0 - 65535
        PWM_Write(TIMER3, 2, 49151);
        return;
    }
    if(strcmp(command, "LED0_100") == 0){
        //0 - 65535
        PWM_Write(TIMER3, 2, 65535);
        return;
    }

    UART_SendString("Erro! Comando não existente\n");
}