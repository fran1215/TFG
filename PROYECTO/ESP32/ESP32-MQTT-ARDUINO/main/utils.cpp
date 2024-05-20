#include "utils.h"
#include <string>
#include <ctime>
#include <Arduino.h>
#include "debug.h"

// Function that gets current epoch time
unsigned long getTime()
{
        time_t now;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
                Serial.println("Failed to obtain time");
                return (0);
        }
        time(&now);
        return now;
}

String unquote(String str)
{
        str.replace("\"", "");
        return str;
}

String genPayload(String client, String method, JSONVar params)
{
        JSONVar obj;

        obj["src"] = client;
        obj["method"] = method;
        obj["ts"] = getTime();
        if(params.keys().length() > 0){
                obj["params"] = params;
        }
        
        debugln(obj);
        return JSON.stringify(obj);
}

String createInfoPayload(String client, double temp, double humidity, int co2, int tvoc)
{
        JSONVar params;

        if (temp != -500.0)
        {
                params["temperature"] = temp;
        }

        if (humidity != -500.0)
        {
                params["humidity"] = humidity;
        }

        if (co2 != -500)
        {
                params["co2"] = co2;
        }

        if (tvoc != -500)
        {
                params["tvoc"] = tvoc;
        }

        String payload = genPayload(client, "info", params);
        return payload;
}

String createResponsePayload(String client, JSONVar params)
{
        String payload = genPayload(client, "response", params);
        return payload;
}

String createRestartResponsePayload(String client)
{
        JSONVar params;
        String payload = genPayload(client, "response", params);
        return payload;
}

JSONVar receivedCmd(String msg, double temperature, double humidity, int co2, int tvoc)
{
        debug("Received message: ");
        debugln(msg);
        JSONVar respParams;
        try
        {
                JSONVar obj = JSON.parse(msg);
                debug("DEBUG --- Parsed message: ");
                debugln(JSON.stringify(obj));

                String src = unquote(JSON.stringify(obj["src"]));
                String method = unquote(JSON.stringify(obj["method"]));

                respParams["dst"] = src;

                if (method == "cmd")
                {
                        debugln("DEBUG --- Command message received.");
                        JSONVar params = obj["params"];

                        if (params.hasOwnProperty("get"))
                        {
                                debugln("DEBUG --- GET CMD RECEIVED.");
                                JSONVar getParams = params["get"];
                                debugln(JSON.typeof(getParams));

                                if (JSON.typeof(getParams) == "array")
                                {
                                        size_t arraySize = getParams.length();
                                        for (int i = 0; i < arraySize; i++)
                                        {
                                                debug("DEBUG --- GETTING PARAM: ");
                                                debugln(getParams[i]);
                                                String param = unquote(JSON.stringify(getParams[i]));

                                                if (param == "temperature")
                                                {
                                                        debugln("DEBUG --- GETTING TEMPERATURE");
                                                        debugln("Temperature: ");
                                                        debugln(temperature);
                                                        respParams["temperature"] = temperature;
                                                }

                                                if (param == "humidity")
                                                {
                                                        debugln("DEBUG --- GETTING HUMIDITY");
                                                        respParams["humidity"] = humidity;
                                                }

                                                if (param == "co2")
                                                {
                                                        debugln("DEBUG --- GETTING CO2");
                                                        respParams["co2"] = co2;
                                                }

                                                if (param == "tvoc")
                                                {
                                                        debugln("DEBUG --- GETTING TVOC");
                                                        respParams["tvoc"] = tvoc;
                                                }
                                        }
                                }
                        }
                        else if (params.hasOwnProperty("restart"))
                        {
                                debugln("RESTART CMD RECEIVED.");
                                respParams["restart"] = "RESTART";
                        }
                }
                debug("DEBUG --- GET CMD RESPONSE: ");
                debugln(JSON.stringify(respParams));
                return respParams;
        }
        catch (std::exception e)
        {
                debug("Error parsing message: ");
                debugln(e.what());
                return respParams;
        }
}

JSONVar removeField(JSONVar obj, const char *field)
{
        JSONVar newObj;
        JSONVar keys = obj.keys();

        for (int i = 0; i < keys.length(); i++)
        {
                String key = keys[i];
                if (key != field)
                {
                        debug("DEBUG --- KEEPING FIELD: ");
                        debugln(key);

                        if(key == "temperature" || key == "humidity"){
                                double value = obj[key];
                                newObj[key] = value;

                                debug("DEBUG --- VALUE: ");
                                debugln(value);
                        }

                        if(key == "co2" || key == "tvoc"){
                                int value = obj[key];
                                newObj[key] = value;
                                
                                debug("DEBUG --- VALUE: ");
                                debugln(value);
                        }

                } else {
                        debug("DEBUG --- REMOVING FIELD: ");
                        debugln(key);
                }
        }

        debug("DEBUG --- NEW OBJECT AFTER REMOVAL: ");
        debugln(JSON.stringify(newObj));

        return newObj;
}