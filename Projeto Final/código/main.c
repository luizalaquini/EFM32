/** ***************************************************************************
 * @file    main.c
 * @brief   Trabalho 2 de Sistemas Embarcados
 * @version 1.0
******************************************************************************/

#include <stdint.h>
/*
 * Including this file, it is possible to define which processor using command line
 * E.g. -DEFM32GG995F1024
 * The alternative is to include the processor specific file directly
 * #include "efm32gg995f1024.h"
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "em_device.h"
#include "pwm.h"
#include "led.h"
#include "gpio.h"
#include "adc.h"
#include "lcd.h"
#include "ultrassom.h"
#include "clock_efm32gg2.h"

#define E_ANDANDO1 1
#define E_ANDANDO2 2 
#define E_MEDINDO 3
#define E_ESQ1 4
#define E_ESQ2 5
#define E_DIR1 6
#define E_DIR2 7
#define E_DIR3 8
#define E_DIR4 9

/*****************************************************************************
 * @brief  SysTick interrupt handler
 *
 * @note   Called every 1/DIVIDER seconds (1 ms)
 */

//{
#define SYSTICKDIVIDER 100000
#define SOFTDIVIDER 1000
#define DELAYVAL 2
#define DELAYVAL2 20
#define MAXDISTANCE 15

int mediaMovelCruzamento = 4095;


int andando1(float *ME, float *MD);
int andando2(float *ME, float *MD);
int medindo();
int esquerda1();
int esquerda2();
int direita1();
int direita2();
int direita3();
int direita4();
void alteraValorLEDs(float ME, float MD);
int estaLendoBranco(int leitura);

int main(void) {
    //Configure inputs
    GPIO_Init(GPIOD, BIT(0), 0);
    GPIO_Init(GPIOD, BIT(1), 0);
    GPIO_Init(GPIOD, BIT(2), 0);
    //Configure Outputs
    GPIO_Init(GPIOD, 0, BIT(7));
    GPIO_Init(GPIOC, 0, BIT(0));

    // Set clock source to external crystal: 48 MHz
    (void) SystemCoreClockSet(CLOCK_HFXO,1,1);
    SysTick_Config(SystemCoreClock/SYSTICKDIVIDER);

    // Enable IRQs
    __enable_irq();

    //Configure Ultrasonic sensor
    Config_Ultrasonic(5, 6, SYSTICKDIVIDER);

    //Configure LCD
    LCD_Init();

    LCD_SetAll();
    Delay(DELAYVAL);
    LCD_ClearAll();
    Delay(DELAYVAL);

    // Configure ADC
    ADC_Init(500000);
    ADC_ConfigChannel(ADC_CH0, 0); //ADC_SINGLECTRL_REF_VDD
    ADC_ConfigChannel(ADC_CH1, 0); //ADC_SINGLECTRL_REF_
    ADC_ConfigChannel(ADC_CH2, 0); //ADC_SINGLECTRL_REF

    // Configure LED PWM
    PWM_Init(TIMER0,PWM_LOC4,PWM_PARAMS_ENABLECHANNEL1);
    PWM_Init(TIMER1,PWM_LOC4,PWM_PARAMS_ENABLECHANNEL1);

    //Máquina de estados
    int estado = E_MEDINDO;

    float ME = 1.0, MD = 0.01;

    while(1){
        switch(estado){
            case E_ANDANDO1:
                //Executa instrução andando
                estado = andando1(&ME, &MD);
                LCD_WriteString("ANDANDO1");
                break;
            case E_ANDANDO2:
                //Executa instrução andando
                estado = andando2(&ME, &MD);
                LCD_WriteString("ANDANDO2");
                break;
            case E_MEDINDO:
                ME = 0.5;
                MD = 0.5;
                alteraValorLEDs(ME, MD);
                //Executa instrução medindo
                estado = medindo();
                LCD_WriteString("MEDINDO");
                ME = 0.5;
                MD = 0.5;
                break;
            case E_ESQ1:
                //Executa instrução esquerda 1
                estado = esquerda1();
                LCD_WriteString("ESQ1");
                ME = 0;
                MD = 1;
                break;
            case E_ESQ2:
                //Executa instrução esquerda 2
                estado = esquerda2();
                LCD_WriteString("ESQ2");
                ME = 0;
                MD = 1;
                break;
            case E_DIR1:
                //Executa instrução direita 1
                estado = direita1();
                LCD_WriteString("DIR1");
                ME = 1;
                MD = 0;
                break;
            case E_DIR2:
                //Executa instrução direita 2
                estado = direita2();
                LCD_WriteString("DIR2");
                ME = 1;
                MD = 0;
                break;
            case E_DIR3:
                //Executa instrução direita 3
                estado = direita3();
                LCD_WriteString("DIR3");
                ME = 1;
                MD = 0;
                break;
            case E_DIR4:
                //Executa instrução direita 4
                estado = direita4();
                LCD_WriteString("DIR4");
                ME = 1;
                MD = 0;
                break;
            default:
                //Executa instrução default
                ME = 0;
                MD = 0;
                break;
        }
        alteraValorLEDs(ME, MD); 
    }     
}

