#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <AM2302-Sensor.h>
#include <SparkFunCCS811.h>
#include "utils.h"
#include "debug.h"
#include <WebServer.h>

#define HTTP_MQTT 1 // 0 - HTTP, 1 - MQTT


// WIFI
const char* ssid        = "Livebox6-E5A7";
const char* password    = "54TNarY4sCbdEvafran2003";

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// MQTT Broker
const char *mqtt_broker = "192.168.1.23";
const char *topic = "events";
const char *mqtt_username = "esp32";
const char *mqtt_password = "testuma";
const int mqtt_port = 1883;
String client_id;

// HTTP Server
WebServer server(80);

double temp;
double humidity;
int co2;
int tvoc;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
String msg;
int value = 0;

constexpr unsigned int SENSOR_PIN {13U};
AM2302::AM2302_Sensor am2302{SENSOR_PIN};


#define CCS811_ADDR 0x5B //Default I2C Address
CCS811 ccs811(CCS811_ADDR);

// MQTT - CALLBACK
void callback(char* topic, byte* payload, unsigned int length) {
  debug("Message arrived [");
  debug(topic);
  debug("] ");
  for (int i=0;i<length;i++) {
    debug((char)payload[i]);
  }
  debugln();
}

// MQTT - RECONNECT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    debugln("Intentando la conexión MQTT...");
    // Attempt to connect
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          debugln("Conexion con el broker MQTT establecida.");
      } else {
          debug("Error con codigo: ");
          debugln(client.state());
          debugln("Reintentando en 2 segundos...");
          delay(2000);
      }
  }
}

// HTTP - HANDLE ROOT
void handleRoot(){
  debugln("CONNECTION ON HTTP SERVER - ROOT");

  const char* msgchar = msg.c_str();
  server.send(200, "application/json", msgchar);
}

// HTTP - START SERVER
void startServer(){
  server.on("/", HTTP_GET, handleRoot);
  debugln("STARTING SERVER");

  server.begin();

  debugln("SERVER STARTED");
}

// SETUP - DEBUG
void debugSensors(double temp, double humidity, int co2, int tvoc){
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

// SETUP - WIFI
void setupWifi(){
  // Connecting to a Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      debugln("Conectando a WiFi..");
  }
  debug("Conexión WiFi establecida.");
  debug("Dirección IP: ");
  debugln(WiFi.localIP());
  client_id = String(WiFi.macAddress());
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Wire.begin();

  setupWifi();
  configTime(0,0,ntpServer);

  if(HTTP_MQTT){
    // Connecting to the MQTT Server
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
  } else {
    startServer();
  }

  am2302.begin();

  auto status = am2302.read();
  debug("\n\nEstado del sensor AM2302: ");
  debugln(status);

  if(!ccs811.begin()){
    debugln("Error al cargar el sensor CCS811. Comprueba las conexiones.");
  }

  // Esperar a que el sensor esté listo
  while(!ccs811.dataAvailable());
}

void loop() {
  if(HTTP_MQTT){
    if(!client.connected()){
      reconnect();
    }
    client.loop();
  } else {
    server.handleClient();
  }

  unsigned long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;

    if(ccs811.dataAvailable()){

      // Read from sensor CCS811
      ccs811.readAlgorithmResults();

      // Read from sensor AM2302
      temp = roundf(am2302.get_Temperature() * 100) / 100;
      humidity = roundf(am2302.get_Humidity() * 100) / 100;
      co2 = ccs811.getCO2();
      tvoc = ccs811.getTVOC();

      debugSensors(temp, humidity, co2, tvoc);

      msg = createInfoPayload(client_id, temp, humidity, co2, tvoc);
      const char* msgchar = msg.c_str();
      
      if(HTTP_MQTT){
        debug("MQTT - Publish message: ");
        debugln(msgchar);
        client.publish(topic, msgchar);
      } 
      
    }   
  }
}
