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


#include <stdio.h>
#include "pico/stdlib.h"
#include "lcd-4bit.h"
#include "lcd-4bit.pio.h"

#define MAX_COLS 16
#define MAX_ROWS 2

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &lcd_4bit_program);
    uint sm = pio_claim_unused_sm(pio, true);

    lcd_pio_init(pio, sm, offset);

    // Terminal startup guide
    printf("\n=== RP2040 PIO LCD Serial Terminal Ready ===\n");
    printf("Type characters here to send them to the LCD screen.\n");
    printf("Press [Enter] or [Ctrl+J] to start a new line.\n");
    printf("Press [BackSpace] or [Ctrl+H] to clear the screen.\n");
    printf("Blinking hardware cursor enabled.\n");
    printf("Press [Tab] to toggle the cursor visibility.\n");
    printf("-----------------------------------------\n");

    uint8_t current_col = 0;
    uint8_t current_row = 0;
    bool cursor_visible = true; // State tracking variable

    lcd_set_cursor(pio, sm, current_row, current_col);

    while (true) {
        int c = getchar();

        if (c == EOF) {
            continue;
        }

        // --- Handle Tab Key (Toggle Cursor) ---
        if (c == '\t') {
            cursor_visible = !cursor_visible; // Flip the state
            
            if (cursor_visible) {
                lcd_send_byte(pio, sm, LCD_CMD_CURSOR_ON, true);
                printf("\n[Cursor: ENABLED]\n");
            } else {
                lcd_send_byte(pio, sm, LCD_CMD_CURSOR_OFF, true);
                printf("\n[Cursor: DISABLED]\n");
            }
            continue;
        }

        // Echo typed character back to PC terminal
        putchar(c);

        // Handle Backspace (Clear screen)
        if (c == '\b' || c == 127 || c == 8) {
            lcd_send_byte(pio, sm, LCD_CMD_CLEAR_DISPLAY, true);
            current_col = 0;
            current_row = 0;
            lcd_set_cursor(pio, sm, current_row, current_col);
            continue;
        }

        // Handle Newline / Enter
        if (c == '\n' || c == '\r') {
            current_row = (current_row + 1) % MAX_ROWS;
            current_col = 0;
            lcd_set_cursor(pio, sm, current_row, current_col);
            continue;
        }

        // Print valid printable ASCII characters
        if (c >= 32 && c <= 126) {
            if (current_col >= MAX_COLS) {
                current_col = 0;
                current_row = (current_row + 1) % MAX_ROWS;
                
                if (current_row == 0) {
                    lcd_send_byte(pio, sm, LCD_CMD_CLEAR_DISPLAY, true);
                }
                lcd_set_cursor(pio, sm, current_row, current_col);
            }

            lcd_send_byte(pio, sm, (uint8_t)c, false);
            current_col++;
            
            if (current_col >= MAX_COLS) {
                uint8_t next_row = (current_row + 1) % MAX_ROWS;
                lcd_set_cursor(pio, sm, next_row, 0);
            }
        }
    }
}
