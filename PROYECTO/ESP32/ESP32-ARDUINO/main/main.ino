#include "utils.h"
#include "debug.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <AM2302-Sensor.h>
#include <SparkFunCCS811.h>
#include <WebServer.h>

#define HTTP_MQTT 1 // 0 - HTTP, 1 - MQTT

// Configuración WIFI
const char *ssid = "SSID";
const char *password = "CLAVE";

// NTP server para obtener el tiempo epoch
const char *ntpServer = "pool.ntp.org";

// Configuración Broker MQTT y tópicos
const char *mqtt_broker = "192.168.1.23";
const char *topic = "events";
const char *topicCmd = "";
const char *mqtt_username = "esp32";
const char *mqtt_password = "testuma";
const int mqtt_port = 1883;
String client_id;

// HTTP Server
WebServer server(80);

// Variables con la información de los sensores
double temp;
double humidity;
int co2;
int tvoc;

// Configuración cliente PubSub
WiFiClient espClient;
PubSubClient client(espClient);

// Variable para controlar el tiempo de envío de mensajes
unsigned long lastMsg = 0;

// Variable para almacenar el mensaje a enviar constantemente
String msg;

// Configuración física de los sensores (PINES)
constexpr unsigned int SENSOR_PIN{13U}; // Pin del sensor AM2302 - 13
AM2302::AM2302_Sensor am2302{SENSOR_PIN};

#define CCS811_ADDR 0x5B // Dirección I2C del sensor CCS811 - 0x5B
CCS811 ccs811(CCS811_ADDR);

// Función de reset para reiniciar el dispositivo
void(* resetFunc) (void) = 0;

// Función para recibir mensajes y enviar las respuestas generadas
void receiveResponse(String resp)
{
        debug("DEBUG --- Receive Response - Received message: ");
        debugln(resp);

        // Parsear el mensaje recibido
        JSONVar cmdResp = receivedCmd(resp, temp, humidity, co2, tvoc);
        String dst = cmdResp["dst"];
        JSONVar sendResp;

        String payload;
        bool restart = false;
        
        // Comprobar si el mensaje recibido es un comando de reinicio
        if (cmdResp.hasOwnProperty("restart"))
        {
                debugln("DEBUG --- Restart message parsed correctly. Sending response and restarting.");
                removeField(cmdResp, "restart");
                restart = true;

                JSONVar empty;
                payload = createResponsePayload(client_id, empty).c_str();
        }
        // Si no es un comando de reinicio, es un comando get
        else
        {
                debugln("DEBUG --- Get message parsed correctly. Sending response.");
                cmdResp = removeField(cmdResp, "dst");

                payload = createResponsePayload(client_id, cmdResp).c_str();
        }

        // Enviar la respuesta por el canal configurado (HTTP o MQTT)
        if(HTTP_MQTT)
        {
                // Crear el tópico de respuesta
                String topicRespStr = client_id + "/" + dst;
                const char *topicResp = topicRespStr.c_str();

                debug("DEBUG --- Response topic: ");
                debugln(topicRespStr);

                client.publish(topicResp, payload.c_str());
        }
        else
        {
                server.send(200, "application/json", payload.c_str());
        }

        // Si se ha recibido un comando de reinicio, 
        // reiniciar el dispositivo tras enviar la respuesta
        if(restart)
        {
                delay(1000);
                resetFunc();
        }
}

// HTTP - Manejador de la ruta raíz ("GET /")
void handleRoot()
{
        debugln("CONNECTION ON HTTP SERVER - ROOT");

        // Se envía el mensaje almacenado en el momento de la última lectura
        const char *msgchar = msg.c_str();
        server.send(200, "application/json", msgchar);
}

// HTTP - Manejador de la ruta "/field" ("GET /field")
void handleField()
{
        debugln("CONNECTION ON HTTP SERVER - POST FIELD");

        const char *msgchar = msg.c_str();
        String message = "";
        JSONVar obj, params, getParams;

        obj["src"] = "test";
        obj["method"] = "cmd";


        // Si se han recibido parámetros GET
        if (server.args() > 0)
        {
                message += "\nGET parameters: ";

                // Recorrer los parámetros GET recibidos
                for (int i = 0; i < server.args(); i++)
                {
                        // Debug de los parámetros GET
                        if (i > 0)
                        {
                                message += ", ";
                        }
                        message += server.argName(i) + " = " + server.arg(i);

                        // Almacenar cada parámetro GET en un objeto JSON auxiliar
                        if(server.arg(i)){
                                getParams[i] = server.argName(i);
                        }
                }
        }

        // Almacenar los parámetros GET en el objeto JSON con la estructura definida
        params["get"] = getParams;
        obj["params"] = params;

        debugln(message);

        // Enviar la respuesta con los parámetros GET recibidos
        receiveResponse(JSON.stringify(obj));
}

// HTTP - Manejador de la ruta "/restart" ("GET /restart")
void handleRestart()
{
        debugln("CONNECTION ON HTTP SERVER - RESTART");
        
        JSONVar obj, params;

        // Se crea un mensaje LWT de reinicio y se almacena en el objeto JSON a enviar
        params["restart"] = 1;
        obj["src"] = "test";
        obj["method"] = "cmd";
        obj["params"] = params;

        // Se envía el mensaje LWT de reinicio
        receiveResponse(JSON.stringify(obj));
}

