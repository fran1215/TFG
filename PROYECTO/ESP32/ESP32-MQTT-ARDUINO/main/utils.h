#ifndef UTILS_H
#define UTILS_H

#include <Arduino_JSON.h>

String genPayload(String client, String method, JSONVar params);

String createInfoPayload(String client, double temp, double humidity, int co2, int tvoc);

String createResponsePayload(String client, JSONVar params);

String receivedCommand(String msg, String client, double temperature, double humidity, int co2, int tvoc);

#endif