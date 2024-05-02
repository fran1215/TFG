#!/usr/bin/env -S docker build . --tag=tfg/raspberry:v1 --network=host --file

# Use a base image like Debian or Alpine
FROM debian:latest

# Install dependencies and tools
RUN apt-get update && \
    apt-get install -y wget curl ca-certificates

# Install Kafka
RUN wget https://downloads.apache.org/kafka/3.7.0/kafka_2.13-3.7.0.tgz && \
    tar -xzf kafka_2.13-3.7.0.tgz && \
    mv kafka_2.13-3.7.0 /usr/local/kafka && \
    rm kafka_2.13-3.7.0.tgz
ENV KAFKA_HEAP_OPTS="-Xmx512M -Xms512M"
ENV KAFKA_LISTENERS="LISTENER_BOB://kafka0:29092,LISTENER_FRED://localhost:9092"
ENV KAFKA_ADVERTISED_LISTENERS="LISTENER_BOB://kafka0:29092,LISTENER_FRED://localhost:9092"
ENV KAFKA_LISTENER_SECURITY_PROTOCOL_MAP="LISTENER_BOB:PLAINTEXT,LISTENER_FRED:PLAINTEXT"
ENV KAFKA_INTER_BROKER_LISTENER_NAME="LISTENER_BOB"

# Install Telegraf
ENV TELEGRAF_VERSION 1.30.2
RUN ARCH= && dpkgArch="$(dpkg --print-architecture)" && \
    case "${dpkgArch##*-}" in \
        amd64) ARCH='amd64';; \
        arm64) ARCH='arm64';; \
        armhf) ARCH='armhf';; \
        armel) ARCH='armel';; \
        *)     echo "Unsupported architecture: ${dpkgArch}"; exit 1;; \
    esac && \
    wget --no-verbose https://dl.influxdata.com/telegraf/releases/telegraf_${TELEGRAF_VERSION}-1_${ARCH}.deb.asc && \
    wget --no-verbose https://dl.influxdata.com/telegraf/releases/telegraf_${TELEGRAF_VERSION}-1_${ARCH}.deb && \
    dpkg -i telegraf_${TELEGRAF_VERSION}-1_${ARCH}.deb && \
    rm -f telegraf_${TELEGRAF_VERSION}-1_${ARCH}.deb*

# Install Grafana
RUN apt-get install -y adduser libfontconfig1 musl && \
    wget https://dl.grafana.com/enterprise/release/grafana-enterprise_10.4.2_arm64.deb && \
    dpkg -i grafana-enterprise_10.4.2_arm64.deb && \
    rm grafana-enterprise_10.4.2_arm64.deb

# Install OpenJDK Java Runtime
ENV JAVA_HOME=/opt/java/openjdk
COPY --from=eclipse-temurin:21 $JAVA_HOME $JAVA_HOME
ENV PATH="${JAVA_HOME}/bin:${PATH}"

# Copy configuration files
COPY conf/telegraf.conf /etc/telegraf/telegraf.conf
COPY conf/grafana.ini /etc/grafana/grafana.ini

# Expose ports
EXPOSE 9092 9093 3000

# Start Kafka, Telegraf, and Grafana
CMD telegraf && \
    service grafana-server start && \
    /usr/local/kafka/bin/zookeeper-server-start.sh /usr/local/kafka/config/zookeeper.properties & \
    /usr/local/kafka/bin/kafka-server-start.sh /usr/local/kafka/config/server.properties

