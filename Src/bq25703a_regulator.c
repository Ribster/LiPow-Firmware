/**
 ******************************************************************************
 * @file           : bq25703a_regulator.c
 * @brief          : Handles battery state information
 ******************************************************************************
 */


#include "bq25703a_regulator.h"
#include "battery.h"
#include "error.h"
#include "main.h"
#include "string.h"

extern I2C_HandleTypeDef hi2c1;

/* Private typedef -----------------------------------------------------------*/
struct Regulator {
	uint8_t connected;
	uint8_t status;
	uint32_t input_current;
	uint32_t output_current;
	uint16_t max_charge_voltage;
	uint8_t input_current_limit;
	uint16_t min_input_voltage_limit;
};

/* Private variables ---------------------------------------------------------*/
struct Regulator regulator;

/* The maximum time to wait for the mutex that guards the UART to become
 available. */
#define cmdMAX_MUTEX_WAIT	pdMS_TO_TICKS( 300 )

/* Private function prototypes -----------------------------------------------*/
void I2C_Transfer(uint8_t *pData, uint16_t size);
void I2C_Receive(uint8_t *pData, uint16_t size);
uint8_t Query_Regulator_Connection(void);
uint8_t Read_Charge_Okay(void);
void Regulator_HI_Z(uint8_t hi_z_en);
void Regulator_OTG_EN(uint8_t otg_en);
void Set_Charge_Voltage(uint8_t number_of_cells);
void I2C_Write_Register(uint8_t addr_to_write, uint8_t *pData);
void I2C_Read_Register(uint8_t addr_to_read, uint8_t *pData);

/**
 * @brief Returns whether the regulator is connected over I2C
 * @retval uint8_t CONNECTED or NOT_CONNECTED
 */
uint8_t Get_Regulator_Connection_State() {
	return regulator.connected;
}

/**
 * @brief Performs an I2C transfer
 * @param pData Pointer to location of data to transfer
 * @param size Size of data to be transferred
 */
void I2C_Transfer(uint8_t *pData, uint16_t size) {

	if ( xSemaphoreTake( xTxMutex_Regulator, cmdMAX_MUTEX_WAIT ) == pdPASS) {
		do
		{
			TickType_t xtimeout_start = xTaskGetTickCount();
			while (HAL_I2C_Master_Transmit_DMA(&hi2c1, (uint16_t)BQ26703A_I2C_ADDRESS, pData, size) != HAL_OK) {
				if (((xTaskGetTickCount()-xtimeout_start)/portTICK_PERIOD_MS) > I2C_TIMEOUT) {
					Set_Error_State(REGULATOR_COMMUNICATION_ERROR);
					break;
				}
			}
		    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
				if (((xTaskGetTickCount()-xtimeout_start)/portTICK_PERIOD_MS) > I2C_TIMEOUT) {
					Set_Error_State(REGULATOR_COMMUNICATION_ERROR);
					break;
				}
		    }
		}
		while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);
		xSemaphoreGive(xTxMutex_Regulator);
	}
}

/**
 * @brief Performs an I2C transfer
 * @param pData Pointer to location to store received data
 * @param size Size of data to be received
 */
void I2C_Receive(uint8_t *pData, uint16_t size) {
	if ( xSemaphoreTake( xTxMutex_Regulator, cmdMAX_MUTEX_WAIT ) == pdPASS) {
		do
		{
			TickType_t xtimeout_start = xTaskGetTickCount();
			while (HAL_I2C_Master_Receive_DMA(&hi2c1, (uint16_t)BQ26703A_I2C_ADDRESS, pData, size) != HAL_OK) {
				if (((xTaskGetTickCount()-xtimeout_start)/portTICK_PERIOD_MS) > I2C_TIMEOUT) {
					Set_Error_State(REGULATOR_COMMUNICATION_ERROR);
					break;
				}
			}
			while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {
				if (((xTaskGetTickCount()-xtimeout_start)/portTICK_PERIOD_MS) > I2C_TIMEOUT) {
					Set_Error_State(REGULATOR_COMMUNICATION_ERROR);
					break;
				}
			}
		}
		while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);
		xSemaphoreGive(xTxMutex_Regulator);
	}
}

/**
 * @brief Checks if the regulator is connected over I2C
 * @retval uint8_t CONNECTED or NOT_CONNECTED
 */
uint8_t Query_Regulator_Connection() {
	/* Get the manufacturer id */
	uint8_t manufacturer_id;
	I2C_Read_Register(MANUFACTURER_ID_ADDR, (uint8_t *) &manufacturer_id);

	/* Get the device id */
	uint8_t device_id;
	I2C_Read_Register(DEVICE_ID_ADDR, (uint8_t *) &device_id);

	if ( (device_id == BQ26703A_DEVICE_ID) && (manufacturer_id == BQ26703A_MANUFACTURER_ID) ) {
		Clear_Error_State(REGULATOR_COMMUNICATION_ERROR);
		return CONNECTED;
	}
	else {
		Set_Error_State(REGULATOR_COMMUNICATION_ERROR);
		return NOT_CONNECTED;
	}
}

