#include "utils.h"
#include <string>
#include <ctime>
#include <Arduino.h>
#include "debug.h"

// Función auxiliar para obtener la hora actual
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

// Función auxiliar para quitar las comillas de un string
String unquote(String str)
{
        str.replace("\"", "");
        return str;
}

// Función para generar el payload de un mensaje
String genPayload(String client, String method, JSONVar params)
{
        JSONVar obj;

        // Se añaden los campos src, method y ts al objeto JSON
        obj["src"] = client;
        obj["method"] = method;
        obj["ts"] = getTime();

        // Si el objeto params no está vacío, se añade al objeto JSON
        if(params.keys().length() > 0){
                obj["params"] = params;
        }
        
        debugln(obj);
        return JSON.stringify(obj);
}

// Función para crear el payload de un mensaje de información
String createInfoPayload(String client, double temp, double humidity, int co2, int tvoc)
{
        JSONVar params;

        // Si los valores de temperatura, humedad, CO2 o TVOC no son erróneos 
        // (un valor mayor a un número muy bajo, como el -500, por ejemplo), 
        // se añaden al objeto params
        if (temp >= -500.0)
        {
                params["temperature"] = temp;
        }

        if (humidity >= -500.0)
        {
                params["humidity"] = humidity;
        }

        if (co2 >= -500)
        {
                params["co2"] = co2;
        }

        if (tvoc >= -500)
        {
                params["tvoc"] = tvoc;
        }

        // Se añade el objeto params al payload y se genera el mensaje completo
        // con el resto de parámetros 
        String payload = genPayload(client, "info", params);
        return payload;
}

// Función para crear el payload de un mensaje de respuesta
String createResponsePayload(String client, JSONVar params)
{
        // Se genera el payload con el método response y los parámetros recibidos
        String payload = genPayload(client, "response", params);
        return payload;
}

// Función para crear el payload de un mensaje de reinicio
String createRestartResponsePayload(String client)
{
        JSONVar params;
        // Se genera el payload con el método response y los parámetros recibidos
        String payload = genPayload(client, "response", params);
        return payload;
}

// Función para procesar un mensaje de comando recibido
JSONVar receivedCmd(String msg, double temperature, double humidity, int co2, int tvoc)
{
        debug("Received message: ");
        debugln(msg);
        JSONVar respParams;
        try
        {
                // Se parsea el mensaje recibido
                JSONVar obj = JSON.parse(msg);
                debug("DEBUG --- Parsed message: ");
                debugln(JSON.stringify(obj));

                // Se obtienen los campos src y method del mensaje
                String src = unquote(JSON.stringify(obj["src"]));
                String method = unquote(JSON.stringify(obj["method"]));

                // Se añade el campo dst al objeto de respuesta
                respParams["dst"] = src;

                // Si el método es cmd, se procesa el mensaje
                if (method == "cmd")
                {
                        debugln("DEBUG --- Command message received.");
                        JSONVar params = obj["params"];

                        // Si el mensaje contiene el parámetro get, se procesa
                        if (params.hasOwnProperty("get"))
                        {
                                debugln("DEBUG --- GET CMD RECEIVED.");
                                JSONVar getParams = params["get"];
                                debugln(JSON.typeof(getParams));
                                
                                // Si getParams es un array, se procesa cada uno de los parámetros
                                if (JSON.typeof(getParams) == "array")
                                {
                                        size_t arraySize = getParams.length();
                                        for (int i = 0; i < arraySize; i++)
                                        {
                                                debug("DEBUG --- GETTING PARAM: ");
                                                debugln(getParams[i]);
                                                String param = unquote(JSON.stringify(getParams[i]));

                                                // Si el parámetro es temperature, humidity, co2 o tvoc, se añade al objeto de respuesta
                                                if (param == "temperature")
                                                {
                                                        debugln("DEBUG --- GETTING TEMPERATURE");
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
                        // Si el mensaje contiene el parámetro restart, se añade al objeto de respuesta
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

// Función para eliminar un campo de un objeto JSON
JSONVar removeField(JSONVar obj, const char *field)
{
        JSONVar newObj; // Se crea un nuevo objeto JSON
        JSONVar keys = obj.keys(); // Se obtienen las claves del objeto recibido como parámetro


        // Se recorren las claves (los campos) del objeto obj
        for (int i = 0; i < keys.length(); i++)
        {
                String key = keys[i];

                // Si la clave no es igual al campo a eliminar, se añade al nuevo objeto
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

                } 
                // Si la clave es igual al campo a eliminar, 
                // se muestra un mensaje de depuración y se 
                // elimina (no se añade al nuevo objeto)
                else { 
                        
                        debug("DEBUG --- REMOVING FIELD: ");
                        debugln(key);
                }
        }

        debug("DEBUG --- NEW OBJECT AFTER REMOVAL: ");
        debugln(JSON.stringify(newObj));

        return newObj;
}