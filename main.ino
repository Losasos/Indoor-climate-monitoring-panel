// ESP32 + EPD-------------------------------------------------
#include "DEV_Config.h"
#include "EPD_CODICO.h"
#include <stdlib.h>
#include "EPD_sensors.h"
//-------------------------------------------------------------

#define LED_PIN 2
#define DO_SLEEP  1
#define DO_EPAPER 1

// Wakeup timer section ---------------------------------------
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP  300         // Time ESP32 will go to sleep (in seconds) 
#define ARR_COUNT 7                // we store the last 7 values ro print
RTC_DATA_ATTR uint32_t bootCount = 0;
RTC_DATA_ATTR int32_t arr_T[ARR_COUNT] = {-999,-999,-999,-999,-999,-999,-999};    // Temperature [0] is the latest, [5] is the oldest value, -999 is impossible value
RTC_DATA_ATTR int32_t arr_H[ARR_COUNT] = {-999,-999,-999,-999,-999,-999,-999};    // Humidity
RTC_DATA_ATTR int32_t arr_P[ARR_COUNT] = {-999,-999,-999,-999,-999,-999,-999};    // Pressure

//-------------------------------------------------------------
// ===== SETUP / INITIALIZATION ==============================================================
void setup()
{
  short need_update_epd = 0;
  // Serial debug interface init
  Serial.begin(115200);
  Serial.println("START\r\n");
  Serial.printf("CNT:%d\r\n", bootCount);
  Serial.printf("--0 T%d H%d P%d\r\n", arr_T[0], arr_H[0], arr_P[0]);
  Serial.printf("--1 T%d H%d P%d\r\n", arr_T[1], arr_H[1], arr_P[1]);
  Serial.printf("--2 T%d H%d P%d\r\n", arr_T[2], arr_H[2], arr_P[2]);
  Serial.printf("--3 T%d H%d P%d\r\n", arr_T[3], arr_H[3], arr_P[3]);
  Serial.printf("--4 T%d H%d P%d\r\n", arr_T[4], arr_H[4], arr_P[4]);
  Serial.printf("--5 T%d H%d P%d\r\n", arr_T[5], arr_H[5], arr_P[5]);
  Serial.printf("--6 T%d H%d P%d\r\n", arr_T[6], arr_H[6], arr_P[6]);
  //
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);   // turn the LED on - indicates awake mode
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(250);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  //
  pinMode(EPD_BUSY_PIN,  INPUT);
  pinMode(EPD_RST_PIN , OUTPUT);
  pinMode(EPD_DC_PIN  , OUTPUT);
  pinMode(EPD_SCK_PIN, OUTPUT);
  pinMode(EPD_MOSI_PIN, OUTPUT);
  pinMode(EPD_CS_PIN , OUTPUT);
  digitalWrite(EPD_CS_PIN , HIGH);
  digitalWrite(EPD_SCK_PIN, LOW);

#if DO_EPAPER
  EPD_Init(); 
  Setup_Sensors();
  
  // PRINTING PART =============
  // Shift values in the archive
  int n;
  for (n=ARR_COUNT-1; n > 0; n--)
  {
    arr_T[n] = arr_T[n-1]; // shifting the values, getting rid of the oldest
    arr_H[n] = arr_H[n-1]; // shifting the values, getting rid of the oldest
    arr_P[n] = arr_P[n-1]; // shifting the values, getting rid of the oldest
  }
  // Print out current values
  EPD_PrintTemperatue();
  EPD_PrintHumidity();
  EPD_PrintPressure();
  // Draw T, H, P histograms
  UpdateSingleHistogram(113, arr_T);
  UpdateSingleHistogram(169, arr_H);
  UpdateSingleHistogram(225, arr_P);
  // Send RED and BLACK matrix to the EPD  
  EPD_SendImageData(RED);
  EPD_SendImageData(BLACK);
  EPD_Refresh();
#endif

#if DO_SLEEP
  //===== Set ESP32 into a deepsleep mode =====
  // Increment boot number and print it every reboot
  ++bootCount;
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);  
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
#endif
}

// ===== MAIN LOOP ============================================================================
void loop()
{
  // Nothing to do    
  digitalWrite(LED_PIN, LOW);   // turn the LED on - indicates awake mode
  delay(950);
  digitalWrite(LED_PIN, HIGH);   // turn the LED on - indicates awake mode
  delay(50);
}

// ===== PRINTING FUNCTIONS ===================================================================
// TEMPERATURE
void EPD_PrintTemperatue(void)
{
  int8_t coma = 1;
  int8_t sign = 1;
  int8_t num_length = 0;
  short x, y;
  y = 68;
  x = 88;
  int32_t value = 0;
  if(is_BMP180() == 1) 
  {
    value = BMP180_GetTemperature();
    arr_T[0] = value; // save the latest value
  }
  else if (is_HTU21() == 1) 
  {
    value = HTU21_GetTemperature(); 
    arr_T[0] = value; // save the latest value
  }
  else
  {
    // prints minus at the most right position as a sign of absence
    EPD_AddFontImage(x, 106, BLACK, 12);
    arr_T[0] = -999; // IMPOSSIBLE value
    return;
  }
  //
  int32_t tmp = value;
  while(tmp != 0)
  {
    num_length++;
    tmp /= 10;
  }
  if (coma>0) { num_length++; }  
  if (sign>0) { num_length++; }
  int8_t num[num_length];
  // 11 and 12 are indexes of plus and minus
  if (sign>0 && value<0) { num[0]=12; }
  else { num[0]=11; }
  tmp = num_length-1-coma;
  for (int i = num_length-1; i>=0+sign; i--)
  {
    if (i == tmp && coma > 0)
    {
      num[i] = 10; //10 is index of coma index
      continue;
    }
    num[i] = value%10;
    value/=10;
  }
  //
  for (int i = 0; i<num_length; i++)
  {
    EPD_AddFontImage(x, y, BLACK, num[i]);
    if (i == num_length-1-coma && coma > 0) { y += 11; }
    else { y += 19; }
  }
}

