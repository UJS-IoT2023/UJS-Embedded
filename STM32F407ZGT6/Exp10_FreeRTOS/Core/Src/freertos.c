/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "stdio.h"
#include "adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WAVE_WIDTH  46
#define WAVE_HEIGHT 220
#define WAVE_Y_TOP  40
#define WAVE_X_LEFT 10

// 3.06 ~ 3.14
#define ADC_MIN (uint16_t)(3.06f / 3.3f * 4095)
#define ADC_MAX (uint16_t)(3.14f / 3.3f * 4095)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static uint16_t wave_buf[WAVE_WIDTH];
static uint16_t wave_pos = 0;
static uint8_t screen_full = 0;
/* USER CODE END Variables */
osThreadId AdcTaskHandle;
osThreadId KeyTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartAdcTask(void const *argument);

void StartKeyTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    /* place for user code */
}

/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* definition and creation of AdcTask */
    osThreadDef(AdcTask, StartAdcTask, osPriorityNormal, 0, 256);
    AdcTaskHandle = osThreadCreate(osThread(AdcTask), NULL);

    /* definition and creation of KeyTask */
    osThreadDef(KeyTask, StartKeyTask, osPriorityAboveNormal, 0, 128);
    KeyTaskHandle = osThreadCreate(osThread(KeyTask), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */
}

/* USER CODE BEGIN Header_StartAdcTask */
/**
  * @brief  Function implementing the AdcTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartAdcTask */
void StartAdcTask(void const *argument) {
    /* USER CODE BEGIN StartAdcTask */
    char buf[32];
    /* Infinite loop */
    for (;;) {
        // 1. 软件触发 ADC1 通道采样
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        uint16_t adc_val = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);

        // 2. 局部打印当前的原始 ADC Code 码值，用于辅助调试
        POINT_COLOR = WHITE;
        sprintf(buf, "ADC Code: %4d ", adc_val);
        LCD_ShowString(WAVE_X_LEFT, WAVE_Y_TOP + WAVE_HEIGHT + 25, 200, 16, 16, (uint8_t *) buf);

        // 3. 将采样值限幅并等比映射到 Y 轴相对坐标轴的像素高度 (0 ~ WAVE_HEIGHT)
        uint16_t wave_y;
        if (adc_val <= ADC_MIN) {
            wave_y = 0; // 对应 3.06V 最底部
        } else if (adc_val >= ADC_MAX) {
            wave_y = WAVE_HEIGHT; // 对应 3.14V 最顶部
        } else {
            // 映射公式
            wave_y = ((uint32_t) (adc_val - ADC_MIN) * WAVE_HEIGHT) / (ADC_MAX - ADC_MIN);
        }

        // 4. 判断满屏状态，进行无闪烁波形滑动/伸展绘制
        if (!screen_full) {
            // 阶段一：向右延伸
            if (wave_pos > 0) {
                POINT_COLOR = WHITE;
                LCD_DrawLine(WAVE_X_LEFT + (wave_pos - 1) * 10, (WAVE_Y_TOP + WAVE_HEIGHT) - wave_buf[wave_pos - 1],
                             WAVE_X_LEFT + wave_pos * 10, (WAVE_Y_TOP + WAVE_HEIGHT) - wave_y);
            }
            wave_buf[wave_pos] = wave_y;
            wave_pos++;
            if (wave_pos >= WAVE_WIDTH) screen_full = 1;
        } else {
            // 阶段二：满屏后的动态滚动（滚动时绝不触碰静态坐标轴）
            // a. 原地用【黑色】重新画一遍旧线段，等同于局部定点擦除，不影响白色的轴和箭头
            POINT_COLOR = BLACK;
            for (uint16_t i = 1; i < WAVE_WIDTH; i++) {
                LCD_DrawLine(WAVE_X_LEFT + (i - 1) * 10, (WAVE_Y_TOP + WAVE_HEIGHT) - wave_buf[i - 1],
                             WAVE_X_LEFT + i * 10, (WAVE_Y_TOP + WAVE_HEIGHT) - wave_buf[i]);
            }

            // b. 队列数据整体向前平移一步（向左滑动）
            for (uint16_t i = 0; i < WAVE_WIDTH - 1; i++) {
                wave_buf[i] = wave_buf[i + 1];
            }
            wave_buf[WAVE_WIDTH - 1] = wave_y; // 将最新的点放在队列最右侧

            // c. 用【白色】重新绘制平移后的新线段
            POINT_COLOR = WHITE;
            for (uint16_t i = 1; i < WAVE_WIDTH; i++) {
                LCD_DrawLine(WAVE_X_LEFT + (i - 1) * 10, (WAVE_Y_TOP + WAVE_HEIGHT) - wave_buf[i - 1],
                             WAVE_X_LEFT + i * 10, (WAVE_Y_TOP + WAVE_HEIGHT) - wave_buf[i]);
            }
        }

        // 5. 每 50ms 刷新采样一次（释放 CPU 让给低优先级或系统空闲任务）
        osDelay(50);
    }
    /* USER CODE END StartAdcTask */
}

