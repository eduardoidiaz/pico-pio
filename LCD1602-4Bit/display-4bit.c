/**
 * Eduardo Diaz
 * 
 * PIO 4-bit Interface for LCD1602.
 *
 * HARDWARE CONNECTIONS
 * - GPIO 0-1 ---> FT232 RX/TX (Default UART)
 * - GPIO 2-12 ---> LCD Data Bits (0-7) + RS + RW + E
 * - RP2040 5V ---> LCD 5V
 * - RP2040 GND ---> LCD GND
 *
 *
 */

#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "display-4bit.pio.h"

#define LCD_BASE_PIN 2   // DB0 starts here
#define LCD_PIN_COUNT 10 // DB0–7 + RS + RW
#define LCD_E_PIN 12     // side-set pin

PIO pio = pio0;
uint sm = 0;

void lcd_pio_init() {
    uint offset = pio_add_program(pio, &display_program);

    pio_sm_config c = display_program_get_default_config(offset);

    // OUT pins: DB0–DB7 + RS + RW (GPIO 0–9)
    sm_config_set_out_pins(&c, LCD_BASE_PIN, LCD_PIN_COUNT);

    // Side-set pin: E (GPIO 10)
    sm_config_set_sideset_pins(&c, LCD_E_PIN);

    // Shift config: shift out LSB first
    sm_config_set_out_shift(&c, true, true, 32);

    // Set pin directions
    pio_sm_set_consecutive_pindirs(pio, sm, LCD_BASE_PIN, LCD_PIN_COUNT, true);
    pio_sm_set_consecutive_pindirs(pio, sm, LCD_E_PIN, 1, true);
    for (int i=LCD_BASE_PIN; i<LCD_PIN_COUNT+3; i++) {
        pio_gpio_init(pio, i);
    }
    // pio_gpio_init(pio, LCD_E_PIN);

    // Clock divider (example: 125 MHz / 10 = 12.5 MHz)
    sm_config_set_clkdiv(&c, 10.0f);

    // Init + enable
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

void lcd_write_cmd(uint8_t cmd) {
    uint32_t value = cmd | (0 << 8) | (0 << 9);
    pio_sm_put_blocking(pio, sm, value);
}

void lcd_write_data(uint8_t data) {
    uint32_t value = data | (1 << 8) | (0 << 9);
    pio_sm_put_blocking(pio, sm, value);
}

void lcd_print(const char *s) {
    while (*s) {
        lcd_write_data(*s++);
        sleep_us(40);
    }
}


int main() {
    stdio_init_all();
    lcd_pio_init();

    lcd_write_cmd(0x38); // Function set
    sleep_ms(100);
    lcd_write_cmd(0x0C); // Display ON
    sleep_ms(100);
    lcd_write_cmd(0x01); // Clear
    sleep_ms(100);

    printf("Writing to LCD!\n");

    lcd_write_data('H');
    sleep_ms(1);
    lcd_write_data('i');
    
    sleep_ms(1000);
    
    lcd_write_cmd(0x01);
    sleep_ms(2);

    lcd_write_cmd(0x80);
    sleep_ms(1);
    lcd_print("Hello");
    sleep_ms(1);
    lcd_write_cmd(0xC0);
    sleep_ms(1);
    lcd_print("PIO LCD-8bit");
    sleep_ms(1);
    lcd_print("End! :)");
    sleep_ms(1);

    printf("Writing done!\n\n");
}
