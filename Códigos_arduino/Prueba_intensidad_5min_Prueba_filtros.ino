const int sensorPin = A1;
int sinalparaorele = 13; 
const unsigned long duracionMuestreo = 300000; // 5 minutos en ms
unsigned long tiempoInicio;

// Configuración de filtros (selección por Serial)
#define FILTRO_NINGUNO 0
#define FILTRO_MEDIA_MOVIL 1
#define FILTRO_KALMAN 2
#define FILTRO_EXPONENCIAL 3
byte filtroActivo = FILTRO_MEDIA_MOVIL; // Filtro inicial

// 1. Configuración Media Móvil
const int ventanaMediaMovil = 20;
float bufferMediaMovil[ventanaMediaMovil];
int indiceMediaMovil = 0;
float sumaMediaMovil = 0;

// 2. Configuración Kalman
float voltajeKalman = 0;
float P = 2.0, Q = 0.04, R = 0.4; // Parámetros Kalman

// 3. Configuración Filtro Exponencial
float voltajeExp = 0;
const float alpha = 0.1; // Factor de suavizado (0.1 a 0.3)

void setup() {
  Serial.begin(9600);
  tiempoInicio = millis();
  
  Serial.println("Sistema listo. Envia por Serial:");
  Serial.println("1 -> Media Móvil (10 muestras)");
  Serial.println("2 -> Kalman (Q=0.01, R=0.1)");
  Serial.println("3 -> Filtro Exponencial (alpha=0.2)");
  Serial.println("0 -> Sin filtro");
  Serial.println("\nFormato de datos:");
  Serial.println("Tiempo(ms)\tCrudo\tMediaMovil\tKalman\tExponencial");

  pinMode(sinalparaorele, OUTPUT); //Define o pino como saida
  
}

void loop() {
  digitalWrite(sinalparaorele, HIGH);
  // Cambiar filtro por Serial
  if (Serial.available() > 0) {
    char opcion = Serial.read();
    switch (opcion) {
      case '1': filtroActivo = FILTRO_MEDIA_MOVIL; break;
      case '2': filtroActivo = FILTRO_KALMAN; break;
      case '3': filtroActivo = FILTRO_EXPONENCIAL; break;
      case '0': filtroActivo = FILTRO_NINGUNO; break;
    }
  }

  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoInicio < duracionMuestreo) {
    float voltajeRaw = analogRead(sensorPin) * (5.0 / 1023.0);
    
    // Aplicar todos los filtros (para comparación)
    float mm = mediaMovil(voltajeRaw);
    float kl = kalman(voltajeRaw);
    float ex = exponencial(voltajeRaw);
    
    // Enviar datos (formato CSV)
    Serial.print(tiempoActual - tiempoInicio); // Tiempo
    Serial.print("\t");
    Serial.print(0); //sp
    Serial.print("\t");
    Serial.print(voltajeRaw, 3);  // Crudo
    Serial.print("\t");
    Serial.print(mm, 3);          // Media Móvil
    Serial.print("\t");
    Serial.print(kl, 3);          // Kalman
    Serial.print("\t");
    Serial.println(ex, 3);        // Exponencial

    delay(10);
  } else {
    Serial.println("Muestreo completado.");
    while(true);
  }
}

// 1. Media Móvil (eficiente)
float mediaMovil(float nuevaMuestra) {
  sumaMediaMovil -= bufferMediaMovil[indiceMediaMovil];
  sumaMediaMovil += nuevaMuestra;
  bufferMediaMovil[indiceMediaMovil] = nuevaMuestra;
  indiceMediaMovil = (indiceMediaMovil + 1) % ventanaMediaMovil;
  return sumaMediaMovil / ventanaMediaMovil;
}

// 2. Kalman
float kalman(float nuevaMuestra) {
  P += Q;
  float K = P / (P + R);
  voltajeKalman += K * (nuevaMuestra - voltajeKalman);
  P *= (1 - K);
  return voltajeKalman;
}

// 3. Filtro Exponencial
float exponencial(float nuevaMuestra) {
  voltajeExp = alpha * nuevaMuestra + (1 - alpha) * voltajeExp;
  return voltajeExp;
}