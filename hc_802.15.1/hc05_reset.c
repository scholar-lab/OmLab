#ifndef F_CPU
#define F_CPU 12000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#define AT_BAUD 38400
#define MYUBRR ((F_CPU / (AT_BAUD * 16UL)) - 1)

/* Pin Configuration (Only LCD and Output LEDs) */
#define LCD_RS_PORT    PORTD   
#define LCD_RS         PD3     
#define LCD_E_PORT     PORTD   
#define LCD_E          PD4     
#define LCD_D4         PD5     
#define LCD_D5         PD6     
#define LCD_D6         PD7     
#define LCD_D7         PB0     

#define LCD_CLEAR         0x01
#define LCD_SET_DDRAM     0x80

/* Basic LCD Drivers */
static void lcd_pulse_enable(void) {
    LCD_E_PORT |= (1 << LCD_E);  _delay_us(5);
    LCD_E_PORT &= ~(1 << LCD_E); _delay_us(100); 
}
static void lcd_write_nibble(uint8_t nibble) {
    PORTD &= ~((1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6)); PORTB &= ~(1 << LCD_D7);
    if (nibble & (1 << 4)) PORTD |= (1 << LCD_D4);
    if (nibble & (1 << 5)) PORTD |= (1 << LCD_D5);
    if (nibble & (1 << 6)) PORTD |= (1 << LCD_D6);
    if (nibble & (1 << 7)) PORTB |= (1 << LCD_D7);
    lcd_pulse_enable();
}
static void lcd_write_byte(uint8_t data, uint8_t is_command) {
    if (is_command) LCD_RS_PORT &= ~(1 << LCD_RS); else LCD_RS_PORT |= (1 << LCD_RS);  
    lcd_write_nibble(data & 0xF0); lcd_write_nibble((data << 4) & 0xF0);
}
void lcd_command(uint8_t cmd) { lcd_write_byte(cmd, 1); _delay_ms(5); }
void lcd_init(void) {
    DDRD |= (1 << LCD_RS) | (1 << LCD_E) | (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6); DDRB |= (1 << LCD_D7);
    _delay_ms(100);
    lcd_write_nibble(0x30); _delay_ms(10); lcd_write_nibble(0x30); _delay_ms(2);
    lcd_write_nibble(0x30); _delay_ms(2); lcd_write_nibble(0x20); _delay_ms(2);
    lcd_command(0x28); lcd_command(0x0C); lcd_command(0x01); lcd_command(0x06);
}
void lcd_print(const char *str) { while (*str) { lcd_write_byte((uint8_t)(*str++), 0); } }

/* UART Drivers */
void uart_init(unsigned int ubrr) {
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;
    UCSRB = (1 << TXEN); 
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0); 
}
void uart_send_at_command(const char *str) {
    while (*str) {
        while (!(UCSRA & (1 << UDRE)));
        UDR = *str++;
    }
    while (!(UCSRA & (1 << UDRE))); 
    UDR = '\r';
    while (!(UCSRA & (1 << UDRE))); 
    UDR = '\n';
}

int main(void)
{
    // Setup status indicators as clean outputs
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);

    lcd_init();
    uart_init(MYUBRR);

    lcd_command(LCD_SET_DDRAM | 0x00); 
    lcd_print("Forced Reset... ");
    
    // 5-second dynamic visual countdown sequence
    for(uint8_t i = 5; i > 0; i--) {
        lcd_command(LCD_SET_DDRAM | 0x40);
        lcd_print("Starting in ");
        lcd_write_byte(i + '0', 0); // Convert number to ASCII character
        lcd_print("s   ");
        
        PORTB |= (1 << PB1);  _delay_ms(500);
        PORTB &= ~(1 << PB1); _delay_ms(500);
    }

    lcd_command(LCD_CLEAR);
    lcd_command(LCD_SET_DDRAM | 0x00); 
    lcd_print("Sending AT+ORGL ");
    lcd_command(LCD_SET_DDRAM | 0x40);
    lcd_print("Flashing Memory ");
    
    // Fire the command 3 times sequentially to guarantee delivery buffer alignment
    for(uint8_t loops = 0; loops < 3; loops++) {
        PORTB |= (1 << PB1); 
        uart_send_at_command("AT+ORGL");
        _delay_ms(1000);
        PORTB &= ~(1 << PB1);
        _delay_ms(500);
    }

    // Force hardware reboot out of configuration space
    uart_send_at_command("AT+RESET");

    lcd_command(LCD_CLEAR);
    lcd_command(LCD_SET_DDRAM | 0x00); 
    lcd_print("Reset Execution ");
    lcd_command(LCD_SET_DDRAM | 0x40);
    lcd_print("Check Phone Now ");

    while (1) {
        // Complete safe execution halt
    }
    return 0;
}