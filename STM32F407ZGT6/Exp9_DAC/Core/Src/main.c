/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "stdio.h"
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define WAVE_WIDTH  46
#define WAVE_HEIGHT 220
#define WAVE_Y_TOP  40
#define WAVE_X_LEFT 10

#define ADC_MAX 4095
#define ADC_MIN 0

// Sine12bit is now computed at runtime using math.h

static uint16_t wave_buf[WAVE_WIDTH];
static uint16_t wave_pos = 0;
static uint8_t screen_full = 0;
static uint16_t Sine12bit[100];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DAC_Init();
  MX_ADC1_Init();
  MX_FSMC_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
    LCD_Init();
    LCD_Clear(BLACK);
    POINT_COLOR = WHITE;
    BACK_COLOR = BLACK;
    LCD_ShowString(30, 10, 200, 16, 16, (uint8_t *) "ADC Sine Wave");

    for (uint16_t i = 0; i < WAVE_WIDTH; i++)
        wave_buf[i] = 0;

    for (uint16_t i = 0; i < 100; i++) {
        Sine12bit[i] = (uint16_t)((sin(2 * M_PI * i / 100) + 1.0) * 2047.5);
    }

    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)Sine12bit, 100, DAC_ALIGN_12B_R);

    HAL_TIM_Base_Start(&htim6);

    // X axis (horizontal) - voltage axis
    LCD_DrawLine(WAVE_X_LEFT, WAVE_Y_TOP + WAVE_HEIGHT, WAVE_X_LEFT + WAVE_WIDTH * 10, WAVE_Y_TOP + WAVE_HEIGHT);
    LCD_DrawLine(WAVE_X_LEFT + WAVE_WIDTH * 10, WAVE_Y_TOP + WAVE_HEIGHT, WAVE_X_LEFT + WAVE_WIDTH * 10 - 8, WAVE_Y_TOP + WAVE_HEIGHT - 4);
    LCD_DrawLine(WAVE_X_LEFT + WAVE_WIDTH * 10, WAVE_Y_TOP + WAVE_HEIGHT, WAVE_X_LEFT + WAVE_WIDTH * 10 - 8, WAVE_Y_TOP + WAVE_HEIGHT + 4);

    // Y axis (vertical) - time axis
    LCD_DrawLine(WAVE_X_LEFT - 1, WAVE_Y_TOP, WAVE_X_LEFT - 1, WAVE_Y_TOP + WAVE_HEIGHT);
    LCD_DrawLine(WAVE_X_LEFT - 1, WAVE_Y_TOP - 5, WAVE_X_LEFT - 4, WAVE_Y_TOP + 3);
    LCD_DrawLine(WAVE_X_LEFT - 1, WAVE_Y_TOP - 5, WAVE_X_LEFT + 2, WAVE_Y_TOP + 3);

    // Axis labels: Y axis (V, voltage), X axis (t, time)
    LCD_ShowChar(WAVE_X_LEFT - 10, WAVE_Y_TOP + 4, 'V', 16, 0);
    LCD_ShowChar(WAVE_X_LEFT + WAVE_WIDTH * 10 - 10, WAVE_Y_TOP + WAVE_HEIGHT + 4, 't', 16, 0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
        uint16_t adc_val = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);

        // Display ADC value
        char buf[32];
        sprintf(buf, "ADC:%4d", adc_val);
        LCD_ShowString(10, 270, 200, 16, 16, (uint8_t *) buf);

        uint16_t wave_y;
        if (adc_val <= ADC_MIN) {
            wave_y = 0;
        } else if (adc_val >= ADC_MAX) {
            wave_y = WAVE_HEIGHT;
        } else {
            wave_y = (uint32_t) adc_val * WAVE_HEIGHT / ADC_MAX;
        }

        if (!screen_full) {
            if (wave_pos > 0) {
                uint16_t prev_y = wave_buf[wave_pos - 1];
                uint16_t prev_x = WAVE_X_LEFT + (wave_pos - 1) * 10;
                LCD_DrawLine(prev_x, WAVE_Y_TOP + (WAVE_HEIGHT - prev_y),
                             WAVE_X_LEFT + wave_pos * 10, WAVE_Y_TOP + (WAVE_HEIGHT - wave_y));
            }
            wave_buf[wave_pos] = wave_y;
            wave_pos++;
            if (wave_pos >= WAVE_WIDTH) screen_full = 1;
        } else {
            POINT_COLOR = BLACK;
            for (uint16_t i = 1; i < WAVE_WIDTH; i++) {
                LCD_DrawLine(WAVE_X_LEFT + (i - 1) * 10, WAVE_Y_TOP + (WAVE_HEIGHT - wave_buf[i - 1]),
                             WAVE_X_LEFT + i * 10, WAVE_Y_TOP + (WAVE_HEIGHT - wave_buf[i]));
            }

            for (uint16_t i = 0; i < WAVE_WIDTH - 1; i++) {
                wave_buf[i] = wave_buf[i + 1];
            }
            wave_buf[WAVE_WIDTH - 1] = wave_y;

            POINT_COLOR = WHITE;
            for (uint16_t i = 1; i < WAVE_WIDTH; i++) {
                LCD_DrawLine(WAVE_X_LEFT + (i - 1) * 10, WAVE_Y_TOP + (WAVE_HEIGHT - wave_buf[i - 1]),
                             WAVE_X_LEFT + i * 10, WAVE_Y_TOP + (WAVE_HEIGHT - wave_buf[i]));
            }
        }

        HAL_Delay(20);
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
