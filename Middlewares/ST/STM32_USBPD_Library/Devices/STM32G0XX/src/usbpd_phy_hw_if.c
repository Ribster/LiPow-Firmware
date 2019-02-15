/**
  ******************************************************************************
  * @file    usbpd_phy_hw_if.c
  * @author  MCD Application Team
  * @brief   This file contains phy interface control functions.
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
#include "usbpd_hw.h"
#include "usbpd_core.h"
#include "usbpd_hw_if.h"
#include "usbpd_porthandle.h"
#include "usbpd_timersserver.h"

/* Private typedef -----------------------------------------------------------*/
/* Variable containing ADC conversions results */

/* Handle for the ports inside @ref UCPD-HW_IF*/
USBPD_PORT_HandleTypeDef Ports[USBPD_PORT_COUNT];

/* Private function prototypes -----------------------------------------------*/

/* Port related Init functions */

/* Optimized functions */
/* Init of decoding support variable */

/* Private functions ---------------------------------------------------------*/


void USBPD_HW_IF_GlobalHwInit(void)
{
  /* ## Backup register access ## */
  RCC->APBENR1 |= RCC_APBENR1_PWREN;
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);

  /* Init timer to detect the reception of goodCRC */
  USBPD_TIM_Init();

  /* Initialise VBUS power */
#if defined(USE_STM32G081B_EVAL_REVB) || defined(USE_STM32G081B_EVAL_REVC)
  BSP_PWR_VBUSInit(0, POWER_ROLE_DUAL);
  BSP_PWR_VBUSInit(1, POWER_ROLE_SINK);
#else
  // BSP_PWR_VBUSInit(USBPD_PORT_COUNT);
#endif
}


//void USBPD_HW_IF_PortHwInit(uint8_t PortNum, USBPD_HW_IF_Callbacks cbs, USBPD_PortPowerRole_TypeDef role, uint8_t *Ptr)
//{
//  Ports[PortNum].cbs = cbs;
//  Ports[PortNum].ptr_RxBuff = Ptr;
//}


void USBPD_HW_IF_StopBISTMode2(uint8_t PortNum)
{
  uint32_t  _cr = READ_REG(Ports[PortNum].husbpd->CR) & ~(UCPD_CR_TXMODE | UCPD_CR_TXSEND);

  LL_UCPD_Disable(Ports[PortNum].husbpd);
  LL_UCPD_Enable(Ports[PortNum].husbpd);

  Ports[PortNum].husbpd->CR = _cr;
}

USBPD_StatusTypeDef USBPD_HW_IF_SendBuffer(uint8_t PortNum, USBPD_SOPType_TypeDef Type, uint8_t *pBuffer, uint32_t Size)
{
  switch (Type)
  {
    case USBPD_SOPTYPE_HARD_RESET :
      LL_UCPD_SendHardReset(Ports[PortNum].husbpd);
      return USBPD_OK;
    case USBPD_SOPTYPE_SOP :
      LL_UCPD_WriteTxOrderSet(Ports[PortNum].husbpd, LL_UCPD_ORDERED_SET_SOP);
      LL_UCPD_SetTxMode(Ports[PortNum].husbpd, LL_UCPD_TXMODE_NORMAL);
      break;
    case USBPD_SOPTYPE_SOP1 :
      LL_UCPD_WriteTxOrderSet(Ports[PortNum].husbpd, LL_UCPD_ORDERED_SET_SOP1);
      LL_UCPD_SetTxMode(Ports[PortNum].husbpd, LL_UCPD_TXMODE_NORMAL);
      break;
    case USBPD_SOPTYPE_SOP2 :
      LL_UCPD_WriteTxOrderSet(Ports[PortNum].husbpd, LL_UCPD_ORDERED_SET_SOP2);
      LL_UCPD_SetTxMode(Ports[PortNum].husbpd, LL_UCPD_TXMODE_NORMAL);
      break;
    case USBPD_SOPTYPE_CABLE_RESET :
      LL_UCPD_SetTxMode(Ports[PortNum].husbpd, LL_UCPD_TXMODE_CABLE_RESET);
      break;
    case USBPD_SOPTYPE_BIST_MODE_2 :
      LL_UCPD_SetTxMode(Ports[PortNum].husbpd, LL_UCPD_TXMODE_BIST_CARRIER2);
      break;
    default :
      return USBPD_ERROR;
  }

  WRITE_REG(Ports[PortNum].hdmatx->CMAR, (uint32_t)pBuffer);
  WRITE_REG(Ports[PortNum].hdmatx->CNDTR, Size);
  Ports[PortNum].hdmatx->CCR |= DMA_CCR_EN;

  LL_UCPD_WriteTxPaySize(Ports[PortNum].husbpd, Size);
  LL_UCPD_SendMessage(Ports[PortNum].husbpd);
  return USBPD_OK;
}

