#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// Inicializa o PIO para controlar os LEDs WS2812
void ws2812_init(uint pin) {
    PIO pio = pio0; // Seleciona o primeiro PIO
    uint sm = 0; // Usa a primeira State Machine (SM)

    // Carregar o programa PIO
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, pin, 800000);
}

// Envia dados para os LEDs WS2812
void ws2812_show(uint32_t *leds, int num_leds) {
    PIO pio = pio0;
    uint sm = 0;

    for (int i = 0; i < num_leds; i++) {
        uint32_t color = leds[i];

        // Reorganiza os bytes para o formato GRB esperado pelo WS2812
        uint32_t grb = ((color & 0xFF0000) >> 8) | ((color & 0x00FF00) << 8) | (color & 0x0000FF);

        // Envia os bits para o PIO
        pio_sm_put_blocking(pio, sm, grb << 8u);
    }
}
