#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/*
 * LCD-Anschlüsse:
 *
 * RS     -> PB4 -> Arduino D12
 * Enable -> PB3 -> Arduino D11
 *
 * LCD D4 -> PD5 -> Arduino D5
 * LCD D5 -> PD4 -> Arduino D4
 * LCD D6 -> PD3 -> Arduino D3
 * LCD D7 -> PD2 -> Arduino D2
 */

#define LCD_RS PB4
#define LCD_E  PB3

static void lcd_enable_pulse(void)
{
    PORTB |= (1 << LCD_E);
    _delay_us(1);

    PORTB &= ~(1 << LCD_E);
    _delay_us(100);
}

static void lcd_write_nibble(uint8_t nibble)
{
    /* Die vier Datenleitungen zunächst löschen. */
    PORTD &= ~(
        (1 << PD5) |
        (1 << PD4) |
        (1 << PD3) |
        (1 << PD2)
    );

    /*
     * Die vier Bits auf die LCD-Leitungen verteilen.
     *
     * Bit 0 -> LCD D4 -> PD5
     * Bit 1 -> LCD D5 -> PD4
     * Bit 2 -> LCD D6 -> PD3
     * Bit 3 -> LCD D7 -> PD2
     */

    if (nibble & (1 << 0)) PORTD |= (1 << PD5);
    if (nibble & (1 << 1)) PORTD |= (1 << PD4);
    if (nibble & (1 << 2)) PORTD |= (1 << PD3);
    if (nibble & (1 << 3)) PORTD |= (1 << PD2);

    lcd_enable_pulse();
}

static void lcd_send(uint8_t value, uint8_t is_data)
{
    if (is_data)
        PORTB |= (1 << LCD_RS);
    else
        PORTB &= ~(1 << LCD_RS);

    /* Zuerst die oberen vier Bits senden. */
    lcd_write_nibble(value >> 4);

    /* Danach die unteren vier Bits senden. */
    lcd_write_nibble(value & 0x0F);

    _delay_ms(2);
}

static void lcd_command(uint8_t command)
{
    lcd_send(command, 0);
}

static void lcd_character(char character)
{
    lcd_send((uint8_t)character, 1);
}

static void lcd_string(const char *text)
{
    while (*text != '\0')
    {
        lcd_character(*text);
        text++;
    }
}

static void lcd_set_cursor(uint8_t column, uint8_t row)
{
    uint8_t address;

    if (row == 0)
        address = column;
    else
        address = 0x40 + column;

    lcd_command(0x80 | address);
}

static void lcd_init(void)
{
    /* PB3 und PB4 als Ausgänge konfigurieren. */
    DDRB |= (1 << LCD_E) | (1 << LCD_RS);

    /* PD2 bis PD5 als Ausgänge konfigurieren. */
    DDRD |=
        (1 << PD2) |
        (1 << PD3) |
        (1 << PD4) |
        (1 << PD5);

    PORTB &= ~((1 << LCD_E) | (1 << LCD_RS));

    _delay_ms(50);

    /*
     * Das Display zunächst sicher in den 8-Bit-Modus bringen.
     * Dabei werden trotzdem nur vier Leitungen benutzt.
     */
    lcd_write_nibble(0x03);
    _delay_ms(5);

    lcd_write_nibble(0x03);
    _delay_us(150);

    lcd_write_nibble(0x03);

    /* Danach auf 4-Bit-Modus umschalten. */
    lcd_write_nibble(0x02);

    lcd_command(0x28); /* 4 Bit, zwei Zeilen, 5x8 Zeichen */
    lcd_command(0x0C); /* Display an, Cursor aus */
    lcd_command(0x06); /* Cursor nach rechts bewegen */
    lcd_command(0x01); /* Display löschen */

    _delay_ms(2);
}

int main(void)
{
    lcd_init();

    lcd_set_cursor(0, 0);
    lcd_string("Hello World!");

    lcd_set_cursor(0, 1);
    lcd_string("Midnight-Worker");

    while (1)
    {
        /* Das Programm bleibt hier für immer laufen. */
    }

    return 0;
}
