#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <SPI.h>
#include <SD.h>
#include <DFRobot_DS1307.h>

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

DFRobot_DS1307 DS1307;

File myFile; //File Setup

//Temp&Humid Setup
#define DHTTYPE DHT11 
DHT dht(tempPin, DHTTYPE);

//Sd ID
int SDID = 0;

//Variables for different sensors
int scrset = -1;
int scrLast = 0;
int laststate;
int perc;
float temp;
float humid;
float value;
float photoData;
String status = "good";

//ScreenOn Variable
bool isScreenOn = false;

// clockread variable
String outputDate;
String outputTime;
String outputDOW;
int hours;
int mins;
int secs;

int secsStore;
int prevMins;
int lastLog;
int lastWater;

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
  
  digitalWrite(soilOn, LOW);

   if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  SDWRITE();
 
  while( !(DS1307.begin()) ){

    Serial.println("Communication with device failed, please check connection");

    delay(3000);

  }
  DS1307.start();
  DS1307.setSqwPinMode(DS1307.eSquareWave_1Hz);
}

void loop() 
{
  allCheck();
  scrOnCheck();
  stopAll();
}

// SCREEN FUNCTIONS
void scrSelect()
 {
  if (scrset >= 0 && scrset <= 7) 
  {
   
    if (scrset == 0)
    {
      lcd.setCursor(0,0);
      lcd.print("PlantKiller3000");
      lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
      lcd.print("Reginald is");
      lcd.setCursor(12,1);
      lcd.print(status);
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
      moistureRead();
      lcd.setCursor(1,0);
      lcd.print("Soil Moisture:");
      lcd.setCursor(2,1);
      lcd.print(perc);
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
      hours = getTimeBuff[2];
      int scrWaterRead = (hours - lastWater);

      lcd.setCursor(0,0);
      lcd.print("Last Watered:");
      lcd.setCursor(0,1);
      lcd.print(scrWaterRead);
    }

    if (scrset == 6)// Displays when data was last recorded
    {
      lastscr();
     mins = getTimeBuff[1];
     //time since sdcard has been logged (minutes)
     
      lcd.setCursor(0,0);
      lcd.print("Last Logged:");
      lcd.setCursor(0,1);
      lcd.print(minsSinceSD);
    }

    if (scrset == 7) // Displays Current Time
    {
      clockRead();
      lcd.setCursor(0,0);
      lcd.print("Current Time");
      lcd.setCursor(0,1);
      lcd.print(timeStamp);
    }
  } else 
    {
        lastscr();
        lcd.setCursor(0, 0);
        lcd.print("Welcome Person");
        lcd.setCursor(0, 1);
        lcd.print("Reginald is");
        lcd.setCursor(12, 1);
        lcd.print(status);
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
      secsStore = getTimeBuff[0];
    }
  }
  if(isScreenOn == true)
  {
    lcd.backlight();

    int look = digitalRead(btnPin);
  
    if (look != laststate)
    {
     if (look == 1)
     {
       secsStore = getTimeBuff[0];
       scrset += 1;
       scrset %= 8;
     }
     laststate = look;
    }
     
    scrSelect();
    secs = getTimeBuff[0];
    if ((secs - secsStore) >= 17)
    {
        isScreenOn = false;
        scrset = -1;    
    }
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


//SD CARD FUNCTIONS
void SDWRITE()
{
  clockRead();
  temp = dht.readTemperature();  
  humid = dht.readHumidity();
  moistureRead();
  
  float photoData = analogRead(photoPin);
  float voltage = (float)photoData * 5 / 1023;
while(!myFile)
{
  myFile.close();
  myFile = SD.open("data.txt", FILE_WRITE);
}

  if (myFile) {
Serial.print("Writing to test.txt...");
  //Clock
  myFile.print(outputDate + "|" + outputTime + "|" + outputDOW);
  myFile.print(", ");
  //Temperature
  myFile.print(temp);
  myFile.print(", ");
  //Humidity
  myFile.print(humid);
  myFile.print(", ");
  //Soil Moisture
  myFile.print(perc);
  myFile.print(", ");
  //Light Sensor
  myFile.println(voltage);
  // close the file
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

} 
//CheckEverything
void allCheck()
{
  mins = getTimeBuff[1];
  if (mins != prevMins && (mins == 0 || mins == 30 ))
    {
          SDWRITE();
          status = "read";
        if (perc <= 25)
        {
          waterPlant();
          status = "dry";
        } 

        if (temp >= 24 || humid >= 55)
        {
    
          digitalWrite(fanReg, 93);
          fanPlant();
          status = "fan";
        }
    }
}

void moistureRead(){
    // Turn on our power for the moisture sensor
    digitalWrite(soilOn, HIGH);
    delay(500);
    perc = map(analogRead(soilPin), 0, 800, 0, 100);
    delay(500);
    digitalWrite(soilOn, LOW);
}

void fanPlant()
{
  digitalWrite(fan1, HIGH);
  digitalWrite(fan2, LOW);
}

void waterPlant()
{
  water = true;
  digitalWrite(pump1, HIGH);
  digitalWrite(pump2, LOW);
}

//CLOCK
void clockRead()
{
  uint16_t getTimeBuff[7] = {0};

  DS1307.getTime(getTimeBuff);
  outputDate = String(getTimeBuff[6]) + "/" + String(getTimeBuff[5]) + "/" + String(getTimeBuff[4]);
  outputTime = String(getTimeBuff[2]) + ":" + String(getTimeBuff[1]) + ":" + String(getTimeBuff[0]);
  outputDOW = String(getTimeBuff[3]);
}

//STOP EVERYTHING
void stopAll()
{
 {
  secs = getTimeBuff[0];
 if ((secs - prevSecs) >= 10)
  {
  digitalWrite(pump1, LOW);
  digitalWrite(pump2, LOW);
  }
  
  if ((secs - prevSecsFan) >= 25)
  {
  digitalWrite(fan1, LOW);
  digitalWrite(fan2, LOW);
  }
}
