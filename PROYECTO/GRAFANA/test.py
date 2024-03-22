import paho.mqtt.client as mqtt
import numpy as np
import time
import utils

MQTTBROKER = '192.168.1.23'
PORT = 1883
TOPIC = "events"

client_id = "client1"

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.username_pw_set("user2", "password")
mqttc.connect(MQTTBROKER, PORT)

while True:
    temperature = np.random.randint(20, 30)
    humidity = np.random.randint(0, 100)
    co2 = np.random.randint(400, 2000)
    tvoc = np.random.randint(0, 100)
    payload = utils.createinfopayload(client_id, temperature, humidity, co2, tvoc)
    mqttc.publish(TOPIC, payload)
    print("Published to " + MQTTBROKER + ': ' + 
          TOPIC + ':' + payload)
    time.sleep(3)