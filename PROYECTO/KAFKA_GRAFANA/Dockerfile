#!/usr/bin/env -S docker build . --tag=tfg/kafka-mqtt-bridge --network=host --file

# Use a base image like Debian or Alpine
FROM ubuntu:20.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y python3 python3-pip netcat

# Copy configuration files
COPY mqtt-kafka-bridge.py /usr/local/bin/mqtt-kafka-bridge.py

# Start Kafka, Telegraf, and Grafana
CMD pip install paho-mqtt && \
    pip install pykafka && \
    python3 -u /usr/local/bin/mqtt-kafka-bridge.py

