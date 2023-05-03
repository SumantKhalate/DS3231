/* An STM32 HAL library written for the DS3231 real-time clock IC. */
/* Library by Sumant Khalate aka pi_filter | github.com/SumantKhalate */
/* Date 03/05/2003 */

#include "DS3231.h"
#include "main.h"

#ifdef __cplusplus
extern "C"{
#endif

static const uint8_t days_in_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const uint8_t dow[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

static I2C_HandleTypeDef *DS3231_device;

/**
 * @brief Initializes the DS3231 module.
 * @details Stores the i2cHandle in #DS3231_device variable for further I2C communication.\n
 * 			<!-- Set the clock halt bit(EOSC) to 0 to start the clock.\n -->
 * 			Disable both the Alarm 1 (A1IE) and Alarm 2 (A2IE) interrupts\n
 * 			<!-- Set Interrupt pin function (INTCN) as alarm interrupt.\n -->
 * 			Clear both the Alarm 1 flag (A1F) and Alarm 2 flag (A2F)\n
 * 			Disable the battery backed square wave (BBSQW) option..
 * @param[in] *i2cHandle Pass the I2C handle pointer.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note Calling this function will change the interrupt pin function (INTCN) to alarm interrupt mode.
 */
HAL_StatusTypeDef DS3231_Init(I2C_HandleTypeDef *i2cHandle) {
	DS3231_device = i2cHandle;
	HAL_StatusTypeDef status;
	status = DS3231_SetAlarm1IntEn(DS3231_DISABLED);
	if (status != HAL_OK)
		return status;
	status = DS3231_SetAlarm2IntEn(DS3231_DISABLED);
	if (status != HAL_OK)
		return status;
	status = DS3231_ClearAlarm1Flag();
	if (status != HAL_OK)
		return status;
	status = DS3231_ClearAlarm2Flag();
	if (status != HAL_OK)
		return status;
	return DS3231_Set32kHzOutput(DS3231_DISABLED);
}

/**
 * @brief Set the battery-backed square wave output mode (BBSQW) for the INT#/SQW pin.
 * @param[in] enable #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_SetBatterySquareWave(DS3231_State enable) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	control = (control & 0xBF) | ((enable & 0x01) << DS3231_BBSQW);
	return DS3231_WriteRegister(DS3231_REG_CONTROL, &control);
}

/**
 * @brief Get the battery-backed square wave output mode (BBSQW) for the INT#/SQW pin.
 * @param[out] *enable Pass a pointer to #DS3231_State type variable to get the state, #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_GetBatterySquareWave(DS3231_State *enable) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	*enable = (control >> DS3231_BBSQW) & 0x01;
	return status;
}

/**
 * @brief Sets the Oscillator (EOSC).
 * @param[in] enable #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_SetOscillator(DS3231_State enable) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	control = (control & 0x7F) | ((!enable & 0x01) << DS3231_EOSC);
	return DS3231_WriteRegister(DS3231_REG_CONTROL, &control);
}

/**
 * @brief Check whether the clock oscillator (OSF) is stopped.
 * @param[out] *enable Pass a pointer to #DS3231_State type variable to get the state, #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_GetOscillatorStoppedFlag(DS3231_State *enable) {
	uint8_t data;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_STATUS, &data);
	*enable = !(data >> DS3231_OSF) & 0x01;
	return status;
}

/**
 * @brief Enable the 32kHz output (EN32kHz) on the 32kHz pin.
 * @param[in] enable #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_Set32kHzOutput(DS3231_State enable) {
	uint8_t temp;
	HAL_StatusTypeDef status;

	status = DS3231_ReadRegister(DS3231_REG_STATUS, &temp);
	if (status != HAL_OK)
		return status;
	temp &= 0xF7;
	temp |= (enable << DS3231_EN32KHZ);
	return DS3231_WriteRegister(DS3231_REG_STATUS, &temp);
}

/**
 * @brief Check whether the 32kHz output (EN32kHz) is enabled.
 * @param[out] *enable Pass a pointer to #DS3231_State type variable to get the state, #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_Get32kHzEnabled(DS3231_State *enable) {
	uint8_t data;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_STATUS, &data);
	*enable = (data >> DS3231_EN32KHZ) & 0x01;
	return status;
}

/**
 * @brief Set the interrupt mode (INTCN) to either alarm interrupt or square wave interrupt.
 * @param[in] mode #DS3231_ALARM_INTERRUPT or #DS3231_SQUARE_WAVE_INTERRUPT.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_SetInterruptMode(DS3231_InterruptMode mode) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	control = (control & 0xFB) | ((mode & 0x01) << DS3231_INTCN);
	return DS3231_WriteRegister(DS3231_REG_CONTROL, &control);
}

/**
 * @brief Get the interrupt mode (INTCN) to either alarm interrupt or square wave interrupt.
 * @param[out] *mode Pass a pointer to #DS3231_InterruptMode type variable, #DS3231_ALARM_INTERRUPT or #DS3231_SQUARE_WAVE_INTERRUPT.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_GetInterruptMode(DS3231_InterruptMode *mode) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	*mode = (control >> DS3231_INTCN) & 0x01;
	return status;
}

/**
 * @brief Set frequency of the square wave output (RS2 RS1).
 * @param[in] rate Frequency to set, DS3231_1HZ, DS3231_1024HZ, DS3231_4096HZ or DS3231_8192HZ.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note Calling this function will change the interrupt pin function (INTCN) to square wave output mode.
 */
HAL_StatusTypeDef DS3231_SetRateSelect(DS3231_Rate rate) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	control = (control & 0xE7) | ((rate & 0x03) << DS3231_RS1);
	status =  DS3231_WriteRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	return DS3231_SetInterruptMode(DS3231_ALARM_INTERRUPT);
}

/**
 * @brief Get frequency of the square wave output (RS2 RS1).
 * @param[out] *rate Pass a pointer to #DS3231_Rate type variable, DS3231_1HZ, DS3231_1024HZ, DS3231_4096HZ or DS3231_8192HZ.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_GetRateSelect(DS3231_Rate *rate) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	*rate = (control >> DS3231_RS1) & 0x03;
	return status;
}

/**
 * @brief Get temperature.
 * @param[out] *temp_real Pass a pointer to float type variable.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_GetTemperature(float *temp_real) {
	HAL_StatusTypeDef status;
	uint8_t buffer[2];

	status = DS3231_ReadRegisters(DS3231_REG_TEMP_MSB, buffer, 2);
	if (status != HAL_OK)
		return status;

	*temp_real = (buffer[0] + (buffer[1] >> 6) * 0.25f);

	return status;
}

/**
 * @brief Sets configuration of alarm 1 sub-module.
 * @details Set alarm 1 registers like Seconds, Minutes, Hour and Day_Date.\n
 * 			Set alarm 1 bits like A1M1, A1M2, A1M3, A1M4, DY/DT.\n
 * 			Set alarm 1 interrupt enable bit.
 * @param[in] *A1_st Pass a pointer to a #D3231_Alarm1 structure.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note Calling this function will change the interrupt pin function (INTCN) to alarm interrupt mode.
 */
HAL_StatusTypeDef DS3231_SetAlarm1(D3231_Alarm1 *A1_st) {
	HAL_StatusTypeDef status;
	uint8_t A1M1 = (A1_st->Mode & 0x01) << 7; // Seconds bit 7.
	uint8_t A1M2 = (A1_st->Mode & 0x02) << 6; // Minutes bit 7.
	uint8_t A1M3 = (A1_st->Mode & 0x04) << 5; // Hour bit 7.
	uint8_t A1M4 = (A1_st->Mode & 0x08) << 4; // Day/Date bit 7.
	uint8_t DY_DT = (A1_st->Mode & 0x10) << 2; // Day/Date bit 6. Date when 0, day of week when 1.

	uint8_t data[4] = {
			DS3231_EncodeBCD(A1_st->Seconds) | A1M1,
			DS3231_EncodeBCD(A1_st->Minutes) | A1M2,
			DS3231_EncodeBCD(A1_st->Hours)	| A1M3,
			DS3231_EncodeBCD(A1_st->DayDate) | DY_DT| A1M4,
	};

	status = DS3231_WriteRegisters(DS3231_REG_A1_SECOND, data, 4);
	if (status != HAL_OK)
			return status;
	return DS3231_SetAlarm1IntEn(A1_st->IntEn);
}

/**
 * @brief Read configuration of alarm 1 sub-module.
 * @details Get alarm 1 registers like Seconds, Minutes, Hour and Day_Date.\n
 * 			Get alarm 1 bits like A1M1, A1M2, A1M3, A1M4, DY/DT.\n
 * 			Get alarm 1 interrupt enable bit (A1IE).
 * @param[out] *A1_st Pass a pointer to a #D3231_Alarm1 structure.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note Based on DY_DT bit, it will only read the lower 4 or lower 6 bits of alarm 1 day_date register.
 */
HAL_StatusTypeDef DS3231_GetAlarm1(D3231_Alarm1 *A1_st) {
	HAL_StatusTypeDef status;
	uint8_t data[4];
	status = DS3231_ReadRegisters(DS3231_REG_A1_SECOND, data, 4);
	if (status != HAL_OK)
		return status;

	uint8_t Mode = (data[0] & 0x80) >> 7	// A1M1
				 | (data[1] & 0x80) >> 6	// A1M2
				 | (data[2]& 0x80) >> 5		// A1M3
				 | (data[3]& 0x80) >> 4		// A1M4
				 | (data[3] & 0x40) >> 2;	// DY_DT
	A1_st->Mode = Mode;

	A1_st->Seconds = DS3231_DecodeBCD(data[0] & 0x7F);
	A1_st->Minutes = DS3231_DecodeBCD(data[1] & 0x7F);
	A1_st->Hours = DS3231_DecodeBCD(data[2] & 0x3F);

	uint8_t DayDate =  (data[3] & 0x40) >> 6;
	if (DayDate)
		A1_st->DayDate = DS3231_DecodeBCD(data[3] & 0x0F);
	else
		A1_st->DayDate = DS3231_DecodeBCD(data[3] & 0x3F);

	return DS3231_GetAlarm1IntEn(&A1_st->IntEn);
}

/**
 * @brief Set alarm 1 interrupt enable bit (A1IE).
 * @param[in] enable #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note Calling this function will change the interrupt pin function (INTCN) to alarm interrupt mode.
*/
HAL_StatusTypeDef DS3231_SetAlarm1IntEn(DS3231_State enable) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	control = (control & 0xFE) | ((enable & 0x01) << DS3231_A1IE);
	DS3231_WriteRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	return DS3231_SetInterruptMode(DS3231_ALARM_INTERRUPT);
}

