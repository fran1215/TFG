# Build

To build, cd to 'KAFKA_GRAFANA' and do

```
docker compose up -d
```

```
docker exec -it kafka_grafana-kafka-1 kafka-topics --create --topic events --partitions 1 --replication-factor 1 --if-not-exists --bootstrap-server localhost:29092
```