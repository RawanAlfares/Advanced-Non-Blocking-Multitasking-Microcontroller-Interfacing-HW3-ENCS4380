#include <Arduino.h>
#include <avr/io.h>

#define LED_A (1<<PB0)
#define LED_B (1<<PB1)
#define LED_C (1<<PB2)
#define LED_D (1<<PB3)
#define BUTTON1 (1<<PD2)
#define LED_BONUS (1 << PB4)

//LED A
bool stateA=HIGH;
unsigned PreviousA=0;
//LED C
bool stateC=HIGH;
unsigned PreviousC=0;

//BOUNCING
bool stableButton=HIGH;
bool lasRawButton=HIGH;
unsigned long debounceStart=0;
unsigned long pressStart=0;
unsigned long previousSafetyFlash=0;

bool saftyMode=false;
bool LedState=LOW;

//serial 
unsigned long previousSerial = 0;

void setup_reg(){
    //OUTPUT PB0-PB4 as output
    DDRB|=(LED_A|LED_B|LED_C|LED_D|LED_BONUS);

    //PD2 as input
    DDRD &=~BUTTON1;

    // Enable internal pull-up resistor
    PORTD |=BUTTON1;
}

void Led_A(){
    unsigned long currentTime=millis();
    if (stateA==HIGH){
        if(currentTime - PreviousA >=200){
            stateA=LOW;
            PreviousA=currentTime;
            PORTB &=~LED_A ;
        }
    }else{
        if(currentTime - PreviousA >=800){
            stateA=HIGH ;
            PreviousA=currentTime;
            PORTB |=LED_A ;
        }
    }
}

void Led_B(){
    unsigned long currentTime=millis();
    if(currentTime - PreviousA >=350){
        PORTB^=LED_B;
        PreviousA=currentTime;
    }
}

void Led_C(){
    unsigned long currentTime=millis();
    if (stateC==HIGH){
        if(currentTime - PreviousC >=150){
            stateC=LOW;
            PreviousA=currentTime;
            PORTB &=~LED_C;
        }
    }else{
        if(currentTime - PreviousC >=650){
            stateA=HIGH ;
            PreviousC=currentTime;
            PORTB |=LED_C ;
        }
    }

}

void setup(){
    setup_reg();
    PORTB|=LED_A|LED_B;

}
void loop(){
    Led_A();
    Led_B();
    Led_C();
}

void updateButton(){
    unsigned long currentTime=millis();  //time in ms
    bool rawButton=(PIND &BUTTON1) ?HIGH:LOW; //  read the instentanious state of the button

    if(rawButton!=lasRawButton){
        debounceStart=currentTime;
        lasRawButton=rawButton;
    }
    if((currentTime-debounceStart)>=40){  //ignoring debouncing for 40ms
        if(rawButton!=stableButton){
            stableButton=rawButton;

            if(stableButton==LOW){
                pressStart=currentTime;
                LedState=!LedState;

                if(LedState)
                    PORTB|=LED_D;
                else
                    PORTB&=~LED_D; 
            }
        }else{
            //valid release
            saftyMode=false;
        }
    }

    //check long press
    if(stableButton==LOW && !saftyMode){
        if(currentTime-pressStart>2000){
            saftyMode=true;
            previousSafetyFlash =currentTime;
        }
    }
    if(saftyMode&& stableButton==LOW){
        if(currentTime-previousSafetyFlash>=50){
            PORTB^=LED_D;
            previousSafetyFlash=currentTime;
        }
    }
}
void BonusLED(){


}

void updateSerial() {

    unsigned long currentTime = millis();

    if (currentTime - previousSerial >= 2500) {

        unsigned long holdDuration = 0;

        if (stableButton == LOW)
            holdDuration = currentTime - pressStart;

        Serial.print("A:");
        Serial.print(stateA);

        Serial.print(" B:");
        Serial.print((PORTB & LED_B) ? 1 : 0);

        Serial.print(" C:");
        Serial.print(stateC);

        Serial.print(" D:");
        Serial.print((PORTB & LED_D) ? 1 : 0);

        Serial.print(" Hold:");
        Serial.print(holdDuration);

        Serial.println("ms");

        previousSerial = currentTime;
    }
}
void setup() {

    setup_reg();

    Serial.begin(9600);

    // A and C start ON
    PORTB |= LED_A | LED_C;
}


void loop() {

    Led_A();
    Led_B();
    Led_C();
    updateButton();
    BonusLED();
    updateSerial();
}