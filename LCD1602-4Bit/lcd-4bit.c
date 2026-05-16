#include "lcd-4bit.h"
#include "hardware/clocks.h"
#include "lcd-4bit.pio.h"

void lcd_send_byte(PIO pio, uint sm, uint8_t data, bool is_cmd) {
    uint32_t rs = is_cmd ? 0 : 1;
    uint32_t rw = 0; 

    uint32_t high_nibble = (data >> 4) & 0x0F;
    uint32_t low_nibble  = data & 0x0F;

    uint32_t high_packet = (rw << 5) | (rs << 4) | high_nibble;
    uint32_t low_packet  = (rw << 5) | (rs << 4) | low_nibble;

    uint32_t fifo_word = (low_packet << 6) | high_packet;

    pio_sm_put_blocking(pio, sm, fifo_word);
    
    if (is_cmd && data <= 3) {
        sleep_ms(2); 
    } else {
        sleep_us(50);
    }
}

void lcd_print_string(PIO pio, uint sm, const char *str) {
    while (*str) {
        lcd_send_byte(pio, sm, *str++, false);
    }
}

void lcd_set_cursor(PIO pio, uint sm, uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? (0x00 + col) : (0x40 + col);
    lcd_send_byte(pio, sm, 0x80 | address, true);
}

void lcd_pio_init(PIO pio, uint sm, uint offset) {
    pio_sm_config c = lcd_4bit_program_get_default_config(offset);

    sm_config_set_out_pins(&c, LCD_BASE_PIN, 6);
    sm_config_set_sideset_pins(&c, LCD_E_PIN);

    for(int i = 0; i < 6; i++) {
        pio_gpio_init(pio, LCD_BASE_PIN + i);
    }
    pio_gpio_init(pio, LCD_E_PIN);
    
    pio_sm_set_consecutive_pindirs(pio, sm, LCD_BASE_PIN, 6, true);
    pio_sm_set_consecutive_pindirs(pio, sm, LCD_E_PIN, 1, true);

    sm_config_set_out_shift(&c, true, true, 12);

    float div = (float)clock_get_hz(clk_sys) / 1000000;
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
    
    sleep_ms(50); 
    
    pio_sm_put_blocking(pio, sm, 0x03); sleep_ms(5);
    pio_sm_put_blocking(pio, sm, 0x03); sleep_us(150);
    pio_sm_put_blocking(pio, sm, 0x03);
    
    pio_sm_put_blocking(pio, sm, 0x02); 
    sleep_ms(2);

    lcd_send_byte(pio, sm, LCD_CMD_FUNCTION_SET, true); 
    lcd_send_byte(pio, sm, LCD_CMD_DISPLAY_CONTROL, true); 
    lcd_send_byte(pio, sm, LCD_CMD_ENTRY_MODE_SET, true); 
    lcd_send_byte(pio, sm, LCD_CMD_CLEAR_DISPLAY, true); 
}
