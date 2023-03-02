/*!
  * \copyright Revised BSD License, see section \ref LICENSE.
  *
  * \code
  *                ______                              _
  *               / _____)             _              | |
  *              ( (____  _____ ____ _| |_ _____  ____| |__
  *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
  *               _____) ) ____| | | || |_| ____( (___| | | |
  *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
  *              (C)2013-2017 Semtech
  *
  * \endcode
  *
  * \author    Miguel Luis ( Semtech )
  *
  * \author    Gregory Cristian ( Semtech )
  
 * \file      muratasx126xmodule.c
 *
 * \brief     Target board Murata Type-1SJ driver implementation
 *
 * \copyright Murata
 *
 * \author    Haitao Dong (haitao.dong@murata.com)
 *
 */
/**
  ******************************************************************************
  * @file    muratasx126xmodule.c
  * @author  Murata/SyChip Team
  * @brief   driver Murata SX126x module board
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics; Copyright (c) 2021 Murata.
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
#include <stdint.h>
#include <stdbool.h>
#include "muratasx126xmodule.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BOARD_WAKEUP_TIME  5 // no TCXO

#ifndef USE_MURATA_SX126X
#error "please define USE_MURATA_SX126X"
#endif

#define RADIO_GET_STATUS                        0xC0

/* Private variables ---------------------------------------------------------*/
/* static */ EXTI_HandleTypeDef hRADIO_DIO_exti[RADIO_DIOn];

/* Private function prototypes -----------------------------------------------*/


static void SX126x_RADIO_SPI_IoInit(SPI_HandleTypeDef *spiHandle);
static void SX126x_RADIO_SPI_IoDeInit(void);
/* Exported functions --------------------------------------------------------*/

void SX126x_RADIO_IoInit(void)
{
  GPIO_InitTypeDef initStruct = {0};
  SPI_HandleTypeDef dummy_hspi;

  /* DIO IO Init */

//   Set DioIrqHandler port in IT_RISING mode
  RADIO_DIO_1_GPIO_CLK_ENABLE();
  initStruct.Mode = GPIO_MODE_IT_RISING;
  initStruct.Pull = GPIO_NOPULL;
  initStruct.Speed = GPIO_SPEED_HIGH;
  initStruct.Pin = RADIO_DIO_1_PIN;
  HAL_GPIO_Init(RADIO_DIO_1_PORT, &initStruct);

  /* Radio IO Init */
  RADIO_DEV_SEL_CLK_ENABLE();
  RADIO_BUSY_CLK_ENABLE();
  initStruct.Mode = GPIO_MODE_INPUT;
  initStruct.Pull = GPIO_NOPULL;
  //initStruct.Pin = DEVICE_SEL_PIN;
  //HAL_GPIO_Init(DEVICE_SEL_PORT, &initStruct);
  initStruct.Pin = RADIO_BUSY_PIN;
  HAL_GPIO_Init(RADIO_BUSY_PORT, &initStruct);

  /* Antenna IO Init */
  RADIO_ANT_SWITCH_RX_CLK_ENABLE();
  initStruct.Pull = GPIO_NOPULL;
  initStruct.Mode = GPIO_MODE_OUTPUT_PP;
  initStruct.Pin = RADIO_ANT_SWITCH_RX_PIN;
  HAL_GPIO_Init(RADIO_ANT_SWITCH_RX_PORT, &initStruct);
  HAL_GPIO_WritePin(RADIO_ANT_SWITCH_RX_PORT, RADIO_ANT_SWITCH_RX_PIN, GPIO_PIN_RESET);

  /* SPI IO Init */
  /* Normally done by the HAL_MSP callback but not for this applic */
  SX126x_RADIO_SPI_IoInit(&dummy_hspi);

  /* NSS initialization */
  RADIO_NSS_CLK_ENABLE();
  initStruct.Mode = GPIO_MODE_OUTPUT_PP;
  initStruct.Pull = GPIO_NOPULL;
  initStruct.Pin = RADIO_NSS_PIN;
  HAL_GPIO_Init(RADIO_NSS_PORT,  &initStruct);
  HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN,  GPIO_PIN_SET);
}

void SX126x_RADIO_IoDeInit(void)
{
  /* SPI IO DeInit */
  SX126x_RADIO_SPI_IoDeInit();
}

/**
  * @brief  FunctionDescription
  * @retval None
  */
