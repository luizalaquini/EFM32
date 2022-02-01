/**
 * @file    main.c
 * @brief   Simple LED Blink Demo for EFM32GG_STK3700
 * @version 1.1
 *
 * @note    Just blinks the LEDs of the STK3700
 *
 * @note    LEDs are on pins 2 and 3 of GPIO Port E
 *
 * @note    It uses a primitive delay mechanism. Do not use it.
 *
 * @author  Hans
 * @date    01/09/2018
 */

#include <stdint.h>
#include "em_device.h"

/**
 * @brief       BIT macro
 *              Defines a constant (hopefully) with a bit 1 in the N position
 * @param       N : Bit index with bit 0 the Least Significant Bit
 */
#define BIT(N) (1U<<(N))

/// Pointer to GPIO Port D registers
GPIO_P_TypeDef * const GPIOD = &(GPIO->P[3]);  // GPIOD
/// Pointer to GPIO Port B registers
GPIO_P_TypeDef * const GPIOB = &(GPIO->P[1]);  // GPIOB

///@{
/// LEDs are on Port D
#define LED1 BIT(0)
#define LED2 BIT(1)
#define LED3 BIT(2)
#define LED4 BIT(3)
#define LED5 BIT(4)
#define LED6 BIT(5)
#define LED7 BIT(6)
#define LED8 BIT(7)

/// Buttons are on Port B
#define PB0 9
#define PB1 10
///@}

/// Default delay value.
#define DELAYVAL 4
/**
 * @brief  Quick and dirty delay function
 * @note   Do not use it in production code
 */
/* EFEITOS:
* 000 Incremento = Vai somando em binário (00000001 -> 00000010 -> 00000011 -> ...)
* 001 Decremento = Vai subtraindo em binário (00000011 -> 00000010 -> 00000001 -> ...)
* 010 Deslocamento à direita de 1 led
* 011 Deslocamento à esquerda de 1 led
* 100 Equalizador = acende todos um por um e depois volta apagando
* 101 Liga/desliga = liga tudo e em sequencia desliga tudo
* 110 Auto fantástico = escolhe 2 e anda na (mesma direção/oposta ?????)
* 111 Própria = acende das extremidades até encontrar no meio
*/
void Incremento();
void Decremento();
void DeslocDir();
void DeslocEsq(); 
void Equalizador();
void OnOff();
void AutoFantastico();
void Propria();

void Delay(uint32_t delay);

int count = 0;
int seq[3];
int statePB0 = 0;
int statePB1 = 0;

void readButton(){
    if(!(GPIOB->DIN & (1 << PB0)) && statePB0 == 0) {   // If PB0 is pressed
        statePB0 = 1;

        seq[count] = 0;
        count++;
        if(count >= 3){
            count = 0;
            //Call the function
            if(seq[0]==0 && seq[1]==0 && seq[2]==0) Incremento();
            else if(seq[0]==0 && seq[1]==0 && seq[2]==1) Decremento();
            else if(seq[0]==0 && seq[1]==1 && seq[2]==0) DeslocDir();
            else if(seq[0]==0 && seq[1]==1 && seq[2]==1) DeslocEsq();
            else if(seq[0]==1 && seq[1]==0 && seq[2]==0) Equalizador();
            else if(seq[0]==1 && seq[1]==0 && seq[2]==1) OnOff();
            else if(seq[0]==1 && seq[1]==1 && seq[2]==0) AutoFantastico();
            else if(seq[0]==1 && seq[1]==1 && seq[2]==1) Propria();
        }
    } else if (statePB0 == 1){
        Delay(3);
        statePB0 = 0;
    }
    
    if(! (GPIOB->DIN & (1 << PB1)) && statePB1 == 0) {  // If PB1 is pressed 
        statePB1 = 1;

        seq[count] = 1;
        count++;
        if(count >= 3){
            count = 0;
            //Call the function
            if(seq[0]==0 && seq[1]==0 && seq[2]==0) Incremento();
            else if(seq[0]==0 && seq[1]==0 && seq[2]==1) Decremento();
            else if(seq[0]==0 && seq[1]==1 && seq[2]==0) DeslocDir();
            else if(seq[0]==0 && seq[1]==1 && seq[2]==1) DeslocEsq();
            else if(seq[0]==1 && seq[1]==0 && seq[2]==0) Equalizador();
            else if(seq[0]==1 && seq[1]==0 && seq[2]==1) OnOff();
            else if(seq[0]==1 && seq[1]==1 && seq[2]==0) AutoFantastico();
            else if(seq[0]==1 && seq[1]==1 && seq[2]==1) Propria();
        }
    } else if (statePB1 == 1){
        Delay(3);
        statePB1 = 0;
    }
}

