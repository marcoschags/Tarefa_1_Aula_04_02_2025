#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include <stdio.h>
#include <ctype.h>
#include "hardware/clocks.h"
#include "ws2812.pio.h" // Biblioteca para controlar os LEDs WS2812

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define WIDTH 128
#define HEIGHT 64

#define LED_PIN 7 // Pino de controle da matriz WS2812
#define NUM_LEDS 25 // Matriz 5x5

#define LED_G_PIN 11
#define LED_B_PIN 12
#define LED_R_PIN 13

#define BOTAO_A_PIN 5
#define BOTAO_B_PIN 6

uint32_t leds[NUM_LEDS]; // Buffer para os LEDs
bool led_verde_status = false;
bool led_azul_status = false;

// Mapeamento simplificado de caracteres para a matriz 5x5
const uint8_t font5x5[10][5] = {
    {0b01110, 0b10001, 0b10001, 0b10001, 0b01110}, // 0
    {0b00100, 0b01100, 0b00100, 0b00100, 0b01110}, // 1
    {0b01110, 0b00001, 0b01110, 0b10000, 0b11111}, // 2
    {0b01110, 0b00001, 0b01110, 0b00001, 0b01110}, // 3
    {0b10001, 0b10001, 0b01110, 0b00001, 0b00001}, // 4
    {0b11111, 0b10000, 0b01110, 0b00001, 0b01110}, // 5
    {0b01110, 0b10000, 0b11110, 0b10001, 0b01110}, // 6
    {0b11111, 0b00001, 0b00010, 0b00100, 0b00100}, // 7
    {0b01110, 0b10001, 0b01110, 0b10001, 0b01110}, // 8
    {0b01110, 0b10001, 0b01111, 0b00001, 0b01110} // 9
};

// Atualiza a matriz de LEDs
void mostrar_caractere_led(char c) {
    if (c < '0' || c > '9') return; // Apenas números suportados
    int index = c - '0';

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            int ledIndex = y * 5 + x;
            if (font5x5[index][y] & (1 << (4 - x))) {
                leds[ledIndex] = 0xFFFFFF; // Branco
            } else {
                leds[ledIndex] = 0x000000; // Preto (desligado)
            }
        }
    }
    ws2812_show(leds, NUM_LEDS);
}

void exibir_animacao(ssd1306_t *ssd) {
    static bool cor = true; // Flag para alternar a cor
    cor = !cor;

    // Atualiza o conteúdo do display com animações
    ssd1306_fill(ssd, !cor); // Limpa o display
    ssd1306_rect(ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(ssd, "EMBARCATECH", 20, 10); // Desenha a string "CEPEDI"
    ssd1306_draw_string(ssd, "Juazeiro BA", 20, 30); // Desenha a string "EMBARCATECH"
    ssd1306_draw_string(ssd, "Marcos Jose", 17, 48); // Desenha a string "Marcos"
    ssd1306_send_data(ssd); // Atualiza o display
}

void botao_a_irq_handler(uint gpio, uint32_t events) {
    led_verde_status = !led_verde_status;
    gpio_put(LED_G_PIN, led_verde_status);
    
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, led_verde_status ? "LED Verde ON" : "LED Verde OFF", 10, 30);
    ssd1306_send_data(&ssd);
    
    printf("Botão A pressionado, LED Verde %s\n", led_verde_status ? "ligado" : "desligado");
}

void botao_b_irq_handler(uint gpio, uint32_t events) {
    led_azul_status = !led_azul_status;
    gpio_put(LED_B_PIN, led_azul_status);
    
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, led_azul_status ? "LED Azul ON" : "LED Azul OFF", 10, 30);
    ssd1306_send_data(&ssd);
    
    printf("Botão B pressionado, LED Azul %s\n", led_azul_status ? "ligado" : "desligado");
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    gpio_init(BOTAO_A_PIN);
    gpio_set_dir(BOTAO_A_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_A_PIN);
    gpio_init(BOTAO_B_PIN);
    gpio_set_dir(BOTAO_B_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_B_PIN);
    
    gpio_set_irq_enabled_with_callback(BOTAO_A_PIN, GPIO_IRQ_EDGE_FALL, true, &botao_a_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B_PIN, GPIO_IRQ_EDGE_FALL, true, &botao_b_irq_handler);
    
    ws2812_init(LED_PIN); // Inicializa WS2812

    while (true) {
        exibir_animacao(&ssd);
        sleep_ms(2000);

        char c = '\0';
        bool char_received = false;

        while (!char_received) {
            if (stdio_usb_connected()) {
                c = getchar_timeout_us(0);
                if (c != PICO_ERROR_TIMEOUT && isdigit(c)) {
                    printf("Caractere recebido: %c\n", c);
                    char_received = true;
                }
            }
            sleep_ms(100);
        }

        // Exibir no OLED
        ssd1306_fill(&ssd, false);
        ssd1306_draw_char(&ssd, c, 60, 28);
        ssd1306_send_data(&ssd);

        // Exibir na matriz de LEDs
        mostrar_caractere_led(c);

        sleep_ms(2000);
    }
    return 0;
}
