void SX126x_RADIO_IoIrqInit(DioIrqHandler **irqHandlers)
{
  HAL_EXTI_GetHandle(&hRADIO_DIO_exti[0], RADIO_DIO_1_EXTI_LINE);
  HAL_EXTI_RegisterCallback(&hRADIO_DIO_exti[0], HAL_EXTI_COMMON_CB_ID, irqHandlers[0]);
  HAL_NVIC_SetPriority(RADIO_DIO_1_IRQn, RADIO_DIO_1_IT_PRIO, 0x00);
  HAL_NVIC_EnableIRQ(RADIO_DIO_1_IRQn);
}

/*!
 * @brief it points the demanded line in the EXTI_HandleTypeDef TABLE before calling HAL_EXTI_IRQHandler()
 * @param [IN]: DIO extiLine demanded
 * @retval none
 * @note "stm32lyxx_it.c" code generated by MX do not call this function but directly the HAL_EXTI_IRQHandler()
 */
void SX126x_RADIO_IRQHandler(RADIO_DIO_TypeDef DIO)
{
  HAL_EXTI_IRQHandler(&hRADIO_DIO_exti[DIO]);
}

uint32_t SX126x_RADIO_GetWakeUpTime(void)
{
  return BOARD_WAKEUP_TIME;
}

void SX126x_RADIO_Reset(void)
{
  GPIO_InitTypeDef initStruct = { 0 };

  RADIO_RESET_CLK_ENABLE();

  initStruct.Mode = GPIO_MODE_OUTPUT_PP;
  initStruct.Pull = GPIO_NOPULL;
  initStruct.Speed = GPIO_SPEED_HIGH;

  // Wait 10 ms
  HAL_Delay(10);

  // Set RESET pin to 0
  initStruct.Pin = RADIO_RESET_PIN;
  HAL_GPIO_Init(RADIO_RESET_PORT, &initStruct);
  HAL_GPIO_WritePin(RADIO_RESET_PORT, RADIO_RESET_PIN, GPIO_PIN_RESET);

  // Wait 20 ms
  HAL_Delay(20);
  // Configure RESET as input
  initStruct.Mode = GPIO_MODE_ANALOG;
  initStruct.Pin = RADIO_RESET_PIN;
  HAL_GPIO_Init(RADIO_RESET_PORT, &initStruct);
  HAL_GPIO_WritePin(RADIO_RESET_PORT, RADIO_RESET_PIN, GPIO_PIN_RESET);  // internal pull-up

  // Wait 10 ms
  HAL_Delay(10);
}

void SX126x_RADIO_WaitOnBusy(void)
{
  while (HAL_GPIO_ReadPin(RADIO_BUSY_PORT, RADIO_BUSY_PIN) == 1);
}

void SX126x_RADIO_WakeUp(void)
{
  //NSS = 0;
  HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN, GPIO_PIN_RESET);

  SX126x_RADIO_SendRecv(RADIO_GET_STATUS);
  SX126x_RADIO_SendRecv(0);

  //NSS = 1;
  HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN, GPIO_PIN_SET);


  SX126x_RADIO_WaitOnBusy();
}

void SX126x_RADIO_ChipSelect(int32_t state)
{
  if (state == 0)
  {
    HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN, GPIO_PIN_SET);
  }
}

uint8_t SX126x_RADIO_GetPaSelect(uint32_t channel)
{
#ifdef USE_MURATA_SX126X
    return SX1262;
#else
    if (HAL_GPIO_ReadPin(DEVICE_SEL_PORT, DEVICE_SEL_PIN) ==  GPIO_PIN_SET)
    {
      return SX1261;
    }
    else
    {
      return SX1262;
    }

#endif

}

void SX126x_RADIO_SetAntSw(RfSw_TypeDef state)
{
  switch (state)
  {
    case RFSW_OFF:
      HAL_GPIO_WritePin(RADIO_ANT_SWITCH_RX_PORT, RADIO_ANT_SWITCH_RX_PIN, GPIO_PIN_RESET);
      break;
    case RFSW_RX:
      HAL_GPIO_WritePin(RADIO_ANT_SWITCH_RX_PORT, RADIO_ANT_SWITCH_RX_PIN, GPIO_PIN_SET);
      break;
    default:
      HAL_GPIO_WritePin(RADIO_ANT_SWITCH_RX_PORT, RADIO_ANT_SWITCH_RX_PIN, GPIO_PIN_SET);
      break;
  }
}

bool SX126x_RADIO_CheckRfFrequency(uint32_t frequency)
{
  // Implement check. Currently all frequencies are supported
  return true;
}

