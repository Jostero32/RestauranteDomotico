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

const int numeroMesas=6;
String mesas[numeroMesas];
String pedidos[numeroMesas];


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
      funciones(String((char*)payload),num);
      webSocket.broadcastTXT(""+String((char*)payload));
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
    pedidos[i]="{\"pedido\":true,\"mesa\":\""+String(i)+"\",\"menu\":[{\"id\":1,\"nombre\":\"Hamburguesa\",\"precio\":5,\"cantidad\":0},{\"id\":2,\"nombre\":\"Pizza\",\"precio\":8,\"cantidad\":0},{\"id\":3,\"nombre\":\"Coca Cola\",\"precio\":1.5,\"cantidad\":0},{\"id\":4,\"nombre\":\"Café\",\"precio\":2,\"cantidad\":0}]}";
  }
}

void loop() {
  if (Serial.available() > 0) { 
    String datoRecibido = Serial.readStringUntil('\n');//{"reserva":true,"mesa":"4"}
    funciones(datoRecibido,-1);
    actualizarMesas();
  }
  // Manejar conexiones del servidor web
  server.handleClient();

  // Manejar conexiones WebSocket
  webSocket.loop();
}
void funciones(String msg,int num){
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    return;
  }
  if(doc["reserva"]){
    String estado= (mesas[doc["mesa"].as<int>()]=="Disponible")?"Ocupado":"Disponible";
    mesas[doc["mesa"].as<int>()]=estado;
    pedidos[doc["mesa"].as<int>()]="{\"pedido\":true,\"mesa\":\""+String(doc["mesa"].as<int>())+"\",\"menu\":[{\"id\":1,\"nombre\":\"Hamburguesa\",\"precio\":5,\"cantidad\":0},{\"id\":2,\"nombre\":\"Pizza\",\"precio\":8,\"cantidad\":0},{\"id\":3,\"nombre\":\"Coca Cola\",\"precio\":1.5,\"cantidad\":0},{\"id\":4,\"nombre\":\"Café\",\"precio\":2,\"cantidad\":0}]}";
    actualizarMesas();
  }
  if(doc["seleccion"]){
    webSocket.sendTXT(num, pedidos[doc["mesa"].as<int>()]);
  }
  if(doc["pedido"]){
    pedidos[doc["mesa"].as<int>()]=msg;
    webSocket.broadcastTXT(pedidos[doc["mesa"].as<int>()]);
  }
}

void actualizarMesas(){
    String jsonMesasDisponibles;
  jsonMesasDisponibles="{\"mesasDisponibles\":[";
      for(int i=0;i<numeroMesas;i++){
        if(mesas[i]=="Ocupado"){
            jsonMesasDisponibles=jsonMesasDisponibles+"{\"id\":"+"\""+(i)+"\",\"estado\":\""+mesas[i]+"\"},";

        }
      }
      if(jsonMesasDisponibles.substring(jsonMesasDisponibles.length()-1,jsonMesasDisponibles.length())==","){
           jsonMesasDisponibles=jsonMesasDisponibles.substring(0,jsonMesasDisponibles.length()-1);
      }
      jsonMesasDisponibles=jsonMesasDisponibles+"]}";
      webSocket.broadcastTXT(jsonMesasDisponibles.c_str());
}
