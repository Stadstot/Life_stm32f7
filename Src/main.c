/**
 ******************************************************************************
 * @file    BSP/Src/main.c
 * @author  MCD Application Team
 * @brief   This example code shows how to use the STM32746G Discovery BSP Drivers
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdbool.h>
/** @addtogroup STM32F7xx_HAL_Examples
 * @{
 */

/** @addtogroup BSP
 * @{
 */

/* Private typedef ------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Global extern variables ---------------------------------------------------*/
uint8_t NbLoop = 1;
#ifndef USE_FULL_ASSERT
uint32_t ErrorCounter = 0;
#endif
#define LCD_FRAME_BUFFER          SDRAM_DEVICE_ADDR

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void GUIThread(void const *argument);
static void TimerCallback(void const *n);

/* Private functions ---------------------------------------------------------*/

static const uint32_t redDot[25] = {
		0x28ff0000, 0xd3ff0000, 0xffff0000, 0xd3ff0000, 0x28ff0000,
		0xd3ff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xd3ff0000,
		0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
		0xd3ff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xd3ff0000,
		0x28ff0000, 0xd3ff0000, 0xffff0000, 0xd3ff0000, 0x28ff0000
};

static const uint32_t blackDot[25] = {
		0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
		0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
		0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
		0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
		0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000
};

extern LTDC_HandleTypeDef  hLtdcHandler;
RNG_HandleTypeDef RngHandle;

#define X_COUNT 79
#define Y_COUNT 44

bool field[X_COUNT][Y_COUNT];

void drawDot(uint32_t x_index, uint32_t y_index, uint32_t* dotArr) {
	uint32_t x = 4 + x_index * 6;
	uint32_t y = 5 + y_index * 6;
	uint32_t address = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress + (((BSP_LCD_GetXSize()*y) + x)*(4));
	for(uint32_t i = 0; i < 5; i += 1) {
		memcpy((uint32_t*) address, &dotArr[i * 5], 4 * 5);
		address += BSP_LCD_GetXSize() * 4;
	}
}

static bool getCellNewState(uint32_t x, uint32_t y) {
	x += X_COUNT;
	y += Y_COUNT;
	uint8_t nCount = 0;
	if (field[(x - 1) % X_COUNT][(y - 1) % Y_COUNT]) nCount += 1;
	if (field[(x) % X_COUNT][(y - 1) % Y_COUNT]) nCount += 1;
	if (field[(x + 1) % X_COUNT][(y - 1) % Y_COUNT]) nCount += 1;
	if (field[(x - 1) % X_COUNT][(y) % Y_COUNT]) nCount += 1;
	if (field[(x + 1) % X_COUNT][(y) % Y_COUNT]) nCount += 1;
	if (field[(x - 1) % X_COUNT][(y + 1) % Y_COUNT]) nCount += 1;
	if (field[(x) % X_COUNT][(y + 1) % Y_COUNT]) nCount += 1;
	if (field[(x + 1) % X_COUNT][(y + 1) % Y_COUNT]) nCount += 1;
	if (field[x % X_COUNT][y % Y_COUNT]) {
		if ((2 <= nCount) && (nCount <= 3)) {
			return true;
		} else {
			return false;
		}
	} else {
		if (nCount == 3) {
			return true;
		} else {
			return false;
		}
	}
}

static void createStartField() {
	RngHandle.Instance = RNG;
	HAL_RNG_DeInit(&RngHandle);
	__RNG_FORCE_RESET();
	__RNG_RELEASE_RESET();
	__RNG_CLK_ENABLE();
	HAL_RNG_Init(&RngHandle);
	uint32_t rndNumber = 0;
	uint32_t bitsCount = 0;
	for(uint i = 0; i < X_COUNT; i += 1) {
		for(uint j = 0; j < Y_COUNT; j += 1) {
			if (bitsCount == 0) {
				HAL_RNG_GenerateRandomNumber(&RngHandle, &rndNumber);
				bitsCount = 32;
			}
			field[i][j] = (rndNumber & 0x00000001) == 0x00000001;
			rndNumber = rndNumber >> 1;
			bitsCount -= 1;
		}
	}
}

static void updateField() {
	static bool updatedField[X_COUNT][Y_COUNT];
	for(uint i = 0; i < X_COUNT; i += 1) {
		for(uint j = 0; j < Y_COUNT; j += 1) {
			updatedField[i][j] = getCellNewState(i, j);
		}
	}
	for(uint i = 0; i < X_COUNT; i += 1) {
		for(uint j = 0; j < Y_COUNT; j += 1) {
			field[i][j] = updatedField[i][j];
		}
	}
}

static void redrawField() {
	for(uint i = 0; i < 79; i += 1) {
		for(uint j = 0; j < 44; j += 1) {
			if (field[i][j]) {
				drawDot(i, j, redDot);
			} else {
				drawDot(i, j, blackDot);
			}
		}
	}
}

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void) {
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();

	BSP_LED_Init(LED1);

	/* Configure the User Button in GPIO Mode */
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FRAME_BUFFER);
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_TS_Init(480, 272);

	BSP_LCD_SetTextColor(0xffffffff);
	BSP_LCD_DrawRect(2, 2, 476, 268);

	createStartField();
	redrawField();

	uint32_t lastUpdatedTick = HAL_GetTick();
	while (1) {
		if ((HAL_GetTick() - lastUpdatedTick) > 250) {
			updateField();
			redrawField();
			lastUpdatedTick = HAL_GetTick();
		}
		HAL_Delay(2);
	}
}

/**
 * @brief  Timer callbacsk (40 ms)
 * @param  n: Timer index
 * @retval None
 */
static void TimerCallback(void const *n) {
	//k_TouchUpdate();
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 200000000
 *            HCLK(Hz)                       = 200000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 25000000
 *            PLL_M                          = 25
 *            PLL_N                          = 400
 *            PLL_P                          = 2
 *            PLL_Q                          = 8
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 5
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void) {
	HAL_StatusTypeDef ret = HAL_OK;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	 clocked below the maximum system frequency, to update the voltage scaling value
	 regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 400;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 8;
	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	ASSERT(ret != HAL_OK);

	/* activate the OverDrive */
	ret = HAL_PWREx_ActivateOverDrive();
	ASSERT(ret != HAL_OK);

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
	ASSERT(ret != HAL_OK);
}

/**
 * @brief  Check for user input.
 * @param  None
 * @retval Input state (1 : active / 0 : Inactive)
 */
uint8_t CheckForUserInput(void) {
	if (BSP_PB_GetState(BUTTON_KEY) != RESET) {
		HAL_Delay(10);
		while (BSP_PB_GetState(BUTTON_KEY) != RESET)
			;
		return 1;
	}
	return 0;
}

/**
 * @brief EXTI line detection callbacks.
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	static uint32_t debounce_time = 0;

	if (GPIO_Pin == KEY_BUTTON_PIN) {
		/* Prevent debounce effect for user key */
		if ((HAL_GetTick() - debounce_time) > 50) {
			debounce_time = HAL_GetTick();
		}
	} else if (GPIO_Pin == AUDIO_IN_INT_GPIO_PIN) {
		/* Audio IN interrupt */
	}
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void) {
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
