#include "Stepper.h"

const int Pasos_Revolucion_360 = 2048;
const int Pasos_Revolucion_90 = Pasos_Revolucion_360 / 4;
Stepper myStepper = Stepper(Pasos_Revolucion_360, 12, 10, 11, 9);

const int sensorPin = A0;

int centimetros;
int Muestras;
int tiempo_antes_volver;

void setup() {
  myStepper.setSpeed(17); // Velocidad del motor en rpm
  Serial.begin(9600); // Inicia comunicación serial
}

void loop() {
  // Pedir los 3 valores al usuario antes de aceptar "start"
  Serial.println("Ingrese la distancia en cm:");
  while (Serial.available() == 0) {}
  centimetros = Serial.readStringUntil('\n').toInt();

  Serial.println("Numero de muestras tomadas por paso:");
  while (Serial.available() == 0) {}
  Muestras = Serial.readStringUntil('\n').toInt();

  Serial.println("Ingrese el tiempo antes de volver (segundos):");
  while (Serial.available() == 0) {}
  tiempo_antes_volver = Serial.readStringUntil('\n').toInt();

  Serial.println("Ingrese 'start' para iniciar el proceso.");
  while (Serial.available() == 0 || Serial.readStringUntil('\n') != "start") {}

  // Calcular desplazamiento y tiempos en milisegundos
  int desplazamiento = centimetros * 5;
  int delay_antes_volver = tiempo_antes_volver * 1000;

  Serial.println("Regresando la mitad de los ciclos de forma continua...");
  for (int ciclo = 0; ciclo < (desplazamiento / 2); ciclo++) {  
    myStepper.step(-Pasos_Revolucion_360);  
  }

  Serial.println("Esperando antes de iniciar mediciones en el extremo lejano...");
  delay(delay_antes_volver);

  Serial.println("Iniciando mediciones con ciclos de giros de 90° con pausa...");
  for (int ciclo = 0; ciclo < desplazamiento; ciclo++) {  
    Serial.println(" ");
    Serial.print("Ciclo: ");
    Serial.println(ciclo + 1);
    Serial.println(" ");

    for (int i = 0; i < 4; i++) {  
      Serial.print("Giro de 90° - Paso ");
      Serial.println(i + 1);
      myStepper.step(Pasos_Revolucion_90);  
      delay(100);  

      // Leer y mostrar 3 valores de voltaje
      Serial.println("Lecturas de voltaje:");
      for (int j = 0; j < Muestras; j++) {
        int sensorValue = analogRead(sensorPin);
        float voltaje = sensorValue * (5.0 / 1023.0);
        Serial.print("Voltaje ");
        Serial.print(j + 1);
        Serial.print(": ");
        Serial.println(voltaje);
        delay(100);
      }
    }
  }

  Serial.println("Esperando antes de volver al punto cero...");
  delay(delay_antes_volver);

  Serial.println("Regresando a punto cero de forma continua...");
  for (int ciclo = 0; ciclo < (desplazamiento / 2); ciclo++) {  
    myStepper.step(-Pasos_Revolucion_360);  
  }

  Serial.println("Ciclo completo. Ingrese nuevos valores para repetir.");
}