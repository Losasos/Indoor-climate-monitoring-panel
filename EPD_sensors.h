// device address without R/W bit
#define BMP180 0x77 // sensor BMP180, module GY-68
#define HTU21  0x40 // sensor HTU21D, module GY-21

// BMP180 >> registers and commands for measurements requests
#define CMD_REG 0xF4      
#define DATA_REG 0xF6    
#define READ_TEMP_CMD 0x2E
#define READ_PRESS_CMD 0x34

// HTU21D >> commands for measurements requests
#define MEASURE_TEMP_CMD  0xF3
#define MEASURE_HUMID_CMD 0xF5

int8_t is_BMP180(void);
int8_t is_HTU21(void);

// INIT function
void Setup_Sensors(void);

// I2C >> look for known i2c devices on the bus
void I2C_ScanBus(void);

// BMP180 FUNCTIONS
// =============================================================================================
// BMP180 >> Read i2c data
int32_t BMP180_ReadData(int8_t addr, int8_t cmd, int8_t len);

// BMP180 >> Get temperature measurements from BMP180 wia I2C bus and procede with calculations
// returns temperature in 0.1 grad C
int32_t BMP180_GetTemperature();

// BMP180 >> Get pressure measurements from BMP180 wia I2C bus and procede with calculations
// returns pressure in Pascal
int32_t BMP180_GetPressure();

// BMP180 >> Calculate Temperature
int32_t BMP180_CalcTemperature(int32_t rawTemp);

// BMP180 >> Calculate Pressure
int32_t BMP180_CalcPressure(int32_t rawPress);

// BMP180 >> Read one of calibration values
void BMP180_ReadCalibration(void);

// HTU21 FUNCTIONS
// =============================================================================================
// HTU21 >> Read i2c data
int32_t HTU21_ReadData(int8_t cmd);

// HTU21 >> Get temperature measurements from HTU21 wia I2C bus and procede with calculations
// returns temperature in 0.1 grad C
int32_t HTU21_GetTemperature();

// HTU21 >> Get humidity measurements from HTU21 wia I2C bus and procede with calculations
// returns relative humidity in %
int32_t HTU21_GetHumidity();

// HTU21 >> Calculate Temperature
int32_t HTU21_CalcTemperature(int32_t rowTemp);

// HTU21 >> Calculate Humidity
int32_t HTU21_CalcHumidity(int32_t rowHumid);
