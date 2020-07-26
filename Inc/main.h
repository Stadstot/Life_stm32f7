/**
  ******************************************************************************
  * @file    BSP/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"
#include "stm32746g_discovery_sd.h"
#include "stm32746g_discovery_eeprom.h"
#include "stm32746g_discovery_camera.h"
#include "stm32746g_discovery_audio.h"
#include "stm32746g_discovery_qspi.h"

/* Macros --------------------------------------------------------------------*/
#ifdef USE_FULL_ASSERT
/* Assert activated */
#define ASSERT(__condition__)                do { if(__condition__) \
                                                   {  assert_failed(__FILE__, __LINE__); \
                                                      while(1);  \
                                                    } \
                                              }while(0)
#else
/* Assert not activated : macro has no effect */
#define ASSERT(__condition__)                  do { if(__condition__) \
                                                   {  ErrorCounter++; \
                                                    } \
                                              }while(0)
#endif /* USE_FULL_ASSERT */

#define RGB565_BYTE_PER_PIXEL     2
#define ARBG8888_BYTE_PER_PIXEL   4

#define SDRAM_WRITE_READ_ADDR_OFFSET ((uint32_t)0x0800)
#define SRAM_WRITE_READ_ADDR_OFFSET  SDRAM_WRITE_READ_ADDR_OFFSET

/* Exported types ------------------------------------------------------------*/
/* Exported variables ---------------------------------------------------*/
extern uint8_t     NbLoop;
extern uint8_t     MfxExtiReceived;
#ifndef USE_FULL_ASSERT
extern uint32_t    ErrorCounter;
#endif
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t CheckForUserInput(void);
void BSP_LCD_DMA2D_IRQHandler(void);
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line);
#endif
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