/*#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include <stdio.h>
#include <ctype.h>
#include "hardware/clocks.h"
#include "ws2812.pio.h"  // Biblioteca para controlar os LEDs WS2812

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define WIDTH 128
#define HEIGHT 64

#define LED_PIN 7  // Pino de controle da matriz WS2812
#define NUM_LEDS 25 // Matriz 5x5

uint32_t leds[NUM_LEDS]; // Buffer para os LEDs

// Mapeamento simplificado de caracteres para a matriz 5x5
const uint8_t font5x5[10][5] = {
    {0b01110, 0b10001, 0b10001, 0b10001, 0b01110}, // 0
    {0b00100, 0b01100, 0b00100, 0b00100, 0b01110}, // 1
    {0b01110, 0b00001, 0b01110, 0b10000, 0b11111}, // 2
    {0b01110, 0b00001, 0b01110, 0b00001, 0b01110}, // 3
    {0b10001, 0b10001, 0b01110, 0b00001, 0b00001}, // 4
    {0b11111, 0b10000, 0b01110, 0b00001, 0b01110}, // 5
    {0b01110, 0b10000, 0b11110, 0b10001, 0b01110}, // 6
    {0b11111, 0b00001, 0b00010, 0b00100, 0b00100}, // 7
    {0b01110, 0b10001, 0b01110, 0b10001, 0b01110}, // 8
    {0b01110, 0b10001, 0b01111, 0b00001, 0b01110}  // 9
};

// Atualiza a matriz de LEDs
void mostrar_caractere_led(char c) {
    if (c < '0' || c > '9') return; // Apenas números suportados
    int index = c - '0';

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            int ledIndex = y * 5 + x;
            if (font5x5[index][y] & (1 << (4 - x))) {
                leds[ledIndex] = 0xFFFFFF; // Branco
            } else {
                leds[ledIndex] = 0x000000; // Preto (desligado)
            }
        }
    }
    ws2812_show(leds, NUM_LEDS);
}

void exibir_animacao(ssd1306_t *ssd) {
    static bool cor = true; // Flag para alternar a cor
    cor = !cor;

    // Atualiza o conteúdo do display com animações
    ssd1306_fill(ssd, !cor); // Limpa o display
    ssd1306_rect(ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(ssd, "    CEPEDI", 8, 10); // Desenha a string "CEPEDI"
    ssd1306_draw_string(ssd, "EMBARCATECH", 20, 30); // Desenha a string "EMBARCATECH"
    ssd1306_draw_string(ssd, "   Marcos", 15, 48); // Desenha a string "Marcos"
    ssd1306_send_data(ssd); // Atualiza o display
}

int main() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    
    stdio_init_all();
    sleep_ms(100);
    
    ws2812_init(LED_PIN); // Inicializa WS2812

    while (true) {
        exibir_animacao(&ssd);
        sleep_ms(2000);

        char c = '\0';
        bool char_received = false;

        while (!char_received) {
            if (stdio_usb_connected()) {
                c = getchar_timeout_us(0);
                if (c != PICO_ERROR_TIMEOUT && isdigit(c)) {
                    printf("Caractere recebido: %c\n", c);
                    char_received = true;
                }
            }
            sleep_ms(100);
        }

        // Exibir no OLED
        ssd1306_fill(&ssd, false);
        ssd1306_draw_char(&ssd, c, 60, 28);
        ssd1306_send_data(&ssd);

        // Exibir na matriz de LEDs
        mostrar_caractere_led(c);

        sleep_ms(2000);
    }
    return 0;
}*/
