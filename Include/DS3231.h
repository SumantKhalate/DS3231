/* An STM32 HAL library written for the DS3231 real-time clock IC. */
/* Library by Sumant Khalate aka pi_filter | github.com/SumantKhalate */
/* Date 03/05/2003 */

#ifndef DS3231_H
#define DS3231_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define SECONDS_FROM_1970_TO_2000 946684800

/*---------------------------------------- HAL FUNCTION TIMEOUT TIME ----------------------------*/
#define DS3231_TIMEOUT			HAL_MAX_DELAY

/*---------------------------------------- DEVICE ADDRESS ---------------------------------------*/
#define DS3231_I2C_ADDR			(0x68 << 1)

/*------------------------------------ TIMEKEEPIGN REGISTERS ------------------------------------*/
#define DS3231_REG_SECOND		0x00
#define DS3231_REG_MINUTE		0x01
#define DS3231_REG_HOUR			0x02
#define DS3231_REG_DAY			0x03

#define DS3231_REG_DATE			0x04
#define DS3231_REG_MONTH		0x05
#define DS3231_REG_YEAR   		0x06

#define DS3231_REG_A1_SECOND	0x07
#define DS3231_REG_A1_MINUTE	0x08
#define DS3231_REG_A1_HOUR		0x09
#define DS3231_REG_A1_DATE		0x0A

#define DS3231_REG_A2_MINUTE	0x0B
#define DS3231_REG_A2_HOUR		0x0C
#define DS3231_REG_A2_DATE		0x0D

#define DS3231_REG_CONTROL 		0x0E
#define DS3231_REG_STATUS		0x0F

#define DS3231_REG_AGING		0x10

#define DS3231_REG_TEMP_MSB		0x11
#define DS3231_REG_TEMP_LSB		0x12

/*------------------------------------ CENTURY REGISTERS BITS------------------------------------*/
#define DS3231_CENTURY			7			/* Toggled when the years register overflows from 99 to 00 */

/*------------------------------------ CONTROL REGISTERS BITS------------------------------------*/
#define DS3231_EOSC				7			/* Not Enable Oscillator, 0 equal ON */
#define DS3231_BBSQW			6			/* Battery-Backed Square-Wave Enable */
#define DS3231_CONV				5			/* Convert Temperature */
#define DS3231_RS2				4			/* Square-wave rate select 2 */
#define DS3231_RS1				3			/* Square-wave rate select 1 */
#define DS3231_INTCN			2			/* Interrupt Control */
#define DS3231_A2IE				1			/* Alarm 2 Interrupt Enable */
#define DS3231_A1IE				0			/* Alarm 1 Interrupt Enable */

/*------------------------------------ STATUS REGISTERS BITS-------------------------------------*/
#define DS3231_A1F				0			/* Alarm 1 Flag */
#define DS3231_A2F				1			/* Alarm 2 Flag */
#define DS3231_BSY				2			/* Device is busy executing TCXO */
#define DS3231_EN32KHZ			3			/* Enable 32KHz Output  */
#define DS3231_OSF				7			/* Oscillator Stop Flag */

/*------------------------------------ ALARM MASK BITS-------------------------------------------*/
#define DS3231_AXMY				7			/* Alarm register mask */
#define DS3231_DYDT				6			/* Day Date Bit, 1 equal Day */


/*------------------------------------ ENUM DEFINATIONS -----------------------------------------*/
typedef enum DS3231_DoW {
	DS3231_MON  = 0x01,
	DS3231_TUE,
	DS3231_WED,
	DS3231_THU,
	DS3231_FRI,
	DS3231_SAT,
	DS3231_SUN
} DS3231_DoW;

typedef enum DS3231_Rate {
	DS3231_RATE_1HZ,
	DS3231_RATE_1024HZ,
	DS3231_RATE_4096HZ,
	DS3231_RATE_8192HZ
} DS3231_Rate;

typedef enum DS3231_InterruptMode {
	DS3231_SQUARE_WAVE_INTERRUPT,
	DS3231_ALARM_INTERRUPT
} DS3231_InterruptMode;

typedef enum DS3231_State {
	DS3231_DISABLED,
	DS3231_ENABLED
} DS3231_State;

typedef enum D3231_Alarm1Mode {
	DS3231_A1_EVERY_S = 0x0F,
	DS3231_A1_MATCH_S = 0x0E,
	DS3231_A1_MATCH_S_M = 0x0C,
	DS3231_A1_MATCH_S_M_H = 0x08,
	DS3231_A1_MATCH_S_M_H_DATE = 0x00,
	DS3231_A1_MATCH_S_M_H_DAY = 0x10
} DS3231_Alarm1Mode;