// HUMIDITY
void EPD_PrintHumidity(void)
{
  int8_t coma = 1;
  int8_t sign = 0;
  int8_t num_length = 0;
  short x, y;
  y = 68;
  x = 144;
  int32_t value = 0;
  if(is_BMP180() == 1) 
  {
    value = HTU21_GetHumidity(); 
    arr_H[0] = value; // save the latest value
  }
  else
  {
    // prints minus at the most right position as a sign of absence
    EPD_AddFontImage(x, 106, BLACK, 12);
    arr_H[0] = -999; // IMPOSSIBLE value    
    return;
  }
  int32_t tmp = value;
  //
  while(tmp != 0)
  {
    num_length++;
    tmp /= 10;
  }
  if (coma>0) { num_length++; }
  if (sign>0) { num_length++; }
  int8_t num[num_length];
  // 11 and 12 are indexes of plus and minus
  if (sign>0 && value<0) { num[0]=12; }
  else { num[0]=11; }
  tmp = num_length-1-coma;
  for (int i = num_length-1; i>=0+sign; i--)
  {
    if (i == tmp && coma > 0)
    {
      num[i] = 10; //10 is index of coma index
      continue;
    }
    num[i] = value%10;
    value/=10;
  }
  //
  for (int i = 0; i<num_length; i++)
  {
    EPD_AddFontImage(x, y, BLACK, num[i]);
    if (i == num_length-1-coma && coma > 0) { y += 11; }
    else { y += 19; }
  }
}

// PRESSURE
void EPD_PrintPressure(void)
{
  int8_t coma = 0;
  int8_t sign = 0;
  int8_t num_length = 0;
  short x, y;
  y = 68;
  x = 200;
  int32_t value = 0;
  if(is_BMP180() == 1) 
  {
    value = Pa_to_mmHg(BMP180_GetPressure()); 
    arr_P[0] = value; // save the latest value
  }
  else
  {
    // prints minus at the most right position as a sign of absence
    EPD_AddFontImage(x, 106, BLACK, 12);
    arr_P[0] = -999; // IMPOSSIBLE value    
    return;
  }
  int32_t tmp = value;
  while(tmp != 0)
  {
    num_length++;
    tmp /= 10;
  }
  if (coma>0) { num_length++; }
  if (sign>0) { num_length++; }
  int8_t num[num_length];
  // 11 and 12 are indexes of plus and minus
  if (sign>0 && value<0) { num[0]=12; }
  else { num[0]=11; }
  tmp = num_length-1-coma;
  for (int i = num_length-1; i>=0+sign; i--)
  {
    if (i == tmp && coma > 0)
    {
      num[i] = 10; //10 is index of coma index
      continue;
    }
    num[i] = value%10;
    value/=10;
  }
  //
  for (int i = 0; i<num_length; i++)
  {
    EPD_AddFontImage(x, y, BLACK, num[i]);
    if (i == num_length-1-coma && coma > 0) { y += 11; }
    else { y += 19; }
  }
}

// Conversion the pression in Pa into mmHg
int32_t Pa_to_mmHg(int32_t preassure)
{
  // mmHg = Pa ÷ 133.3
  return int32_t(preassure / 133.3);
}

// Update Histogram
void UpdateSingleHistogram(int16_t X, int32_t *values)
{
  int16_t vmax, vmin;
  int16_t Y = 244;
  int16_t dX = 40;
  int16_t dY = 130;
  //
  int16_t y_min = Y+5;
  int16_t y_max = Y+dY-5;
  int16_t y_step = floor((y_max-y_min)/ARR_COUNT);
  int16_t x_min = X-8;
  int16_t x_max = X-dX+2;
  int16_t x_0;
  int8_t i;
  //
  if (values[0] == -999) { return; } // EXIT
  vmax = values[0]; 
  vmin = values[0];
  for (i=1; i<ARR_COUNT; i++)
  {
    if (values[i] == -999) { continue; }
    if (values[i] > vmax) { vmax = values[i]; }
    if (values[i] < vmin) { vmin = values[i]; }
  }
  // Drawing graph
  for (i=0; i<ARR_COUNT; i++)
  {
    if (values[i] == -999) { continue; }
    x_0 = floor((values[i]-vmin)*(x_max-x_min)/(vmax-vmin+1));
    printf("x0=%d vmin=%d vmax=%d \r\n",x_0, vmin, vmax, y_min, y_step);
    if (i == 0) { EPD_Rectangle(X-1, y_min+y_step*(ARR_COUNT-i-1), x_0-8+X, y_min+y_step*(ARR_COUNT-i)-3, RED, 1, 1); }
    else { EPD_Rectangle(X-1, y_min+y_step*(ARR_COUNT-i-1), x_0-8+X, y_min+y_step*(ARR_COUNT-i)-3, BLACK, 1, 1); }    
  }
}