bool SX126x_RADIO_BoardIsTcxo(void)
{
  /* TCXO on board*/
  return true;
}

void SX126x_RADIO_BoardSetLedTx(bool state)
{
#ifdef RADIO_LED_IND
  if (state == false)
  {
    HAL_GPIO_WritePin(RADIO_LEDTX_PORT, RADIO_LEDTX_PIN, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(RADIO_LEDTX_PORT, RADIO_LEDTX_PIN, GPIO_PIN_SET);
  }
#endif
}

void SX126x_RADIO_BoardSetLedRx(bool state)
{
#ifdef RADIO_LED_IND
  if (state == false)
  {
    HAL_GPIO_WritePin(RADIO_LEDRX_PORT, RADIO_LEDRX_PIN, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(RADIO_LEDRX_PORT, RADIO_LEDRX_PIN, GPIO_PIN_SET);
  }
#endif
}

/* Bus mapping to SPI */
void SX126x_RADIO_Bus_Init(void)
{
  RADIO_SPI_Init();
}

void SX126x_RADIO_Bus_deInit(void)
{
  RADIO_SPI_DeInit();
}

/**
  * @brief  Send Receive data via SPI
  * @param  Data to send
  * @retval Received data
  */
uint16_t SX126x_RADIO_SendRecv(uint16_t txData)
{
  uint16_t rxData ;

  /*RADIO_SPI_SendRecv(( uint8_t * ) &txData, ( uint8_t* ) &rxData, 1);*/
  RADIO_SPI_SendRecv((uint8_t *) &txData, (uint8_t *) &rxData, 1);

  return rxData;
}

/* Private functions --------------------------------------------------------*/

/**
  * @brief  Initializes SPI MSP.
  * @param  hspi  SPI handler
  * @retval None
*/
static void SX126x_RADIO_SPI_IoInit(SPI_HandleTypeDef *spiHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
  /* Enable Peripheral clock */
  RADIO_SPI_SCK_GPIO_CLK_ENABLE();
  RADIO_SPI_MOSI_GPIO_CLK_ENABLE();
  RADIO_SPI_MISO_GPIO_CLK_ENABLE();

  /**SPI1 GPIO Configuration
  PA5     ------> SPI1_SCK
  PA6     ------> SPI1_MISO
  PA7     ------> SPI1_MOSI
    */
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  GPIO_InitStruct.Alternate = RADIO_SPI_MOSI_GPIO_AF;
  GPIO_InitStruct.Pin = RADIO_SPI_MOSI_GPIO_PIN;
  HAL_GPIO_Init(RADIO_SPI_MOSI_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Alternate = RADIO_SPI_MISO_GPIO_AF;
  GPIO_InitStruct.Pin = RADIO_SPI_MISO_GPIO_PIN;
  HAL_GPIO_Init(RADIO_SPI_MISO_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Alternate = RADIO_SPI_SCK_GPIO_AF;
  GPIO_InitStruct.Pin = RADIO_SPI_SCK_GPIO_PIN;
  HAL_GPIO_Init(RADIO_SPI_SCK_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  DeInitializes SPI MSP.
  * @param  hspi  SPI handler
  * @retval None
  */
static void SX126x_RADIO_SPI_IoDeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Peripheral clock disable */
  /* no need to call SPI1_CLK_DISABLE() because going in LowPower it gets disabled automatically */

  /* DeInitialize Peripheral GPIOs */
  /* Instead of using HAL_GPIO_DeInit() which set ANALOG mode
     it's preferred to set in OUTPUT_PP mode, with the pins set to 0 */

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = RADIO_SPI_MOSI_GPIO_PIN;
  HAL_GPIO_Init(RADIO_SPI_MOSI_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = RADIO_SPI_MISO_GPIO_PIN;
  HAL_GPIO_Init(RADIO_SPI_MISO_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = RADIO_SPI_SCK_GPIO_PIN;
  HAL_GPIO_Init(RADIO_SPI_SCK_GPIO_PORT, &GPIO_InitStruct);


  HAL_GPIO_WritePin(RADIO_SPI_MOSI_GPIO_PORT, RADIO_SPI_MOSI_GPIO_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(RADIO_SPI_MISO_GPIO_PORT, RADIO_SPI_MISO_GPIO_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(RADIO_SPI_SCK_GPIO_PORT, RADIO_SPI_SCK_GPIO_PIN, GPIO_PIN_RESET);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/