/**
 * @brief Get alarm 1 interrupt enable bit (A1IE).
 * @param[out] *enable Pass a pointer to #DS3231_State type variable to get the state, #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
*/
HAL_StatusTypeDef DS3231_GetAlarm1IntEn(DS3231_State *enable) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	*enable = (control >> DS3231_A1IE ) & 0x01;
	return status;
}

/**
 * @brief Check if alarm 1 (A1F) is triggered.
 * @param[out] *enable Pass a pointer to #DS3231_State type variable to get the state, #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note If the alarm flag is triggered and alarm interrupt mode is enabled along with interrupt mode set to alarm then
 * the INT#/SQW pin will be asserted low until alarm flag is manually cleared using #DS3231_ClearAlarm1Flag function.
 */
HAL_StatusTypeDef DS3231_GetAlarm1Flag(DS3231_State *enable) {
	uint8_t data;
	HAL_StatusTypeDef status;

	status = DS3231_ReadRegister(DS3231_REG_STATUS, &data);
	*enable = (data >> DS3231_A1F) & 0x01;
	return status;
}

/**
 * @brief Clears alarm 1 flag (A1F).
 * @param void
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note If the alarm flag is triggered and alarm interrupt mode is enabled along with interrupt mode set to alarm then
 * the INT#/SQW pin will be asserted low until alarm flag is manually cleared using #DS3231_ClearAlarm1Flag function.
 */
