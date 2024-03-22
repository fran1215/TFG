import time
import utils
from umqtt.simple import MQTTClient

global client_id

temperature = 0
humidity = 0
co2 = 0
tvoc = 0

# FUNCIÓN DE REINICIO Y RECONEXIÓN AL SERVIDOR MQTT
def restart_and_reconnect():
  print('Failed to connect to MQTT broker. Reconnecting...')
  time.sleep(10)
  machine.reset()

def sub_cb(topic, msg):
    payload = utils.receivedCommand(msg.decode("utf-8"), client_id, temperature, humidity, co2, tvoc)
    if payload is not None:
        client.publish(topic_response, payload)

# FUNCIÓN DE CONEXIÓN AL SERVIDOR MQTT
def connect():
    global mqtt_host, user, password
    client = MQTTClient(client_id, mqtt_host,user=user, password=password, keepalive=60)
    client.set_callback(sub_cb)
    client.connect()
    print('Connected to %s MQTT broker' % mqtt_host)
    return client

try:
    client = connect()
    client.subscribe(topic_sub)
except OSError as e:
    restart_and_reconnect()

# VARIABLES DE LOS SENSORES
sda_pin = machine.Pin(15)
scl_pin = machine.Pin(14)
sda_am_pin = machine.Pin(13)
i2c = machine.SoftI2C(sda=sda_pin, scl=scl_pin)

import CCS811
import dht

# INICIALIZACIÓN DE LOS SENSORES
try:
    s = CCS811.CCS811(i2c, addr=91)
    sensor = dht.DHT22(sda_am_pin)
except ValueError as cserror:
    print(cserror)
except OSError as amerror:
    print("Failed reception")

count = 0
while count <= 100:
    client.check_msg()
    # SI HAY DATOS DISPONIBLES
    if s.data_ready():
        temperature = sensor.temperature()
        humidity = sensor.humidity()
        co2 = s.eCO2
        tvoc = s.tVOC
        # LECTURA DE LOS SENSORES
        sensor.measure() 
        print('---- AM2320 sensor ----')
        print(f"Temperature : {temperature:.1f}C")
        print(f"Humidity    : {humidity:.1f}%")
        print('---- CCS811 sensor ----')
        if(s.eCO2 <= 0):
            print("CO2 Sensor in warm-up")
        else:
            print(f"eCO2 : {co2} ppm")

        print(f"TVOC : {tvoc} ppb")

        # CREACIÓN DEL PAYLOAD A ENVIAR
        payload = utils.createinfopayload(client_id, temperature, humidity, co2, tvoc)

        # ENVÍO DE LOS DATOS AL SERVIDOR MQTT
        try:
            client.publish(topic_eventos, payload)
        except OSError as e:
            print("Failed to publish message")
      
    time.sleep(1)
    count += 1

  