void Delay(uint32_t delay) {
volatile uint32_t counter;
int i;

    for(i=0;i<delay;i++) {
        counter = 100000;
        while( counter ){
            //readButton();
            counter--;
        } 
    }
}

/**
 * @brief  Main function
 *
 * @note   Using default clock configuration
 * @note   HFCLK     = HFRCO 14 MHz
 * @note   HFCORECLK = HFCLK
 * @note   HFPERCLK  = HFCLK

 */

int main(void) {
    /* Enable Clock for GPIO */
    CMU->HFPERCLKDIV |= CMU_HFPERCLKDIV_HFPERCLKEN;     // Enable HFPERCLK
    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;           // Enable HFPERCKL for GPIO

    /* Configure Pins in GPIOD */
    GPIOD->MODEL = 0x44444444;          // Clear bits

    /* Configure Pins in GPIOB */
    GPIOB->MODEH = (2 << 4) | (2 << 8);          // Clear bits
    GPIOB->DOUT = (1 << PB0) | (1 << PB1);     // Enable Pull-ups on PB0 and PB1

    /* Initial values for LEDs */
    //GPIOD->DOUT = (0x000000FF);       // Turn ON LEDs
    GPIOD->DOUT = 0x00000000;           // Turn OFF LEDs

    while(1){
        readButton();
    }
}


void Incremento(){
    int num=0;
    while(1){
        for(num=0; num<=255; num++){
            GPIOD->DOUT = num;
            Delay(DELAYVAL);
        }
    }

}

void Decremento(){
    int num=0;
    while(1){
        for(num=255; num>=0; num--){
            GPIOD->DOUT = num;
            Delay(DELAYVAL);
        }
    }    
}

void DeslocDir(){
    int num=0;
    while(1){
        for(num=0; num<8; num++){
            GPIOD->DOUT = 128>>(num);
            Delay(DELAYVAL);
        }
    }   
}

void DeslocEsq(){
    int num=0;
    while(1){
        for(num=0; num<8; num++){
            GPIOD->DOUT = 1U<<(num);
            Delay(DELAYVAL);
        }
    }   
}

void Equalizador(){
    int num=0;
    while(1){
        GPIOD->DOUT = 0;
        Delay(DELAYVAL);
        for(num=0; num<8; num++){
            GPIOD->DOUT |= 128>>(num);
            Delay(DELAYVAL);
        }
        for(num=0; num<8; num++){
            GPIOD->DOUT = 0xFF<<(num);
            Delay(DELAYVAL);
        }
    } 
}

void OnOff(){
    while(1){
        GPIOD->DOUT = 0;
        Delay(DELAYVAL);
        GPIOD->DOUT = 0x00FF;
        Delay(DELAYVAL);
    } 
}

void AutoFantastico(){
    int num=0;
    while(1){
        for(num=0; num<7; num++){
            GPIOD->DOUT = 3<<(num);
            Delay(DELAYVAL);
        }
        for(num=5; num>=1; num--){
            GPIOD->DOUT = 3<<(num);
            Delay(DELAYVAL);
        }
    }
}

void Propria(){
    int num=0;
    while(1){
        for(num=0; num<4; num++){
            GPIOD->DOUT = 1<<(num) | 128>>(num);
            Delay(DELAYVAL);
        }
    }  
}