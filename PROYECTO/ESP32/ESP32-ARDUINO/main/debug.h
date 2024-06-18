// Librería para imprimir mensajes de depuración en el puerto serie
// de manera condicional.

#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif