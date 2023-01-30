//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "mupeMQTTdata.h"
#include "mupeMQTTflow.h"
#include "mupeMQTTtools.h"

#include "mupeMdnsNtp.h"
#include "mupeMQTTnvs.h"
#include "mupeMQTT.h"

typedef struct {
	uint8_t Reserved :1;
	uint8_t CleanSession :1;
	uint8_t WillFlag :1;
	uint8_t WillQoS :2;
	uint8_t WillRetain :1;
	uint8_t Password :1;
	uint8_t UserName :1;
} connectFlagS;

static const char *TAG = "mupeMQTTflow";

void createPublish(uint8_t **buffer, uint8_t willQoS, uint8_t willRetain,
		char *willTopicP, uint16_t swt, uint8_t *willMessageP, uint32_t swm,
		uint8_t protocolLevel) {

	ESP_LOGD(TAG, "willTopicP Ende -1 %c", willTopicP[swt - 1]);
	ESP_LOGD(TAG, "willTopicP Ende -2 %c", willTopicP[swt - 2]);
	if (willTopicP[swt - 1] == '\0') {
		swt--;
	}

	uint8_t lenghtsize = 2;

	if ((swt + swm + 2 + ((willQoS > 0) ? 2 : 0)) > 128) {
		lenghtsize++;
	}
	if ((swt + swm + 2 + ((willQoS > 0) ? 2 : 0)) > 16383) {
		lenghtsize++;
	}
	if ((swt + swm + 2 + ((willQoS > 0) ? 2 : 0)) > 2097151) {
		lenghtsize++;
	}
	if ((swt + swm + 2 + ((willQoS > 0) ? 2 : 0)) > 268435455) {
		lenghtsize++;
	}

	if (protocolLevel == 5) {
		lenghtsize++;
	}

	*buffer = malloc(swt + swm + 2 + ((willQoS > 0) ? 2 : 0) + lenghtsize + 1);
	ESP_LOGI(TAG,
			"createPublish  willQoS %i willRetain %i willTopicP %.*s swt %i protocolLevel %i willMessageP  %.*s swm %lu",
			willQoS, willRetain, swt, willTopicP, swt, protocolLevel, (int )swm,
			(char* )willMessageP, swm);

	FixHeaderS *fixHeader;
	fixHeader = (FixHeaderS*) (*buffer);
	fixHeader->dupFlag = 0;
	fixHeader->mQTTControlPacketType = 3;
	fixHeader->qoSlevel = willQoS;
	fixHeader->retain = willRetain;
	uint8_t pos = 1;
	uint32_t ll = swt + swm + 2 + ((willQoS > 0) ? 2 : 0)
			+ ((protocolLevel == 5) ? 1 : 0);
	if ((ll) <= 127) {
		(*buffer)[pos] = ll;
		pos++;
	} else if (((ll) >= 128) && ((ll) <= 16383)) {
		(*buffer)[pos] = (uint8_t) ((ll % 128) + 128);
		pos++;
		(*buffer)[pos] = (uint8_t) (ll / 128);
		pos++;
	} else if (((ll) >= 16384) && ((ll) <= 2097151)) {

		(*buffer)[pos] = (ll % 16384) % 128;
		pos++;
		(*buffer)[pos] = (ll % 16384) / 128 + 128;
		pos++;
		(*buffer)[pos] = (ll / 16384) + 128;
		pos++;
	} else if (((ll) >= 2097152) && ((ll) <= 268435455)) {
		(*buffer)[pos] = ((ll % 2097152) % 16384) % 128;
		pos++;
		(*buffer)[pos] = ((ll % 2097152) % 16384) / 128 + 128;
		pos++;
		(*buffer)[pos] = (ll % 2097152) / 16384 + 128;
		pos++;
		(*buffer)[pos] = (ll / 2097152) + 128;
		pos++;
	}
	(*buffer)[pos] = (swt) / 256;
	pos++;
	(*buffer)[pos] = (swt) % 256;
	pos++;
	memcpy(&(*buffer)[pos], willTopicP, swt);
	pos = pos + swt;
	if (willQoS != 0) {
		(*buffer)[pos] = '-';
		pos++;
		(*buffer)[pos] = '-';
		pos++;
	}
	if (protocolLevel == 5) {
		(*buffer)[pos] = 0;
		pos++;
	}

	memcpy(&(*buffer)[pos], willMessageP, swm);

	ESP_LOGD(TAG, " ll %lu %lu %lu;", ll, getRemainingLength((*buffer)),
			pos + swm);
}