HAL_StatusTypeDef DS3231_ClearAlarm1Flag(void) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_STATUS, &control);
	if (status != HAL_OK)
		return status;
	control &= ~(0x01 << DS3231_A1F);
	return DS3231_WriteRegister(DS3231_REG_STATUS, &control);
}

/**
 * @brief Sets configuration of alarm 2 sub-module.
 * @details Set alarm 2 registers like Minutes, Hour and Day_Date.\n
 * 			Set alarm 2 bits like A1M2, A1M3, A1M4, DY/DT.\n
 * 			Set alarm 2 interrupt enable bit.
 * @param[in] *A2_st Pass a pointer to a #D3231_Alarm2 structure.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note Calling this function will change the interrupt pin function (INTCN) to alarm interrupt mode.
 */
HAL_StatusTypeDef DS3231_SetAlarm2(D3231_Alarm2 *A2_st) {
	HAL_StatusTypeDef status;
	uint8_t A2M2 = (A2_st->Mode & 0x01) << 7; // Minutes bit 7.
	uint8_t A2M3 = (A2_st->Mode & 0x02) << 6; // Hour bit 7.
	uint8_t A2M4 = (A2_st->Mode & 0x04) << 5; // Day/Date bit 7.
	uint8_t DY_DT = (A2_st->Mode & 0x08) << 3; // Day/Date bit 6. Date when 0, day of week when 1.

	uint8_t data[3] = {
			DS3231_EncodeBCD(A2_st->Minutes) | A2M2,
			DS3231_EncodeBCD(A2_st->Hours)	| A2M3,
			DS3231_EncodeBCD(A2_st->DayDate) | DY_DT| A2M4,
	};

	status = DS3231_WriteRegisters(DS3231_REG_A2_MINUTE, data, 3);
	if (status != HAL_OK)
			return status;
	return DS3231_SetAlarm2IntEn(A2_st->IntEn);
}

