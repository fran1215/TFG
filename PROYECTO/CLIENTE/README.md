# Ejecutar servicios

Para ejecutar el servicio, cd hasta la carpeta 'CLIENTE' y ejecutar los siguientes comandos:

```
docker compose up -d --pull always
```

para ejecutar el grupo de contenedores descargando siempre la última versión de la imagen de Docker de cada servicio.

Importante:

- Reemplazar la dirección IP en el ajuste llamado `servers` dentro del grupo `inputs.mqtt_consumer` en el archivo de configuración [telegraf.conf](/CLIENTE/telegraf/telegraf.conf) con la IP de su broker MQTT.
- Reemplazar la dirección MAC en el ajuste llamado `topics` dentro del grupo `inputs.mqtt_consumer` en el archivo de configuración [telegraf.conf](/CLIENTE/telegraf/telegraf.conf) con la MAC de su dispositivo ESP32.
- Reemplazar la dirección IP en el ajuste llamado `urls` dentro del grupo `inputs.http` en el archivo de configuración [telegraf.conf](/CLIENTE/telegraf/telegraf.conf) con la IP de su dispositivo ESP32.