ConnackS connectMQTT(uint8_t *buffer, uint16_t *keepAlive, int sock,
		uint8_t *protocolLevel, uint8_t *willRetain,PropertiesS properties) {
	;
	ESP_LOGI(TAG, "connectMQTT %i", sock);

	char *userName;
	char *password;

	ConnackS connack;
	connectFlagS *connectFlag;
	char *clientIdentifier;

	int pos = getSizeOfRemainingLength(buffer) + 1;

	ESP_LOGD(TAG, "length : %i", buffer[pos] * 256 + buffer[pos + 1]);
	ESP_LOGD(TAG, "mqtt : %s", (char* )&buffer[pos + 2]);
	pos = pos + buffer[pos] * 256 + buffer[pos + 1] + 2;

	ESP_LOGD(TAG, "protocolLevel : %i", buffer[pos]);
	*protocolLevel = buffer[pos];
	pos++;
	ESP_LOGD(TAG, "connectFlagBits : %i", buffer[pos]);

	connectFlag = (connectFlagS*) &buffer[pos];
	ESP_LOGD(TAG, "UserName : %i", connectFlag->UserName);
	ESP_LOGD(TAG, "Password : %i", connectFlag->Password);
	ESP_LOGD(TAG, "WillRetain : %i", connectFlag->WillRetain);
	*willRetain = connectFlag->WillRetain;
	ESP_LOGD(TAG, "WillQoS : %i", connectFlag->WillQoS);
	ESP_LOGD(TAG, "WillFlag : %i", connectFlag->WillFlag);
	ESP_LOGD(TAG, "CleanSession : %i", connectFlag->CleanSession);
	ESP_LOGD(TAG, "Reserved : %i", connectFlag->Reserved);
	pos++;
	ESP_LOGD(TAG, "keepAlive : %i", buffer[pos] * 256 + buffer[pos + 1]);
	*keepAlive = buffer[pos] * 256 + buffer[pos + 1];
	pos++;
	pos++;
	if (*protocolLevel == 5) {
		pos = getProperties(pos, buffer, properties);
	}
	clientIdentifier = getString((uint8_t*) &buffer[pos]);
	pos = pos + buffer[pos] * 256 + buffer[pos + 1] + 2;

	ESP_LOGD(TAG, "clientIdentifier : %s", clientIdentifier);
	if (connectFlag->WillFlag == 1) {

		if (*protocolLevel == 5) {
			pos = getProperties(pos, buffer, properties);
		}
		uint16_t ls1 = 0;
		uint16_t ls2 = 0;
		uint8_t qosdelta = (connectFlag->WillQoS == 0) ? 0 : 2;

		ls1 = buffer[pos] * 256 + buffer[pos + 1];
		pos++;
		pos++;
		char *posx = (char*) &buffer[pos];
		ESP_LOGD(TAG, "willTopic : '%.*s'", ls1, (uint8_t* )posx);
		pos = pos + ls1;
		ls2 = buffer[pos] * 256 + buffer[pos + 1];
		pos++;
		pos++;
		uint8_t *posy = &buffer[pos];

		ESP_LOGD(TAG, "willMessage : '%.*s'", ls2, posy);
		pos = pos + ls2;

		uint8_t *tx_buffer;
		createPublish(&tx_buffer, connectFlag->WillQoS, connectFlag->WillRetain,
				posx, ls1, posy, ls2, 4);
		ESP_LOGD(TAG, "size  : %lu %i %i ", getRemainingLength(tx_buffer),
				getSizeOfRemainingLength(tx_buffer), ls1 + ls2 + 2 + qosdelta);
		storeMQTTData(tx_buffer, sock + 100, 4);
		free(tx_buffer);
	}

	if (connectFlag->UserName == 1) {
		userName = getString((uint8_t*) &buffer[pos]);
		uint16_t lss = buffer[pos] * 256 + buffer[pos + 1];
		pos = pos + buffer[pos] * 256 + buffer[pos + 1] + 2;
		ESP_LOGD(TAG, "userName : '%.*s' ", lss, userName);
	}
	if (connectFlag->Password == 1) {
		password = getString((uint8_t*) &buffer[pos]);
		uint16_t lss = buffer[pos] * 256 + buffer[pos + 1];
		pos = pos + buffer[pos] * 256 + buffer[pos + 1] + 2;
		ESP_LOGD(TAG, "password : '%.*s' ", lss, password);
	}
	connack.fixheader.dupFlag = 0;
	connack.fixheader.qoSlevel = 0;
	connack.fixheader.retain = 0;
	connack.fixheader.mQTTControlPacketType = 2;
	connack.fixheader.remainingLength = 2;
	connack.connectAcknowledgeFlags = 1;
	connack.connectReturnCode = 0;
	if (*protocolLevel == 5) {
		connack.fixheader.remainingLength = 3;
		connack.connectPropertyLength = 0;
	}
	return connack;
}

