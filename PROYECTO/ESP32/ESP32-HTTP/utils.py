import json
import utime

def qs_parse(qs):
    parameters = {}
    ampersandSplit = qs.split("&")
    for element in ampersandSplit:
        equalSplit = element.split("=")
        parameters[equalSplit[0]] = equalSplit[1]
    return parameters

def genPayload(client, method, params):
    obj = {
        "src": client,
        "method": method,
        "ts": utime.time() + 946684800, # 946684800 es la diferencia entre 1970 y 2000. ESP32 no soporta fechas anteriores a 2000
        "params": params
    }

    print(obj)
    return obj

def createinfopayload(client, temp, humidity, co2, tvoc):
    params = {
        "temperature": temp,
        "humidity": humidity,
        "co2": co2,
        "tvoc": tvoc
    }
    
    obj = genPayload(client, "info", params)
    payload = json.dumps(obj)
    return payload

def createResponsePayload(client, params):
    obj = genPayload(client, "response", params)
    payload = json.dumps(obj)
    return payload

def receivedCommand(msg, client, temperature, humidity, co2, tvoc):
    print("Received message: ", msg)
    try:
        obj = json.loads(msg)
        if obj["method"] == "cmd":
            if(obj["params"][0] == "get"):
                param = obj["params"][1]
                if param == "temperature":
                    return createResponsePayload(client, {"temperature": temperature})
                elif param == "humidity":
                    return createResponsePayload(client, {"humidity": humidity})
                elif param == "co2":
                    return createResponsePayload(client, {"co2": co2})
                elif param == "tvoc":
                    return createResponsePayload(client, {"tvoc": tvoc})
            
        return None
    except ValueError as e:
        print("Error parsing message: ", e)
        return None