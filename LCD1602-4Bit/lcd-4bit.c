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
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "lcd-4bit.pio.h"

// Hardware Pin Wiring: Pins MUST be consecutive
// GPIO 6 -> DB4
// GPIO 7 -> DB5
// GPIO 8 -> DB6
// GPIO 9 -> DB7
// GPIO 10 -> RS
// GPIO 11 -> RW
#define LCD_BASE_PIN 6 
#define LCD_E_PIN    12

// Pack data/commands and send to the PIO state machine
void lcd_send_byte(PIO pio, uint sm, uint8_t data, bool is_cmd) {
    uint32_t rs = is_cmd ? 0 : 1;
    uint32_t rw = 0; 

    // Isolate upper and lower 4 bits
    uint32_t high_nibble = (data >> 4) & 0x0F;
    uint32_t low_nibble  = data & 0x0F;

    // Pack: Bit 5=RW, Bit 4=RS, Bits 3-0=Data
    uint32_t high_packet = (rw << 5) | (rs << 4) | high_nibble;
    uint32_t low_packet  = (rw << 5) | (rs << 4) | low_nibble;

    // Merge into 32-bit FIFO word (low packet shifted up 6 bits)
    uint32_t fifo_word = (low_packet << 6) | high_packet;

    pio_sm_put_blocking(pio, sm, fifo_word);
    
    // Give commands extra time to complete inside the LCD controller
    if (is_cmd && data <= 3) {
        sleep_ms(2); 
    } else {
        sleep_us(50);
    }
}

// Helper function to print raw strings
void lcd_print_string(PIO pio, uint sm, const char *str) {
    while (*str) {
        lcd_send_byte(pio, sm, *str++, false);
    }
}

// Move cursor to specific coordinates (0-indexed)
void lcd_set_cursor(PIO pio, uint sm, uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? (0x00 + col) : (0x40 + col);
    lcd_send_byte(pio, sm, 0x80 | address, true);
}

// Driver configuration and screen initialization
void lcd_pio_init(PIO pio, uint sm, uint offset) {
    pio_sm_config c = lcd_4bit_program_get_default_config(offset);

    // Map 6 outputs (DB4-DB7, RS, RW) and 1 Side-Set pin (E)
    sm_config_set_out_pins(&c, LCD_BASE_PIN, 6);
    sm_config_set_sideset_pins(&c, LCD_E_PIN);

    // Setup GPIO pins
    for(int i = 0; i < 6; i++) {
        pio_gpio_init(pio, LCD_BASE_PIN + i);
    }
    pio_gpio_init(pio, LCD_E_PIN);
    
    // Set pins as outputs
    pio_sm_set_consecutive_pindirs(pio, sm, LCD_BASE_PIN, 6, true);
    pio_sm_set_consecutive_pindirs(pio, sm, LCD_E_PIN, 1, true);

    // Set configuration: Shift Right, Autopull enabled, Threshold = 12 bits
    sm_config_set_out_shift(&c, true, true, 12);

    // Scale PIO clock down to roughly 1 MHz for safe timing operations
    float div = (float)clock_get_hz(clk_sys) / 1000000;
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
    
    // --- Hardware Power-On Reset Sequence ---
    sleep_ms(50); 
    
    // Force 8-bit interface mode three times to reset the controller state
    pio_sm_put_blocking(pio, sm, 0x03); sleep_ms(5);
    pio_sm_put_blocking(pio, sm, 0x03); sleep_us(150);
    pio_sm_put_blocking(pio, sm, 0x03);
    
    // Switch the LCD controller to 4-bit bus mode
    pio_sm_put_blocking(pio, sm, 0x02); 
    sleep_ms(2);

    // Configure display settings
    lcd_send_byte(pio, sm, 0x28, true); // 2 lines, 5x8 pixel font
    lcd_send_byte(pio, sm, 0x0C, true); // Turn display on, remove blinking cursor
    lcd_send_byte(pio, sm, 0x06, true); // Auto-increment cursor position
    lcd_send_byte(pio, sm, 0x01, true); // Clear screen contents
}

int main() {
    stdio_init_all();

    // Use PIO instance 0 and allocate an open state machine
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &lcd_4bit_program);
    uint sm = pio_claim_unused_sm(pio, true);

    // Initialize hardware configuration
    lcd_pio_init(pio, sm, offset);

    // Print static startup message
    lcd_set_cursor(pio, sm, 0, 0);
    lcd_print_string(pio, sm, "RP2040 Pico PIO");
    lcd_set_cursor(pio, sm, 1, 0);
    lcd_print_string(pio, sm, "4-Bit LCD1602");

    sleep_ms(3000);

    // Loop executing active display shifts
    while (true) {
        lcd_send_byte(pio, sm, 0x01, true); // Clear Screen
        lcd_set_cursor(pio, sm, 0, 0);
        lcd_print_string(pio, sm, "System Status:");
        
        lcd_set_cursor(pio, sm, 1, 3);
        lcd_print_string(pio, sm, "Running OK");
        sleep_ms(2000);

        lcd_send_byte(pio, sm, 0x01, true);
        lcd_set_cursor(pio, sm, 0, 2);
        lcd_print_string(pio, sm, "Hello World!");
        sleep_ms(2000);
    }
}