/**
 * @brief Read configuration of alarm 2 sub-module.
 * @details Get alarm 2 registers like Minutes, Hour and Day_Date.\n
 * 			Get alarm 2 bits like A2M2, A2M3, A2M4, DY/DT.\n
 * 			Get alarm 2 interrupt enable bit (A2IE).
 * @param[out] *A2_st Pass a pointer to a #D3231_Alarm2 structure.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note Based on DY_DT bit, it will only read the lower 4 or lower 6 bits of alarm 2 day_date register.
 */
HAL_StatusTypeDef DS3231_GetAlarm2(D3231_Alarm2 *A2_st) {
	HAL_StatusTypeDef status;
	uint8_t data[3];
	status = DS3231_ReadRegisters(DS3231_REG_A2_MINUTE, data, 3);
	if (status != HAL_OK)
		return status;

	uint8_t Mode = (data[0] & 0x80) >> 7 	// A2M2
				 | (data[1] & 0x80) >> 6	// A2M3
				 | (data[2]& 0x80) >> 5		// A2M4
				 | (data[2] & 0x40) >> 3;	// DY_DT

	A2_st->Mode = Mode;

	A2_st->Minutes = DS3231_DecodeBCD(data[0] & 0x7F);
	A2_st->Hours = DS3231_DecodeBCD(data[1] & 0x7F);

	uint8_t DayDate =  (data[2] & 0x40) >> 6;
	if (DayDate)
		A2_st->DayDate = DS3231_DecodeBCD(data[2] & 0x0F);
	else
		A2_st->DayDate = DS3231_DecodeBCD(data[2] & 0x3F);

	return DS3231_GetAlarm2IntEn(&A2_st->IntEn);
}

/**
 * @brief Set alarm 2 interrupt enable bit (A2IE).
 * @param[in] enable #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note Calling this function will change the interrupt pin function (INTCN) to alarm interrupt mode.
*/
HAL_StatusTypeDef DS3231_SetAlarm2IntEn(DS3231_State enable) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	control = (control & 0xFD) | ((enable & 0x01) << DS3231_A2IE);
	status = DS3231_WriteRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	return DS3231_SetInterruptMode(DS3231_ALARM_INTERRUPT);
}

