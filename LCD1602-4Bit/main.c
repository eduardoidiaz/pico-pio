/**
 * Eduardo Diaz
 * 
 * UART over USB demonstration of PIO 4-bit Interface for LCD1602.
 * Using 'screen' on MacOS.
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
    // Initialize standard I/O (USB Serial)
    stdio_init_all();

    // Setup PIO state machine
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &lcd_4bit_program);
    uint sm = pio_claim_unused_sm(pio, true);

    // Initialize 4-bit LCD hardware configuration
    lcd_pio_init(pio, sm, offset);

    // Terminal startup guide
    printf("\n=== RP2040 PIO LCD Serial Terminal Ready ===\n");
    printf("Type characters here to send them to the LCD screen.\n");
    printf("Press [Enter] or [Ctrl+J] to start a new line (Shift to next line on the LCD).\n");
    printf("Press [BackSpace] or [Ctrl+H] to clear the screen (Clears previous character on LCD).\n");
    printf("Blinking hardware cursor enabled.\n");
    printf("Press [Tab] to toggle the cursor visibility.\n");
    printf("-----------------------------------------\n");

    uint8_t current_col = 0;
    uint8_t current_row = 0;
    bool cursor_visible = true;

    // Synchronize starting position
    lcd_set_cursor(pio, sm, current_row, current_col);

    while (true) {
        // Read character from USB serial queue without freezing execution loops
        int c = getchar_timeout_us(0);

        if (c == PICO_ERROR_TIMEOUT || c == EOF) {
            tight_loop_contents();
            continue;
        }

        // --- 1. HANDLE TAB KEY (Toggle Cursor Visibility) ---
        if (c == '\t' || c == 9) {
            cursor_visible = !cursor_visible;
            if (cursor_visible) {
                lcd_send_byte(pio, sm, LCD_CMD_CURSOR_ON, true);
            } else {
                lcd_send_byte(pio, sm, LCD_CMD_CURSOR_OFF, true);
            }
            lcd_set_cursor(pio, sm, current_row, current_col);
            continue;
        }

        // --- 2. HANDLE BACKSPACE KEY (Multi-Line Destructive Deletion) ---
        if (c == '\b' || c == 127 || c == 8) {
            bool processed_backspace = false;

            if (current_col > 0) {
                // Normal deletion on the same line
                current_col--;
                processed_backspace = true;
            } 
            else if (current_row > 0) {
                // Line wrapping deletion: Jump from start of Line 2 to end of Line 1
                current_row--;
                current_col = MAX_COLS - 1;
                processed_backspace = true;
            }

            if (processed_backspace) {
                // A. Update physical LCD screen content
                lcd_set_cursor(pio, sm, current_row, current_col); // Target old letter
                lcd_send_byte(pio, sm, ' ', false);               // Erase letter with a blank space
                lcd_set_cursor(pio, sm, current_row, current_col); // Move cursor back onto space

                // B. Update computer terminal screen visualization
                // \b shifts cursor left, ' ' wipes letter, \b shifts cursor left again
                printf("\b \b");
                fflush(stdout); // Force macOS terminal layout engine to draw immediately
            }
            continue;
        }

        // --- 3. HANDLE ENTER / RETURN KEY (New Line Sequence) ---
        if (c == '\n' || c == '\r') {
            current_row = (current_row + 1) % MAX_ROWS;
            current_col = 0;
            
            // Advance computer terminal cursor down to a new row line
            printf("\r\n");
            fflush(stdout);

            lcd_set_cursor(pio, sm, current_row, current_col);
            continue;
        }

        // --- 4. HANDLE VALID PRINTABLE TEXT CHARACTERS ---
        if (c >= 32 && c <= 126) {
            // Forward character directly back to macOS screen utility
            putchar(c);
            fflush(stdout);

            // Forward character directly down to the LCD1602 hardware
            lcd_send_byte(pio, sm, (uint8_t)c, false);
            current_col++;

            // Handle horizontal text line wrap when hitting column boundary limits
            if (current_col >= MAX_COLS) {
                current_col = 0;
                current_row = (current_row + 1) % MAX_ROWS;
                
                // Advance computer terminal layout down cleanly
                printf("\r\n");
                fflush(stdout);

                lcd_set_cursor(pio, sm, current_row, current_col);
            }
        }
    }
}
