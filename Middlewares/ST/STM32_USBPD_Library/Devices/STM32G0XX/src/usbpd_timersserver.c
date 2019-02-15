/**
  ******************************************************************************
  * @file    usbpd_timersserver.c
  * @author  MCD Application Team
  * @brief   This file contains timer server functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbpd_devices_conf.h"
#include "usbpd_timersserver.h"

/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_DEVICE
  * @{
  */

/** @addtogroup USBPD_DEVICE_TIMESERVER
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize Timer 2
  * @retval None
  */
void USBPD_TIM_Init(void)
{
  TIMX_CLK_ENABLE;
  /***************************/
  /* Time base configuration */
  /***************************/
  /* Counter mode: select up-counting mode */
  LL_TIM_SetCounterMode(TIMX, LL_TIM_COUNTERMODE_UP);

  /* Set the pre-scaler value to have TIMx counter clock equal to 1 MHz */
  LL_TIM_SetPrescaler(TIMX, __LL_TIM_CALC_PSC(SystemCoreClock, 1000000));

  /* Set the auto-reload value to have a counter frequency of 100Hz */
  LL_TIM_SetAutoReload(TIMX, __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIMX), 100));

  /*********************************/
  /* Output waveform configuration */
  /*********************************/
  /* Set output compare mode: TOGGLE */
  LL_TIM_OC_SetMode(TIMX, TIMX_CHANNEL_CH1, LL_TIM_OCMODE_TOGGLE);
  LL_TIM_OC_SetMode(TIMX, TIMX_CHANNEL_CH2, LL_TIM_OCMODE_TOGGLE);
  LL_TIM_OC_SetMode(TIMX, TIMX_CHANNEL_CH3, LL_TIM_OCMODE_TOGGLE);
  LL_TIM_OC_SetMode(TIMX, TIMX_CHANNEL_CH4, LL_TIM_OCMODE_TOGGLE);

  /* Set output channel polarity: OC is active high */
  LL_TIM_OC_SetPolarity(TIMX, TIMX_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH);
  LL_TIM_OC_SetPolarity(TIMX, TIMX_CHANNEL_CH2, LL_TIM_OCPOLARITY_HIGH);
  LL_TIM_OC_SetPolarity(TIMX, TIMX_CHANNEL_CH3, LL_TIM_OCPOLARITY_HIGH);
  LL_TIM_OC_SetPolarity(TIMX, TIMX_CHANNEL_CH4, LL_TIM_OCPOLARITY_HIGH);

  /* Enable counter */
  LL_TIM_EnableCounter(TIMX);
}

void USBPD_TIM_Start(TIM_identifier id, uint16_t us_time)
{
  /* Positionne l'evenement pour sa detection */
  switch (id)
  {
    case TIM_PORT0_CRC:
      TIMX_CHANNEL1_SETEVENT;
      break;
    case TIM_PORT0_RETRY:
      TIMX_CHANNEL2_SETEVENT;
      break;
    case TIM_PORT1_CRC:
      TIMX_CHANNEL3_SETEVENT;
      break;
    case TIM_PORT1_RETRY:
      TIMX_CHANNEL4_SETEVENT;
      break;
    default:
      break;
  }
}

uint8_t USBPD_TIM_IsExpired(TIM_identifier id)
{
  switch (id)
  {
    case TIM_PORT0_CRC:
      return TIMX_CHANNEL1_GETFLAG(TIMX);
    case TIM_PORT0_RETRY:
      return TIMX_CHANNEL2_GETFLAG(TIMX);
    case TIM_PORT1_CRC:
      return TIMX_CHANNEL3_GETFLAG(TIMX);
    case TIM_PORT1_RETRY:
      return TIMX_CHANNEL4_GETFLAG(TIMX);
    default:
      break;
  }
  return 1;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

