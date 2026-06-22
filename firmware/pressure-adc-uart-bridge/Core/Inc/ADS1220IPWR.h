#ifndef ADS1220IPWR_H
#define ADS1220IPWR_H

#include "stm32g0xx_hal.h"

/* ---- CS and DRDY pins ---- */
#define ADS1220_CS_Pin          GPIO_PIN_0
#define ADS1220_CS_GPIO_Port    GPIOA
#define ADS1220_DRDY_Pin        GPIO_PIN_0
#define ADS1220_DRDY_GPIO_Port  GPIOB

/* ---- Commands ---- */
#define ADS1220_CMD_RESET       0x06
#define ADS1220_CMD_START       0x08
#define ADS1220_CMD_POWERDOWN   0x02
#define ADS1220_CMD_RDATA       0x10
#define ADS1220_CMD_RREG        0x20
#define ADS1220_CMD_WREG        0x40

/* ---- Register addresses ---- */
#define ADS1220_REG0            0x00
#define ADS1220_REG1            0x01
#define ADS1220_REG2            0x02
#define ADS1220_REG3            0x03

/* ---- Configuration values (Bridge 0-20mV, Gain=64) ---- */
#define ADS1220_REG0_CH1         0x0C  // MUX=AIN0-AIN1, Gain=64, PGA enabled
#define ADS1220_REG0_CH2         0x5C  // MUX=AIN2-AIN3, Gain=64, PGA enabled
#define ADS1220_REG1_VAL         0x40  // DR=010(90SPS), MODE=00(Normal), CM=0(Single-shot)
#define ADS1220_REG2_VAL         0x00  // Internal reference 2.048V, no filter
#define ADS1220_REG3_VAL         0x00  // No IDAC

typedef enum {
    ADS1220_CHANNEL_1 = 0,
    ADS1220_CHANNEL_2 = 1
} ADS1220_Channel_t;

/* ---- Public functions ---- */
void ADS1220_Init(SPI_HandleTypeDef *hspi);
void ADS1220_Reset(void);
void ADS1220_Configure(void);
void ADS1220_SelectChannel(ADS1220_Channel_t channel);
void ADS1220_StartSync(void);
void ADS1220_PowerDown(void);

float ADS1220_RawToVoltage_mV(int32_t raw);
float ADS1220_VoltageToPressure_Pa(float voltage_mV);

void ADS1220_WriteRegister(uint8_t reg, uint8_t value);
uint8_t ADS1220_ReadRegister(uint8_t reg);

int32_t ADS1220_ReadData(void);
uint8_t ADS1220_IsDataReady(void);
HAL_StatusTypeDef ADS1220_WaitForData(uint32_t timeout_ms);

#endif /* ADS1220IPWR_H */