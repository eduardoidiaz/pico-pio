/**
 * Eduardo Diaz
 * 
 * PIO 4-bit Interface for LCD1602.
 *
 * HARDWARE CONNECTIONS
 * - GPIO 0-1 ---> FT232 RX/TX (Default UART)
 * - GPIO 6-12 ---> LCD Data Bits (4-7) + RS + RW + E
 * - RP2040 5V ---> LCD 5V
 * - RP2040 GND ---> LCD GND
 *
 *
 */

#ifndef LCD_4BIT_H_
#define LCD_4BIT_H_

#include "pico/stdlib.h"
#include "hardware/pio.h"

// Hardware Pin Wiring: Pins MUST be consecutive
// GPIO 6 -> DB4, GPIO 7 -> DB5, GPIO 8 -> DB6, GPIO 9 -> DB7, GPIO 10 -> RS, GPIO 11 -> RW
#define LCD_BASE_PIN 6 
#define LCD_E_PIN    12

#define LCD_CMD_CURSOR_ON       0x0F  // Display ON, Cursor ON, Blink ON
#define LCD_CMD_CURSOR_OFF      0x0C  // Display ON, Cursor OFF, Blink OFF


// Common HD44780 LCD Commands
#define LCD_CMD_CLEAR_DISPLAY   0x01
#define LCD_CMD_RETURN_HOME     0x02
#define LCD_CMD_ENTRY_MODE_SET  0x06
#define LCD_CMD_DISPLAY_CONTROL LCD_CMD_CURSOR_ON
#define LCD_CMD_FUNCTION_SET    0x28


/**
 * @brief Initializes the PIO state machine and configures the LCD1602 hardware into 4-bit mode.
 * @param pio The PIO hardware instance (pio0 or pio1).
 * @param sm The allocated state machine index (0-3).
 * @param offset The memory offset where the PIO program was loaded.
 */
void lcd_pio_init(PIO pio, uint sm, uint offset);

/**
 * @brief Sends a raw byte of data or command packet to the LCD screen.
 * @param pio The PIO hardware instance.
 * @param sm The allocated state machine index.
 * @param data The 8-bit byte payload to send.
 * @param is_cmd Set to true if sending an instruction command, false if sending standard text character.
 */
void lcd_send_byte(PIO pio, uint sm, uint8_t data, bool is_cmd);

/**
 * @brief Prints a null-terminated string to the current cursor position.
 * @param pio The PIO hardware instance.
 * @param sm The allocated state machine index.
 * @param str Pointer to the C-string array.
 */
void lcd_print_string(PIO pio, uint sm, const char *str);

/**
 * @brief Repositions the display cursor using absolute coordinates.
 * @param pio The PIO hardware instance.
 * @param sm The allocated state machine index.
 * @param row The row target line index (0 or 1).
 * @param col The column character position offset (0 to 15).
 */
void lcd_set_cursor(PIO pio, uint sm, uint8_t row, uint8_t col);

#endif // LCD_4BIT_H_
