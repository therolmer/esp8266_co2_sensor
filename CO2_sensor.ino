#include <SoftwareSerial.h>
#include "MHZ19.h"
#include <U8g2lib.h>

// Инициализация дисплея
U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(/* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);

// Иницализация софтового RS-232
SoftwareSerial ss(12,13);

// Инициализация модуля получения данных с сенсора MH-Z19B
MHZ19 mhz(&ss);

int MaxCo2 = 1;
int MinCo2 = 5000;

int MaxCo2TwoHours = 1;
int MinCo2TwoHours = 5000;
int TwoHours = 360 * 2;

// Длительность одного тика в миллисекундах
int OneTick = 10000;

// Общее количество тиков
int NumberOfTicks = 0;

// Количество тиков для инициализации
int NumberOfInitTicks = 1;


// Среднее значение
int Average = 0;

// Коэффициент усреденения (при тике равном 10 секунд получается скользящее среднее на 200 секунд)
int AverageFactor = 20;

// считает количество тиков необходимых для расчёта среднего
int NumberOfTicksForAverage = 0;

int InitTicks = 0;

float AverageLong = 0;

// 6 Часов
int AverageLongFactor = 21600;

int NumberOfTicksForAverageLong = 0;

// Первоначальная установка параметров
void setup()
{
  Serial.begin(115200);
  Serial.println(F("Starting..."));

  ss.begin(9600);
  mhz.setAutoCalibration(false);
  u8x8.begin();

  delay(5000);
}

// Основной рабочий цикл
void loop()
{
  MHZ19_RESULT response = mhz.retrieveData();
  if (response == MHZ19_RESULT_OK)
  {
    //Serial.print(F("Temperature: "));
    //Serial.println(mhz.getTemperature());

    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    char cstr[16];
    int co2;

    InitTicks++;
    if (InitTicks < NumberOfInitTicks)
    {
      u8x8.drawString(0, 2, " Calibration...");
      goto nextCycle;
    }
    else
    {     
      sprintf(cstr, "               ");
      u8x8.drawString(0, 0, cstr);
      u8x8.drawString(0, 1, cstr);
      u8x8.drawString(0, 2, cstr);
    }

    co2 = mhz.getCO2();
    
    sprintf(cstr, "C02 %04d",co2);
    u8x8.drawString(0, 0, cstr);

   
    if (co2 > MaxCo2)
    {
      MaxCo2 = co2;      
    }

    if ((co2 > 400) && (co2 < MinCo2))
    {
      MinCo2 = co2;      
    }

    sprintf(cstr, "Mi %04d", MinCo2);
    u8x8.drawString(0, 1, cstr);

    sprintf(cstr, "Ma %04d", MaxCo2);
    u8x8.drawString(0, 2, cstr);

    
    if (NumberOfTicksForAverage <= AverageFactor)
    {
      NumberOfTicksForAverage++;
    }
    
    Average = Average + (co2 - Average) / min(NumberOfTicksForAverage, AverageFactor);
    sprintf(cstr, "A  %04d", Average);
    u8x8.drawString(9, 0, cstr);

    if (NumberOfTicksForAverageLong <= AverageLongFactor)
    {
      NumberOfTicksForAverageLong++;
    }
    AverageLong = AverageLong + (float)(((float)co2 - AverageLong) / ((float) (min(NumberOfTicksForAverageLong, AverageLongFactor))));

    char tempStr[4];
    dtostrf(AverageLong, 4, 0, tempStr);
    sprintf(cstr, "A6h%s", tempStr);

   // sprintf(cstr, "A6 %04d", AverageLong);
    u8x8.drawString(8, 1, cstr);
    
    //if (co2 > MaxCo2TwoHours)
    //{
//      MaxCo2TwoHours = co2;      
    //}

    //if ((co2 > 400) && (co2 < MinCo2TwoHours))
    //{
      //MinCo2TwoHours = co2;      
    //}

    //sprintf(cstr, "3h %04d", MinCo2TwoHours);
    //u8x8.drawString(9, 1, cstr);

    //sprintf(cstr, "3h %04d", MaxCo2TwoHours);
    //u8x8.drawString(9, 2, cstr);

    //NumberOfTicks++;
    //if (NumberOfTicks >= TwoHours)
    //{
      // MaxCo2TwoHours = 0;
       //MinCo2TwoHours = 5000;
       //NumberOfTicks = 0;
    //}

    nextCycle:
    {}
  }
  else
  {
    Serial.print(F("Error, code: "));
    Serial.println(response);
  }
  
  delay(OneTick);
}
