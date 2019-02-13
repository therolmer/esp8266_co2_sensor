//#define BLYNK_PRINT Serial

#include <SoftwareSerial.h>
#include "MHZ19.h"
#include <U8g2lib.h>

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Ключ для авторизации на Blynk
char auth[] = "fbf8213529004401bef4287215e787b2";

// Имя Wifi сети
char ssid[] = "Hous";
// Пароль от Wifi
char pass[] = "saymellon";

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
int NumberOfInitTicks = 20;

// Среднее значение
float Average = 0;

// Коэффициент усреденения (при тике равном 10 секунд получается скользящее среднее на 30 секунд)
int AverageFactor = 3;

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

  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
  char cstr[16];

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.disconnect();
  }

  delay(5000);
  
  WiFi.begin(ssid, pass);
  
  int attemps = 0;
  while ((WiFi.status() != WL_CONNECTED) || (attemps > 20))
  {
    delay(500);    
    Serial.print(".");
    attemps++;
  }

  if (attemps <= 20)
  {    
    sprintf(cstr, "Wi-Fi connected");
  }
  else
  {
    sprintf(cstr, "Wi-Fi ERROR");
  }

  u8x8.drawString(0, 1, cstr);
  delay(1000);

  Blynk.config(auth);
  Blynk.connect();
}

// Основной рабочий цикл
void loop()
{
  MHZ19_RESULT response = mhz.retrieveData();
  if (response == MHZ19_RESULT_OK)
  {
    //Serial.print(F("Temperature: "));
    //Serial.println(mhz.getTemperature());
        
    Blynk.run();
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
    
    //sprintf(cstr, "C02 %04d",co2);
    //u8x8.drawString(0, 0, cstr);

   
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
    
    Average = Average + ((float)co2 - Average) / min(NumberOfTicksForAverage, AverageFactor);

    char tempStr[4];
    dtostrf(Average, 4, 0, tempStr);
    sprintf(cstr, "A  %s", tempStr);
    u8x8.drawString(0, 0, cstr);

    if (NumberOfTicksForAverageLong <= AverageLongFactor)
    {
      NumberOfTicksForAverageLong++;
    }
    AverageLong = AverageLong + (((float)co2 - AverageLong) / ((float) (min(NumberOfTicksForAverageLong, AverageLongFactor))));

    dtostrf(AverageLong, 4, 0, tempStr);
    sprintf(cstr, "A6h%s", tempStr);

    u8x8.drawString(8, 0, cstr);

    if (InitTicks % 6 == 0)
    {
      Blynk.virtualWrite(V0, co2);  
    }
    
    
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
