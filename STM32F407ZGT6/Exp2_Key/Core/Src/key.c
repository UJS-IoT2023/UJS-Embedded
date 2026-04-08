#include "key.h"

uint8_t KEY_Scan(uint8_t mode)
{
    static uint8_t key_up = 1;
    if (mode) key_up = 1;

    if (key_up && (HAL_GPIO_ReadPin(GPIOE, KEY0_Pin) == GPIO_PIN_RESET || \
                   HAL_GPIO_ReadPin(GPIOE, KEY1_Pin) == GPIO_PIN_RESET || \
                   HAL_GPIO_ReadPin(GPIOE, KEY2_Pin) == GPIO_PIN_RESET || \
                   HAL_GPIO_ReadPin(GPIOA, KEY_UP_Pin) == GPIO_PIN_SET))
    {
        HAL_Delay(10);
        key_up = 0;

        if (HAL_GPIO_ReadPin(GPIOE, KEY0_Pin) == GPIO_PIN_RESET)      return KEY0_PRES;
        else if (HAL_GPIO_ReadPin(GPIOE, KEY1_Pin) == GPIO_PIN_RESET) return KEY1_PRES;
        else if (HAL_GPIO_ReadPin(GPIOE, KEY2_Pin) == GPIO_PIN_RESET) return KEY2_PRES;
        else if (HAL_GPIO_ReadPin(GPIOA, KEY_UP_Pin) == GPIO_PIN_SET) return WKUP_PRES;
    }
    else if (HAL_GPIO_ReadPin(GPIOE, KEY0_Pin) == GPIO_PIN_SET && \
             HAL_GPIO_ReadPin(GPIOE, KEY1_Pin) == GPIO_PIN_SET && \
             HAL_GPIO_ReadPin(GPIOE, KEY2_Pin) == GPIO_PIN_SET && \
             HAL_GPIO_ReadPin(GPIOA, KEY_UP_Pin) == GPIO_PIN_RESET)
    {
        key_up = 1;
    }
    return NO_KEY_PRES;
}