void USBPD_HW_IF_Send_BIST_Pattern(uint8_t PortNum)
{
  LL_UCPD_SetTxMode(Ports[PortNum].husbpd, LL_UCPD_TXMODE_BIST_CARRIER2);
  LL_UCPD_SendMessage(Ports[PortNum].husbpd);
}

void USBPDM1_AssertRp(uint8_t PortNum)
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
//  RCC->APBENR2|= RCC_APBENR2_SYSCFGEN;
  LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_NONE);
  LL_UCPD_SetSRCRole(Ports[PortNum].husbpd);
  switch(Ports[PortNum].params->RpResistor)
  {
  case vRp_Default :
    LL_UCPD_SetRpResistor(Ports[PortNum].husbpd, LL_UCPD_RESISTOR_DEFAULT);
  break;
  case vRp_1_5A:
    LL_UCPD_SetRpResistor(Ports[PortNum].husbpd, LL_UCPD_RESISTOR_1_5A);
    break;
  case vRp_3_0A:
    LL_UCPD_SetRpResistor(Ports[PortNum].husbpd, LL_UCPD_RESISTOR_3_0A);
    break;
  }
  LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_CC1CC2);
  SET_BIT(SYSCFG->CFGR1,Ports[PortNum].husbpd==UCPD1?SYSCFG_CFGR1_UCPD1_STROBE:SYSCFG_CFGR1_UCPD2_STROBE);
}

void USBPDM1_DeAssertRp(uint8_t PortNum)
{
  /* not needed on G0, so nothing to do, keep only for compatibility */
}

void USBPDM1_AssertRd(uint8_t PortNum)
{
  LL_UCPD_TypeCDetectionCC2Disable(Ports[PortNum].husbpd);
  LL_UCPD_TypeCDetectionCC1Disable(Ports[PortNum].husbpd);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  //RCC->APBENR2|= RCC_APBENR2_SYSCFGEN;
  LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_NONE);
  LL_UCPD_SetSNKRole(Ports[PortNum].husbpd);
  LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_CC1CC2);

  HAL_Delay(1);
  SET_BIT(SYSCFG->CFGR1,Ports[PortNum].husbpd==UCPD1?SYSCFG_CFGR1_UCPD1_STROBE:SYSCFG_CFGR1_UCPD2_STROBE);
  HAL_Delay(1);

  LL_UCPD_TypeCDetectionCC2Enable(Ports[PortNum].husbpd);
  LL_UCPD_TypeCDetectionCC1Enable(Ports[PortNum].husbpd);

}

void USBPDM1_EnterErrorRecovery(uint8_t PortNum)
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
//  RCC->APBENR2|= RCC_APBENR2_SYSCFGEN;
//  LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_NONE);
  LL_UCPD_SetSRCRole(Ports[PortNum].husbpd);
  LL_UCPD_SetRpResistor(Ports[PortNum].husbpd, LL_UCPD_RESISTOR_NONE);
//  LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_CC1CC2);
  SET_BIT(SYSCFG->CFGR1,Ports[PortNum].husbpd==UCPD1?SYSCFG_CFGR1_UCPD1_STROBE:SYSCFG_CFGR1_UCPD2_STROBE);
}