FixHeaderPiS publishMQTT(uint8_t *buffer, uint8_t lx, uint8_t qoSlevel,
		uint8_t protocolLevel,PropertiesS properties) {
	ESP_LOGI(TAG, "publishMQTT ");
	FixHeaderPiS puback;


	int pos = getSizeOfRemainingLength(buffer) + 1;

	uint16_t l0 = buffer[pos] * 256 + buffer[pos + 1];
	pos++;
	pos++;

	ESP_LOGD(TAG, "topicName : %*.s", l0, &buffer[pos + 1]);

	pos = pos + l0;

	if (qoSlevel > 0) {
		uint16_t pident = buffer[pos] * 256 + buffer[pos + 1];
		ESP_LOGD(TAG, "Packet Identifier : %x %i %i %c %c", pident, buffer[pos],
				buffer[pos + 1], buffer[pos], buffer[pos + 1]);
		uint8_t pi1 = buffer[pos];
		uint8_t pi2 = buffer[pos + 1];
		pos++;
		pos++;
		puback.packetIdentifier1 = pi1;
		puback.packetIdentifier2 = pi2;
	}

	if (protocolLevel == 5) {
		pos = getProperties(pos, buffer, properties);
	}

	uint8_t *message;

	message = &(buffer[pos]);
	ESP_LOGD(TAG, "message : %.*s",
			(int )(getRemainingLength(buffer) - getSizeOfRemainingLength(buffer)
					- l0 - ((qoSlevel == 0) ? 0 : 2) - 2), message);
	puback.fixheader.dupFlag = 0;
	puback.fixheader.qoSlevel = 0;
	puback.fixheader.retain = 0;
	if (qoSlevel == 1) {
		puback.fixheader.mQTTControlPacketType = 4;
	} else {
		puback.fixheader.mQTTControlPacketType = 5;
	}

	if (protocolLevel == 5) {
		puback.fixheader.remainingLength = 4;
		puback.reasonCode = 0;
		puback.propertyLength = 0;
	} else {
		puback.fixheader.remainingLength = 2;
	}
	return puback;
}

typedef struct retainStruct {
	char *topic;
	int sock;
	uint8_t protocolLevel;
	uint8_t qos;
} retainStruct;

