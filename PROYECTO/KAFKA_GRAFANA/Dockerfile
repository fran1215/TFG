#!/usr/bin/env -S docker build . --tag=tfg/kafka-mqtt-bridge --network=host --file

# Use a base image like Debian or Alpine
FROM python:latest

# Copy configuration files
COPY mqtt-kafka-bridge.py /usr/local/bin/mqtt-kafka-bridge.py

# Start Kafka, Telegraf, and Grafana
CMD pip install paho-mqtt && \
    pip install pykafka && \
    python /usr/local/bin/mqtt-kafka-bridge.py

