#include "Wire.h"

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

// BMP180 >> calibration constants and variables
int8_t   act_BMP180 = 0;
int16_t  c_AC1, c_AC2, c_AC3, c_B1, c_B2, c_MB, c_MC, c_MD;
uint16_t c_AC4, c_AC5, c_AC6;
int32_t  v_B5;

// HTU21 >> calibration constants and variables 
int8_t   act_HTU21 = 0;

// INIT function 
void setup(void)
{
  // init
  Wire.begin();
  Serial.begin(115200);
  delay(20);
  // I2C_ScanBus
  I2C_ScanBus();
  // run startup procedures if devices present on i2c bus
  if (act_BMP180) BMP180_ReadCalibration();
}

// MAIN loop function
void loop(void)
{ 
  int32_t  BMP180_Temperature;
  int32_t  HTU21_Temperature;
  int32_t  Pressure;
  int32_t  Humidity;
  
  // read data if devices present on i2c bus
  //act_BMP180 = 0;
  if (act_BMP180)
  { 
    Serial.println("BMP180 Data:");
    BMP180_Temperature = BMP180_GetTemperature();
    Serial.printf("T = %d.%d °C\r\n", (int32_t)(BMP180_Temperature/10), abs(BMP180_Temperature%10));
    Pressure = BMP180_GetPressure();
    Serial.printf("P = %d Pa\r\n", Pressure);
  }
  //
  //act_HTU21 = 0;
  if (act_HTU21)
  { 
    Serial.println("HTU21 Data:");
    HTU21_Temperature = HTU21_GetTemperature();
    Serial.printf("T = %d.%d °C\r\n", (int32_t)(HTU21_Temperature/10), abs(HTU21_Temperature%10));
    Humidity = HTU21_GetHumidity();
    Serial.printf("H = %d.%d %%\r\n", (int32_t)(Humidity/10), abs(Humidity%10));
  }
  //
  delay(5000);
}

// I2C >> look for known i2c devices on the bus
void I2C_ScanBus(void)
{
  uint8_t err, addr;
  uint8_t  n_dev = 0;
  //
  Serial.print("I2C bus scanning: ");
  for(addr = 0x01; addr < 0x7F; addr++)
  {
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();
    if (err == 0)
    {
      n_dev++;
      Serial.printf(" %d:0x%02X", n_dev, addr);
      if (addr == BMP180) act_BMP180 = 1;
      if (addr == HTU21)  act_HTU21 = 1;
    } 
    else if(err != 2)
    {
      Serial.printf(" E:0x%02X", addr);
    }
  }
  if (n_dev == 0) { Serial.println("\r\nNo i2c devices found"); }
  else { Serial.printf("\r\nIn total %d devices found\r\n", n_dev); }
}

// BMP180 FUNCTIONS
// =============================================================================================
// BMP180 >> Read i2c data
int32_t BMP180_ReadData(int8_t addr, int8_t cmd, int8_t len)
{ 
  // Serial.printf("ReadData(%.2X,%.2X,%d) = ",(uint8_t)addr,(uint8_t)cmd,len);
  Wire.beginTransmission(BMP180);
  Wire.write(addr);
  // if there is a command
  if (cmd > 0)
  {
    Wire.write(cmd);  
    Wire.write(DATA_REG);
  }
  Wire.endTransmission();
  delay(10);
  // read
  Wire.requestFrom(BMP180, len);
  int32_t tmp = Wire.read();
  tmp <<= 8;
  tmp |= Wire.read();
  if (len == 3) Wire.read();
  // Serial.printf("%.6X\r\n",(uint32_t)tmp);
  return tmp;
}

// BMP180 >> Get temperature measurements from BMP180 wia I2C bus and procede with calculations
// returns temperature in 0.1 grad C
int32_t BMP180_GetTemperature()
{
  int32_t rawTemperature;
  rawTemperature = BMP180_ReadData(CMD_REG, READ_TEMP_CMD, 2);
  return BMP180_CalcTemperature(rawTemperature);
}

// BMP180 >> Get pressure measurements from BMP180 wia I2C bus and procede with calculations
// returns pressure in Pascal
int32_t BMP180_GetPressure()
{
  int32_t rawPressure;
  rawPressure = BMP180_ReadData(CMD_REG, READ_PRESS_CMD, 3);
  return BMP180_CalcPressure(rawPressure);
}

