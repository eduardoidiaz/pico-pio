/**
 * Eduardo Diaz
 * 
 * Demonstration of PIO 4-bit Interface for LCD1602.
 *
 * HARDWARE CONNECTIONS
 * - GPIO 0-1 ---> FT232 RX/TX (Default UART)
 * - GPIO 2-12 ---> LCD Data Bits (0-7) + RS + RW + E
 * - RP2040 5V ---> LCD 5V
 * - RP2040 GND ---> LCD GND
 *
 *
 */


#include "pico/stdlib.h"
#include "lcd-4bit.h"
#include "lcd-4bit.pio.h"

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &lcd_4bit_program);
    uint sm = pio_claim_unused_sm(pio, true);

    lcd_pio_init(pio, sm, offset);

    lcd_set_cursor(pio, sm, 0, 0);
    lcd_print_string(pio, sm, "Driver Ready!");

    while (true) {
        tight_loop_contents();
    }
}
