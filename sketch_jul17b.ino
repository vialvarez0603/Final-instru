#include <SoftwareSerial.h>
const int emgPin = A0; // Pin de entrada para el sensor EMG
const int samplingInterval = 20; // Intervalo de muestreo en milisegundos (50 Hz)
SoftwareSerial bluetooth(10, 11);

void setup() {
  bluetooth.begin(9600); // Inicialización de Bluetooth
}

void loop() {
  int emgValue = analogRead(emgPin);
  bluetooth.print(emgValue);
  bluetooth.print(","); // Separador entre valores
  delay(samplingInterval); // Mantener la frecuencia de 50 Hz
}
