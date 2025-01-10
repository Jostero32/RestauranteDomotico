#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>



// Configuración del Access Point
const char *ssid = "Restaurante";

// Crear servidor web en el puerto 80
WebServer server(80);

// Crear servidor WebSocket en el puerto 81
WebSocketsServer webSocket = WebSocketsServer(81);

// Página HTML que incluye un cliente WebSocket
String htmlPage = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Pagina Restaurante</title>"
"<style>"
"body {"
"    font-family: Arial, sans-serif;"
"    margin: 0;"
"    padding: 0;"
"    background-color: #f8f9fa;"
"    color: #343a40;"
"}"
"h1 {"
"    text-align: center;"
"    background-color: #007bff;"
"    color: white;"
"    padding: 20px;"
"    margin: 0;"
"}"
"p {"
"    text-align: center;"
"    font-size: 18px;"
"    margin: 20px 0;"
"}"
"#status {"
"    font-weight: bold;"
"}"
"h3 {"
"    text-align: center;"
"    margin: 20px 0;"
"}"
"select {"
"    display: block;"
"    margin: 0 auto 20px;"
"    padding: 10px;"
"    font-size: 16px;"
"}"
"button {"
"    display: block;"
"    margin: 0 auto;"
"    padding: 10px 20px;"
"    font-size: 16px;"
"    background-color: #28a745;"
"    color: white;"
"    border: none;"
"    border-radius: 5px;"
"    cursor: pointer;"
"}"
"button:hover {"
"    background-color: #218838;"
"}"
"</style>"
"</head>"
"<body>"
"<h1>Restaurante</h1>"
"<p id=\"status\">Estado: Desconectado</p>"
"<h3>Seleccione su mesa</h3>"
"<select name=\"mesas\" id=\"mesas\"></select>"
"<button onClick=\"seleccionar()\">Seleccionar</button>"
"<script>"
"var mesasDisponibles;"
"const ws = new WebSocket(\"ws://192.168.4.1:81\");"
"ws.addEventListener(\"open\", () => {"
"    document.getElementById(\"status\").innerText = \"Estado: Conectado\";"
"    console.log(\"conectado\");"
"});"
"ws.addEventListener(\"message\", (msg) => {"
"    let json = JSON.parse(msg.data);"
"    console.log(json);"
"    if (json.mesasDisponibles != null) {"
"        document.getElementById(\"mesas\").innerHTML = \"\";"
"        json.mesasDisponibles.forEach(mesa => {"
"            let opcion = document.createElement(\"option\");"
"            opcion.innerText = \"Mesa \" + (Number(mesa.id) + 1);"
"            opcion.id = mesa.id;"
"            document.getElementById(\"mesas\").appendChild(opcion);"
"        });"
"    }"
"});"
"function seleccionar() {"
"    let combo = document.getElementById(\"mesas\");"
"    window.location.href = \"/menu?mesa=\" + combo.options[combo.selectedIndex].id;"
"}"
"</script>"
"</body>"
"</html>";


