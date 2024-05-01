import time
import utils
import picoweb

global client_id
global sta_if

ip=[]
app = picoweb.WebApp(__name__)

myip = sta_if.ifconfig()
print(myip)

temperature = 0
humidity = 0
co2 = 0
tvoc = 0

# FUNCIÓN DE REINICIO Y RECONEXIÓN AL SERVIDOR MQTT
def restart_and_reconnect():
  print('Failed to connect to MQTT broker. Reconnecting...')
  time.sleep(10)
  machine.reset()

# VARIABLES DE LOS SENSORES
sda_pin = machine.Pin(15)
scl_pin = machine.Pin(14)
sda_am_pin = machine.Pin(13)
i2c = machine.SoftI2C(sda=sda_pin, scl=scl_pin)

import CCS811
import dht

# INICIALIZACIÓN DE LOS SENSORES
try:
    s = CCS811.CCS811(i2c, addr=91)
    sensor = dht.DHT22(sda_am_pin)
except ValueError as cserror:
    print(cserror)
except OSError as amerror:
    print("Failed reception")

@app.route("/")
def index(req, resp):
    queryString = req.qs
    if len(queryString.split('?')) > 1:
        parameters = utils.qs_parse(queryString)

    payload = ""

    if s.data_ready():
        temperature = sensor.temperature()
        humidity = sensor.humidity()
        co2 = s.eCO2
        tvoc = s.tVOC
        # LECTURA DE LOS SENSORES
        sensor.measure() 
        print('---- AM2320 sensor ----')
        print(f"Temperature : {temperature:.1f}C")
        print(f"Humidity    : {humidity:.1f}%")
        print('---- CCS811 sensor ----')
        if(s.eCO2 <= 0):
            print("CO2 Sensor in warm-up")
        else:
            print(f"eCO2 : {co2} ppm")

        print(f"TVOC : {tvoc} ppb")

        # CREACIÓN DEL PAYLOAD A ENVIAR
        payload = utils.createinfopayload(client_id, temperature, humidity, co2, tvoc)

    yield from picoweb.start_response(resp, content_type = "application/json")
    yield from resp.awrite(payload)

app.run(debug=True,host=myip[0],port=80)

  



