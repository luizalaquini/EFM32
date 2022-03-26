#include "ultrassom.h"
#include "gpio.h"
#include "led.h"

#define SOUND_SPEED 350
#define PWMPERIOD 2000 //2000 ciclos de 10us -> 20ms

//Ultrassom
static int clock_frequency;
static int trigger;
static int echo;

//Systic
static uint32_t ticks = 0;
volatile uint64_t count = 0;

void SysTick_Handler(void) {
    ticks++;  
    count++; 
}

void Delay(int delay) { //Delay em milisegundos
    delay = delay * 100;
    uint64_t l = count+delay;
    while(count<l) {}
}


void Config_Ultrasonic(int triggerPin, int echoPin, int frequency){
    //Configure echo
    GPIO_Init(GPIOD, BIT(echoPin), 0); //Ultrassom Echo
    echo = echoPin;
    //Configure Trigger
    GPIO_Init(GPIOD, 0, BIT(triggerPin)); //Ultrassom Trigger
    trigger = triggerPin;

    GPIO_WritePins(GPIOD, BIT(trigger)|BIT(echo), 0);
    
    clock_frequency = frequency;
}

float Read_Ultrasonic(){
    ticks = 0;
    while(ticks<1);

    //Set trigger to high
    GPIO_WritePins(GPIOD, 0, BIT(trigger));
    while(ticks<26);
    //Set trigger to low after 10us
    GPIO_WritePins(GPIOD,  BIT(trigger), 0);    
    
    while((GPIO_ReadPins(GPIOD) & BIT(echo)) == 0){}
    ticks = 0;
    while((GPIO_ReadPins(GPIOD) & BIT(echo)) > 0){}
    
    float time = ticks*1.0/clock_frequency;
    
    return SOUND_SPEED*time/2.0;
}