/**
 * @brief Get alarm 2 interrupt enable bit (A2IE).
 * @param[out] *enable Pass a pointer to #DS3231_State type variable to get the state, #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
*/
HAL_StatusTypeDef DS3231_GetAlarm2IntEn(DS3231_State *enable) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &control);
	if (status != HAL_OK)
		return status;
	*enable = (control >> DS3231_A2IE) & 0x01;
	return status;
}

/**
 * @brief Check if alarm 2 (A2F) is triggered.
 * @param[out] *enable Pass a pointer to #DS3231_State type variable to get the state, #DS3231_DISABLED or #DS3231_ENABLED
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note If the alarm flag is triggered and alarm interrupt mode is enabled along with interrupt mode set to alarm then
 * the INT#/SQW pin will be asserted low until alarm flag is manually cleared using #DS3231_ClearAlarm2Flag function.
 */
HAL_StatusTypeDef DS3231_GetAlarm2Flag(DS3231_State *enable) {
	uint8_t data;
	HAL_StatusTypeDef status;

	status = DS3231_ReadRegister(DS3231_REG_STATUS, &data);
	*enable = (data >> DS3231_A2F) & 0x01;
	return status;
}

/**
 * @brief Clears alarm 2 flag (A2F).
 * @param void
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note If the alarm flag is triggered and alarm interrupt mode is enabled along with interrupt mode set to alarm then
 * the INT#/SQW pin will be asserted low until alarm flag is manually cleared using #DS3231_ClearAlarm2Flag function.
 */
HAL_StatusTypeDef DS3231_ClearAlarm2Flag(void) {
	uint8_t control;
	HAL_StatusTypeDef status;
	status = DS3231_ReadRegister(DS3231_REG_STATUS, &control);
	if (status != HAL_OK)
		return status;
	control &= ~(0x01 << DS3231_A2F);
	return DS3231_WriteRegister(DS3231_REG_STATUS, &control);
}

/**
 * @brief Sets the current date and time of RTC and also the enable oscillator (EOSC).
 * @param[in] *dt Pass a pointer to #DS3231_DateTime type variable to set the current date, time and enable oscillator (EOSC) bit.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note It sets the enable oscillator (EOSC) bit based on the Enable member of #DS3231_DateTime structure variable.\n
 * It only support 24H mode.
 */
HAL_StatusTypeDef DS3231_SetDateTime(DS3231_DateTime *dt) {
	HAL_StatusTypeDef status;
	uint8_t buffer[7];

	if ((dt->Day >= 1) | (dt->Day <= 7))
		buffer[3] = DS3231_EncodeBCD(dt->Day);
	else
		return HAL_ERROR;

	if ((dt->Date >= 1) | (dt->Date <= 31))
		buffer[4] = DS3231_EncodeBCD(dt->Date);
	else
		return HAL_ERROR;

	if ((dt->Month >= 1) | (dt->Month <= 12))			// Century bit implementation is pending
			buffer[5] = DS3231_EncodeBCD(dt->Month);
	else
		return HAL_ERROR;

	if ((dt->Year >= 0) | (dt->Year <= 99))
			buffer[6] = DS3231_EncodeBCD(dt->Year - 2000U);
	else
		return HAL_ERROR;

	if ((dt->Hour_24mode >= 0) | (dt->Hour_24mode <= 23))		// Only 24HR mode is supported
			buffer[2] = DS3231_EncodeBCD(dt->Hour_24mode);
	else
		return HAL_ERROR;

	if ((dt->Minute >= 0) | (dt->Minute <= 59))
			buffer[1] = DS3231_EncodeBCD(dt->Minute);
	else
		return HAL_ERROR;

	if ((dt->Second >= 0) | (dt->Second <= 59))
			buffer[0] = DS3231_EncodeBCD(dt->Second);
	else
		return HAL_ERROR;

	status  = DS3231_WriteRegisters(DS3231_REG_SECOND, buffer, 7);
	if (status != HAL_OK)
		return status;

	uint8_t regCONTROL;
	status = DS3231_ReadRegister(DS3231_REG_CONTROL, &regCONTROL);
	if (status != HAL_OK)
		return status;

	if (dt->Enable == DS3231_ENABLED)
		regCONTROL &= ~(0x80);
	else
		regCONTROL |= (0x80);

	return DS3231_WriteRegister(DS3231_REG_CONTROL, &regCONTROL);
}

