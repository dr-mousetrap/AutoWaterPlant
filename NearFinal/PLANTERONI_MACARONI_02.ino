#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <SPI.h>
#include <SD.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

File myFile;

const int tempPin = 4;
const int btnPin = 2;
const int soilOn = 5;
const int chipSelect = 10;
const int soilPin = A0;
const int photoPin = A1;

#define DHTTYPE DHT11

DHT dht(tempPin, DHTTYPE);

int scrset = -1;
int scrlast;
int laststate;
float temp;
float humid;
float value;
float photoData;

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

  digitalWrite(soilOn, LOW);

   if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  SDWRITE();
}

void loop() 
{
  whatScreen();
  scrSelect();
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

      lcd.setCursor(1,0);
      lcd.print("Soil Moisture:");
      lcd.setCursor(2,1);
      lcd.print(value);
      
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

void SDWRITE()
{
  temp = dht.readTemperature();  
  humid = dht.readHumidity();

  digitalWrite(soilOn, HIGH);
  value = analogRead(soilPin);
  digitalWrite(soilOn, LOW);

  float photoData = analogRead(photoPin);
  float voltage = (float)photoData * 5 / 1023;

  myFile = SD.open("data.txt", FILE_WRITE);

  if (myFile) {
    Serial.print("Writing to test.txt...");
	//Temperature
	myFile.print(temp);
	myFile.print(", ");
	//Humidity
	myFile.print(humid);
	myFile.print(", ");
  //Soil Moisture
  myFile.print(value);
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
