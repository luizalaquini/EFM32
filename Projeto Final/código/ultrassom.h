#ifndef ULTRASSOM_H
#define ULTRASSOM_H


//Delay in miliseconds
void Delay(int delay);

void Config_Ultrasonic(int triggerPin, int echoPin, int frequency);

float Read_Ultrasonic();


#endif