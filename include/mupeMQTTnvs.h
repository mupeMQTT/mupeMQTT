#ifndef MUPEMNVS_H
#define MUPEMNVS_H
#include "esp_http_server.h"

void mupeMQTTnvsInit(void);

void removeMQTTData(uint64_t sock) ;
void storeMQTTData(uint8_t *rx_buffer, int storeMQTTDataTyp, uint8_t protocolLevel);

//uint8_t topicGetI(int sock, char *topic);

size_t messageGetSize( char *key);
uint8_t* messageGet( char *key, uint8_t * buffer);
uint8_t* messageGetI(int sock, uint8_t *message) ;
size_t messageGetSizeI(int sock);
void messageSet( char *key, uint8_t *buffer,size_t size);

size_t topicGetSize( char *key);
uint32_t topicGetSizeI(int sock);
char* topicGet( char *key, char *topic);
uint8_t topicGetI(int sock, char *topic);
bool topicItraInit();
char* topicItraGetKey(char *);
bool topicItraNext();

void removeNVAMQTTAll();
void topicListToWeb(httpd_req_t *req);
void messageGetAllToWeb(httpd_req_t *req);
uint64_t getTopckeyMQTTData(char *topic);







#endif
