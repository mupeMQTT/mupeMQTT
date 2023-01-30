
// Copyright Peter Müller mupe
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

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
