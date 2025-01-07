#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>



// Configuración del Access Point
const char *ssid = "yourAP";
const char *password = "yourPassword";

// Crear servidor web en el puerto 80
WebServer server(80);

// Crear servidor WebSocket en el puerto 81
WebSocketsServer webSocket = WebSocketsServer(81);

// Página HTML que incluye un cliente WebSocket
String htmlPage = "<!DOCTYPE html>"
"<html>"
"<head>"
"  <title>ESP32 Web + WebSocket</title>"
"</head>"
"<body>"
"  <h1>Servidor Web y WebSocket</h1>"
"  <p id='status'>Estado: Desconectado</p>"
"  <input type='text' id='message' placeholder='Escribe un mensaje'>"
"  <button onclick='sendMessage()'>Enviar</button>"
"  <div id='log'></div>"

"  <script>"
"      const ws = new WebSocket('ws://192.168.4.1:81');"

"      ws.addEventListener('open', () => {"
"          document.getElementById('status').innerText = 'Estado: Conectado';"
"          console.log('conectado');"
"      });"

"      ws.addEventListener('message', (event) => {"
"          const log = document.getElementById('log');"
"          log.innerHTML += '<p>Servidor: ' + event.data + '</p>';"
"      });"

"      function sendMessage() {"
"          const message = document.getElementById('message').value;"
"          ws.send(message);"
"      }"
"  </script>"
"</body>"
"</html>";

const int numeroMesas=8;
String mesas[numeroMesas];


// Manejo de eventos WebSocket
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("Cliente [%u] conectado\n", num);
      actualizarMesas();
      break;

    case WStype_DISCONNECTED:
      Serial.printf("Cliente [%u] desconectado\n", num);
      break;

    case WStype_TEXT:
      Serial.printf("Mensaje recibido del cliente [%u]: %s\n", num, payload);
      // Enviar respuesta al cliente
      funciones(String((char*)payload));
      webSocket.sendTXT(num, "Mensaje recibido: " + String((char*)payload));
      break;

    case WStype_BIN:
      Serial.printf("Mensaje binario recibido del cliente [%u]\n", num);
      break;
  }}

  
void setup() {
  Serial.begin(115200);

  // Configurar ESP32 como Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point configurado");
  Serial.print("IP del Access Point: ");
  Serial.println(WiFi.softAPIP()); // Generalmente 192.168.4.1

  // Configurar servidor web
    server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage); // Enviar el HTML
  });
  server.begin();
  Serial.println("Servidor Web iniciado en el puerto 80");

  // Configurar servidor WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("Servidor WebSocket iniciado en el puerto 81");
  for(int i=0;i<numeroMesas;i++){
    mesas[i]="Disponible";
  }
}

void loop() {
  // Manejar conexiones del servidor web
  server.handleClient();

  // Manejar conexiones WebSocket
  webSocket.loop();
}
void funciones(String msg){
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  if(doc["reserva"]){
    String estado= (mesas[doc["mesa"].as<int>()]=="Disponible")?"Ocupado":"Disponible";
    Serial.println(estado);
    mesas[doc["mesa"].as<int>()]=estado;
    actualizarMesas();
  }
}

void actualizarMesas(){
    String jsonMesasDisponibles;
  jsonMesasDisponibles="{\"mesasDisponibles\":[";
      for(int i=0;i<numeroMesas;i++){
        if(mesas[i]=="Disponible"){
            jsonMesasDisponibles=jsonMesasDisponibles+"{\"id\":"+"\""+(i)+"\",\"estado\":\""+mesas[i]+"\"}";
          if(i!=numeroMesas-1){
            jsonMesasDisponibles=jsonMesasDisponibles+",";
          }
        }
      }
      jsonMesasDisponibles=jsonMesasDisponibles+"]}";
      webSocket.broadcastTXT(jsonMesasDisponibles.c_str());
}
