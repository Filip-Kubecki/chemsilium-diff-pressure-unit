#include "ADS1220IPWR.h"

static SPI_HandleTypeDef *ads1220_hspi;

/**
 * @brief Sets CS pin low.
 */
static inline void ADS1220_CS_Low(void) {
    HAL_GPIO_WritePin(ADS1220_CS_GPIO_Port, ADS1220_CS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Sets CS pin high.
 */
static inline void ADS1220_CS_High(void) {
    HAL_GPIO_WritePin(ADS1220_CS_GPIO_Port, ADS1220_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief Initializes the ADS1220 driver.
 * @param hspi Pointer to the SPI handle.
 */
void ADS1220_Init(SPI_HandleTypeDef *hspi) {
    ads1220_hspi = hspi;
    ADS1220_CS_High();
}

/**
 * @brief Sends a reset command to the device.
 */
void ADS1220_Reset(void) {
    uint8_t cmd = ADS1220_CMD_RESET;
    ADS1220_CS_Low();
    HAL_SPI_Transmit(ads1220_hspi, &cmd, 1, 100);
    ADS1220_CS_High();
    HAL_Delay(1);
}

/**
 * @brief Sends a start/sync command to trigger conversion.
 */
void ADS1220_StartSync(void) {
    uint8_t cmd = ADS1220_CMD_START;
    ADS1220_CS_Low();
    HAL_SPI_Transmit(ads1220_hspi, &cmd, 1, 100);
    ADS1220_CS_High();
}

/**
 * @brief Sends a power-down command to the device.
 */
void ADS1220_PowerDown(void) {
    uint8_t cmd = ADS1220_CMD_POWERDOWN;
    ADS1220_CS_Low();
    HAL_SPI_Transmit(ads1220_hspi, &cmd, 1, 100);
    ADS1220_CS_High();
}

/**
 * @brief Writes a value to a specific register.
 * @param reg Register address.
 * @param value Data to write.
 */
void ADS1220_WriteRegister(uint8_t reg, uint8_t value) {
    uint8_t tx[2];
    tx[0] = ADS1220_CMD_WREG | (reg << 2);
    tx[1] = value;
    ADS1220_CS_Low();
    HAL_SPI_Transmit(ads1220_hspi, tx, 2, 100);
    ADS1220_CS_High();
}

/**
 * @brief Reads the value from a specific register.
 * @param reg Register address.
 * @return Register value.
 */
uint8_t ADS1220_ReadRegister(uint8_t reg) {
    uint8_t tx = ADS1220_CMD_RREG | (reg << 2);
    uint8_t rx = 0;
    ADS1220_CS_Low();
    HAL_SPI_Transmit(ads1220_hspi, &tx, 1, 100);
    HAL_SPI_Receive(ads1220_hspi, &rx, 1, 100);
    ADS1220_CS_High();
    return rx;
}

/**
 * @brief Checks if data is ready via DRDY pin.
 * @return 1 if data is ready, 0 otherwise.
 */
uint8_t ADS1220_IsDataReady(void) {
    return (HAL_GPIO_ReadPin(ADS1220_DRDY_GPIO_Port, ADS1220_DRDY_Pin) == GPIO_PIN_RESET);
}

/**
 * @brief Waits for data with a timeout.
 * @param timeout_ms Maximum time to wait in ms.
 * @return HAL_OK on success, HAL_TIMEOUT on failure.
 */
HAL_StatusTypeDef ADS1220_WaitForData(uint32_t timeout_ms) {
    uint32_t tickstart = HAL_GetTick();
    while (!ADS1220_IsDataReady()) {
        if ((HAL_GetTick() - tickstart) > timeout_ms) {
            return HAL_TIMEOUT;
        }
    }
    return HAL_OK;
}

/**
 * @brief Reads the latest conversion result.
 * @return 24-bit signed ADC value.
 */
int32_t ADS1220_ReadData(void) {
    uint8_t cmd = ADS1220_CMD_RDATA;
    uint8_t rx[3] = {0};
    int32_t result;

    ADS1220_CS_Low();
    HAL_SPI_Transmit(ads1220_hspi, &cmd, 1, 100);
    HAL_SPI_Receive(ads1220_hspi, rx, 3, 100);
    ADS1220_CS_High();

    result = ((int32_t)rx[0] << 16) | ((int32_t)rx[1] << 8) | rx[2];
    if (result & 0x00800000) { 
        result |= 0xFF000000;
    }
    return result;
}

/**
 * @brief Configures configuration registers (REG1-REG3).
 */
void ADS1220_Configure(void) {
    ADS1220_WriteRegister(ADS1220_REG1, ADS1220_REG1_VAL);
    ADS1220_WriteRegister(ADS1220_REG2, ADS1220_REG2_VAL);
    ADS1220_WriteRegister(ADS1220_REG3, ADS1220_REG3_VAL);
}

/**
 * @brief Selects the input channel and triggers a new conversion.
 * @param channel The channel to select.
 */
void ADS1220_SelectChannel(ADS1220_Channel_t channel) {
    uint8_t reg0_value = (channel == ADS1220_CHANNEL_1) ? ADS1220_REG0_CH1 : ADS1220_REG0_CH2;
    ADS1220_WriteRegister(ADS1220_REG0, reg0_value);
    ADS1220_StartSync();
}

/**
 * @brief Converts raw ADC data to voltage in mV.
 * @param raw 24-bit raw value.
 * @return Voltage in millivolts.
 */
float ADS1220_RawToVoltage_mV(int32_t raw) {
    return (float)raw / 262144.0f;
}

/**
 * @brief Converts voltage to pressure in Pa.
 * @param voltage_mV Voltage in millivolts.
 * @return Pressure in Pascals.
 */
float ADS1220_VoltageToPressure_Pa(float voltage_mV) {
    return (voltage_mV / 20.0f) * 200000.0f;
}