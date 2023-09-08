#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <SPI.h>
#include <SD.h>

//All data pins
const int tempPin = 4;
const int btnPin = 2;
const int soilOn = 7;
const int chipSelect = 10;
const int soilPin = A0;
const int photoPin = A1;
const int pump1 = 8;
const int pump2 = 9;
const int fan1 = 5;
const int fan2 = 6;
const int fanReg = 3; 


LiquidCrystal_I2C lcd(0x27, 16, 2); //LCD screen setup

File myFile; //File Setup

//Temp&Humid Setup
#define DHTTYPE DHT11 
DHT dht(tempPin, DHTTYPE);

//Variables for different sensors
int scrset = -1;
int scrLast;
int laststate;
int perc;
float temp;
float humid;
float value;
float photoData;
bool wasWater = false;
bool water = false;

//ScreenOn Variable
bool isScreenOn = false;

//millis commands
unsigned long currentMillis = 0;
unsigned long previousSDMillis = 0;
unsigned long scrOffCalculate = 0;
unsigned long previousWater =0;
unsigned long previousFan =0;

//millis intervals
const int hourCheck = 18000;
//3600000
const int screenTimeCheck = 18000;
const int waterTime = 10000;
const int fanTime = 30000;

void setup() {
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  Serial.begin(9600);
  dht.begin();

  pinMode(btnPin, INPUT);
  pinMode(soilPin, INPUT);
  pinMode(photoPin, INPUT);

  pinMode(chipSelect, OUTPUT);

  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  
  pinMode(fan1, OUTPUT);
  pinMode(fan2, OUTPUT);
  pinMode(fanReg, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 scrOnCheck();
}

 void scrSelect()
 {
  
    if (scrset == 0)
    {
      lastscr();
      lcd.print("AAAAAA");

   }

    if(scrset == 1) //Displays Current temperature
    {
      lastscr();
      temp = dht.readTemperature();  

      lcd.setCursor(2,0);
      lcd.print("Temperature:");
      lcd.setCursor(2,1);   //Move cursor to character 2 on line 1
      lcd.print(temp);
    }

    if (scrset == 2) // Displays current Humidity
    {
      lastscr();
      humid = dht.readHumidity();

      lcd.setCursor(2,0);
      lcd.print("Humidity:");
      lcd.setCursor(2,1);   //Set cursor to character 2 on line 0
      lcd.print(humid);
   }

    if (scrset == 3) // Displays Soil Moisture
    {
      lastscr();
      digitalWrite(soilOn, HIGH);
      value = analogRead(soilPin);
      int per = map(value, 0, 1023, 0, 100);
    
      lcd.setCursor(1,0);
      lcd.print("Soil Moisture:");
      lcd.setCursor(2,1);
      lcd.print(per);
      digitalWrite(soilOn, LOW);
   }

    if (scrset == 4) // Displays Amount of light in volts
    {
      lastscr();
      float photoData = analogRead(photoPin);
      
      float voltage = (float)photoData * 5 / 1023;
      lcd.setCursor(1,0);
      lcd.print("Sun in volts:");
      lcd.setCursor(2,1);
      lcd.print(voltage);
    }

    if(scrset == 5) // Displays when plant last watered (in hours)
    {
      lastscr();
      currentMillis = millis();
      float minsSinceWater = (currentMillis - previousWater) / 60000;

      lcd.setCursor(0,0);
      lcd.print("Last Watered:");
      lcd.setCursor(0,1);
      lcd.print(minsSinceWater);
       Serial.println("5");
    }

    if (scrset == 6)// Displays when data was last recorded
    {
      lastscr();
     currentMillis = millis();
    float minsSinceSD = (currentMillis - previousSDMillis) / 60000;
     
      lcd.setCursor(0,0);
      lcd.print("Last Logged:");
      lcd.setCursor(0,1);
      lcd.print(minsSinceSD);
     Serial.println("6");
    }
 }

void whatScreen()
{
  int look = digitalRead(btnPin);
  
  if (look != laststate)
  {
    if (look == 1)
    {
      currentMillis = millis();
      scrOffCalculate = currentMillis;
      scrset += 1;
      if (scrset >= 7)
      {
        scrset = 0;
      }
      Serial.println(scrset);
    }
    laststate = look;
  }
}

void lastscr()
{
  if (scrLast != scrset)
  {
    lcd.clear();
    scrLast = scrset;
  }
}

void scrOnCheck()
{
  if (isScreenOn == false)
  {
    lcd.clear();
    lcd.noBacklight();
    if(digitalRead(btnPin) == HIGH)
    {
      isScreenOn = true;
      scrOffCalculate = currentMillis;
    }
  }
  if(isScreenOn == true)
  {
    currentMillis = millis();
    lcd.backlight();
    whatScreen();
    scrSelect();
    if (currentMillis - scrOffCalculate >= screenTimeCheck)
    {
        isScreenOn = false;
    }
  }
}