void retain(void *pvParameters) {
	retainStruct ret = *(retainStruct*) pvParameters;
	ESP_LOGI(TAG, "retain %s", ret.topic);
	topicItraInit();

	while (topicItraInit()) {
		vTaskDelay(50);
		char key[20];
		topicItraGetKey(key);
		if (atoi(key) > 1000) {
			char a[topicGetSize(key)];
			topicGet(key, a);
			ESP_LOGV(TAG, "RetainList ****** : %s %s", a, key);

			ESP_LOGV(TAG, "RetainListTopic ****** : %s %s", a, ret.topic);
			if (isTopic(a, ret.topic)) {
				uint8_t *buffer = malloc(messageGetSize(key));
				messageGet(key, buffer);
				uint8_t *rx_buffer;
				createPublish(&rx_buffer, 0, 1, a, strlen(a), buffer,
						messageGetSize(key), ret.protocolLevel);
				transmit(ret.sock, (FixHeaderS *)rx_buffer);
				free(rx_buffer);
				free(buffer);
			}
		}
		topicItraNext();

	}
	free(ret.topic);
	vTaskDelete(NULL);
}
static 	 retainStruct retpar;

SubackS subscribeMQTT(uint8_t *buffer, uint8_t lx, int sock,
		uint8_t protocolLevel,PropertiesS properties) {
	ESP_LOGI(TAG, "subscribeMQTT %i", sock);
	esp_log_level_set("*", ESP_LOG_VERBOSE);

	int pos = getSizeOfRemainingLength(buffer) + 1;

	uint16_t pident = buffer[pos] * 256 + buffer[pos + 1];
	ESP_LOGD(TAG, "Packet Identifier : %x %i %i", pident, buffer[pos],
			buffer[pos + 1]);
	uint8_t pi1 = buffer[pos];
	uint8_t pi2 = buffer[pos + 1];
	pos++;
	pos++;
	SubackS suback;
	suback.fixheader.remainingLength = 2;
	if (protocolLevel == 5) {
		pos = getProperties(pos, buffer, properties);
	}

	do {
		uint16_t l0 = buffer[pos] * 256 + buffer[pos + 1];
		pos++;
		pos++;
		ESP_LOGD(TAG, "laenge topicName : %i", l0);

		char topicName[l0 + 1];
		memcpy(topicName, &buffer[pos], l0);
		topicName[l0] = '\0';
		ESP_LOGD(TAG, "topicName : %s", (char* )topicName);

		pos = pos + l0;
		uint8_t requestedQoS;
		uint8_t noLocal;
		uint8_t retainAsPublished;
		uint8_t retainHandling;
		requestedQoS = buffer[pos] & 0x03;
		ESP_LOGD(TAG, "requestedQoS : %i", requestedQoS);
		if (protocolLevel == 5) {
			noLocal = buffer[pos] & 0x04;
			ESP_LOGD(TAG, "noLocal : %i", noLocal);
			retainAsPublished = buffer[pos] & 0x08;
			ESP_LOGD(TAG, "retainAsPublished : %i", retainAsPublished);
			retainHandling = buffer[pos] & 0x48;
			ESP_LOGD(TAG, "retainHandling : %i", retainHandling);
		}
		pos++;

		suback.returnCode[suback.fixheader.remainingLength - 2
				+ ((protocolLevel == 5) ? 1 : 0)] = requestedQoS;
		suback.fixheader.remainingLength++;

		topicListAdd(topicName, sock, protocolLevel, requestedQoS);


		retpar.topic = malloc(strlen(topicName)+1);
			strcpy(retpar.topic,	topicName);
		retpar.sock = sock;
		retpar.protocolLevel = protocolLevel;
		retpar.qos = requestedQoS;

		xTaskCreatePinnedToCore(retain, "retain", 4096, &retpar, 1, NULL, 0);

	} while (pos
			< (getRemainingLength(buffer) - getSizeOfRemainingLength(buffer) - 1));

	suback.fixheader.dupFlag = 0;
	suback.fixheader.qoSlevel = 0;
	suback.fixheader.retain = 0;
	suback.fixheader.mQTTControlPacketType = 9;
	suback.packetIdentifier1 = pi1;
	suback.packetIdentifier2 = pi2;
	suback.returnCode[0] = 0;
	suback.fixheader.remainingLength = suback.fixheader.remainingLength
			+ ((protocolLevel == 5) ? 1 : 0);

	return suback;
}