void USBPDM1_DeAssertRd(uint8_t PortNum)
{
  /* not needed on G0, so nothing to do, keep only for compatibility */
}

void USBPDM1_Set_CC(uint8_t PortNum, CCxPin_TypeDef cc)
{
  /* Set the correct pin on the comparator*/
  Ports[PortNum].CCx = cc;
  LL_UCPD_SetCCPin(Ports[PortNum].husbpd , cc == CC1 ? LL_UCPD_CCPIN_CC1 : LL_UCPD_CCPIN_CC2);
}

void USBPDM1_RX_EnableInterrupt(uint8_t PortNum)
{
  /* Enable the RX interrupt process */
  MODIFY_REG(Ports[PortNum].husbpd->IMR, UCPD_IMR_RXORDDETIE | UCPD_IMR_RXHRSTDETIE | UCPD_IMR_RXOVRIE | UCPD_IMR_RXMSGENDIE,
                                         UCPD_IMR_RXORDDETIE | UCPD_IMR_RXHRSTDETIE | UCPD_IMR_RXOVRIE | UCPD_IMR_RXMSGENDIE);
  LL_UCPD_RxDMAEnable(Ports[PortNum].husbpd);
}

void USBPD_HW_IF_EnableRX(uint8_t PortNum)
{
  LL_UCPD_RxEnable(Ports[PortNum].husbpd);

}

void USBPD_HW_IF_DisableRX(uint8_t PortNum)
{
  LL_UCPD_RxDisable(Ports[PortNum].husbpd);
}

void HW_SignalAttachement(uint8_t PortNum, CCxPin_TypeDef cc)
{
  /* Prepare ucpd to handle PD message
            RX message start listen
            TX prepare the DMA to be transfer ready
            Detection listen only the line corresponding CC=Rd for SRC/SNK */
  Ports[PortNum].hdmatx = USBPD_HW_Init_DMATxInstance(PortNum);
  Ports[PortNum].hdmarx = USBPD_HW_Init_DMARxInstance(PortNum);

  /* Set the RX dma to allow reception */
  WRITE_REG(Ports[PortNum].hdmarx->CPAR, (uint32_t)&Ports[PortNum].husbpd->RXDR);
  WRITE_REG(Ports[PortNum].hdmarx->CMAR, (uint32_t)Ports[PortNum].ptr_RxBuff);
  Ports[PortNum].hdmarx->CNDTR = SIZE_MAX_PD_TRANSACTION_UNCHUNK;
  Ports[PortNum].hdmarx->CCR |= DMA_CCR_EN;

  /* Set the TX dma only UCPD address */
  Ports[PortNum].hdmatx->CPAR = (uint32_t)&Ports[PortNum].husbpd->TXDR;


  /* disabled non Rd line set CC line enable */
#define INTERRUPT_MASK  UCPD_IMR_TXMSGDISCIE | UCPD_IMR_TXMSGSENTIE | UCPD_IMR_HRSTDISCIE  | UCPD_IMR_HRSTSENTIE | UCPD_IMR_TXMSGABTIE |\
                        UCPD_IMR_TXUNDIE     | UCPD_IMR_RXORDDETIE  | UCPD_IMR_RXHRSTDETIE | UCPD_IMR_RXOVRIE    | UCPD_IMR_RXMSGENDIE

  MODIFY_REG(Ports[PortNum].husbpd->IMR, INTERRUPT_MASK, INTERRUPT_MASK);

  /* Handle CC enable */
  Ports[PortNum].CCx = cc;
  if (USBPD_PORTPOWERROLE_SRC == Ports[PortNum].params->PE_PowerRole)
  {
    LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_CC1CC2);
  }
  else
  {
    /* disconnect is done by VBUS */
    LL_UCPD_SetccEnable(Ports[PortNum].husbpd, Ports[PortNum].CCx == CC1 ? LL_UCPD_CCENABLE_CC1 : LL_UCPD_CCENABLE_CC2);
  }

  /* Set CC pin for PD message */
  LL_UCPD_SetCCPin(Ports[PortNum].husbpd, Ports[PortNum].CCx == CC1 ? LL_UCPD_CCPIN_CC1 : LL_UCPD_CCPIN_CC2);

