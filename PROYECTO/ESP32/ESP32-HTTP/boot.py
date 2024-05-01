import ubinascii
import wifi
import machine
import network
import ntptime
import esp
esp.osdebug(None)
import gc
gc.collect()

# CONEXIÓN A LA RED WIFI
wifi.do_connect("Livebox6-E5A7","54TNarY4sCbdEvafran2003")
sta_if = network.WLAN(network.STA_IF)
print("Wifi status: ", sta_if.isconnected())
print("Wifi config (IP/MASCARA/GATEWAY/DNS): ", sta_if.ifconfig())

# SINCRONIZACIÓN DE LA HORA A TRAVÉS DE LA CONEXIÓN WIFI
ntptime.settime() 
machine.RTC().datetime()

client_id           = ubinascii.hexlify(machine.unique_id())