/**
  ******************************************************************************
  * @file    usbpd_phy.h
  * @author  MCD Application Team
  * @brief   This file contains the headers of usbpd_phy.h.
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

#ifndef __USBPD_PHY_H_
#define __USBPD_PHY_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STM32_USBPD_LIBRARY
  * @{
  */

/** @addtogroup USBPD_DEVICE
  * @{
  */

/** @addtogroup USBPD_DEVICE_PHY
  * @{
  */

/* Exported typedef ----------------------------------------------------------*/
/** @defgroup USBPD_DEVICE_PHY_Exported_TypeDef USBPD DEVICE PHY Exported TypeDef
  * @{
  */
/**
  * @brief CallBacks exposed by the @ref USBPD_DEVICE_PHY to the USBPD_CORE_PRL
  */
typedef struct
{
  /**
   * @brief  Reports that a message has been received on a specified port.
   * @param  PortNum:    The handle of the port
   * @param  Type:    The type of the message received
   * @retval None
   * @note Received data are stored inside PortNum->pRxBuffPtr
   */
  void (*USBPD_PHY_MessageReceived)(uint8_t PortNum, USBPD_SOPType_TypeDef Type);

  /**
   * @brief  Reports to the PRL that a Reset received from channel.
   * @param  PortNum:    The handle of the port
   * @param  Type:    The type of reset performed
   * @retval None
   */
  void (*USBPD_PHY_ResetIndication)(uint8_t PortNum, USBPD_SOPType_TypeDef Type);

  /**
   * @brief  Reports to the PRL that a Reset operation has been completed.
   * @param  PortNum:    The handle of the port
   * @param  Type:    The type of reset performed
   * @retval None
   */
  void (*USBPD_PHY_ResetCompleted)(uint8_t PortNum, USBPD_SOPType_TypeDef Type);

  /**
   * @brief  Reports the PHY layer discarded last message to sent because the bus is not idle.
   * @param  PortNum:    The handle of the port
   * @retval None
   * @note See Section 5.7 of USB Power Delivery specification Rev2, V1.1
   */
  void (*USBPD_PHY_ChannelIdleAfterBusy)(uint8_t PortNum);

  /**
   * @brief  Reports to the PRL that a Bist operation has been completed.
   * @param  PortNum:    The handle of the port
   * @param  Type:    The type of Bist performed
   * @retval None
   */
  void (*USBPD_PHY_BistCompleted)(uint8_t PortNum, USBPD_BISTMsg_TypeDef bistmode);

  /**
    * @brief  Reports to the PRL that a tx operation has been completed.
    * @param  PortNum:    The handle of the port
    * @retval None
    */
  void (*USBPD_PHY_TxCompleted)(uint8_t PortNum);

} USBPD_PHY_Callbacks;

/**
  * @}
  */

/* Exported define -----------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup USBPD_DEVICE_PHY_Exported_Functions
  * @{
  */
USBPD_StatusTypeDef USBPD_PHY_Init(uint8_t PortNum, USBPD_PHY_Callbacks *cbs, uint8_t *pRxBuffer, USBPD_PortPowerRole_TypeDef role, uint32_t SupportedSOP);
void                USBPD_PHY_Reset(uint8_t PortNum);
uint32_t            USBPD_PHY_GetRetryTimerValue(uint8_t PortNum);

USBPD_StatusTypeDef USBPD_PHY_ResetRequest(uint8_t PortNum, USBPD_SOPType_TypeDef Type);
USBPD_StatusTypeDef USBPD_PHY_SendMessage(uint8_t PortNum, USBPD_SOPType_TypeDef Type, uint8_t *pBuffer, uint16_t Size);
USBPD_StatusTypeDef USBPD_PHY_Send_BIST_Pattern(uint8_t PortNum);
USBPD_StatusTypeDef USBPD_PHY_ExitTransmit(uint8_t PortNum, USBPD_SOPType_TypeDef BistType);
void                USBPD_PHY_SetResistor_SinkTxNG(uint8_t PortNum);
void                USBPD_PHY_SetResistor_SinkTxOK(uint8_t PortNum);
uint8_t             USBPD_PHY_IsResistor_SinkTxOk(uint8_t PortNum);
void                USBPD_PHY_FastRoleSwapSignalling(uint8_t PortNum);
void                USBPD_PHY_SOPSupported(uint8_t PortNum,uint32_t SOPSupported);

void                USBPD_PHY_EnableRX(uint8_t PortNum);
void                USBPD_PHY_DisableRX(uint8_t PortNum);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBPD_PHY_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

