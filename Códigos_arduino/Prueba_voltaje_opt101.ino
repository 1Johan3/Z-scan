const int sensorPin = A1;   
int sinalparaorele = 13; 

void setup() {
  Serial.begin(9600);
  pinMode(sinalparaorele, OUTPUT); //Define o pino como saida
}

void loop() {
  // read the value from the sensor:
  int sensorValue = analogRead(sensorPin);
  digitalWrite(sinalparaorele, HIGH);

  float voltaje = sensorValue * (5.0 / 1023.0);
  Serial.print(sensorValue); // Tiempo
   Serial.print("\t");
  Serial.print("Voltaje: ");
  Serial.println(voltaje);
  delay(50);
}