/* USER CODE BEGIN Header_StartKeyTask */
/**
* @brief Function implementing the KeyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeyTask */
void StartKeyTask(void const *argument) {
    /* USER CODE BEGIN StartKeyTask */
    uint8_t key_lock = 0; // 按键自锁标志，防止长按按键导致系统高频重复调用挂起/恢复

    /* Infinite loop */
    for (;;) {
        // 读取正点原子硬件按键引脚电平
        GPIO_PinState k0_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0); // KEY0 (WK_UP) 按下为高电平(SET)
        GPIO_PinState k2_state = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2); // KEY2 按下为低电平(RESET)

        // 检测是否有按键初次被按下
        if (key_lock == 0 && (k0_state == GPIO_PIN_SET || k2_state == GPIO_PIN_RESET)) {
            osDelay(20); // FreeRTOS 优雅消抖：休眠 20ms 释放 CPU 权限

            // 再次确认按键状态
            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
                // KEY0 按下 -> 挂起（暂停）ADC 任务，LCD 画面会瞬间冻结
                osThreadSuspend(AdcTaskHandle);
                key_lock = 1;
            } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_RESET) {
                // KEY2 按下 -> 恢复（唤醒）ADC 任务，波形继续采集并向前滚动
                osThreadResume(AdcTaskHandle);
                key_lock = 1;
            }
        }
        // 按键被释放，解除自锁状态
        else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET &&
                 HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_SET) {
            key_lock = 0;
        }

        // 限制按键轮询频率，每 10ms 扫描一次，在空闲时让出运行权
        osDelay(10);
    }
    /* USER CODE END StartKeyTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Draw_Coordinate_System(void) {
    POINT_COLOR = WHITE;

    // X axis (horizontal) - voltage axis
    LCD_DrawLine(WAVE_X_LEFT, WAVE_Y_TOP + WAVE_HEIGHT, WAVE_X_LEFT + WAVE_WIDTH * 10, WAVE_Y_TOP + WAVE_HEIGHT);
    LCD_DrawLine(WAVE_X_LEFT + WAVE_WIDTH * 10, WAVE_Y_TOP + WAVE_HEIGHT, WAVE_X_LEFT + WAVE_WIDTH * 10 - 8,
                 WAVE_Y_TOP + WAVE_HEIGHT - 4);
    LCD_DrawLine(WAVE_X_LEFT + WAVE_WIDTH * 10, WAVE_Y_TOP + WAVE_HEIGHT, WAVE_X_LEFT + WAVE_WIDTH * 10 - 8,
                 WAVE_Y_TOP + WAVE_HEIGHT + 4);

    // Y axis (vertical) - time axis
    LCD_DrawLine(WAVE_X_LEFT - 1, WAVE_Y_TOP, WAVE_X_LEFT - 1, WAVE_Y_TOP + WAVE_HEIGHT);
    LCD_DrawLine(WAVE_X_LEFT - 1, WAVE_Y_TOP - 5, WAVE_X_LEFT - 4, WAVE_Y_TOP + 3);
    LCD_DrawLine(WAVE_X_LEFT - 1, WAVE_Y_TOP - 5, WAVE_X_LEFT + 2, WAVE_Y_TOP + 3);

    // Axis labels: Y axis (V, voltage), X axis (t, time)
    LCD_ShowChar(WAVE_X_LEFT - 10, WAVE_Y_TOP + 4, 'V', 16, 0);
    LCD_ShowChar(WAVE_X_LEFT + WAVE_WIDTH * 10 - 10, WAVE_Y_TOP + WAVE_HEIGHT + 4, 't', 16, 0);
}

/* USER CODE END Application */
