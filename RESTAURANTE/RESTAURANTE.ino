#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <NewPing.h>
#include <Servo.h>
#include <ArduinoJson.h>

/////////////////////////////////////////////////////////////////////
// COMUNICACION SERIAL ESP32
const int numeroMesas = 6;
String mesas[numeroMesas];
int mesaPresionada = -1;
/////////////////////////////////////////////////////////////////////
// SENSOR ULTRASONICO
#define TRIGGER_PIN 5     // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 4        // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200  // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);  // NewPing setup of pins and maximum distance.
/////////////////////////////////////////////////////////////////////
// SENSOR DE TEMPERATURA DHT11
// Definiciones
#define DHTPIN 2       // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT

// Inicialización del sensor y la pantalla LCD
DHT dht(DHTPIN, DHTTYPE);

/////////////////////////////////////////////////////////////////////
//LCD 1602 I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Dirección I2C (0x27 puede variar)

/////////////////////////////////////////////////////////////////////
// CONTADOR DE VEHICULOS

#define IR_SENSOR_PIN 6

int contador = 0;  // Variable para almacenar el conteo
bool sensorState = false;
bool lastSensorState = false;

// ALARMA BUZZER
int buzzer = 8;
// VENTILADOR
int ventilador = 7;
//SERVOMOTOR
Servo servoPuerta;

/////////////////////////////////////////////////////////////////////
// MESAS
#define mesa_1_V 23
#define mesa_1_R 25
#define pul_mesa_1 27

#define mesa_2_V 29
#define mesa_2_R 31
#define pul_mesa_2 33

#define mesa_3_V 35
#define mesa_3_R 37
#define pul_mesa_3 39

#define mesa_4_V 41
#define mesa_4_R 43
#define pul_mesa_4 45

#define mesa_5_V 22
#define mesa_5_R 24
#define pul_mesa_5 26

#define mesa_6_V 38
#define mesa_6_R 40
#define pul_mesa_6 42



int focosVMesas[numeroMesas] = { mesa_1_V, mesa_2_V, mesa_3_V, mesa_4_V, mesa_5_V, mesa_6_V };
int focosRMesas[numeroMesas] = { mesa_1_R, mesa_2_R, mesa_3_R, mesa_4_R, mesa_5_R, mesa_6_R };
int focosMesasPrograma[numeroMesas] ={10,9,16,17,18,19}; 
/////////////////////////////////////////////////////////////////////

void setup() {
  //mesa 1
  pinMode(mesa_1_V, OUTPUT);   //VERDE
  pinMode(mesa_1_R, OUTPUT);   //ROJO
  pinMode(pul_mesa_1, INPUT);  //PULSADOR MESA 1
  //mesa 2
  pinMode(mesa_2_V, OUTPUT);   //VERDE
  pinMode(mesa_2_R, OUTPUT);   //ROJO
  pinMode(pul_mesa_2, INPUT);  //PULSADOR MESA 2
  //mesa 3
  pinMode(mesa_3_V, OUTPUT);   //VERDE
  pinMode(mesa_3_R, OUTPUT);   //ROJO
  pinMode(pul_mesa_3, INPUT);  //PULSADOR MESA 3
  //mesa 4
  pinMode(mesa_4_V, OUTPUT);   //VERDE
  pinMode(mesa_4_R, OUTPUT);   //ROJO
  pinMode(pul_mesa_4, INPUT);  //PULSADOR MESA 4
  //mesa 5
  pinMode(mesa_5_V, OUTPUT);   //VERDE
  pinMode(mesa_5_R, OUTPUT);   //ROJO
  pinMode(pul_mesa_5, INPUT);  //PULSADOR MESA 5
  //mesa 6
  pinMode(mesa_6_V, OUTPUT);   //VERDE
  pinMode(mesa_6_R, OUTPUT);   //ROJO
  pinMode(pul_mesa_6, INPUT);  //PULSADOR MESA 6


  for (int i = 0; i < 6; i++) {
    pinMode(focosMesasPrograma[i], OUTPUT);
    digitalWrite(focosMesasPrograma[i], LOW);
  }

  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);

  pinMode(buzzer, OUTPUT);
  pinMode(ventilador, OUTPUT);
  servoPuerta.attach(3);

  //CONTADOR GARAJE
  pinMode(IR_SENSOR_PIN, INPUT);  // Configurar el sensor como entrada

  // Inicialización del LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Inicializando...");
  delay(2000);
  lcd.clear();

  digitalWrite(mesa_1_V, HIGH);
  digitalWrite(mesa_2_V, HIGH);
  digitalWrite(mesa_3_V, HIGH);
  digitalWrite(mesa_4_V, HIGH);
  digitalWrite(mesa_5_V, HIGH);
  digitalWrite(mesa_6_V, HIGH);

  digitalWrite(mesa_1_R, LOW);
  digitalWrite(mesa_2_R, LOW);
  digitalWrite(mesa_3_R, LOW);
  digitalWrite(mesa_4_R, LOW);
  digitalWrite(mesa_5_R, LOW);
  digitalWrite(mesa_6_R, LOW);

  // Inicialización del DHT
  dht.begin();
  servoPuerta.write(5);
  delay(2000);
  // Inicialización de comunicacion serial

  Serial.begin(9600);
  Serial3.begin(9600);
  // MESAS DISPONIBLES
  for (int i = 0; i < numeroMesas; i++) {
    mesas[i] = "Disponible";
  }
}

void loop() {
  if (Serial3.available() > 0) {
    String datoRecibido = Serial3.readStringUntil('\n');  //{"reserva":true,"mesa":"4"}
    funciones(datoRecibido);
  }
  Mesa();
  temperatura();
  //luz();
  distancia();
  Contador();
}

