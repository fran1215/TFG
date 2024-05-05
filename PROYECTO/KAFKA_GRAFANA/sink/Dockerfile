FROM confluentinc/cp-server-connect-base:7.6.1

COPY kafka-connect-mqtt\target/kafka-connect-mqtt-1.1.0-package/kafka-connect-mqtt.zip /tmp/kafka-connect-mqtt.zip

RUN confluent-hub install --no-prompt /tmp/kafka-connect-mqtt.zip