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
int scrlast;
int laststate;
int perc;
float temp;
float humid;
float value;
float photoData;
bool wasWater = false;

//ScreenOn Variable
bool isScreenOn = false;

//millis commands
unsigned long currentMillis = 0;
unsigned long previousSDMillis = 0;
unsigned long scrOffCalculate = 0;
unsigned long lastWatered = 0;
unsigned long previousWater =0;

//millis intervals
const int hourCheck = 3600000;
const int screenTimeCheck = 18000;
const int waterTime = 10000;

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
  digitalWrite(fanReg, 95);

   if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  SDWRITE();
}

void loop() 
{
  SDCheck();
  scrOnCheck();
}

 void scrSelect()
 {
  switch (scrset) 
  {
    case 0:
      lastscr(scrlast);
      lcd.print("AAAAAA");

    break;

    case 1: //Displays Current temperature
      lastscr(scrlast);
      temp = dht.readTemperature();  

      lcd.setCursor(2,0);
      lcd.print("Temperature:");
      lcd.setCursor(2,1);   //Move cursor to character 2 on line 1
      lcd.print(temp);
    break;

    case 2: // Displays current Humidity
      lastscr(scrlast);
      humid = dht.readHumidity();

      lcd.setCursor(2,0);
      lcd.print("Humidity:");
      lcd.setCursor(2,1);   //Set cursor to character 2 on line 0
      lcd.print(humid);
    break;

    case 3: // Displays Soil Moisture
      lastscr(scrlast);
      digitalWrite(soilOn, HIGH);
      value = analogRead(soilPin);
      int per = map(value, 0, 1023, 0, 100);
    
      lcd.setCursor(1,0);
      lcd.print("Soil Moisture:");
      lcd.setCursor(2,1);
      lcd.print(per);
      lcd.print("%");
      digitalWrite(soilOn, LOW);
    break;

    case 4: // Displays Amount of light in volts
      lastscr(scrlast);
      float photoData = analogRead(photoPin);
      
      float voltage = (float)photoData * 5 / 1023;
      lcd.setCursor(1,0);
      lcd.print("Sun in volts:");
      lcd.setCursor(2,1);
      lcd.print(voltage);
    break;

    case 5: // Displays when plant last watered (in hours)
      lastscr(scrlast);



    break;

    case 6: // Displays when data was last recorded
      lastscr(scrlast);
     currentMillis = millis();
    float minsSinceSD = (currentMillis - previousSDMillis) / 60000;
     
      lcd.setCursor(0,0);
      lcd.print("Last Logged:");
      lcd.setCursor(0,1);
      lcd.print(minsSinceSD);

    break;

    default:
      lastscr(scrlast);

      lcd.setCursor(0,0);
      lcd.print("Plantkiller 3000");
      lcd.setCursor(0,1);
      lcd.print("Reginald is well");

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
      currentMillis = millis();
      scrOffCalculate = currentMillis;
      scrset += 1;
      scrset %= 7;
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

void SDWRITE()
{
  temp = dht.readTemperature();  
  humid = dht.readHumidity();

  digitalWrite(soilOn, HIGH);
  value = analogRead(soilPin);
  perc = map(value, 0, 1023, 0, 100);
  digitalWrite(soilOn, LOW);

  float photoData = analogRead(photoPin);
  float voltage = (float)photoData * 5 / 1023;
while(!myFile)
{
  myFile.close();
  myFile = SD.open("data.txt", FILE_WRITE);
}

  if (myFile) {
    Serial.print("Writing to test.txt...");
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
    }
}

void plantCheck()
{
   if(wasWater != true)
      {
        if (perc <= 25)
       {
        waterPlant();
        wasWater = true;
       } 
      }
    else if (wasWater == true)
    {
        wasWater = false;
    }
}

void waterPlant()
{
  digitalWrite(pump1, HIGH);
  digitalWrite(pump2, LOW);
  previousWater = millis();
  while (!((currentMillis - previousWater) <= waterTime))
  {
    currentMillis = millis();
  }
  digitalWrite(pump1, LOW);
  digitalWrite(pump2, LOW);
}
