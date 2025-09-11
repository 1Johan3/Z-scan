const int sensorPin = A1;
const unsigned long duracionMuestreo = 300000; // 5 minutos en milisegundos (5 * 60 * 1000)
unsigned long tiempoInicio;

void setup() {
  Serial.begin(9600);
  tiempoInicio = millis(); // Guarda el tiempo inicial
}

void loop() {
  unsigned long tiempoActual = millis();
  
  // Verifica si ha pasado el tiempo de muestreo
  if (tiempoActual - tiempoInicio < duracionMuestreo) {
    int sensorValue = analogRead(sensorPin);
    float voltaje = sensorValue * (5.0 / 1023.0);
    
    //Serial.print("Tiempo (ms): ");
    //Serial.print(tiempoActual - tiempoInicio);
    //Serial.print(", Voltaje: ");
    Serial.print(0);
    Serial.print('\t');
    Serial.println(voltaje, 4); // 4 decimales de precisión
    
    delay(50); // Pequeña pausa entre lecturas (ajustable)
  } else {
    Serial.println("Muestreo completado. 5 minutos han transcurrido.");
    while (true); // Detiene el programa (bucle infinito)
  }
}