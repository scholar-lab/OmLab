#ifndef F_CPU
#define F_CPU 12000000UL   /* 12 MHz External Crystal */
#endif

#include <avr/io.h>
#include <util/delay.h>

#define BAUD 9600
#define MYUBRR ((F_CPU / (BAUD * 16UL)) - 1)

/* ------------------------------------------------------------------ */
/* ATMEGA8L PIN CONFIGURATION (PERFECTLY ALIGNED)                     */
/* ------------------------------------------------------------------ */

// Control Pins (Physical Pins 5 and 6)
#define LCD_RS_DDR     DDRD    
#define LCD_RS_PORT    PORTD   
#define LCD_RS         PD3     /* Physical Pin 5 */

#define LCD_E_DDR      DDRD    
#define LCD_E_PORT     PORTD   
#define LCD_E          PD4     /* Physical Pin 6 */

// Data Pins (Physical Pins 11, 12, 13, 14)
#define LCD_D4         PD5     /* Physical Pin 11 */
#define LCD_D5         PD6     /* Physical Pin 12 */
#define LCD_D6         PD7     /* Physical Pin 13 */
#define LCD_D7         PB0     /* Physical Pin 14 */

/* HD44780 Instruction Commands */
#define LCD_CLEAR         0x01
#define LCD_4BIT_2LINE    0x28
#define LCD_DISPLAY_ON    0x0C
#define LCD_ENTRY_MODE    0x06
#define LCD_SET_DDRAM     0x80

/* LCD Driver Functions */
static void lcd_pulse_enable(void)
{
    LCD_E_PORT |= (1 << LCD_E);
    _delay_us(5);
    LCD_E_PORT &= ~(1 << LCD_E);
    _delay_us(100); 
}

static void lcd_write_nibble(uint8_t nibble)
{
    PORTD &= ~((1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6));
    PORTB &= ~(1 << LCD_D7);

    if (nibble & (1 << 4)) PORTD |= (1 << LCD_D4);
    if (nibble & (1 << 5)) PORTD |= (1 << LCD_D5);
    if (nibble & (1 << 6)) PORTD |= (1 << LCD_D6);
    if (nibble & (1 << 7)) PORTB |= (1 << LCD_D7);

    lcd_pulse_enable();
}

static void lcd_write_byte(uint8_t data, uint8_t is_command)
{
    if (is_command) {
        LCD_RS_PORT &= ~(1 << LCD_RS); 
    } else {
        LCD_RS_PORT |= (1 << LCD_RS);  
    }
    lcd_write_nibble(data & 0xF0);
    lcd_write_nibble((data << 4) & 0xF0);
}

void lcd_command(uint8_t cmd)
{
    lcd_write_byte(cmd, 1);
    if (cmd == LCD_CLEAR) {
        _delay_ms(5);
    }
}

void lcd_init(void)
{
    LCD_RS_DDR |= (1 << LCD_RS);
    LCD_E_DDR  |= (1 << LCD_E);
    DDRD       |= (1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6);
    DDRB       |= (1 << LCD_D7);

    _delay_ms(100);

    lcd_write_nibble(0x30); _delay_ms(10);
    lcd_write_nibble(0x30); _delay_ms(2);
    lcd_write_nibble(0x30); _delay_ms(2);
    lcd_write_nibble(0x20); _delay_ms(2);

    lcd_command(LCD_4BIT_2LINE);
    lcd_command(LCD_DISPLAY_ON);
    lcd_command(LCD_CLEAR);
    lcd_command(LCD_ENTRY_MODE);
}

void lcd_print(const char *str)
{
    while (*str) {
        lcd_write_byte((uint8_t)(*str++), 0);
    }
}

/* ------------------------------------------------------------------ */
/* UART / BLUETOOTH DRIVER (ATmega8 / ATmega8L specific)              */
/* ------------------------------------------------------------------ */
void uart_init(unsigned int ubrr)
{
    // Set baud rate registers
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;
    
    // Enable UART receiver and transmitter
    UCSRB = (1 << RXEN) | (1 << TXEN);
    
    // Set frame format: 8 data bits, 1 stop bit (URSEL required on ATmega8)
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

uint8_t uart_receive(void)
{
    // Wait for data to arrive in the RX buffer
    while (!(UCSRA & (1 << RXC)));
    return UDR;
}

int main(void)
{
    uint8_t received_char;

    // Status LEDs setup (Physical Pins 15-18)
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);

    // Initialize systems
    lcd_init();
    uart_init(MYUBRR);

    // Initial message
    lcd_command(LCD_SET_DDRAM | 0x00); 
    lcd_print("BT Link Ready");
    
    // Set cursor to line 2 for incoming text streaming
    lcd_command(LCD_SET_DDRAM | 0x40);

    while (1)
    {
        // Code hangs here safely until a Bluetooth byte streams in
        received_char = uart_receive();
        
        // Wipe line 2 if a newline or return flag is parsed
        if (received_char == '\n' || received_char == '\r') {
            lcd_command(LCD_SET_DDRAM | 0x40);
            lcd_print("                "); 
            lcd_command(LCD_SET_DDRAM | 0x40);
        } else {
            // Echo character straight to the panel
            lcd_write_byte(received_char, 0);
        }
        
        // Heartbeat pulse indicator on PB1 (Physical pin 15)
        PORTB |= (1 << PB1);
        _delay_ms(15);
        PORTB &= ~(1 << PB1);
    }
    return 0;
}