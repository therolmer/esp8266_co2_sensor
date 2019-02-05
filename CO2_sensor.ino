#include <SoftwareSerial.h>
#include "MHZ19.h"
#include <U8g2lib.h>

U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(/* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);

SoftwareSerial ss(12,13);
MHZ19 mhz(&ss);

int Average10Minutes = 0;

int MaxCo2 = 1;
int MinCo2 = 5000;

int MaxCo2ThreeHours = 1;
int MinCo2ThreeHours = 5000;

int OneTick = 10000;
int NumberOfTicks = 0;
int NumberOfTicksForAverage = 0;
int NumberOfInitTicks = 0;
int ThreeHours = 360 * 3;
int TenMinutes = 600;

void setup()
{
  Serial.begin(115200);
  Serial.println(F("Starting..."));

  ss.begin(9600);
  mhz.setAutoCalibration(false);
  u8x8.begin();

  delay(5000);
}

void loop()
{
  MHZ19_RESULT response = mhz.retrieveData();
  if (response == MHZ19_RESULT_OK)
  {
    //Serial.print(F("CO2: "));
   // Serial.println(mhz.getCO2());
    //Serial.print(F("Min CO2: "));
    //Serial.println(mhz.getMinCO2());
   //Serial.print(F("Temperature: "));
    //Serial.println(mhz.getTemperature());
    Serial.print(F("Accuracy: "));
    Serial.println(mhz.getAccuracy());

    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    char cstr[16];
    int co2;

    NumberOfInitTicks++;
    if (NumberOfInitTicks < 20)
    {
      u8x8.drawString(0, 2, "Calibration");
      goto nextCycle;
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

    sprintf(cstr, "Min %04d", MinCo2);
    u8x8.drawString(0, 1, cstr);

    sprintf(cstr, "Max %04d", MaxCo2);
    u8x8.drawString(0, 2, cstr);

    NumberOfTicksForAverage++;
    Average10Minutes = Average10Minutes + co2;
    sprintf(cstr, "A %04d", Average10Minutes/NumberOfTicksForAverage);
    u8x8.drawString(9, 0, cstr);
    
    if (NumberOfTicksForAverage >= TenMinutes)
    {
       Average10Minutes = 0;
       NumberOfTicksForAverage = 0;
    }

    if (co2 > MaxCo2ThreeHours)
    {
      MaxCo2ThreeHours = co2;      
    }

    if ((co2 > 400) && (co2 < MinCo2ThreeHours))
    {
      MinCo2ThreeHours = co2;      
    }

    sprintf(cstr, "3h %04d", MinCo2ThreeHours);
    u8x8.drawString(9, 1, cstr);

    sprintf(cstr, "3h %04d", MaxCo2ThreeHours);
    u8x8.drawString(9, 2, cstr);

    NumberOfTicks++;
    if (NumberOfTicks >= ThreeHours)
    {
       MaxCo2ThreeHours = 0;
       MinCo2ThreeHours = 5000;
       NumberOfTicks = 0;
    }

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