// BMP180 >> Calculate Temperature
int32_t BMP180_CalcTemperature(int32_t rawTemp)
{ 
  // calculations
  int32_t x1 = ((rawTemp - c_AC6) * c_AC5)>>15;
  int32_t x2 = (c_MC<<11) / (x1 + c_MD);
  v_B5 = x1 + x2;
  return (v_B5 + 8)>>4;
}

// BMP180 >> Calculate Pressure
int32_t BMP180_CalcPressure(int32_t rawPress)
{ 
  // calculations
  int32_t b6 = v_B5 - 4000;
  int32_t x1 = (c_B2 * ((b6 * b6)>>12))>>11;
  int32_t x2 = (c_AC2 * b6)>>11;
  int32_t x3 = x1 + x2;
  int32_t b3 = (((int32_t)c_AC1 * 4 + x3) + 2)>>2;
  x1 = (c_AC3 * b6)>>13;
  x2 = (c_B1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  uint32_t b4 = (c_AC4 * (uint32_t)(x3 + 32568))>>15;
  uint32_t b7 = ((uint32_t)rawPress - b3) * 50000;
  int32_t p;
  if (b7 < 0x80000000) { p = (b7<<1) / b4; }
  else { p = (b7 / b4)<<1; }
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p = p + ((x1 + x2 + 3791)>>4);
  return p;
}

// BMP180 >> Read one of calibration values
void BMP180_ReadCalibration(void)
{ 
  c_AC1 = BMP180_ReadData(0xAA, 0, 2);
  c_AC2 = BMP180_ReadData(0xAC, 0, 2);
  c_AC3 = BMP180_ReadData(0xAE, 0, 2);
  c_AC4 = BMP180_ReadData(0xB0, 0, 2);
  c_AC5 = BMP180_ReadData(0xB2, 0, 2);
  c_AC6 = BMP180_ReadData(0xB4, 0, 2);
  c_B1  = BMP180_ReadData(0xB6, 0, 2);
  c_B2  = BMP180_ReadData(0xB8, 0, 2);
  c_MB  = BMP180_ReadData(0xBA, 0, 2);
  c_MC  = BMP180_ReadData(0xBC, 0, 2);
  c_MD  = BMP180_ReadData(0xBE, 0, 2);
}

// HTU21 FUNCTIONS
// =============================================================================================
// HTU21 >> Read i2c data
int32_t HTU21_ReadData(int8_t cmd)
{ 
  // Serial.printf("ReadData(%.2X) = ",(uint8_t)cmd);
  Wire.beginTransmission(HTU21);
  Wire.write(cmd);
  Wire.endTransmission();
  delay(50);
  // read
  Wire.requestFrom(HTU21, 3);
  int32_t tmp = Wire.read();
  tmp <<= 8;
  tmp |= Wire.read();
  Wire.read();
  // Serial.printf("%.4X\r\n",(uint32_t)tmp);
  return tmp;
}

// HTU21 >> Get temperature measurements from HTU21 wia I2C bus and procede with calculations
// returns temperature in 0.1 grad C
int32_t HTU21_GetTemperature()
{
  int32_t rawTemperature;
  rawTemperature = HTU21_ReadData(MEASURE_TEMP_CMD);
  return HTU21_CalcTemperature(rawTemperature);
}

// HTU21 >> Get humidity measurements from HTU21 wia I2C bus and procede with calculations
// returns relative humidity in %
int32_t HTU21_GetHumidity()
{
  int32_t rawHumidity;
  rawHumidity = HTU21_ReadData(MEASURE_HUMID_CMD);
  return HTU21_CalcHumidity(rawHumidity);
}

// HTU21 >> Calculate Temperature
int32_t HTU21_CalcTemperature(int32_t rowTemp)
{
  rowTemp = rowTemp & 0xFFFC; // sets two lowest bits to zero
  int32_t T = (int32_t)((175.72 * rowTemp / 65536 - 46.85) * 10);
  return T;
}

// HTU21 >> Calculate Humidity
int32_t HTU21_CalcHumidity(int32_t rowHumid)
{
  rowHumid = rowHumid & 0xFFFC; // sets two lowest bits to zero
  int32_t RH = (int32_t)((125.0 * rowHumid / 65536 - 6) * 10);
  return RH;
}
