#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "include/ssd1306.h"
#include "math.h"

#define VRX_PIN 26
#define VRY_PIN 27

#define JOYSTICK_BUTTON 22
#define BUTTON_A 5

#define GREEN_LED 11
#define BLUE_LED 12
#define RED_LED 13

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ADRESS 0x3C

static volatile uint32_t last_time_joystick = 0; // Tempo de última interrupção do botão do joystick
static volatile uint32_t last_time_A = 0;        // Tempo de última interrupção do botão A
bool pwm_state = true;

uint pwm_init_gpio(uint gpio, uint wrap)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, pwm_state);
    return slice_num;
}

bool button_debounce(volatile uint32_t *lastTime)
{
    uint32_t currentTime = to_us_since_boot(get_absolute_time());

    // Verifica se o tempo de debouncing passou (250ms)
    if (currentTime - *lastTime > 250000)
    {
        *lastTime = currentTime;
        return true;
    }
    return false;
}

void handle_button_callback(uint gpio, uint32_t events)
{
    if (gpio == 22 && button_debounce(&last_time_joystick))
    {
        gpio_put(GREEN_LED, !gpio_get(GREEN_LED));
    }
    else if (gpio == 5 && button_debounce(&last_time_A))
    {
        pwm_state = !pwm_state;
    }
}

// Função para configurar a interrupção no pino do botão
void set_interruption(int pin)
{
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL, true); // Ativa interrupção para o pino
    gpio_set_irq_callback(&handle_button_callback);      // Registra a função de callback uma vez
    irq_set_enabled(IO_IRQ_BANK0, true);                 // Ativa interrupções no banco de GPIOs
}

int main()
{
    stdio_init_all();
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);
    set_interruption(JOYSTICK_BUTTON);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    set_interruption(BUTTON_A);

    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);

    uint pwm_wrap = 4096;
    pwm_init_gpio(RED_LED, pwm_wrap);
    pwm_init_gpio(BLUE_LED, pwm_wrap);

    // Inicializa comunicação I2C com o display OLED a 400kHz
    i2c_init(I2C_PORT, 400 * 1000);

    uint32_t last_print_time = 0;

    // Configuração dos pinos de SDA e SCL para comunicação I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                     // Pull up the data line
    gpio_pull_up(I2C_SCL);                     // Pull up the clock line
    ssd1306_t ssd;                             // Inicializa a estrutura do display

    // Inicializa o display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ADRESS, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                       // Configura o display
    ssd1306_send_data(&ssd);                                    // Envia os dados para o display

    while (true)
    {
        if (pwm_state)
        {
            adc_select_input(0); // Eixo X
            uint16_t vrx_value = adc_read();
            adc_select_input(1); // Eixo y
            uint16_t vry_value = adc_read();

            
            // printf("VRX: %d", vrx_value);
            // printf("VRY: %d", vry_value);
            
            // Alterar caso o joystick esteja desregulado
            uint16_t x_max_value = 4095;
            uint16_t y_max_value = 4095;

            uint16_t x_position = roundf(120 - (vrx_value * 120 / x_max_value));
            uint16_t y_position = roundf(56 - (vry_value * 56 / y_max_value));

            ssd1306_fill(&ssd, false);
            ssd1306_rect(&ssd, y_position, x_position, 8, 8, true, true); // eixo y, eixo x, largura, altura, visibilidade, preenchimento
            ssd1306_send_data(&ssd);                                      // Atualiza o display

            // Ajuste da escala
            vrx_value = (vrx_value >= 2047) ? (vrx_value - 2047) * 2 : (2048 - vrx_value) * 2; // x
            vry_value = (vry_value >= 2047) ? (vry_value - 2047) * 2 : (2048 - vry_value) * 2; // y

            pwm_set_gpio_level(RED_LED, vrx_value);
            pwm_set_gpio_level(BLUE_LED, vry_value);
        }
        else
        {
            pwm_set_gpio_level(RED_LED, 0);
            pwm_set_gpio_level(BLUE_LED, 0);
        }
    }
    return 0;
}

// uint32_t current_time = to_ms_since_boot(get_absolute_time());
// if (current_time - last_print_time >= 1000)
// {
//     printf("VRX: %u --- Duty Cycle RED_LED: %.2f%%\n", vrx_value, (vrx_value / (float)(pwm_wrap)) * 100);
//     printf("VRY: %u --- Duty Cycle BLUE_LED: %.2f%%\n", vry_value, (vry_value / (float)(pwm_wrap)) * 100);
//     printf("Estado do LED Verde:%s\n", gpio_get(GREEN_LED) ? "ligado" : "desligado");
//     printf("Estado da PWM %s\n", pwm_state ? "ativada" : "desativada");
//     last_print_time = current_time;
// }