typedef enum D3231_Alarm2Mode {
	DS3231_A2_EVERY_M = 0x07,
	DS3231_A2_MATCH_M = 0x06,
	DS3231_A2_MATCH_M_H = 0x04,
	DS3231_A2_MATCH_M_H_DATE = 0x00,
	DS3231_A2_MATCH_M_H_DAY = 0x08
} DS3231_Alarm2Mode;


/*------------------------------------ STRUCTURE DEFINATIONS ------------------------------------*/
typedef struct DS3231_DateTime {
	uint8_t Day;
	uint8_t Date;
	uint8_t Month;
	uint16_t Year;
	uint8_t Hour_24mode;
	uint8_t Minute;
	uint8_t Second;
	DS3231_State Enable;
} DS3231_DateTime;

typedef struct D3231_Alarm1 {
	uint8_t Seconds;
	uint8_t Minutes;
	uint8_t Hours;
	uint8_t DayDate;
	DS3231_Alarm1Mode Mode;
	DS3231_State IntEn;
} D3231_Alarm1;

typedef struct D3231_Alarm2 {
	uint8_t Minutes;
	uint8_t Hours;
	uint8_t DayDate;
	DS3231_Alarm2Mode Mode;
	DS3231_State IntEn;
} D3231_Alarm2;


/*------------------------------------ FUNCTION DEFINATIONS -------------------------------------*/
extern I2C_HandleTypeDef *i2cHandle;

HAL_StatusTypeDef DS3231_Init(I2C_HandleTypeDef *i2cHandle);

HAL_StatusTypeDef DS3231_SetBatterySquareWave(DS3231_State enable);
HAL_StatusTypeDef DS3231_GetBatterySquareWave(DS3231_State *enable);

HAL_StatusTypeDef DS3231_SetOscillator(DS3231_State enable);
HAL_StatusTypeDef DS3231_GetOscillatorStoppedFlag(DS3231_State *enable);

HAL_StatusTypeDef DS3231_Set32kHzOutput(DS3231_State enable);
HAL_StatusTypeDef DS3231_Get32kHzEnabled(DS3231_State *enable);

HAL_StatusTypeDef DS3231_SetInterruptMode(DS3231_InterruptMode mode);
HAL_StatusTypeDef DS3231_GetInterruptMode(DS3231_InterruptMode *mode);

HAL_StatusTypeDef DS3231_SetRateSelect(DS3231_Rate rate);
HAL_StatusTypeDef DS3231_GetRateSelect(DS3231_Rate *rate);

HAL_StatusTypeDef DS3231_GetTemperature(float *temp_real);

HAL_StatusTypeDef DS3231_SetAlarm1(D3231_Alarm1 *A1_st);
HAL_StatusTypeDef DS3231_GetAlarm1(D3231_Alarm1 *A1_st);
HAL_StatusTypeDef DS3231_SetAlarm1IntEn(DS3231_State enable);
HAL_StatusTypeDef DS3231_GetAlarm1IntEn(DS3231_State *enable);
HAL_StatusTypeDef DS3231_GetAlarm1Flag(DS3231_State *enable);
HAL_StatusTypeDef DS3231_ClearAlarm1Flag(void);

HAL_StatusTypeDef DS3231_SetAlarm2(D3231_Alarm2 *A2_st);
HAL_StatusTypeDef DS3231_GetAlarm2(D3231_Alarm2 *A2_st);
HAL_StatusTypeDef DS3231_SetAlarm2IntEn(DS3231_State enable);
HAL_StatusTypeDef DS3231_GetAlarm2IntEn(DS3231_State *enable);
HAL_StatusTypeDef DS3231_GetAlarm2Flag(DS3231_State *enable);
HAL_StatusTypeDef DS3231_ClearAlarm2Flag(void);

HAL_StatusTypeDef DS3231_SetDateTime(DS3231_DateTime *dt);
HAL_StatusTypeDef DS3231_GetDateTime(DS3231_DateTime *dt);

void DS3231_ToUnixTime(DS3231_DateTime *dt, uint32_t *unixtime);
void DS3231_ToDateTime(uint32_t *unixtime, DS3231_DateTime *dt);

uint8_t DS3231_DecodeBCD(uint8_t bin);
uint8_t DS3231_EncodeBCD(uint8_t dec);

HAL_StatusTypeDef DS3231_WriteRegister(uint8_t reg, uint8_t *data);
HAL_StatusTypeDef DS3231_WriteRegisters(uint8_t reg, uint8_t *data, uint8_t len);
HAL_StatusTypeDef DS3231_ReadRegister(uint8_t reg, uint8_t *data);
HAL_StatusTypeDef DS3231_ReadRegisters(uint8_t reg, uint8_t *data, uint8_t len);

#ifdef __cplusplus
			}
#endif

#endif /* DS3231_H */
