#include <avr/io.h>
#include <avr/eeprom.h>
#include <string.h>

#define F_CPU 16000000UL

#define BAUD 2400
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

#define DEVICE_ID   "ALGO-UNO-V1"
#define END_CHAR    '\n'
#define EEPROM_SIZE 1024   // EEPROM size for ATmega328P

/* ---------- UART low-level routines ---------- */

static void uart_init(void)
{
    // Set baud rate
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)(UBRR_VALUE);

    // Enable transmitter and receiver
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // 8 data bits, no parity, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static uint8_t uart_rx(void)
{
    // Wait until a byte is received
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

static void uart_tx(uint8_t b)
{
    // Wait until transmit buffer is empty
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = b;
}

static void uart_tx_str(const char *s)
{
    // Send a null-terminated string
    while (*s)
        uart_tx(*s++);
}

/* ---------- Main application ---------- */

int main(void)
{
    uart_init();

    while (1)
    {
        uint8_t *eeptr = (uint8_t *)0;  // EEPROM write pointer
        char c;
        char cmd[4];

        /* Read first three characters to detect command */
        for (uint8_t i = 0; i < 3; i++)
            cmd[i] = uart_rx();
        cmd[3] = 0;

        // Fourth character should be newline
        c = uart_rx();

        /* Handle device identification request */
        if (strcmp(cmd, "ID?") == 0 && c == '\n')
        {
            uart_tx_str("ID:");
            uart_tx_str(DEVICE_ID);
            uart_tx('\n');
            continue;
        }

        /* Treat input as normal message and store it */
        eeprom_write_byte(eeptr++, cmd[0]);
        eeprom_write_byte(eeptr++, cmd[1]);
        eeprom_write_byte(eeptr++, cmd[2]);
        eeprom_write_byte(eeptr++, c);

        // Store remaining bytes until newline or EEPROM limit
        while (c != END_CHAR && (uintptr_t)eeptr < EEPROM_SIZE - 1)
        {
            c = uart_rx();
            eeprom_write_byte(eeptr++, c);
        }

        /* Read back stored data and echo it */
        uint8_t *rd = (uint8_t *)0;

        while (rd < eeptr)
            uart_tx(eeprom_read_byte(rd++));
    }
}
