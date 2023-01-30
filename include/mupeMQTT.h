#ifndef MUPEMQQT
#define MUPEMQQT
#include <mupeMQTTflow.h>


void mupeMqttInit(void );
void mupeMqttServerTask(void *pvParameters);
void transmit(const int sock, FixHeaderS *p);

#endif
