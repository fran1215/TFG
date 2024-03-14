import network
import time
import machine

sda_pin = machine.Pin(15)
scl_pin = machine.Pin(14)

sda_am_pin = machine.Pin(13)

sta_if = network.WLAN(network.STA_IF)
print("Wifi status: ", sta_if.isconnected())
print("Wifi config (IP/MASCARA/GATEWAY/DNS): ", sta_if.ifconfig())

i2c = machine.SoftI2C(sda=sda_pin, scl=scl_pin)

import CCS811
import dht

try:
  s = CCS811.CCS811(i2c, addr=91)
  sensor = dht.DHT22(sda_am_pin)
  
  while True:
      sensor.measure()     # Recovers measurements from the sensor
      print('---- AM2320 sensor ----')
      print(f"Temperature : {sensor.temperature():.1f}ºC")
      print(f"Humidity    : {sensor.humidity():.1f}%")
      
      if s.data_ready():
        print('---- CCS811 sensor ----')
        if(s.eCO2 <= 0):
          print("CO2 Sensor in warm-up")
        else:
          print(f"eCO2 : {s.eCO2} ppm")

        print(f"TVOC : {s.tVOC} ppb")
      
      time.sleep(1)
except ValueError as cserror:
  print(cserror)
except OSError as amerror:
    print("Failed reception")


