import paho.mqtt.client as mqtt
from pykafka import KafkaClient
import time

mqtt_broker = "192.168.1.23"
mqtt_client = mqtt.Client(protocol=mqtt.MQTTv5)

kafka_client = KafkaClient(hosts="kafkabroker:29092")
print(kafka_client.brokers)
print(kafka_client.topics)
kafka_topic = kafka_client.topics['events']
kafka_producer = kafka_topic.get_sync_producer()

def on_connect(client, userdata, flags, reason_code, properties):
        print(f"Connected with result code {reason_code}")

def on_message(client, userdata, message):
        msg_payload = str(message.payload)
        print("Received MQTT message: ", msg_payload)
        kafka_producer.produce(msg_payload.encode('ascii'))
        print("KAFKA: Just published " + msg_payload + " to topic events")

mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.username_pw_set("cliente", "testuma")
mqtt_client.connect(mqtt_broker, 1883)

mqtt_client.subscribe("events")

mqtt_client.loop_forever()