// HTTP - Inicio del servidor HTTP
void startServer()
{
        // Configuración de los manejadores de las rutas
        server.on("/", HTTP_GET, handleRoot);
        server.on("/field", HTTP_GET, handleField);
        server.on("/restart", HTTP_GET, handleRestart);

        debugln("STARTING SERVER");

        // Inicio del servidor HTTP
        server.begin();

        debugln("SERVER STARTED");
}

// MQTT - Callback para recibir mensajes
void callback(char *topic, byte *payload, unsigned int length)
{
        debug("Message arrived [");
        debug(topic);
        debugln("] ");
        String resp = "";

        for (int i = 0; i < length; i++)
        {
                resp += (char)payload[i];
        }

        receiveResponse(resp); 
}

// MQTT - Función para reconectar con el broker MQTT
void reconnect()
{
        // Loop hasta que se establezca la conexión
        while (!client.connected())
        {
                debugln("Intentando la conexión MQTT...");
                // Intentar conectar
                if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
                {
                        debugln("Conexion con el broker MQTT establecida.");
                        debug("MQTT --- SUBSCRIBED TO ");
                        debugln(topicCmd);
                        client.subscribe(topicCmd);
                }
                else
                {
                        debug("Error con codigo: ");
                        debugln(client.state());
                        debugln("Reintentando en 2 segundos...");
                        delay(2000);
                }
        }
}

// SETUP - Debug con todos los datos actuales de los sensores
void debugSensors()
{
        debugln("---- AM2320 sensor ----");
        debug("Temperature: ");
        debug(temp);
        debugln("C");
        debug("Humidity: ");
        debug(humidity);
        debugln("%");
        debugln("---- CCS811 sensor ----");
        debug("CO2: ");
        debugln(co2);
        debug("TVOC: ");
        debugln(tvoc);
}

// SETUP - Configuración de la conexión WiFi
void setupWifi()
{
        // Connecting to a Wi-Fi network
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED)
        {
                delay(500);
                debugln("Conectando a WiFi..");
        }
        debug("Conexión WiFi establecida.");
        debug("Dirección IP: ");
        debugln(WiFi.localIP());
        client_id = String(WiFi.macAddress());
}


// SETUP - Función setup principal. Inicialización de los sensores y la comunicación
void setup()
{
        // Inicialización de la comunicación serie
        Serial.begin(115200);

        // Inicialización de la comunicación I2C
        Wire.begin();

        // Inicialización de la comunicación WiFi
        setupWifi();
        configTime(0, 0, ntpServer);

        // Configuración del topic MQTT donde recibir comandos (dirección MAC del dispositivo)
        topicCmd = client_id.c_str();

        // Si se ha configurado el uso de MQTT
        if (HTTP_MQTT)
        {
                // Conexion con el broker MQTT
                client.setServer(mqtt_broker, mqtt_port);
                // Callback para recibir mensajes
                client.setCallback(callback);
        }
        else
        { // Si se ha configurado el uso de HTTP
                // Inicialización del servidor HTTP
                startServer();
        }

        // Inicialización de los sensores
        am2302.begin();

        auto status = am2302.read();
        debug("\n\nEstado del sensor AM2302: ");
        debugln(status);

        if (!ccs811.begin())
        {
                debugln("Error al cargar el sensor CCS811. Comprueba las conexiones.");
        }

        // Esperar a que el sensor esté listo
        while (!ccs811.dataAvailable())
                ;
}

// LOOP - Función loop principal. Lectura de los sensores y envío de mensajes
void loop()
{
        // Si se ha configurado el uso de MQTT
        if (HTTP_MQTT)
        {
                // Si no se ha establecido la conexión con el broker MQTT o se ha perdido
                if (!client.connected())
                {
                        // Intentar reconectar
                        reconnect();
                }
                // Mantener la conexión con el broker MQTT
                client.loop();
        }
        else
        { // Si se ha configurado el uso de HTTP
                // Mantener la conexión con el servidor HTTP
                server.handleClient();
        }

        // Si han pasado más de 3 segundos desde el último mensaje
        unsigned long now = millis();
        if (now - lastMsg > 10000)
        {
                lastMsg = now;

                // Si hay datos disponibles en el sensor CCS811
                // Se leen los resultados del sensor CCS811 solo, 
                // porque el sensor AM2302 no dispondrá de nuevos 
                // datos hasta que se lea de nuevo.
                if (ccs811.dataAvailable())
                {

                        // Leer los resultados del algoritmo del sensor CCS811
                        ccs811.readAlgorithmResults();

                        // Leer los datos de los sensores
                        temp = roundf(am2302.get_Temperature() * 100) / 100;
                        humidity = roundf(am2302.get_Humidity() * 100) / 100;
                        co2 = ccs811.getCO2();
                        tvoc = ccs811.getTVOC();

                        // Mostrar los datos de los sensores en el monitor serie
                        debugSensors();
                        
                        // Crear el mensaje con los datos de los sensores
                        msg = createInfoPayload(client_id, temp, humidity, co2, tvoc);
                        const char *msgchar = msg.c_str();

                        // Si se ha configurado el uso de MQTT
                        if (HTTP_MQTT)
                        {
                                // Publicar el mensaje en el topic MQTT
                                debug("MQTT - Publish message: ");
                                debugln(msgchar);
                                client.publish(topic, msgchar);
                        }
                }
        }
}