int andando1(float *ME, float *MD){
    int valueReadEsq = ADC_Read(ADC_CH0);
    int valueReadDir = ADC_Read(ADC_CH1);
    int valueReadFr = ADC_Read(ADC_CH2);

    if(valueReadDir>valueReadEsq){
        *ME = 1.0;
        //Find ME value between 0.5 and 1
        float diference = valueReadEsq / valueReadDir;
        //Convert from 0-1 to 0.5 to 1
        diference = 0.5 + diference/2.0;
        *MD = diference;
    } else {
        *MD = 1.0;
        //Find ME value between 0.5 and 1
        float diference = valueReadDir / valueReadEsq;
        //Convert from 0-1 to 0.5 to 1
        diference = 0.5 + diference/2.0;
        *ME = diference;
    }
    
    //Verificar se chegou num cruzamento
    if(estaLendoBranco(valueReadFr)){
        return E_ANDANDO2;
    }   

    Delay(DELAYVAL2);
    return E_ANDANDO1;
}

int andando2(float *ME, float *MD){
    int valueReadEsq = ADC_Read(ADC_CH0);
    int valueReadDir = ADC_Read(ADC_CH1);
    int valueReadFr = ADC_Read(ADC_CH2);


    if(valueReadDir>valueReadEsq){
        *ME = 1.0;
        //Find ME value between 0.5 and 1
        float diference = valueReadEsq / valueReadDir;
        //Convert from 0-1 to 0.5 to 1
        diference = 0.5 + diference/2.0;
        *MD = diference;
    } else {
        *MD = 1.0;
        //Find ME value between 0.5 and 1
        float diference = valueReadDir / valueReadEsq;
        //Convert from 0-1 to 0.5 to 1
        diference = 0.5 + diference/2.0;
        *ME = diference;
    }
    
    //Verificar se chegou num cruzamento
    if(!estaLendoBranco(valueReadFr)){
        return E_MEDINDO;
    }   

    Delay(DELAYVAL2);
    return E_ANDANDO2;
}

int medindo(){
    LCD_WriteString("AP. ESQ");
    Delay(3000);
    char buffer[50];
    int leituraESQ = (int)(Read_Ultrasonic()*100);
    itoa(leituraESQ, buffer, 10);
    LCD_WriteString(buffer);
    Delay(1000);

    LCD_WriteString("AP. FRNT");
    Delay(3000);
    int leituraFRNT = (int)(Read_Ultrasonic()*100);
    itoa(leituraFRNT, buffer, 10);
    LCD_WriteString(buffer);
    Delay(1000);

    LCD_WriteString("AP. DIR");
    Delay(3000);
    int leituraDIR = (int)(Read_Ultrasonic()*100);
    itoa(leituraDIR, buffer, 10);
    LCD_WriteString(buffer);
    Delay(1000);

    if(leituraESQ > MAXDISTANCE){ //Se a esquerda estiver desobstruida, vira pra direita 
        return E_ESQ1;
    }
    if(leituraFRNT > MAXDISTANCE){ //Se a frente estiver desobstruida, segue adiante
        return E_ANDANDO1;
    }    
    if(leituraDIR > MAXDISTANCE){ //Se a direita estiver desobstruida, vira pra direita
        return E_DIR3;
    }

    //Dá a meia volta
    return E_DIR1;
}

int esquerda1(){
    int sensorCruzamento = ADC_Read(ADC_CH2);

    Delay(DELAYVAL2);

    if(estaLendoBranco(sensorCruzamento)){
        return E_ESQ2;
    }
    return E_ESQ1;
}

int esquerda2(){
    int sensorCruzamento = ADC_Read(ADC_CH2);

    Delay(DELAYVAL2);

    if(!estaLendoBranco(sensorCruzamento)){
        return E_ANDANDO1;
    }
    return E_ESQ2;
}

int direita1(){
    int sensorCruzamento = ADC_Read(ADC_CH2);

    Delay(DELAYVAL2);

    if(estaLendoBranco(sensorCruzamento)){
        return E_DIR2;
    }
    return E_DIR1;
}

int direita2(){
    int sensorCruzamento = ADC_Read(ADC_CH2);

    Delay(DELAYVAL2);

    if(!estaLendoBranco(sensorCruzamento)){
        return E_DIR3;
    }
    return E_DIR2;
}

int direita3(){
    int sensorCruzamento = ADC_Read(ADC_CH2);

    Delay(DELAYVAL2);

    if(estaLendoBranco(sensorCruzamento)){
        return E_DIR4;
    }
    return E_DIR3;
}

int direita4(){
    int sensorCruzamento = ADC_Read(ADC_CH2);

    Delay(DELAYVAL2);

    if(!estaLendoBranco(sensorCruzamento)){
        return E_ANDANDO1;
    }
    return E_DIR4;
}

void alteraValorLEDs(float ME, float MD){
    //Convert ME and MD from range 0-1 to 0-65535
    ME = ME * 65535;
    MD = MD * 65535;

    //Escrever MD no PD7
    PWM_Write(TIMER1,1,MD);    
    //Escrever ME no PC0
    PWM_Write(TIMER0,1,ME);
}

int estaLendoBranco(int leitura){
    char buffer[50];

    mediaMovelCruzamento = (2*mediaMovelCruzamento + leitura)/3;

    itoa(mediaMovelCruzamento, buffer, 10);
    //LCD_WriteString(buffer);
    
    if(mediaMovelCruzamento < 1500){
        return 1;
    } 
    return 0;
}