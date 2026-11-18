#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"

const static char *TAG = "MODULE 3.1";

#define LED_PIN 5
#define SMA_SAMPLE_COUNT 10
#define ADC_CHAN          ADC_CHANNEL_3
#define ADC_ATTEN         ADC_ATTEN_DB_12
#define LDR_VALUE_LED_ON 1600
#define LDR_VALUE_LED_OFF 2200

size_t writeIndex = 0;
size_t count = 0;
uint32_t sum = 0;
int16_t samples[SMA_SAMPLE_COUNT] = {};
bool isLedOn = false;

void app_main() {
    gpio_config_t conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = ((1ULL << LED_PIN)),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&conf);

    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHAN, &config));

    while(1) {
        int raw = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHAN, &raw));

        if (count == SMA_SAMPLE_COUNT) {
            sum -= samples[writeIndex];
        } else {
            count++;
        }

        samples[writeIndex] = raw;
        sum += raw;
        writeIndex = (writeIndex + 1) % SMA_SAMPLE_COUNT;
        uint16_t average = (uint16_t)(sum/count);

        if (!isLedOn && average <= LDR_VALUE_LED_ON) {
            isLedOn = true;
        } else if (isLedOn && average >= LDR_VALUE_LED_OFF) {
            isLedOn = false;
        }
        gpio_set_level(LED_PIN, isLedOn);
        ESP_LOGI(TAG, "Raw Data: %d Average: %d", raw, average);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}