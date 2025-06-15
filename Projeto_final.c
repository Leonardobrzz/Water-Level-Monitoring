#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

/* Sensor de distancia */

#define VL53L0X_ADDR 0x29  // Endereço padrão do sensor
#define I2C_PORT i2c0
#define SDA_PIN 8   // GP8 como SDA
#define SCL_PIN 9   // GP9 como SCL

void write_register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    int ret = i2c_write_blocking(I2C_PORT, VL53L0X_ADDR, buffer, 2, false);
    if (ret < 0) {
        printf("Erro ao escrever no registrador 0x%02X\n", reg);
    }
}

uint16_t read_distance() {
    uint8_t reg = 0x1E;  // Tentar outro registrador de leitura
    uint8_t buffer[2] = {0};

    int ret = i2c_write_blocking(I2C_PORT, VL53L0X_ADDR, &reg, 1, true);
    if (ret < 0) {
        printf("Erro ao solicitar leitura\n");
        return 0xFFFF;
    }

    ret = i2c_read_blocking(I2C_PORT, VL53L0X_ADDR, buffer, 2, false);
    if (ret < 0) {
        printf("Erro ao ler dados do sensor\n");
        return 0xFFFF;
    }

    uint16_t distancia = (buffer[0] << 8) | buffer[1];

    if (distancia == 0 || distancia > 2000) {
        printf("Medição inválida: %d mm\n", distancia);
        return 0xFFFF;
    }

    return distancia;
}

void vl53l0x_reset() {
    printf("Resetando sensor...\n");
    write_register(0x00, 0x00); // Reset
    sleep_ms(10);
    write_register(0x00, 0x01); // Liga de novo
    sleep_ms(10);
}

void vl53l0x_init() {
    printf("Inicializando VL53L0X...\n");

    vl53l0x_reset();  // Garante que o sensor está em um estado limpo

    write_register(0x80, 0x01);
    write_register(0xFF, 0x01);
    write_register(0x00, 0x00);
    write_register(0x91, 0x20);  // Ajuste da calibração
    write_register(0x00, 0x01);
    write_register(0xFF, 0x00);
    write_register(0x80, 0x00);

    // Ativa medições contínuas
    write_register(0x00, 0x02);  

    printf("VL53L0X pronto!\n");
}
/* ---------------------------------------- */

/* Display */
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
/* ---------------------------------------- */

/* Buzzer */
// Configuração do pino do buzzer
#define BUZZER_PIN 21

// Configuração da frequência do buzzer (em Hz)
#define BUZZER_FREQUENCY 100

// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Definição de uma função para emitir um beep com duração especificada
void beep(uint pin, uint duration_ms) {
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}
/* ---------------------------------------- */

/* LED RGB*/
// Função para atualizar os estados dos LEDs
void set_led_color(uint red_pin, uint green_pin, uint blue_pin, bool R, bool G, bool B) {
    gpio_put(red_pin, R);   // Configura o estado do LED vermelho
    gpio_put(green_pin, G); // Configura o estado do LED verde
    gpio_put(blue_pin, B);  // Configura o estado do LED azul
}

/* ---------------------------------------- */

int main() {
    stdio_init_all();

    /*Comunicação I2C com o sensor de distancia*/
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(500);
    printf("Iniciando...\n");

    vl53l0x_init();

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display
    ssd1306_init();

    // Define a área de renderização do display
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Limpa o display
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Exibe um número no display
    char str_numero[10];

    // Inicializar o PWM no pino do buzzer
    pwm_init_buzzer(BUZZER_PIN);

    // Configuração dos pinos GPIO
    const uint red_pin = 13;   // Pino para o LED vermelho
    const uint green_pin = 11; // Pino para o LED verde
    const uint blue_pin = 12;  // Pino para o LED azul

    gpio_init(red_pin);
    gpio_init(green_pin);
    gpio_init(blue_pin);

    gpio_set_dir(red_pin, GPIO_OUT);
    gpio_set_dir(green_pin, GPIO_OUT);
    gpio_set_dir(blue_pin, GPIO_OUT);

    while (true) {
        uint16_t distancia = read_distance();
        snprintf(str_numero, sizeof(str_numero), "%d", distancia); // Converte número para string
        if (distancia == 0xFFFF) {
            printf("Falha na medição\n");
            ssd1306_draw_string(ssd, 5, 10, "Falha");
        } else {
            if (distancia < 50){
                // Atualiza os LEDs conforme os estados de R, G e B
                set_led_color(red_pin, green_pin, blue_pin, 0, 1, 0); // Verde
            }else if(distancia < 250){
                // Atualiza os LEDs conforme os estados de R, G e B
                set_led_color(red_pin, green_pin, blue_pin, 0, 0, 1); // Azul
            }else if (distancia > 250){
                // Atualiza os LEDs conforme os estados de R, G e B
                set_led_color(red_pin, green_pin, blue_pin, 1, 0, 0); // Vermelho
                //Ativa o buzzer
                beep(BUZZER_PIN, 500); // Bipe de 500ms
            }
            printf("Distância: %d mm\n", distancia);
            strcat(str_numero, " mm");
            ssd1306_draw_string(ssd, 5, 10, "Distancia");
            ssd1306_draw_string(ssd, 5, 20, str_numero);
        }
        render_on_display(ssd, &frame_area);
        sleep_ms(500);
    }

    return 0;
}
