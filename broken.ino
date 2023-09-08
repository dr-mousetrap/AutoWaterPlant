const int fan1 = 5;
const int fan2 = 6;
const int fanENA = 9;

void setup() {
  // put your setup code here, to run once:
  pinMode(fan1, OUTPUT);
  pinMode(fan2, OUTPUT);
  pinMode(fanENA, OUTPUT);

  digitalWrite(fan1, HIGH);
  digitalWrite(fan2, LOW);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i <= 255; i++)
  {
    digitalWrite(fanENA, i);
    Serial.println(i);
    delay(500);
  }
}
