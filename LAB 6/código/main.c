#include <stdint.h>
#include "em_device.h"

#include "gpio.h"

#define DELAYVAL 10000
#define TIME_UNIT 1000 //100 rápido / 1000 - devagar
#define BUTTON_TIME_LIMIT 200

/**
 * @brief   create a bit mask with bit N set
 */
#ifndef BIT
#define BIT(N) (1U<<(N))
#endif

static const int hex[10] = {
   0b00111111, // 0 
   0b00000110, // 1 
   0b01011011, // 2
   0b01001111, // 3
   0b01100110, // 4
   0b01101101, // 5
   0b01111101, // 6
   0b00000111, // 7
   0b01111111, // 8
   0b01101111  // 9
}; 
#define INT_TO_SEG(N) (hex[N])

int ehmanha = 1;
int modo = -1;

int countButton0 = 0;
int countButton1 = 0;

void display_4digits(int d1, int d2, int d3, int d4);
void corrida_livre(int* digitos);
void relogio24h(int* digitos);
void relogio12h(int* digitos);
void checkButtons(int* digitos);
void clearArray(int *digitos);


/**
 * @brief  Quick and dirty delay function
 */

void Delay(uint32_t delay) {
    volatile uint32_t counter;
    int i;

    for(i=0;i<delay;i++) {
        counter = 100;
        while( counter ) counter--;
    }
}

/**
 * @brief  Main function
 *
 * @note   Using default clock configuration
 * @note         HFCLK = HFRCO
 * @note         HFCORECLK = HFCLK
 * @note         HFPERCLK  = HFCLK
 */

int main(void) {

    /* Configure Pins in GPIOD for controlling the segments */
    GPIO_Init(GPIOD, 0, BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5)|BIT(6)|BIT(7)); 
    /* Configure Pins in GPIOC for controlling witch display is active */
    GPIO_Init(GPIOC, 0, BIT(0)|BIT(3)|BIT(4)|BIT(5));
    /* Configure Pins in GPIOB for reading the buttons PB0 and PB1 */
    GPIO_Init(GPIOB, BIT(9)|BIT(10), 0);
    /* Configure Pins in GPIOE for reading the buttons PB0 and PB1 */
    GPIO_Init(GPIOE, 0, BIT(2)|BIT(3));
    
    int vetDigitos[4] = {0, 0, 0, 0};
    /* loop */
    while (1) {
        //Logic to define the next digits to be shown
        switch (modo){
        case 0: // Apertando somente PB0 (Contador de corrida livre)
            corrida_livre(vetDigitos);
            break;
        case 1: // Apertando somente PB1 (Relógio 24h)
            relogio24h(vetDigitos);
            break;
        case 2: // Apertando PB0 e PB1 juntos (Relógio 12h)
            relogio12h(vetDigitos);
            break;
        default:
            clearArray(vetDigitos);
            break;
        }
        
        int i=0;
        for (i=0; i<TIME_UNIT; i++){
            //Function that displays the 4 char numbers given
            display_4digits(vetDigitos[0], vetDigitos[1], vetDigitos[2], vetDigitos[3]);
            checkButtons(vetDigitos);
        }
    }
}

void corrida_livre(int* digitos){
    GPIO_WritePins(GPIOE, BIT(3)|BIT(2),0);
    int i=0;
    digitos[3] ++;
    for(i=3; i>=0; i--){
        if(digitos[i] > 9){
            digitos[i] -= 10;
            if(i == 0){
                clearArray(digitos);
            } else {
                digitos[i-1]++;
            }
        }
    }
}

void relogio24h(int* digitos){
    GPIO_WritePins(GPIOE, BIT(3)|BIT(2),0);
    int i=0;
    digitos[3] ++;
    if(digitos[3] > 9){
        digitos[3] = 0;

        digitos[2] ++;
        if(digitos[2] > 5){
            digitos[2] = 0;
            
            digitos[1]++;
            if(digitos[0] == 2 && digitos[1] == 4){
                clearArray(digitos);
                return;
            } 
            if(digitos[1] > 9){
                digitos[1] = 0;
                digitos[0] ++;
            }
        }
    }
}

void relogio12h(int* digitos){
    int i=0;
    digitos[3] ++;
    if(digitos[3] > 9){
        digitos[3] = 0;

        digitos[2] ++;
        if(digitos[2] > 5){
            digitos[2] = 0;
            
            digitos[1]++;
            if(digitos[0] == 1 && digitos[1] == 2){
                clearArray(digitos);

                if(ehmanha){ // PM
                    ehmanha = 0;
                } else { // AM
                    ehmanha = 1;
                }
            } 
            if(digitos[1] > 9){
                digitos[1] = 0;
                digitos[0] ++;
            }
        }
    }
    if(ehmanha){
        GPIO_WritePins(GPIOE, BIT(3), BIT(2));
    } else {
        GPIO_WritePins(GPIOE, BIT(2), BIT(3));
    }
}

void display_4digits(int d1, int d2, int d3, int d4){
    //Display first digit
    GPIOC->DOUT = 0xFF;
    GPIO_WritePins(GPIOD, ~INT_TO_SEG(d1), INT_TO_SEG(d1));
    GPIOC->DOUT = ~(BIT(0));

    Delay(1);

    //Display first digit
    GPIOC->DOUT = 0xFF;
    GPIO_WritePins(GPIOD, ~INT_TO_SEG(d2), INT_TO_SEG(d2));
    GPIOC->DOUT = ~(BIT(3));

    Delay(1);

    //Display first digit
    GPIOC->DOUT = 0xFF;
    GPIO_WritePins(GPIOD, ~INT_TO_SEG(d3), INT_TO_SEG(d3));
    GPIOC->DOUT = ~(BIT(4));

    Delay(1);

    //Display first digit
    GPIOC->DOUT = 0xFF;
    GPIO_WritePins(GPIOD, ~INT_TO_SEG(d4), INT_TO_SEG(d4));
    GPIOC->DOUT = ~(BIT(5));

    Delay(1);
}

void checkButtons(int* vetDigitos){
    if(countButton0 > BUTTON_TIME_LIMIT){
        //Verifica se o button1 está pressionado countButton1 > 0
        if(countButton1 > 0){
            //Ambos os botões estão pressionados
            countButton0 = 0;
            countButton1 = 0;
            if(modo != 2){
                modo = 2;
                clearArray(vetDigitos);
            }
        } else {
            countButton0 = 0;
            countButton1 = 0;
            if(modo != 0){
                modo = 0;
                clearArray(vetDigitos);
            }
        }
    }
    if(countButton1 > BUTTON_TIME_LIMIT){
        //Verifica se o button0 está pressionado countButton0 > 0
        if(countButton0 > 0){
            //Ambos os botões estão pressionados
            countButton0 = 0;
            countButton1 = 0;
            if(modo != 2){
                modo = 2;
                clearArray(vetDigitos);
            }
        } else {
            countButton0 = 0;
            countButton1 = 0;
            if(modo != 1){
                modo = 1;
                clearArray(vetDigitos);
            }
        }
    }

    if(!(GPIOB->DIN & BIT(9))){ //Verifica se o PB0 está pressionado
        countButton0++;
    } else {
        countButton0 = 0;
    }

    if(!(GPIOB->DIN & BIT(10))){ //Verifica se o PB1 está pressionado
        countButton1++;
    } else {
        countButton1 = 0;
    }
}

void clearArray(int *digitos){
    digitos[0] = 0;
    digitos[1] = 0;
    digitos[2] = 0;
    digitos[3] = 0;
}

