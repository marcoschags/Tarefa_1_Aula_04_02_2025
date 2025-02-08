#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include <stdio.h>
#include <ctype.h>  // Para utilizar a função isdigit()

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define WIDTH 128
#define HEIGHT 64

// Função para exibir o nome "Marcos" e outras animações
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
    // Inicialização do I2C. Usando a taxa de 400kHz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura o pino SDA para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura o pino SCL para I2C
    gpio_pull_up(I2C_SDA); // Habilita o pull-up no pino SDA
    gpio_pull_up(I2C_SCL); // Habilita o pull-up no pino SCL

    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display começa com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa a comunicação serial
    stdio_init_all();  // Inicializa a comunicação serial
    sleep_ms(100);     // Aguarda um pouco para garantir a inicialização

    while (true) {
        // Exibe a animação do nome "Marcos" e outras strings
        exibir_animacao(&ssd);

        char c = '\0';
        bool char_received = false;

        // Espera até que um caractere numérico seja digitado
        while (!char_received) {
            // Verifica se há caracteres disponíveis no Serial Monitor
            if (stdio_usb_connected()) {
                // Tenta ler um caractere
                c = getchar_timeout_us(0); 
                if (c != PICO_ERROR_TIMEOUT) { // Se um caractere foi recebido
                    printf("Caractere recebido (hex): 0x%X\n", c);  // Exibe o valor hexadecimal

                    // Verifica se o caractere é um número de '0' a '9'
                    if (isdigit(c)) {
                        printf("Caractere numérico recebido: %c\n", c);  // Exibe o caractere numérico
                        char_received = true;  // Marca que o caractere foi recebido
                    } else {
                        // Não exibe mensagem para caracteres não numéricos
                        // printf("Caractere não numérico ignorado: %c\n", c); // Se quiser logar a ignorância, pode ativar
                    }
                }
            }

            sleep_ms(100); // Espera 100ms antes de tentar ler novamente
        }

        // Limpa o display e exibe o caractere recebido
        ssd1306_fill(&ssd, false);
        ssd1306_draw_char(&ssd, c, 60, 28); // Exibe o caractere no centro do display
        ssd1306_send_data(&ssd);

        // Aguarda 2 segundos antes de retornar à animação
        sleep_ms(2000); // 2000ms = 2 segundos
    }

    return 0;
}