/**
 * @brief Reads the current date and time from RTC and also the state of oscillator stop flag (OSF).
 * @param[out] *dt Pass a pointer to #DS3231_DateTime type variable to get the current date, time and oscillator stop flag (OSF).
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 * @note It reads the oscillator stop flag (OSF) bit into the Enable member of #DS3231_DateTime structure variable.\n
 * It only support 24H mode.
 */
HAL_StatusTypeDef DS3231_GetDateTime(DS3231_DateTime *dt) {
	HAL_StatusTypeDef status;
	uint8_t buffer[7];

	status = DS3231_ReadRegisters(DS3231_REG_SECOND, buffer, 7);
	if (status != HAL_OK)
		return status;

	dt->Second =  DS3231_DecodeBCD(buffer[0] & 0x7F);
	dt->Minute =  DS3231_DecodeBCD(buffer[1] & 0x7F);
	dt->Hour_24mode =  DS3231_DecodeBCD(buffer[2] & 0x3F);
	dt->Day =  DS3231_DecodeBCD(buffer[3] & 0x07);
	dt->Date =  DS3231_DecodeBCD(buffer[4] & 0x3F);
	dt->Month =  DS3231_DecodeBCD(buffer[5] & 0x1F);
	dt->Year =  DS3231_DecodeBCD(buffer[6]) + 2000U;

	uint8_t regSTATUS;
	status = DS3231_ReadRegister(DS3231_REG_STATUS, &regSTATUS);
	if (status != HAL_OK)
		return status;
	regSTATUS &= (0x80);
	regSTATUS >>= DS3231_EOSC;

	dt->Enable = regSTATUS? DS3231_DISABLED : DS3231_ENABLED;

	return status;
}

/**
 * @brief Converts the broken down Date Time to unix time
 * @param[in] *dt Pass a pointer to #DS3231_DateTime type variable with current broken down date, time information.
 * @param[out] *unixtime Pass a pointer to uint32_t variable to get unix time, i.e. seconds since epoch.
 * @return void
 */
void DS3231_ToUnixTime(DS3231_DateTime *dt, uint32_t *unixtime) {
	uint16_t days, years;
	uint8_t months, hours, minutes, seconds;

	years = dt->Year;
	months = dt->Month;
	days = dt->Date;
	hours = dt->Hour_24mode;
	minutes = dt->Minute;
	seconds = dt->Second;


	 if (years >= 2000)
		 years -= 2000;
	 else
		 return;

	 days -= 1;
	 for (uint8_t i=1; i<months; i++)
		 days += days_in_month[i - 1];

	 if (months > 2 && years % 4 == 0)
		 days++;

	 days += (365 * years + (years + 3) / 4);

	 *unixtime = ((days * 24UL + hours) * 60 + minutes) * 60 + seconds + SECONDS_FROM_1970_TO_2000;
}

/**
 * @brief Converts the unix time to broken down Date Time
 * @param[in] *unixtime Pass a pointer to uint32_t variable containing the current unix time, i.e. seconds since epoch.
 * @param[out] *dt Pass a pointer to #DS3231_DateTime type variable to get current broken down date, time information.
 * @return void
 */
