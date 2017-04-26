//Camila e Renato

#include "xtea.c.h"

#define LED 4

int strl;
uint32_t key[] = { 1, 2, 3, 4};
uint32_t v2[]   = { 10, 20 };
uint32_t v[] = {'T','E','S','T','E', ' ','T','E','S','T','E', ' ','T','E','S','T','E', ' ','T','E','S','T','E', ' '};
//strl = 24;
int led = 0;
int numExec = 0;
unsigned long t1, t2, b, n;

void setup () {
    pinMode(LED, OUTPUT);
    Serial.begin(9600);
    strl = sizeof(v) / sizeof(uint32_t);
}

void loop () {
    
    n=millis();
    if(n>b+1000){
        b=n;
        digitalWrite(LED, led=!led);
    }
    
    if(numExec%11==0)
        t1 = millis();
    int i=0;
    
    /*Serial.print("antes:");
    for(i=0;i<strl;i++){
        Serial.print(" ");
        Serial.print(v[i]);
    }
    Serial.println(" ");*/

    for(i=0;i<strl;i+=2){
        encipher(32, v+i, key);
    }
    /*Serial.print("durante: ");
    for(i=0;i<strl;i++){
        Serial.print(" ");
        Serial.print(v[i]);
    }
    Serial.println(" ");*/

    for(i=0;i<strl;i+=2){
        decipher(32, v+i, key);
    }

    /*Serial.print("depois: ");
    for(i=0;i<strl;i++){
        Serial.print(" ");
        Serial.print(v[i]);
    }
    Serial.println(" ");  */  
    numExec++;
    if(numExec%11==10){
        t2 = millis();
        Serial.println(t2-t1);
    }
}