/**
 * @brief Checks the state of the Charge okay pin and returns the value
 * @retval 0 if VBUS falls below 3.2 V or rises above 26 V, 1 if VBUS is between 3.5V and 24.5V
 */
uint8_t Read_Charge_Okay() {
	return HAL_GPIO_ReadPin(CHRG_OK_GPIO_Port, CHRG_OK_Pin);
}

/**
 * @brief Enables or disables high impedance mode on the output of the regulator
 * @param hi_z_en 0 puts the output of the regulator in hiz mode. 1 takes the regulator out of hi_z and allows charging
 */
void Regulator_HI_Z(uint8_t hi_z_en) {
	if (hi_z_en == 0) {
		HAL_GPIO_WritePin(ILIM_HIZ_GPIO_Port, ILIM_HIZ_Pin, GPIO_PIN_RESET);
	}
	else {
		HAL_GPIO_WritePin(ILIM_HIZ_GPIO_Port, ILIM_HIZ_Pin, GPIO_PIN_SET);
	}
}

/**
 * @brief Enables or disables On the Go Mode
 * @param otg_en 0 disables On the GO Mode. 1 Enables.
 */
void Regulator_OTG_EN(uint8_t otg_en) {
	if (otg_en == 0) {
		HAL_GPIO_WritePin(EN_OTG_GPIO_Port, EN_OTG_Pin, GPIO_PIN_RESET);
	}
	else {
		HAL_GPIO_WritePin(EN_OTG_GPIO_Port, EN_OTG_Pin, GPIO_PIN_SET);
	}
}

/**
 * @brief Sets the charging voltage based on the number of cells. 1 - 4.192V, 2 - 8.400V, 3 - 12.592V, 4 - 16.800V
 * @param number_of_cells number of cells connected
 */
void Set_Charge_Voltage(uint8_t number_of_cells) {

	uint8_t max_charge_register_1_value = 0;
	uint8_t max_charge_register_2_value = 0;

	if ((number_of_cells > 4) || (number_of_cells < 1)) {
		max_charge_register_1_value = 0;
		max_charge_register_2_value = 0;
		I2C_Write_Register(MAX_CHARGE_VOLTAGE_ADDR_1, (uint8_t *) &max_charge_register_1_value);
		I2C_Write_Register(MAX_CHARGE_VOLTAGE_ADDR_2, (uint8_t *) &max_charge_register_2_value);
		return;
	}

	switch (number_of_cells) {
	case 1:
		max_charge_register_1_value = 0b00010000;
		max_charge_register_2_value = 0b01100000;
		break;
	case 2:
		max_charge_register_1_value = 0b00100000;
		max_charge_register_2_value = 0b11010000;
		break;
	case 3:
		max_charge_register_1_value = 0b00110001;
		max_charge_register_2_value = 0b00110000;
		break;
	case 4:
		max_charge_register_1_value = 0b01000001;
		max_charge_register_2_value = 0b10100000;
		break;
	}

	I2C_Write_Register(MAX_CHARGE_VOLTAGE_ADDR_1, (uint8_t *) &max_charge_register_1_value);
	I2C_Write_Register(MAX_CHARGE_VOLTAGE_ADDR_2, (uint8_t *) &max_charge_register_2_value);
}

/**
 * @brief Automatically performs two I2C writes to write a register on the regulator
 * @param pData Pointer to data to be transferred
 */
void I2C_Write_Register(uint8_t addr_to_write, uint8_t *pData) {
		I2C_Transfer((uint8_t *)&addr_to_write, 1);
		I2C_Transfer(pData, 1);
}

/**
 * @brief Automatically performs one I2C write and an I2C read to get the value of a register
 * @param pData Pointer to where to store data
 */
void I2C_Read_Register(uint8_t addr_to_read, uint8_t *pData) {
		I2C_Transfer((uint8_t *)&addr_to_read, 1);
		I2C_Receive(pData, 1);
}

/**
 * @brief Main regulator task
 */
void vRegulator(void const *pvParameters) {

	TickType_t xDelay = 500 / portTICK_PERIOD_MS;

	/* Disable the output of the regulator for safety */
	Regulator_HI_Z(0);

	/* Disable OTG mode */
	Regulator_OTG_EN(0);

	/* Check if the regulator is connected */
	regulator.connected = Query_Regulator_Connection();

	//Set_Charge_Voltage(1);
	//Regulator_HI_Z(1);

	for (;;) {

		if (Read_Charge_Okay() != 1) {
			Set_Error_State(VOLTAGE_INPUT_ERROR);
		}
		else if ((Get_Error_State() & VOLTAGE_INPUT_ERROR) == VOLTAGE_INPUT_ERROR) {
			Clear_Error_State(VOLTAGE_INPUT_ERROR);
		}

		if ((Get_Error_State() & REGULATOR_COMMUNICATION_ERROR) == REGULATOR_COMMUNICATION_ERROR) {
			regulator.connected = 0;
		}


		vTaskDelay(xDelay);
	}
}