void DS3231_ToDateTime(uint32_t *unixtime, DS3231_DateTime *dt) {

    int32_t currYear, daysTillNow, extraTime, extraDays;
    uint8_t index, day, date, month, flag = 0;

    // Calculate total days unix time T
    daysTillNow = (*unixtime / (24 * 60 * 60));
    extraTime = (*unixtime % (24 * 60 * 60));
    currYear = 1970;

    // Calculating current year
    while (1) {
    	if (currYear % 400 == 0 || (currYear % 4 == 0 && currYear % 100 != 0)) {
            if (daysTillNow < 366) {
                break;
            }
            daysTillNow -= 366;
        }
        else {
            if (daysTillNow < 365) {
                break;
            }
            daysTillNow -= 365;
        }
        currYear += 1;
    }

    // Updating extradays because it
    // will give days till previous day
    // and we have include current day
    extraDays = daysTillNow + 1;

    if (currYear % 400 == 0 || (currYear % 4 == 0 && currYear % 100 != 0))
        flag = 1;

    // Calculating MONTH and DATE
    month = 0, index = 0;
    if (flag == 1) {
        while (1) {

            if (index == 1) {
                if (extraDays - 29 < 0)
                    break;
                month += 1;
                extraDays -= 29;
            }
            else {
                if (extraDays - days_in_month[index] < 0) {
                    break;
                }
                month += 1;
                extraDays -= days_in_month[index];
            }
            index += 1;
        }
    }
    else {
        while (1) {
            if (extraDays - days_in_month[index] < 0) {
                break;
            }
            month += 1;
            extraDays -= days_in_month[index];
            index += 1;
        }
    }

    // Current Month
    if (extraDays > 0) {
        month += 1;
        date = extraDays;
    }
    else {
        if (month == 2 && flag == 1)
            date = 29;
        else
            date = days_in_month[month - 1];
    }

    // Calculating HH:MM:YYYY
    dt->Date = (uint8_t)date;
    dt->Month = (uint8_t)month;
    dt->Year = (uint16_t)currYear;
    dt->Hour_24mode = extraTime / 3600;
    dt->Minute = (extraTime % 3600) / 60;
    dt->Second = (extraTime % 3600) % 60;

    currYear -= month < 3;
    day = (currYear + currYear / 4 - currYear / 100 + currYear / 400 + dow[month - 1] + date) % 7;
    if (day == 0)
        day = 7;

    dt->Day = day;
}

/**
 * @brief Decodes the binary value from BCD format.
 * @param[in] bin binary value.
 * @return Decoded BCD value.
 */
uint8_t DS3231_DecodeBCD(uint8_t bin) {
	return (((bin & 0xF0) >> 4) * 10) + (bin & 0x0F);
}

/**
 * @brief Encodes the raw binary value to BCD format.
 * @param[in] dec BCD value.
 * @return Decoded binary value.
 */
uint8_t DS3231_EncodeBCD(uint8_t dec) {
	return (dec % 10 + ((dec / 10) << 4));
}

/**
 * @brief Writes one byte of data to the designated DS3231 register.
 * @param[in] *reg Pointer to a register address to write.
 * @param[in] *data Pointer to a date variable to write from.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_WriteRegister(uint8_t reg, uint8_t *data) {
	return HAL_I2C_Mem_Write(DS3231_device, DS3231_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, DS3231_TIMEOUT);
}

/**
 * @brief Writes multiple byte of data to the consecutive DS3231 registers.
 * @param[in] *reg Pointer to a starting register address to write.
 * @param[in] *data Pointer to a date buffer to write from.
 * @param[in] *len Pointer to a variable containing the number of bytes to write.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_WriteRegisters(uint8_t reg, uint8_t *data, uint8_t len) {
	return HAL_I2C_Mem_Write(DS3231_device, DS3231_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, len, DS3231_TIMEOUT);
}

/**
 * @brief Reads one byte of data from the designated DS3231 register.
 * @param[in] *reg Pointer to a register address to read from.
 * @param[out] *data Pointer to a date variable to read to.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_ReadRegister(uint8_t reg, uint8_t *data) {
	return HAL_I2C_Mem_Read(DS3231_device, DS3231_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, DS3231_TIMEOUT);
}

/**
 * @brief Reads multiple byte of data from consecutive DS3231 registers.
 * @param[in] *reg Pointer to a starting register address to read from.
 * @param[out] *data Pointer to a date buffer to read to.
 * @param[in] *len Pointer to a variable containing the number of bytes to read.
 * @return HAL_StatusTypeDef variable describing if it was successful or not.
 */
HAL_StatusTypeDef DS3231_ReadRegisters(uint8_t reg, uint8_t *data, uint8_t len) {
	return HAL_I2C_Mem_Read(DS3231_device, DS3231_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, len, DS3231_TIMEOUT);
}

#ifdef __cplusplus
}
#endif
