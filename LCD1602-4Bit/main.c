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