FixHeaderPiS unSubscribeMQTT(uint8_t *buffer, uint8_t lx, int sock,
		uint8_t protocolLevel,PropertiesS properties) {

	ESP_LOGI(TAG, "unSubscribeMQTT ***********");
	uint8_t *payload;
	uint8_t ln = 0;
	int pos = getSizeOfRemainingLength(buffer) + 1;
	uint16_t pident = buffer[pos] * 256 + buffer[pos + 1];
	ESP_LOGD(TAG, "Packet Identifier : %x %i %i", pident, buffer[pos],
			buffer[pos + 1]);
	uint8_t pi1 = buffer[pos];
	uint8_t pi2 = buffer[pos + 1];
	FixHeaderPiS suback;
	suback.fixheader.remainingLength = 2;
	pos++;
	pos++;

	if (protocolLevel == 5) {
		pos = getProperties(pos, buffer, properties);
	}

	payload = &(buffer[pos]);

	ln = pos;

	do {
		ESP_LOGD(TAG, "lx : %i", lx);
		ESP_LOGD(TAG, "ln : %i", ln);
		uint16_t l0 = payload[0] * 256 + payload[1];
		ESP_LOGD(TAG, "lo : %i", l0);

		char topicName[l0 + 1];
		for (int x1 = 0; x1 < l0; ++x1) {
			topicName[x1] = payload[x1 + 2];
			ln++;
		}
		topicName[l0] = '\0';
		ESP_LOGD(TAG, "topicName : %s", (char* )topicName);
		uint8_t requestedQoS;
		ln++;
		requestedQoS = payload[l0 + 2];
		ESP_LOGD(TAG, "requestedQoS : %i", requestedQoS);

		payload = &(payload[l0 + 3]);
		topicListRemove(topicName, sock);
	} while (ln < lx - 4);

	suback.fixheader.dupFlag = 0;
	suback.fixheader.qoSlevel = 0;
	suback.fixheader.retain = 0;
	suback.fixheader.mQTTControlPacketType = 11;
	suback.fixheader.remainingLength = 2;

	suback.packetIdentifier1 = pi1;
	suback.packetIdentifier2 = pi2;
	if (protocolLevel == 5) {
		suback.fixheader.remainingLength = 4;
		suback.reasonCode = 0;
		suback.propertyLength = 0;
	} else {
		suback.fixheader.remainingLength = 2;
	}
	return suback;
}

FixHeaderPiS pubrelMQTT(uint8_t *buffer, uint8_t lx, uint8_t protocolLevel, PropertiesS properties) {
	int pos = getSizeOfRemainingLength(buffer) + 1;
	ESP_LOGI(TAG, "pubrelMQTT ***********");
	FixHeaderPiS pubcomp;
	uint16_t pident = buffer[pos] * 256 + buffer[pos + 1];
	ESP_LOGD(TAG, "Packet Identifier : %x %i %i", pident, buffer[pos],
			buffer[pos + 1]);
	uint8_t pi1 = buffer[pos];
	uint8_t pi2 = buffer[pos + 1];
	pubcomp.fixheader.dupFlag = 0;
	pubcomp.fixheader.qoSlevel = 0;
	pubcomp.fixheader.retain = 0;
	pubcomp.fixheader.mQTTControlPacketType = 7;
	pubcomp.fixheader.remainingLength = 2;
	pubcomp.packetIdentifier1 = pi1;
	pubcomp.packetIdentifier2 = pi2;
	if (protocolLevel == 5) {
		pubcomp.fixheader.remainingLength = 4;
		pubcomp.reasonCode = 0;
		pubcomp.propertyLength = 0;
	}

	return pubcomp;
}
FixHeaderPiS pubrecMQTT(uint8_t *buffer, uint8_t lx, uint8_t protocolLevel,PropertiesS properties) {
	ESP_LOGI(TAG, "pubrecMQTT ***********");
	int pos = getSizeOfRemainingLength(buffer) + 1;
	FixHeaderPiS pubrel;
	uint16_t pident = buffer[pos] * 256 + buffer[pos + 1];
	ESP_LOGD(TAG, "Packet Identifier : %x %i %i", pident, buffer[pos],
			buffer[pos + 1]);
	uint8_t pi1 = buffer[pos];
	uint8_t pi2 = buffer[pos + 1];

	pubrel.fixheader.mQTTControlPacketType = 6;
	pubrel.fixheader.dupFlag = 0;
	pubrel.fixheader.qoSlevel = 1;
	pubrel.fixheader.retain = 0;

	pubrel.fixheader.remainingLength = 2;
	pubrel.packetIdentifier1 = pi1;
	pubrel.packetIdentifier2 = pi2;

	if (protocolLevel == 5) {
		pubrel.fixheader.remainingLength = 4;
		pubrel.reasonCode = 0;
		pubrel.propertyLength = 0;
	}

	return pubrel;
}