void temperatura() {
  // Leer datos del sensor
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  /*// Verificar si hubo error en la lectura
    if (isnan(temperatura) || isnan(humedad)) {
    lcd.clear();
    lcd.print("Error lectura");
    delay(2000);
    return;
    }*/

  // Mostrar temperatura y humedad en la LCD
  lcd.clear();
  lcd.setCursor(0, 0);  // Primera línea
  lcd.print("Temp: ");
  lcd.print(temperatura);
  lcd.print("C");

  lcd.setCursor(0, 1);  // Segunda línea
  lcd.print("Humedad: ");
  lcd.print(humedad);
  lcd.print("%");

  delay(2000);  // Actualización cada 2 segundos
}

void distancia() {
  delay(50);  // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  Serial.print("Ping: ");
  Serial.print(sonar.ping_cm());  // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");
  if (sonar.ping_cm() < 8) {
    servoPuerta.write(90);
    digitalWrite(12,HIGH);
    delay(2000);
    servoPuerta.write(5);
    digitalWrite(12,LOW);
    delay(100);
  }
}

void luz() {
  int sensorValue = analogRead(A0);
  Serial.println(sensorValue);
  delay(10);
}

void Mesa() {

  int mesa1 = digitalRead(pul_mesa_1);
  int mesa2 = digitalRead(pul_mesa_2);
  int mesa3 = digitalRead(pul_mesa_3);
  int mesa4 = digitalRead(pul_mesa_4);
  int mesa5 = digitalRead(pul_mesa_5);
  int mesa6 = digitalRead(pul_mesa_6);
  /*
    Serial.print("mesa1= ");
    Serial.print(mesa1);
    Serial.print(" mesa2= ");
    Serial.print(mesa2);
    Serial.print(" mesa3= ");
    Serial.print(mesa3);
    Serial.print(" mesa4= ");
    Serial.print(mesa4);
    Serial.print(" mesa5= ");
    Serial.print(mesa5);
    Serial.print(" mesa6= ");
    Serial.println(mesa6);
  */

  if (mesa1 == 1) {
    mesas[0] = "Ocupado";
    if (mesaPresionada != 0) {
      Serial3.println("{\"reserva\":true,\"mesa\":\"0\"}");
    }
    mesaPresionada = 0;
    actualizarFocos(0);
  }
  if (mesa2 == 1) {
    mesas[1] = "Ocupado";
    if (mesaPresionada != 1) {
      Serial3.println("{\"reserva\":true,\"mesa\":\"1\"}");
    }
    mesaPresionada = 1;
    actualizarFocos(1);
  }
  if (mesa3 == 1) {
    mesas[2] = "Ocupado";
    if (mesaPresionada != 2) {
      Serial3.println("{\"reserva\":true,\"mesa\":\"2\"}");
    }
    mesaPresionada = 2;
    actualizarFocos(2);
  }
  if (mesa4 == 1) {
    mesas[3] = "Ocupado";
    if (mesaPresionada != 3) {
      Serial3.println("{\"reserva\":true,\"mesa\":\"3\"}");
    }
    mesaPresionada = 3;
    actualizarFocos(3);
  }
  if (mesa5 == 1) {
    mesas[4] = "Ocupado";
    if (mesaPresionada != 4) {
      Serial3.println("{\"reserva\":true,\"mesa\":\"4\"}");
    }
    mesaPresionada = 4;
    actualizarFocos(4);
  }
  if (mesa6 == 1) {
    mesas[5] = "Ocupado";
    if (mesaPresionada != 5) {
      Serial3.println("{\"reserva\":true,\"mesa\":\"5\"}");
    }
    mesaPresionada = 5;
    actualizarFocos(5);
  }

  delay(10);
}

void Contador() {
  // Leer el estado del sensor
  sensorState = digitalRead(IR_SENSOR_PIN);

  // Detectar flanco ascendente (cuando el sensor se activa)
  if (sensorState && !lastSensorState) {
    // Incrementar el contador solo si no ha llegado a 6
    if (contador < 6) {
      contador++;  // Incrementar el contador
      Serial.print("Conteo: ");
      Serial.println(contador);  // Mostrar el conteo en el monitor serial

      // Encender el LED por un momento para indicar detección
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
      digitalWrite(11,HIGH);
      delay(500);
      digitalWrite(11,LOW);


      // Verificar si el contador llega a 6
      if (contador == 6) {
        Serial.println("EL GARAJE ESTA LLENO");
        lcd.clear();
        lcd.setCursor(0, 0);  // Primera línea
        lcd.print(" GARAJE LLENO");
        delay(2000);
      }
    }
  }

  // Actualizar el estado anterior del sensor
  lastSensorState = sensorState;
}

void funciones(String msg) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    return;
  }
  if (doc["reserva"]) {
    String estado = (mesas[doc["mesa"].as<int>()] == "Disponible") ? "Ocupado" : "Disponible";
    mesas[doc["mesa"].as<int>()] = estado;
    Serial.println(String(doc["mesa"].as<int>()) + " " + estado);
    actualizarFocos(doc["mesa"].as<int>());
  }
}
void actualizarFocos(int mesa) {
  if (mesas[mesa] == "Disponible") {
    digitalWrite(focosVMesas[mesa], HIGH);
    digitalWrite(focosRMesas[mesa], LOW);
    digitalWrite(focosMesasPrograma[mesa], LOW);
  } else {
    digitalWrite(focosVMesas[mesa], LOW);
    digitalWrite(focosRMesas[mesa], HIGH);
    digitalWrite(focosMesasPrograma[mesa], HIGH);
  }

}
