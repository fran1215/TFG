#include "utils.h"
#include <iostream>
#include <string>
#include <ctime>
#include <Arduino.h>
#include "debug.h"

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

String genPayload(String client, String method, JSONVar params) {
    JSONVar obj;

    obj["src"] = client;
    obj["method"] = method;
    obj["ts"] = getTime();
    obj["params"] = params;

    debugln(obj);
    return JSON.stringify(obj);
}

String createInfoPayload(String client, double temp, double humidity, int co2, int tvoc) {
    JSONVar params;
    
    if(temp != -500.0){
      params["temperature"] = temp;
    }

    if(humidity != -500.0){
      params["humidity"] = humidity;
    }
    
    if(co2 != -500){
      params["co2"] = co2;
    }

    if(tvoc != -500){
      params["tvoc"] = tvoc;
    }

    String payload = genPayload(client, "info", params);
    return payload;
}

String createResponsePayload(String client, JSONVar params) {
    String payload = genPayload(client, "response", params);
    return payload;
}

String receivedCommand(String msg, String client, double temperature, double humidity, int co2, int tvoc) {
    debug("Received message: ");
    debugln(msg);
    try {
        JSONVar obj = JSON.parse(msg);
        if (JSON.stringify(obj["method"]) == "cmd") {
            if (JSON.stringify(obj["params"][0]) == "get") {
                String param = obj["params"][1];
                JSONVar respParams;
                if (param == "temperature") {
                    respParams["temperature"] = temperature;
                } else if (param == "humidity") {
                    respParams["humidity"] = humidity;
                } else if (param == "co2") {
                    respParams["co2"] = co2;
                } else if (param == "tvoc") {
                    respParams["tvoc"] = tvoc;
                }
                return createResponsePayload(client, respParams);
            }
        }
        return "";
    } catch (std::exception e) {
        debug("Error parsing message: ");
        debugln(e.what());
        return "";
    }
}