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
bool wasWater = false;
bool water = false;
String status = "good";

//ScreenOn Variable
bool isScreenOn = false;

//millis commands
unsigned long currentMillis = 0;
unsigned long previousSDMillis = 0;
unsigned long scrOffCalculate = 0;
unsigned long previousWater =0;
unsigned long previousFan =0;

//millis intervals
const int hourCheck = 3600000;
const int screenTimeCheck = 18000;
const int waterTime = 5000;
const int fanTime = 15000;

// clockread variable
char timeStamp[128];

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
  SDCheck();
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
      currentMillis = millis();
      float minsSinceWater = (currentMillis - previousWater) / 60000;

      lcd.setCursor(0,0);
      lcd.print("Last Watered:");
      lcd.setCursor(0,1);
      lcd.print(minsSinceWater);
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
      scrset %= 8;
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
        scrset = -1;    
    }
  }
}
//SD CARD FUNCTIONS
void SDWRITE()
{
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
  clockRead();
  myFile.print(timeStamp);
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
  // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void SDCheck()
{
 currentMillis = millis();
  if (currentMillis - previousSDMillis >= hourCheck)
    {
      SDWRITE();
      plantCheck();
      previousSDMillis = currentMillis;
      status = "read";
    }
}
//Plant Functions
void plantCheck()
{
   if(wasWater != true)
      {
        if (perc <= 25)
       {
        waterPlant();
        wasWater = true;
        status = "dry";
       } 
      }
    else if (wasWater == true)
    {
        wasWater = false;
        status = "wet";
    }

  if (temp >= 24 || humid >= 55)
  {
    digitalWrite(fanReg, 93);
    fanPlant();
    status = "fan";
  }
  if (temp >= 30 || temp <= -2 || humid >= 75 || humid <= 0 || perc >= 60 || perc <= 0)
  {
    status = "dead";
  }
}

void moistureRead(){
    // Turn on our power for the moisture sensor
    digitalWrite(soilOn, HIGH);
    delay(10);
    perc = map(analogRead(soilPin), 0, 800, 0, 100);
    delay(10);
    digitalWrite(soilOn, LOW);
}

void fanPlant()
{
  digitalWrite(fan1, HIGH);
  digitalWrite(fan2, LOW);
  previousFan = millis();
}

void waterPlant()
{
  water = true;
  digitalWrite(pump1, HIGH);
  digitalWrite(pump2, LOW);
  previousWater = millis();
}
//CLOCK
void clockRead()
{
  uint16_t getTimeBuff[7] = {0};

  DS1307.getTime(getTimeBuff);
  sprintf(timeStamp, "%d/%d/%d,%d:%d:%d,",
            getTimeBuff[6],
            getTimeBuff[5],
            getTimeBuff[4],
            getTimeBuff[2],
            getTimeBuff[1],
            getTimeBuff[0]
            );
}
//STOP EVERYTHING
void stopAll()
{
 if (water == true)
 {
 if (currentMillis - previousWater >= waterTime)
  {
  digitalWrite(pump1, LOW);
  digitalWrite(pump2, LOW);
  water = false;
  }
 }
  if (currentMillis - previousFan >= fanTime)
  {
  digitalWrite(fan1, LOW);
  digitalWrite(fan2, LOW);
  }
}