sendPublishS sendpublishMQTT(uint8_t *buffer, uint8_t len, char *topic,
		uint8_t lenTopic, uint8_t qoslevel) {
	ESP_LOGI(TAG, "sendpublishMQTT ***********");
	ESP_LOGD(TAG, "buffer len  : %i", len);
	ESP_LOGD(TAG, "topic len  : %i", lenTopic);
	sendPublishS sendPublish;
	sendPublish.mQTTControlPacketType = 3;
	sendPublish.dupFlag = 0;
	sendPublish.qoSlevel = qoslevel;
	uint8_t dx = 0;
	if (qoslevel != 0) {
		dx = 2;
	}
	sendPublish.retain = 1;
	sendPublish.remainingLength = len + lenTopic + 2 + dx;
	sendPublish.msg[0] = 0;
	sendPublish.msg[1] = lenTopic;
	memcpy(&sendPublish.msg[2], topic, lenTopic);

	if (dx != 0) {
		sendPublish.msg[3 + lenTopic - 1] = 1;
		sendPublish.msg[4 + lenTopic - 1] = 1;
	}
	memcpy(&sendPublish.msg[3 + lenTopic - 1 + dx], buffer, len);
	return sendPublish;
}

FixHeaderS pingrequestMQTT(uint8_t *buffer, uint8_t lx) {
	ESP_LOGI(TAG, "pingrequestMQTT ***********");
	FixHeaderS fixheader;
	fixheader.dupFlag = 0;
	fixheader.qoSlevel = 0;
	fixheader.retain = 0;
	fixheader.mQTTControlPacketType = 13;
	fixheader.remainingLength = 0;
	return fixheader;
}

void disconnectMQTT(uint8_t *buffer, uint8_t lx) {
	ESP_LOGI(TAG, "disconnectMQTT ***********");
}
/*
 uint8_t retainSend(int sock, uint8_t protocolLevel,
 void (*transmit)(const int sock, uint8_t *rx_buffer)) {
 ESP_LOGD(TAG, "sendRetain %i", sock);
 while (retainList != NULL) {
 RetainListS *ret = retainList;
 ESP_LOGV(TAG, "RetainList ****** : %s ", ret->topic);
 while (topicItraInit()) {
 char key[20];
 topicItraGetKey(key);
 if (atoi(key) > 1000) {
 char a[topicGetSize(key)];
 topicGet(key, a);
 ESP_LOGV(TAG, "RetainList ****** : %s %s", a, key);

 ESP_LOGV(TAG, "RetainListTopic ****** : %s %s", a, ret->topic);
 if (isTopic(a, ret->topic)) {
 uint8_t *buffer = malloc(messageGetSize(key));
 messageGet(key, buffer);
 uint8_t *rx_buffer;
 createPublish(&rx_buffer, 0, 1, a, strlen(a), buffer,
 sizeof(buffer), protocolLevel);
 (*transmit)(sock, rx_buffer);
 FixHeaderS *header;
 header = (FixHeaderS*) &buffer[0];
 if (header->qoSlevel != 0) {
 if (topicItraNext()) {
 retainList = ret->next;
 free(ret->topic);
 free(ret);
 }
 free(rx_buffer);
 free(buffer);
 return 1;
 }
 free(rx_buffer);
 free(buffer);
 }

 }
 topicItraNext();
 }
 retainList = ret->next;
 free(ret->topic);
 free(ret);
 }
 return 0;
 }
 */
