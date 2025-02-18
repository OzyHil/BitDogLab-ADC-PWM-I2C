#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "include/ssd1306.h" // Biblioteca para o display OLED
#include "math.h"

// Definição dos pinos dos componentes
#define VRX_PIN 26               // Pino do joystick eixo X
#define VRY_PIN 27               // Pino do joystick eixo Y
#define JOYSTICK_BUTTON 22       // Botão do joystick
#define BUTTON_A 5               // Botão A
#define GREEN_LED 11             // LED verde
#define BLUE_LED 12              // LED azul
#define RED_LED 13               // LED vermelho
#define I2C_PORT i2c1            // Porta I2C
#define I2C_SDA 14               // Pino SDA da I2C
#define I2C_SCL 15               // Pino SCL da I2C
#define DISPLAY_ADDRESS 0x3C     // Endereço I2C do display
#define PWM_WRAP 4066            // Resolução do PWM
#define LOWEST_AXIS_VALUE 16     // Menor valor lido pelo ADC do joystick
#define HIGHEST_AXIS_VALUE 4082  // Maior valor lido pelo ADC do joystick
#define DEBOUNCE_DELAY_US 250000 // Tempo de debounce dos botões (microssegundos)

// Variáveis para debounce e estado do sistema
static volatile uint32_t last_time_joystick = 0; // Último tempo dese a iterrupção do botão do joystick
static volatile uint32_t last_time_A = 0; // Último tempo dese a iterrupção do botão A
bool pwm_state = true;                    // Estado do PWM (ligado/desligado)
bool rec_border_state = true;             // Estado da borda retangular
bool dashed_border_state = false;         // Estado da borda tracejada
static volatile uint8_t num_leds_x = 119; // Número de LEDs no eixo X (largura do display)
static volatile uint8_t num_leds_y = 55;  // Número de LEDs no eixo Y (altura do display)
ssd1306_t ssd;                            // Estrutura para o display OLED

// Inicializa o PWM em um pino GPIO
uint pwm_init_gpio(uint gpio, uint wrap)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, pwm_state);
    return slice_num;
}

// Função de debounce para os botões
bool button_debounce(volatile uint32_t *lastTime)
{
    uint32_t currentTime = to_us_since_boot(get_absolute_time());
    if (currentTime - *lastTime > DEBOUNCE_DELAY_US)
    {
        *lastTime = currentTime;
        return true;
    }
    return false;
}

// Callback para interrupções dos botões
void handle_button_callback(uint gpio, uint32_t events)
{
    if (gpio == JOYSTICK_BUTTON && button_debounce(&last_time_joystick))
    {
        gpio_put(GREEN_LED, !gpio_get(GREEN_LED));  // Inverte o estado do LED verde
        rec_border_state = !rec_border_state;       // Inverte o estado da borda retangular
        dashed_border_state = !dashed_border_state; // Inverte o estado da borda tracejada
    }
    else if (gpio == BUTTON_A && button_debounce(&last_time_A))
    {
        pwm_state = !pwm_state; // Inverte o estado do PWM
    }
}

// Configura a interrupção para um pino
void set_interruption(int pin)
{
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL, true); // Interrupção na borda de descida
    gpio_set_irq_callback(handle_button_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

// Configura os pinos GPIO
void configure_gpio()
{
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

    pwm_init_gpio(RED_LED, PWM_WRAP);
    pwm_init_gpio(BLUE_LED, PWM_WRAP);
}

// Configura o display I2C
void configure_i2c_display()
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, 128, 64, false, DISPLAY_ADDRESS, I2C_PORT); // Inicializa o display OLED (assumindo resolução 128x64)
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
}

// Lê os valores do joystick do ADC
void read_joystick(uint16_t *vrx_value, uint16_t *vry_value)
{
    adc_select_input(1);
    *vrx_value = adc_read();
    adc_select_input(0);
    *vry_value = adc_read();
}

// Atualiza o display OLED
void update_display(uint16_t x_position, uint16_t y_position)
{
    ssd1306_fill(&ssd, false); // Limpa o display
    if (rec_border_state)
    {
        ssd1306_rect(&ssd, 0, 0, 128, 64, true, false); // Desenha um retângulo
    }
    if (dashed_border_state)
    {
        // Desenha uma borda tracejada
        for (int x = 0; x <= 120; x += 15)
        {
            ssd1306_hline(&ssd, x, x + 7, 0, true);
            ssd1306_hline(&ssd, x, x + 7, 63, true);
        }
        for (int y = 0; y <= 57; y += 14)
        {
            ssd1306_vline(&ssd, 0, y, y + 6, true);
            ssd1306_vline(&ssd, 127, y, y + 6, true);
        }
    }
    ssd1306_rect(&ssd, y_position, x_position, 8, 8, true, true); // Desenha um retângulo preenchido representando a posição do joystick
    ssd1306_send_data(&ssd);
}

// Atualiza os sinais PWM para controlar os LEDs
void update_pwm(uint16_t vrx_value, uint16_t vry_value)
{
    if (pwm_state)
    {
        vrx_value -= LOWEST_AXIS_VALUE;
        vry_value -= LOWEST_AXIS_VALUE;
        vrx_value = abs(vrx_value - 2033) * 2; // Centrado em 2033
        vry_value = abs(vry_value - 2033) * 2; // Centrado em 2033
        
        // Limiar para ativação dos LEDs
        if (vrx_value >= 440 || vry_value >= 170)
        {
            pwm_set_gpio_level(RED_LED, vrx_value);
            pwm_set_gpio_level(BLUE_LED, vry_value);
        }
        else
        {
            pwm_set_gpio_level(RED_LED, 0);
            pwm_set_gpio_level(BLUE_LED, 0);
        }
    }
    else
    {
        pwm_set_gpio_level(RED_LED, 0);
        pwm_set_gpio_level(BLUE_LED, 0);
    }
}

int main()
{
    stdio_init_all();
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);
    configure_gpio();
    configure_i2c_display();
    uint16_t range = HIGHEST_AXIS_VALUE - LOWEST_AXIS_VALUE;
    uint16_t vrx_value, vry_value;

    while (true)
    {
        read_joystick(&vrx_value, &vry_value);
        // Mapeia os valores do joystick para coordenadas do display
        uint16_t x_position = roundf(num_leds_x * (float)(vrx_value - LOWEST_AXIS_VALUE) / range);
        uint16_t y_position = roundf(num_leds_y * (float)(vry_value - LOWEST_AXIS_VALUE) / range);
        update_display(x_position, y_position);
        update_pwm(vrx_value, vry_value);
    }
    return 0;
}