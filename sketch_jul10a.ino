#include <SoftwareSerial.h>

const int ledPin = 13;
int emgPin = A0;
int emgValue = 0;
int maxAmplitude = 0;
int umbral = 0;
unsigned long startTime = 0;
unsigned long ventanaStartTime = 0;
unsigned long haypico = 0;
boolean umbralCalculado = false;
int episodios = 0;
boolean diagnostico = false;
boolean ultimaventana = false;
float frecMC = 0;
float TiempoTotal = 0;
String msg = "";
float T = 0;
SoftwareSerial bluetooth(10, 11);

const int sampleRate = 250; 
const int duration = 5; 
const int numSamples = sampleRate * duration;
byte emgValues[numSamples];  

const int duration2 = 15; 
const int numSamples2 = sampleRate * duration2;
byte emgValues2[numSamples];
int sampleIndex = 0;

void setup() {
  bluetooth.begin(9600);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  while (!bluetooth) {
    bluetooth.println("noooo");
  }
}

boolean pruebaterminada = false;
boolean leerBluetooth = true;

void loop() {
  if (leerBluetooth && bluetooth.available() > 0) {
    char incomingChar = bluetooth.read();
    digitalWrite(ledPin, HIGH);
    if (incomingChar != '\n') {
      msg += incomingChar;
    } else {
      handleIncomingMessage(msg);
      msg = "";
    }
  }
}

void wait(int wait) { 
  unsigned long myPrevMillis = millis();
  while (millis() - myPrevMillis <= wait);
}


void handleIncomingMessage(String msg) {
  if (!pruebaterminada) {
    if (msg == "1") {
      maxAmplitude = 0;
      sampleIndex = 0;
      startTime = millis();
      while (millis() - startTime < 5000) {
        emgValue = analogRead(emgPin);
        emgValues[sampleIndex++] = emgValue / 4; // Scale down to fit byte range (0-255)
        if (emgValue > maxAmplitude) {
          maxAmplitude = emgValue;
        }
        wait(4);
      }
      umbral = maxAmplitude * 0.4;
      umbralCalculado = true;

      // Enviar los valores de EMG
      bluetooth.print("EMG:");
      for (int i = 0; i < numSamples; i++) {
        bluetooth.print(emgValues[i]);
        if (i < numSamples - 1) {
          bluetooth.print(",");
        }
      }
      bluetooth.print("\n");

      // Enviar el umbral
      bluetooth.print("UMB:" + String(umbral) + "\n");
      digitalWrite(ledPin, HIGH);
    } 

    if (umbralCalculado && msg == "2" && !ultimaventana) {
      leerBluetooth = false;  // Dejar de leer mensajes Bluetooth

      unsigned long tiempoInicio = millis();
      unsigned long tiempoFinal;
      boolean primerpico = false;
      wait(1800000); // espero 30 minutos
      
      while (!ultimaventana) {
        ventanaStartTime = millis();
        boolean necesitoventana15 = false;
        while (millis() - ventanaStartTime < 44 && necesitoventana15 = false) { // 11 muestras aprox
          emgValue = analogRead(emgPin);
          if ((emgValue > umbral) // && !primerpico)
           { 
             primerpico = true;
             episodios++;
             necesitoventana15 = true; 
          }
        }
        if (necesitoventana15 = true){
          ventanapico = millis();
          sampleIndex = 0;
          while (millis() - ventanapico < 15001 ){
                emgValue = analogRead(emgPin);
                emgValues2[sampleIndex++] = emgValue / 4;
                if ((emgValue > umbral) && primerpico) {
                  if ((millis() - ventanapico) >= 5000) {
                    episodios ++;
                    ventanapico = millis();
                  }
                }
          }
          bluetooth.print("EMG:");
          for (int i = 0; i < numSamples; i++) {
            bluetooth.print(emgValues[i]);
              if (i < numSamples - 1) {
                bluetooth.print(",");
              }
          }   
          bluetooth.print("\n");
          primerpico = false;
          necesitoventana15 = false;
        }
        wait(5000);
        if (millis() - ventanaStartTime > 28800000){ //a partir de las 8 horas de estudio vuelvo a leer bluetooth para esperar el final '3'
          leerBluetooth = true;
          if (msg == "3") {
            ultimaventana = true;
            tiempoFinal = millis();
            break;
        }
        }

        }

        
      }

      TiempoTotal = (tiempoFinal - tiempoInicio) / 1000.0;
      T =  TiempoTotal / 3600;
      frecMC = (episodios / T); // lo pasamos a horas
      if (frecMC >= 10.17) {
        diagnostico = true;
      } else {
        diagnostico = false;
      }
      pruebaterminada = true;
    }
  }

  if (pruebaterminada && msg == "3") {
    leerBluetooth = false;
    int horas = int(TiempoTotal) / 3600;
    int minutos = (int(TiempoTotal) % 3600) / 60;
    int segundos = int(TiempoTotal) % 60;

    String tiempoStr = String(horas) + ":" + (minutos < 10 ? "0" : "") + String(minutos) + ":" + (segundos < 10 ? "0" : "") + String(segundos);
    if (diagnostico) {
      bluetooth.print("T:" + tiempoStr + "\n");
      bluetooth.print("EP:" + String(episodios) + "\n");
      bluetooth.print("FREQ:" + String(frecMC) + "\n");
      bluetooth.print("DIAG:1\n");
    } else {
      bluetooth.print("T:" + tiempoStr + "\n");
      bluetooth.print("EP:" + String(episodios) + "\n");
      bluetooth.print("FREQ:" + String(frecMC) + "\n");
      bluetooth.print("DIAG:0\n");
    }
    while (true) {
      // Permanecer aquí indefinidamente
    }
  }
}

