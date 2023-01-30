#ifndef MUPEMQTTDATA
#define MUPEMQTTDATA
#include "esp_http_server.h"







void topicListAdd(char *topic, int sock, uint8_t protocolLevel, uint8_t qos);
void topicListRemove(char *topic, int sock);
void topicListRemoveSock(int sock);
int topicListGetSock(char *topic,uint8_t *protocolLevel );
void topicListGet(httpd_req_t *req);





#endif
