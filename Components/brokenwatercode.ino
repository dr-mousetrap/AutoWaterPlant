const int pump1 = 8;
const int pump2 = 9;

unsigned long currentMillis = 0;
unsigned long previousWater =0;

const int waterTime = 10000;


void setup() {
  // put your setup code here, to run once:
pinMode(pump1, OUTPUT);
pinMode(pump2, OUTPUT);
Serial.begin(9600);
}

  // put your main code here, to run repeatedly:
void loop() {
delay(500);
waterPlant();
}

void waterPlant()
{
  Serial.println("Watering");
  digitalWrite(pump1, HIGH);
  digitalWrite(pump2, LOW);
  previousWater = millis();
  while (((currentMillis - previousWater) <= waterTime))
  {
    currentMillis = millis();
  }
  digitalWrite(pump1, LOW);
  digitalWrite(pump2, LOW);
  Serial.println("done Watering");
}
