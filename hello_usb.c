/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include <math.h>

#define START_GPIO 0
#define START_GPIO_2 8
#define START_GPIO_3 16
#define END_GPIO 22

int bits[10] = {
    0x3f, // 0
    0x06, // 1
    0x5b, // 2
    0x4f, // 3
    0x66, // 4
    0x6d, // 5
    0x7d, // 6
    0x07, // 7
    0x7f, // 8
    0x67, // 9
};

int main()
{
    stdio_init_all();
    adc_init();

    for (int gpio = START_GPIO; gpio <= END_GPIO; gpio++)
    {
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_OUT);
    }

    gpio_init(ADC_BASE_PIN);
    gpio_set_dir(ADC_BASE_PIN, GPIO_IN);
    adc_gpio_init(ADC_BASE_PIN);
    adc_select_input(0);

    int val = 0;
    float T = 0.0;
    float T_acc = 0.0;
    float T_prom = 0.0;
    float V = 0;
    int i = 0;

    // Time used to normalize LM35 Vout on Start-Up Response ~40ms (Ref. https://www.ti.com/mx/lit/gpn/lm35)
    sleep_ms(100);
    int32_t mask = 0;
    uint16_t result = 0;
    const float conversion_factor = 3.3f / (1<<12);
    long n = 0L;
    while (true)
    {
        // printf("Temp, 7 segment\n");

        result = adc_read();
        // printf("Raw value: %d \n", result);
        V = result * conversion_factor;
        // printf("Voltage: %f V\n", V);
        T = V * 100;
        // T=(V*27)/0.706f;
        // printf("Temperatura: %f C\n", T);
       // T_acc += T;
        T_acc=(T+(n*T_acc))/(n+1);
        n++;
        if (i == 60)
        {
            T_prom = T_acc;
            printf("%f\n",T_prom);
            i = 0;
        }

        val = floor(T_prom * 10);
        val = val % 10;
        mask = bits[val] << START_GPIO;
        gpio_set_mask(mask);

        val = floor(T_prom);
        val = val % 10;
        mask = bits[val] << START_GPIO_2;
        gpio_set_mask(mask);

        val = floor(T_prom / 10);
        mask = bits[val] << START_GPIO_3;
        gpio_set_mask(mask);

        sleep_ms(1000);
        for (int gpio = START_GPIO; gpio <= END_GPIO; gpio++)
        {
            gpio_put(gpio, 0);
        }
        i++;
    }
}
