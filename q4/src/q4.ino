#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <LiquidCrystal.h>

// LCD
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// Counter
volatile int pulse = 0;

// Interrupt flags
volatile bool int0Flag = false;
volatile bool int1Flag = false;

// Interrupt event times
volatile unsigned long int0Time = 0;
volatile unsigned long int1Time = 0;

// Last accepted interrupt times
unsigned long lastINT0Time = 0;
unsigned long lastINT1Time = 0;

const unsigned long DEBOUNCE_TIME = 100;

// INT0 ISR - Button 1
// Increment counter
ISR(INT0_vect)
{
    int0Time = millis();
    int0Flag = true;
}

// INT1 ISR - Button 2
// Reset counter

ISR(INT1_vect)
{
    int1Time = millis();
    int1Flag = true;
}

// SETUP
void setup()
{
    // D2 = PD2 = INT0 -> INPUT
    // D3 = PD3 = INT1 -> INPUT

    DDRD &= ~(1 << DDD2);
    DDRD &= ~(1 << DDD3);

    // Enable internal pull-up resistors

    PORTD |= (1 << PORTD2);
    PORTD |= (1 << PORTD3);

    // LCD
    lcd.begin(16, 2);

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Pulse Counter");

    lcd.setCursor(0, 1);
    lcd.print("Count: 0");

    // Serial
    Serial.begin(9600);

    // EICRA
    // ISC01 = 1, ISC00 = 0 -> INT0 FALLING
    // ISC11 = 1, ISC10 = 0 -> INT1 FALLING
    EICRA = (1 << ISC01) | (1 << ISC11);

    // EIMSK
    // Enable INT0 and INT1
    EIMSK = (1 << INT0) | (1 << INT1);

    // Enable global interrupts
    sei();
}

// LOOP
void loop()
{
    // INT0 EVENT
    if (int0Flag)
    {
        noInterrupts();

        int0Flag = false;
        unsigned long eventTime = int0Time;

        interrupts();

        // Debounce
        if (eventTime - lastINT0Time > DEBOUNCE_TIME)
        {
            pulse++;
            lastINT0Time = eventTime;

            // LCD
            lcd.setCursor(0, 1);
            lcd.print("Count:          ");

            lcd.setCursor(0, 1);
            lcd.print("Count: ");
            lcd.print(pulse);

            // Serial notification
            Serial.print("INT0: Pulse count = ");
            Serial.println(pulse);
        }
    }

    // INT1 EVENT
    if (int1Flag)
    {
        noInterrupts();

        int1Flag = false;
        unsigned long eventTime = int1Time;

        interrupts();

        // Debounce
        if (eventTime - lastINT1Time > DEBOUNCE_TIME)
        {
            pulse = 0;
            lastINT1Time = eventTime;

            // LCD
            lcd.setCursor(0, 1);
            lcd.print("Count:          ");

            lcd.setCursor(0, 1);
            lcd.print("Count: 0");

            // Serial notification
            Serial.println("INT1: Counter RESET");
        }
    }
}