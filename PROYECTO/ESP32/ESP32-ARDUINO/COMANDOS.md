
Desde cliente 1 (Raspberry):
```sh
mosquitto_sub -t "30:AE:A4:28:3B:5/responseTopic"
```

Desde cliente 2 (PC):
```sh
mosquitto_pub -t "30:AE:A4:28:3B:5C" -m "{\"src\":\"responseTopic\",\"method\":\"cmd\",\"params\":{\"restart\":1}}" -h 192.168.1.23
```

```sh
mosquitto_pub -t "30:AE:A4:28:3B:5C" -m "{\"src\":\"responseTopic\",\"method\":\"cmd\",\"params\":{\"get\":[\"temperature\"]}}" -h 192.168.1.23
mosquitto_pub -t "30:AE:A4:28:3B:5C" -m "{\"src\":\"responseTopic\",\"method\":\"cmd\",\"params\":{\"get\":[\"humidity\"]}}" -h 192.168.1.23
mosquitto_pub -t "30:AE:A4:28:3B:5C" -m "{\"src\":\"responseTopic\",\"method\":\"cmd\",\"params\":{\"get\":[\"co2\"]}}" -h 192.168.1.23
mosquitto_pub -t "30:AE:A4:28:3B:5C" -m "{\"src\":\"responseTopic\",\"method\":\"cmd\",\"params\":{\"get\":[\"tvoc\"]}}" -h 192.168.1.23
mosquitto_pub -t "30:AE:A4:28:3B:5C" -m "{\"src\":\"responseTopic\",\"method\":\"cmd\",\"params\":{\"get\":[\"temperature\",\"humidity\"]}}" -h 192.168.1.23
mosquitto_pub -t "30:AE:A4:28:3B:5C" -m "{\"src\":\"responseTopic\",\"method\":\"cmd\",\"params\":{\"get\":[\"temperature\",\"co2\"]}}" -h 192.168.1.23
```