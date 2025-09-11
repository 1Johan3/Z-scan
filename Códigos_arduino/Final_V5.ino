#include <Stepper.h>

// ======== Configuración Global ======== //
const int SENSOR_PIN = A1;
const int PIN_RELE = 2;
const int PIN_LED_CALIB = 4;
const unsigned long TIEMPO_VERIFICACION_OSCURIDAD = 10000; // 10 segundos
const unsigned long TIEMPO_VERIFICACION_INTENSIDAD = 60000; // 1 minuto
const float MARGEN_ERROR = 0.2;
const int TIEMPO_MUESTREO_MS = 50; 

const int Pasos_Revolucion_360 = 2048;
const int Pasos_Revolucion_90 = Pasos_Revolucion_360 / 4;
const float cm_por_paso_90 = 0.05; // 1 cm = 5 pasos -> 90° = 0.2 cm

Stepper myStepper(Pasos_Revolucion_360, 12, 10, 11, 9);

// ======== Variables de Estado ======== //
enum EstadoSistema {
  ESPERANDO_INICIO,
  VERIFICANDO_OSCURIDAD,
  VERIFICANDO_INTENSIDAD,
  MEDICION_MOTOR,
  FINALIZADO
};

EstadoSistema estadoActual = ESPERANDO_INICIO;
unsigned long tiempoInicioEstado = 0;
bool calibracionExitosa = false;

// Variables de medición
int centimetros;
int Muestras = 10;
int tiempo_antes_volver = 5;
int delay_entre_pasos = 1000;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_RELE, OUTPUT);
  pinMode(PIN_LED_CALIB, OUTPUT);
  digitalWrite(PIN_RELE, LOW);
  digitalWrite(PIN_LED_CALIB, LOW);

  myStepper.setSpeed(15);
  Serial.println("Sistema listo. Envie 'start' para comenzar.");
}

void loop() {
  switch (estadoActual) {
    case ESPERANDO_INICIO:
      if (Serial.available() && Serial.readStringUntil('\n') == "start") {
        estadoActual = VERIFICANDO_OSCURIDAD;
        tiempoInicioEstado = millis();
        Serial.println("Iniciando verificacion de oscuridad...");
      }
      break;

    case VERIFICANDO_OSCURIDAD:
      verificarOscuridad();
      break;

    case VERIFICANDO_INTENSIDAD:
      verificarIntensidad();
      break;

    case MEDICION_MOTOR:
      ejecutarMedicionMotor();
      break;

    case FINALIZADO:
      estadoActual = ESPERANDO_INICIO;
      calibracionExitosa = false;
      digitalWrite(PIN_LED_CALIB, LOW);
      digitalWrite(PIN_RELE, LOW);
      Serial.println("Sistema reiniciado. Envie 'start' para nuevo ciclo.");
      break;
  }
}

void verificarOscuridad() {
  float voltajeRaw = analogRead(SENSOR_PIN) * (5.0 / 1023.0);
  unsigned long tiempoActual = millis();

  Serial.print((tiempoActual - tiempoInicioEstado) / 1000);
  Serial.print("\t");
  Serial.println(voltajeRaw, 3);

  if (voltajeRaw > MARGEN_ERROR) {
    tiempoInicioEstado = millis();
    Serial.println("¡Luz detectada! Reiniciando verificacion...");
    return;
  }

  if (tiempoActual - tiempoInicioEstado >= TIEMPO_VERIFICACION_OSCURIDAD) {
    Serial.println("¡Oscuridad verificada correctamente!");
    estadoActual = VERIFICANDO_INTENSIDAD;
    tiempoInicioEstado = millis();
    digitalWrite(PIN_RELE, HIGH); // Activa láser
  }
  delay(100);
}

void verificarIntensidad() {
  float voltajeRaw = analogRead(SENSOR_PIN) * (5.0 / 1023.0);
  
  unsigned long tiempoActual = millis();
  unsigned long tiempoTranscurrido = tiempoActual - tiempoInicioEstado;

  Serial.print((tiempoActual - tiempoInicioEstado) / 1000);
  Serial.print("\t");
  Serial.println(voltajeRaw, 3);

  if (tiempoTranscurrido >= TIEMPO_VERIFICACION_INTENSIDAD) {
    calibracionExitosa = true;
    digitalWrite(PIN_LED_CALIB, HIGH);
    Serial.println("¡Calibración completada con éxito!");
    delay(2000);
    estadoActual = MEDICION_MOTOR;
    solicitarParametrosMotor();
  }
  delay(TIEMPO_MUESTREO_MS);
}

void solicitarParametrosMotor() {
  Serial.println("Ingrese la distancia en cm:");
  while (Serial.available() == 0) {}
  centimetros = Serial.readStringUntil('\n').toInt();
}

void ejecutarMedicionMotor() {
  int desplazamiento = centimetros * 5;
  int delay_antes_volver = tiempo_antes_volver * 1000;

  // Posición inicial en cm
  float posicion = -(centimetros / 2.0);

  Serial.println("Regresando a posición inicial...");
  for (int ciclo = 0; ciclo < (desplazamiento / 2); ciclo++) {
    myStepper.step(-Pasos_Revolucion_360);
    delay(delay_entre_pasos);
  }
  delay(delay_antes_volver);

  Serial.println("Iniciando mediciones...\nMuestra, POS(cm), Voltaje(V)");

  for (int ciclo = 0; ciclo < desplazamiento; ciclo++) {
    for (int i = 0; i < 4; i++) {
      myStepper.step(Pasos_Revolucion_90);
      posicion += cm_por_paso_90;
      delay(delay_entre_pasos);

      for (int j = 0; j < Muestras; j++) {
        float voltaje = analogRead(SENSOR_PIN) * (5.0 / 1023.0);
        Serial.print(posicion, 2);
        Serial.print("\t");
        Serial.println(voltaje, 3);
        delay(TIEMPO_MUESTREO_MS);
      }
    }
  }

  Serial.println("Regresando a punto cero...");
  for (int ciclo = 0; ciclo < (desplazamiento / 2); ciclo++) {
    myStepper.step(-Pasos_Revolucion_360);
    delay(delay_entre_pasos);
  }

  Serial.println("¡Medición completada!");
  estadoActual = FINALIZADO;
}
