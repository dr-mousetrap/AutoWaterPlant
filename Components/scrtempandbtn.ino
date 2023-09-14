#include <LiquidCrystal_I2C.h>
#include "DHT.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int tempPin = 4;
const int btnPin = 2;

#define DHTTYPE DHT11

DHT dht(tempPin, DHTTYPE);

int scrset = -1;
int scrlast;
int laststate;
float temp;
float humid;

void setup() {
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  Serial.begin(9600);
  dht.begin();

  pinMode(btnPin, INPUT);

}

void loop() 
{
  whatScreen();
  scrSelect();
  Serial.println(scrset);
}

 void scrSelect()
 {
  switch (scrset) 
  {
    case 0:
      lastscr(scrlast);
      lcd.print("AAAAAA");

    break;

    case 1: //temp screen
      lastscr(scrlast);
      temp = dht.readTemperature();  

      lcd.setCursor(2,0);
      lcd.print("Temperature:");
      lcd.setCursor(2,1);   //Move cursor to character 2 on line 1
      lcd.print(temp);
    break;

    case 2: 
      lastscr(scrlast);
      humid = dht.readHumidity();

      lcd.setCursor(2,0);
      lcd.print("Humidity:");
      lcd.setCursor(2,1);   //Set cursor to character 2 on line 0
      lcd.print(humid);
    break;

    case 3:
      lastscr(scrlast);

      lcd.setCursor(1,0);
      lcd.print("Soil Moisture:");
    break;

    default:
      lastscr(scrlast);

      lcd.setCursor(0,0);
      lcd.print("Hello Reginald!");
      lcd.setCursor(0,1);
      lcd.print("Barry is well.");

    break;
  }
 }


void whatScreen()
{
  int look = digitalRead(btnPin);
  
  if (look != laststate)
  {
    if (look == 1)
    {
      scrset += 1;
      scrset %= 4;
    }
    laststate = look;
  }
}

void lastscr(int)
{
  if (scrlast != scrset)
  {
    lcd.clear();
    scrlast = scrset;
  }
}
