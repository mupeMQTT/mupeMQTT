
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

#ifndef MUPEMQQTF
#define MUPEMQQTF


#include "freertos/FreeRTOS.h"
#include "mupeMQTTV5.h"

typedef struct
{
	uint8_t retain : 1;
	uint8_t qoSlevel : 2;
	uint8_t dupFlag : 1;
	uint8_t mQTTControlPacketType : 4;
	uint8_t remainingLength;
} FixHeaderS;

typedef struct
{
	FixHeaderS fixheader;
	uint8_t connectAcknowledgeFlags;
	uint8_t connectReturnCode;
	uint8_t connectPropertyLength;
} ConnackS;

typedef struct
{
	FixHeaderS fixheader;
	uint8_t packetIdentifier1;
	uint8_t packetIdentifier2;
	uint8_t reasonCode;
	uint8_t propertyLength;
} FixHeaderPiS;

typedef struct
{
	FixHeaderS fixheader;
	uint8_t packetIdentifier1;
	uint8_t packetIdentifier2;
	uint8_t returnCode[20];
} SubackS;

typedef struct
{
	uint8_t retain : 1;
	uint8_t qoSlevel : 2;
	uint8_t dupFlag : 1;
	uint8_t mQTTControlPacketType : 4;
	uint8_t remainingLength;
	uint8_t msg[253];
} sendPublishS;

const char *byte_to_binary(uint8_t x);

void createPublish(uint8_t **buffers, uint8_t willQoS, uint8_t willRetain, char *willTopicP, uint16_t swt, uint8_t *willMessageP, uint32_t swm,uint8_t protocolLevel);
ConnackS connectMQTT(uint8_t *buffer, uint16_t *keepAlive, int sock, uint8_t *protocolLevel,uint8_t * willRetain,PropertiesS properties);
FixHeaderPiS publishMQTT(uint8_t *buffer, uint8_t lx, uint8_t qoSlevel,uint8_t protocolLevel,PropertiesS properties);
SubackS subscribeMQTT(uint8_t *buffer, uint8_t lx, int sock, uint8_t protocolLevel, PropertiesS properties);
FixHeaderPiS unSubscribeMQTT(uint8_t *buffer, uint8_t lx, int sock,uint8_t protocolLevel, PropertiesS properties);
FixHeaderPiS pubrelMQTT(uint8_t *buffer, uint8_t lx,uint8_t protocolLevel, PropertiesS properties);
sendPublishS sendpublishMQTT(uint8_t *buffer, uint8_t len, char *topic,
							 uint8_t lenTopic, uint8_t qoslevel);
FixHeaderS pingrequestMQTT(uint8_t *buffer, uint8_t lx);
FixHeaderPiS pubrecMQTT(uint8_t *buffer, uint8_t lx,uint8_t protocolLevel,PropertiesS properties);
void disconnectMQTT(uint8_t *buffer, uint8_t lx);

void topicListRemove(char *topic, int sock);

uint8_t retainSend(int sock, uint8_t protocolLevel, void (*transmit)(const int sock, uint8_t *rx_buffer));


#endif