#if defined(USBPD_REV30_SUPPORT)
  if( Ports[PortNum].settings->PE_PD3_Support.d.PE_FastRoleSwapSupport == USBPD_TRUE)
  {
    /* Set GPIO to allow the FRSTX handling */
    USBPD_HW_SetFRSSignalling(PortNum, Ports[PortNum].CCx == CC1 ? 1 : 2);
  }
#endif /* USBPD_REV30_SUPPORT */

#if defined(_VCONN_SUPPORT)
  /* Initialize Vconn managment */
  BSP_PWR_VCONNInit(PortNum, Ports[PortNum].CCx == CC1 ? 1 : 2);
#endif /* _VCONN_SUPPORT */
  /* Disable the Resistor on Vconn PIN */
  Ports[PortNum].CCx == CC1 ? LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_CC1): LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_CC2);

  /* Prepare the rx processing */
  LL_UCPD_SetRxMode(Ports[PortNum].husbpd, LL_UCPD_RXMODE_NORMAL);
  LL_UCPD_RxDMAEnable(Ports[PortNum].husbpd);
  LL_UCPD_TxDMAEnable(Ports[PortNum].husbpd);
  LL_UCPD_RxEnable(Ports[PortNum].husbpd);
}


void HW_SignalDetachment(uint8_t PortNum)
{
  LL_UCPD_RxDMADisable(Ports[PortNum].husbpd);
  LL_UCPD_TxDMADisable(Ports[PortNum].husbpd);
  LL_UCPD_RxDisable(Ports[PortNum].husbpd);

  /* Enable only detection interrupt */
  WRITE_REG(Ports[PortNum].husbpd->IMR, UCPD_IMR_TYPECEVT1IE | UCPD_IMR_TYPECEVT2IE);

  /* stop DMA RX/TX */
  LL_UCPD_RxDMADisable(Ports[PortNum].husbpd);
  LL_UCPD_TxDMADisable(Ports[PortNum].husbpd);

  USBPD_HW_DeInit_DMATxInstance(PortNum);
  USBPD_HW_DeInit_DMARxInstance(PortNum);

  LL_UCPD_SetccEnable(Ports[PortNum].husbpd, LL_UCPD_CCENABLE_CC1CC2);
}

void USBPD_HW_IF_SetResistor_SinkTxNG(uint8_t PortNum)
{
  /* set the resistor SinkTxNG 1.5A5V */
  LL_UCPD_SetRpResistor(Ports[PortNum].husbpd, LL_UCPD_RESISTOR_1_5A);
}

void USBPD_HW_IF_SetResistor_SinkTxOK(uint8_t PortNum)
{
  /* set the resistor SinkTxNG 3.0A5V */
  LL_UCPD_SetRpResistor(Ports[PortNum].husbpd, LL_UCPD_RESISTOR_3_0A);
}

uint8_t USBPD_HW_IF_IsResistor_SinkTxOk(uint8_t PortNum)
{
  switch(Ports[PortNum].CCx)
  {
    case CC1 :
      if (Ports[PortNum].CC1 == LL_UCPD_SNK_CC1_VRP30A)
      {
        return USBPD_TRUE;
      }
      break;
    case CC2 :
      if (Ports[PortNum].CC2 == LL_UCPD_SNK_CC2_VRP30A)
      {
        return USBPD_TRUE;
      }
      break;
    default:
      break;
  }

  return USBPD_FALSE;
}

void USBPD_HW_IF_FastRoleSwapSignalling(uint8_t PortNum)
{
  LL_UCPD_SignalFRSTX(Ports[PortNum].husbpd);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
