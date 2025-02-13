#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define VRX_PIN 26
#define VRY_PIN 27
#define BLUE_LED 12
#define RED_LED 13

uint pwm_init_gpio(uint gpio, uint wrap)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

int main()
{
    stdio_init_all();
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    uint pwm_wrap = 4096;
    pwm_init_gpio(RED_LED, pwm_wrap);
    pwm_init_gpio(BLUE_LED, pwm_wrap);

    uint32_t last_print_time = 0;

    while (true)
    {
        adc_select_input(0); // Eixo X
        adc_select_input(1); // Eixo y
        
        uint16_t vrx_value = adc_read();
        uint16_t vry_value = adc_read();

        // Ajuste da escala
        vrx_value = (vrx_value >= 2047) ? (vrx_value - 2047) * 2 : (2048 - vrx_value) * 2; // x
        vry_value = (vry_value >= 2047) ? (vry_value - 2047) * 2 : (2048 - vry_value) * 2; // y

        pwm_set_gpio_level(RED_LED, vrx_value);
        pwm_set_gpio_level(BLUE_LED, vry_value);

        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_print_time >= 1000)
        {
            printf("VRX: %u --- Duty Cycle RED_LED: %.2f%%\n\n", vrx_value, (vrx_value / (float)(pwm_wrap)) * 100);
            printf("VRY: %u --- Duty Cycle BLUE_LED: %.2f%%\n\n", vry_value, (vry_value / (float)(pwm_wrap)) * 100);
            last_print_time = current_time;
        }

        sleep_ms(100);
    }
    return 0;
}
