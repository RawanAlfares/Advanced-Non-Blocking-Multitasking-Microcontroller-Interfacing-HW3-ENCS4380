#include <Arduino.h>
#include <avr/io.h>
#include <stdint.h>

#define LCD_DATA_MASK 0xF0

#define RS_BIT (1 << PB0)
#define EN_BIT (1 << PB1)

void setup_registers() {
    DDRB |= RS_BIT | EN_BIT;
    DDRD |= LCD_DATA_MASK;
}

void lcd_pulse_en() {
    PORTB |= EN_BIT;
    asm volatile("nop\n nop\n nop\n nop\n");
    PORTB &= ~EN_BIT;
}

void lcd_send_nibble(uint8_t nibble) {
    PORTD = (PORTD & ~LCD_DATA_MASK) | (nibble & LCD_DATA_MASK);
    lcd_pulse_en();
}

void wait_us(unsigned long us) {
    unsigned long start = micros();

    while (micros() - start < us) {
    }
}

void lcd_command(uint8_t cmd){
    PORTB &=~RS_BIT;                        // RS = 0 
    lcd_send_nibble(cmd & 0xF0);            // High nibble        
    lcd_send_nibble((cmd << 4) & 0xF0);   // Low nibble

    if (cmd==0x01)                        // dispaly command 
        wait_us(1640);
    else
        wait_us(40);
}

void lcd_data(uint8_t b) {
    PORTB |= RS_BIT;                     // RS = 1 
    lcd_send_nibble(b & 0xF0);          // High nibble
    lcd_send_nibble((b << 4) & 0xF0);   // Low nibble
    wait_us(40);
}

void lcd_setCursor(uint8_t col, uint8_t row) {
    uint8_t address;
    if(row ==0)
        address=0x00+col;
    else
        address=0x40 +col;
    lcd_command (0x80 | address);
    
}

void lcd_print(const char *c){
    while(*c){
        lcd_data(*c);
        c++;
    }
}
void lcd_init() {
    setup_registers();

    wait_us(15000);

    PORTB &= ~RS_BIT;

    lcd_send_nibble(0x30);
    wait_us(4500);

    lcd_send_nibble(0x30);
    wait_us(4500);

    lcd_send_nibble(0x30);
    wait_us(150);

    lcd_send_nibble(0x20);
    wait_us(150);

    lcd_command(0x28);
    lcd_command(0x08);  //display off
    lcd_command(0x01);  // clear display
    lcd_command(0x06);  //cursor moves to rights
    lcd_command(0x0C);  //display on , blink off , cusor off
}


void setup() {
    lcd_init();

    lcd_setCursor(0, 0);
    lcd_print("Rawan Abo ALRous");
}

void loop() {
}