String htmlPageMenu = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Menu</title>"
"<meta charset=\"UTF-8\">"
"<style>"
"body {"
"    font-family: Arial, sans-serif;"
"    margin: 0;"
"    padding: 0;"
"    background-color: #f8f9fa;"
"    color: #343a40;"
"}"
"h1 {"
"    text-align: center;"
"    background-color: #007bff;"
"    color: white;"
"    padding: 20px;"
"    margin: 0;"
"}"
"p {"
"    text-align: center;"
"    font-size: 18px;"
"    margin: 20px 0;"
"}"
"#status {"
"    font-weight: bold;"
"}"
"table {"
"    width: 90%;"
"    margin: 20px auto;"
"    border-collapse: collapse;"
"    background-color: white;"
"    box-shadow: 0px 2px 5px rgba(0, 0, 0, 0.1);"
"}"
"table th, table td {"
"    padding: 15px;"
"    text-align: center;"
"    border: 1px solid #ddd;"
"}"
"table th {"
"    background-color: #007bff;"
"    color: white;"
"}"
".total {"
"    text-align: center;"
"    font-size: 20px;"
"    margin: 20px 0;"
"    font-weight: bold;"
"}"
"button {"
"    display: block;"
"    margin: 20px auto;"
"    padding: 10px 20px;"
"    font-size: 16px;"
"    background-color: #28a745;"
"    color: white;"
"    border: none;"
"    border-radius: 5px;"
"    cursor: pointer;"
"}"
"button:hover {"
"    background-color: #218838;"
"}"
".btn {"
"    padding: 5px 10px;"
"    font-size: 14px;"
"    margin: 0 5px;"
"    border-radius: 5px;"
"    cursor: pointer;"
"}"
".btn-add {"
"    background-color: #007bff;"
"    color: white;"
"    border: none;"
"}"
".btn-add:hover {"
"    background-color: #0056b3;"
"}"
".btn-remove {"
"    background-color: #dc3545;"
"    color: white;"
"    border: none;"
"}"
".btn-remove:hover {"
"    background-color: #c82333;"
"}"
"</style>"
"</head>"
"<body>"
"<h1>Menu</h1>"
"<p id=\"status\">Estado: Desconectado</p>"
"<table>"
"    <thead>"
"        <tr>"
"            <th>Item</th>"
"            <th>Precio</th>"
"            <th>Cantidad</th>"
"            <th>Acción</th>"
"        </tr>"
"    </thead>"
"    <tbody id=\"menu\">"
"    </tbody>"
"</table>"
"<div class=\"total\">Total: $<span id=\"total\">0.00</span></div>"
"<button onclick=\"liberarMesa()\">Liberar Mesa</button>"
"<script>"
"var mesasDisponibles;"
"const ws = new WebSocket(\"ws://192.168.4.1:81\");"
"ws.addEventListener(\"open\", () => {"
"    document.getElementById(\"status\").innerText = \"Estado: Conectado\";"
"    let mesaSeleccionada = {}; "
"    let parametros = new URLSearchParams(window.location.search);"
"    mesaSeleccionada.seleccion = true;"
"    mesaSeleccionada.mesa = parametros.get(\"mesa\");"
"    console.log(JSON.stringify(mesaSeleccionada));"
"    ws.send(JSON.stringify(mesaSeleccionada));"
"    console.log(\"conectado\");"
"});"
"ws.addEventListener(\"message\", (msg) => {"
"    let json = JSON.parse(msg.data);"
"    console.log(json);"
"    let parametros = new URLSearchParams(window.location.search);"
"    var mesaSel = parametros.get(\"mesa\");"
"    if (json.mesasDisponibles != null) {"
"        let encontro = false;"
"        json.mesasDisponibles.forEach(mesa => {"
"            if (mesa.id == mesaSel) {"
"                encontro = true;"
"                return;"
"            }"
"        });"
"        if (!encontro) {"
"            window.location.href = \"/\";"
"        }"
"    }"
"    if (json.pedido) {"
"        if (json.mesa == mesaSel) {"
"            console.log(json.menu);"
"            menuItems = json.menu;"
"            renderMenu();"
"        }"
"    }"
"});"
"function liberarMesa() {"
"    let reserva = {}; "
"    let parametros = new URLSearchParams(window.location.search);"
"    reserva.reserva = true;"
"    reserva.mesa = parametros.get(\"mesa\");"
"    console.log(JSON.stringify(reserva));"
"    ws.send(JSON.stringify(reserva));"
"    window.location.href = \"/\";"
"}"
"var menuItems = []; "
"const menuTable = document.getElementById(\"menu\");"
"const totalElement = document.getElementById(\"total\");"
"function renderMenu() {"
"    menuTable.innerHTML = \"\";"
"    menuItems.forEach(item => {"
"        const row = document.createElement(\"tr\");"
"        row.innerHTML = `"
"        <td>${item.nombre}</td>"
"        <td>$${item.precio.toFixed(2)}</td>"
"        <td>${item.cantidad}</td>"
"        <td>"
"            <button class=\"btn btn-add\" onclick=\"updateQuantity(${item.id}, 1)\">Agregar</button>"
"            <button class=\"btn btn-remove\" onclick=\"updateQuantity(${item.id}, -1)\">Quitar</button>"
"        </td>"
"        `;"
"        menuTable.appendChild(row);"
"    });"
"    updateTotal();"
"}"
"function updateQuantity(id, change) {"
"    const item = menuItems.find(item => item.id === id);"
"    if (item) {"
"        item.cantidad = Math.max(0, item.cantidad + change);"
"        enviarJsonPedido();"
"        renderMenu();"
"    }"
"}"
"function updateTotal() {"
"    const total = menuItems.reduce((sum, item) => sum + item.cantidad * item.precio, 0);"
"    totalElement.textContent = total.toFixed(2);"
"}"
"function enviarJsonPedido() {"
"    let json = {}; "
"    let parametros = new URLSearchParams(window.location.search);"
"    json.pedido = true;"
"    json.mesa = parametros.get(\"mesa\");"
"    json.menu = menuItems;"
"    console.log(JSON.stringify(json));"
"    ws.send(JSON.stringify(json));"
"}"
"</script>"
"</body>"
"</html>";


const int numeroMesas=6;
String mesas[numeroMesas];
String pedidos[numeroMesas];

#define RXp2 16
#define TXp2 17

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
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  // Configurar ESP32 como Access Point
 // WiFi.softAP(ssid, password);
  WiFi.softAP(ssid);
  Serial.println("Access Point configurado");
  Serial.print("IP del Access Point: ");
  Serial.println(WiFi.softAPIP()); // Generalmente 192.168.4.1

  // Configurar servidor web
    server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage); // Enviar el HTML
  });
      server.on("/menu", HTTP_GET, []() {
    server.send(200, "text/html", htmlPageMenu); // Enviar el HTML
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
  if (Serial2.available() > 0) { 
    String datoRecibido = Serial2.readStringUntil('\n');//{"reserva":true,"mesa":"4"}
    Serial.println("arduino: "+datoRecibido);
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
    Serial2.println("{\"reserva\":true,\"mesa\":\""+String(doc["mesa"].as<int>())+"\"}");
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
