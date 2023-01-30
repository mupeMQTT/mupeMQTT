
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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <lwip/netdb.h>

#include <mupeMQTT.h>

#include <mupeMQTTdata.h>
#include <mupeMQTTtools.h>
#include "mupeMdnsNtp.h"
#include "mupeMQTTweb.h"
#include "mupeMQTTnvs.h"

#define NUM_RECORDS 100
static const char *TAG = "mupeMQTT";

int _port = 1883;
int _keepIdle = 120;
int _keepInterval = 120;
int _keepCount = 100;

void transmit(const int sock, FixHeaderS *p) {
	ESP_LOGI(TAG, "transmit : sock %i mQTTControlPacketType %i", sock,
			p->mQTTControlPacketType);

	int to_write = getRemainingLength((uint8_t*) p)
			+ getSizeOfRemainingLength((uint8_t*) p) + 1;
	while (to_write > 0) {
		int written = send(sock, p, to_write, 0);
		if (written < 0) {
			ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
			// Failed to retransmit, giving up
			return;
		}
		to_write -= written;
	}
}

uint8_t sendPublish(int sock, uint8_t qos, uint8_t retain) {
	ESP_LOGI(TAG, "sendPublish : sock %i qos %i retain %i", sock, qos, retain);
	int tsock = 0;
	uint8_t ret;
	uint8_t later = 0;
	uint8_t protocolLevel;

	size_t ls2 = messageGetSizeI(sock);
	uint32_t ls1 = topicGetSizeI(sock);

	char *topic = malloc(ls1);
	uint8_t *message = malloc(ls2);

	messageGetI(sock, message);
	if (ls2 == 0) {
		ESP_LOGE(TAG, "Error occurred during getMQTTData:");
	}
	ret = topicGetI(sock, topic);
	ESP_LOGD(TAG, "getMQTTDataTopic: %s", topic);
	if (ret == 0) {
		ESP_LOGE(TAG, "Error occurred during getMQTTDataTopic:");
	}

	int sockt = topicListGetSock(topic, &protocolLevel);
	tsock = sockt;
	uint8_t *rx_buffer = NULL;
	uint8_t protocolLevelLater=0;
	while (sockt > 0) {
		ESP_LOGD(TAG, "sendPublishSend : %s %i", topic, sockt);
		if (sock == sockt) {
			later = 1;
			protocolLevelLater=protocolLevel;
		} else {
			if (rx_buffer != NULL) {
				free(rx_buffer);
				rx_buffer = NULL;
			}
			createPublish(&rx_buffer, qos, retain, topic, ls1, message, ls2,
					protocolLevel);
			transmit(sockt, (FixHeaderS*) rx_buffer);
		}
		sockt = topicListGetSock(topic, &protocolLevel);
		while (sockt == tsock && sockt > 0) {
			sockt = topicListGetSock(topic, &protocolLevel);
			ESP_LOGD(TAG, "getTopicListSock : %i %i", tsock, sockt);
		}
		tsock = sockt;
	}
	if (later == 1) {
		if (rx_buffer != NULL) {
			free(rx_buffer);
			rx_buffer = NULL;
		}
		createPublish(&rx_buffer, qos, retain, topic, ls1, message, ls2,
				protocolLevelLater);
		transmit(sock, (FixHeaderS*) rx_buffer);
	}
	if (ls2 == 0) {
		removeMQTTData(getTopckeyMQTTData(topic));
	}
	free(topic);
	free(message);
	removeMQTTData(sock);
	if (rx_buffer != NULL) {
		free(rx_buffer);
	}

	return 0;
}

void do_retransmit(void *socks) {
	uint8_t *rx_buffer = NULL;
	uint8_t lw_retain = 0;
	uint8_t retain = 0;
	uint8_t qos = 0;

	uint8_t protocolLevel = 0;
	int sock = (int) socks;
	ESP_LOGI(TAG, "do_retransmit :  %i", sock);
	ConnackS connack;
	SubackS suback;
	FixHeaderPiS fixHeaderPi;
	FixHeaderS fixHeader;
	FixHeaderS *header;
	PropertiesS properties;


	removeMQTTData(sock + 100);
	enum ConnectStatus {
		INIT, CONNECT, RETRANSMITT, DISCONNECT, DISCONNECTING
	} status = INIT;

	do {

		int len;
		uint8_t buf[5];
		len = recv(sock, buf, 5, 0);
		uint32_t le = getRemainingLength(buf);
		uint8_t les = getSizeOfRemainingLength(buf);
		if (len <= 0) {
			int ret = 0;
			le = 0;
			les = 0;
			status = DISCONNECT;
			ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);

			uint32_t ls1 = topicGetSizeI(sock + 100);

			char *topic = malloc(ls1);

			uint8_t *message = malloc(messageGetSizeI(sock + 100));
			messageGetI(sock + 100, message);
			if (sizeof(message) == 0) {
				ESP_LOGE(TAG, "Error occurred during getMQTTData:");
			}
			ret = topicGetI(sock + 100, topic);

			if (ret == 0) {
				ESP_LOGE(TAG, "Error occurred during getMQTTDataTopic:");
				topicListRemoveSock(sock);
				break;
			}
			ESP_LOGD(TAG, "getMQTTDataTopic: %s", topic);

			createPublish(&rx_buffer, 0, lw_retain, topic, ls1, message,
					sizeof(message), protocolLevel);
			free(topic);
			free(message);
			topicListRemoveSock(sock);
			removeMQTTData(sock + 100);
		} else {
			rx_buffer = malloc(10 + le + les);
			if (rx_buffer == NULL) {
				ESP_LOGD(TAG, "rx_buffer == NULL");
			}
			memcpy(rx_buffer, buf, 5);
			if ((le + les + 1) != len) {
				len = len + recv(sock, rx_buffer + len, le + les + 1 - len, 0);
			}
			ESP_LOGV(TAG, "*************Memory******** : %i %lu %i", len, le,
					les);
		}
		ESP_LOGD(TAG, "----------------------------------");

		header = (FixHeaderS*) rx_buffer;
		ESP_LOGD(TAG, "mQTTControlPacketType : %i",
				header->mQTTControlPacketType);
		ESP_LOGD(TAG, "dupFlag : %i qoSlevel : %i retain : %i", header->dupFlag,
				header->qoSlevel, header->retain);
		ESP_LOGD(TAG, "remaining Length : %lu Size: %i",
				getRemainingLength(rx_buffer),
				getSizeOfRemainingLength(rx_buffer));

		switch (header->mQTTControlPacketType) {
		case 1: // CONNECT
			if (status == INIT) {
				ESP_LOGD(TAG, "status == INIT");
				uint16_t keepAlive = 0;
				connack = connectMQTT(rx_buffer, &keepAlive, sock,
						&protocolLevel, &lw_retain, properties);
				struct timeval tv;
				tv.tv_sec = keepAlive * 1.5;
				tv.tv_usec = 0;
				if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))
						< 0) {
					ESP_LOGE(TAG, "... failed to set socket receiving timeout");
				}
				ESP_LOGD(TAG, "set socket receiving timeout %lli", tv.tv_sec);
				transmit(sock, (FixHeaderS*) &connack);
				switch (protocolLevel) {
				case 3:
					ESP_LOGD(TAG, "status = CONNECT");
					status = CONNECT;
					break;
				case 4:
					ESP_LOGD(TAG, "status = CONNECT");
					status = CONNECT;
					break;
				case 5:
					ESP_LOGD(TAG, "status = CONNECT");
					status = CONNECT;
					break;
				default:
					status = DISCONNECT;
					ESP_LOGD(TAG, "status = DISCONNECT");
					break;
				}
			} else {
				ESP_LOGD(TAG, "status = DISCONNECT");
				status = DISCONNECT;
			}
			break;
		case 3: // PUBLISH

			storeMQTTData(rx_buffer, sock, protocolLevel);
			retain = header->retain;
			qos = header->qoSlevel;

			if (header->retain == 1) {
				storeMQTTData(rx_buffer, 0, protocolLevel);
			}
			switch (header->qoSlevel) {
			case 0:
				publishMQTT(rx_buffer, header->remainingLength,
						header->qoSlevel, protocolLevel, properties);
				sendPublish(sock, qos, retain);
				break;
			case 1:
				fixHeaderPi = publishMQTT(rx_buffer, header->remainingLength,
						header->qoSlevel, protocolLevel, properties);
				transmit(sock, (FixHeaderS*) &fixHeaderPi);
				sendPublish(sock, qos, retain);
				break;
			case 2:
				fixHeaderPi = publishMQTT(rx_buffer, header->remainingLength,
						header->qoSlevel, protocolLevel, properties);
				transmit(sock, (FixHeaderS*) &fixHeaderPi);
				if (status == DISCONNECT) {
					sendPublish(sock, qos, retain);
				}
				break;
			}
			break;
		case 5: // PUBREC
			fixHeaderPi = pubrecMQTT(rx_buffer, header->remainingLength,
					protocolLevel, properties);
			transmit(sock, (FixHeaderS*) &fixHeaderPi);
			break;
		case 6: // PUBREL
			fixHeaderPi = pubrelMQTT(rx_buffer, header->remainingLength,
					protocolLevel, properties);
			transmit(sock, (FixHeaderS*) &fixHeaderPi);
			sendPublish(sock, qos, retain);
			break;
		case 7: // PUBCOMP
			ESP_LOGD(TAG, "Packet Identifier : %x %i %i",
					rx_buffer[2] * 256 + rx_buffer[3], rx_buffer[2],
					rx_buffer[3]);
			break;
		case 8: // SUBSCRIBE
			suback = subscribeMQTT(rx_buffer, header->remainingLength, sock,
					protocolLevel, properties);
			transmit(sock, (FixHeaderS*) &suback);
			break;
		case 10: // UNSUBSCRIBE
			fixHeaderPi = unSubscribeMQTT(rx_buffer, header->remainingLength,
					sock, protocolLevel, properties);
			transmit(sock, (FixHeaderS*) &fixHeaderPi);
			break;
		case 12: // PINGREQ
			fixHeader = pingrequestMQTT(rx_buffer, header->remainingLength);
			transmit(sock, (FixHeaderS*) &fixHeader);
			break;
		case 14: // DISCONNECT
			disconnectMQTT(rx_buffer, header->remainingLength);
			removeMQTTData(sock + 100);

			topicListRemoveSock(sock);
			status = DISCONNECT;
			break;
		case 15: // DISCONNECT
			ESP_LOGD(TAG,
					"Case 15 *****************************************************************");
			break;
		}
		free(rx_buffer);

		ESP_LOGD(TAG, " xPortGetFreeHeapSize %lu", xPortGetFreeHeapSize());

	} while (status != DISCONNECT);

	removeMQTTData(sock + 100);
	ESP_LOGD(TAG, " task ende del");
	shutdown(sock, 0);
	close(sock);
	vTaskDelete(NULL);
}

void mupeMqttServerTask(void *pvParameters) {
	ESP_LOGI(TAG, "mupeMqttServerTask");
	char addr_str[128];
	int addr_family = AF_INET;
	int ip_protocol = 0;
	int keepAlive = 1;
	int keepIdle = _keepIdle;
	int keepInterval = _keepInterval;
	int keepCount = _keepCount;
	struct sockaddr_storage dest_addr;

	waitForNTPConnect();
	if (addr_family == AF_INET) {
		struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in*) &dest_addr;
		dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr_ip4->sin_family = AF_INET;
		dest_addr_ip4->sin_port = htons(_port);
		ip_protocol = IPPROTO_IP;
	}
	int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
	if (listen_sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
		return;
	}
	int opt = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	ESP_LOGD(TAG, "Socket created");
	int err = bind(listen_sock, (struct sockaddr*) &dest_addr,
			sizeof(dest_addr));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
		ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
		goto CLEAN_UP;
	}
	ESP_LOGD(TAG, "Socket bound, port %d", _port);

	err = listen(listen_sock, 1);
	if (err != 0) {
		ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
		goto CLEAN_UP;
	}
	topicItraInit();
	while (topicItraInit()) {
		char key[20];
		topicItraGetKey(key);
		if (atoi(key) > 100 && atoi(key) < 1000) {
			removeMQTTData(atoi(key));

		}
		topicItraNext();
	}

	while (1) {

		ESP_LOGD(TAG, "Socket listening");

		struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
		socklen_t addr_len = sizeof(source_addr);
		int sock = accept(listen_sock, (struct sockaddr*) &source_addr,
				&addr_len);

		if (sock < 0) {
			ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
			break;
		}

		// Set tcp keepalive option
		setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval,
				sizeof(int));
		setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
		// Convert ip address to string
		if (source_addr.ss_family == PF_INET) {
			inet_ntoa_r(((struct sockaddr_in* )&source_addr)->sin_addr,
					addr_str, sizeof(addr_str) - 1);
		}

		ESP_LOGD(TAG, "Socket accepted ip address: %s", addr_str);

		xTaskCreatePinnedToCore(do_retransmit, "do_retransmit", 4500,
				(void*) sock, 1, NULL, 0);
	}

	CLEAN_UP: close(listen_sock);
	vTaskDelete(NULL);
}

void mupeMqttInit(void) {
	mupeMQTTnvsInit();
	mupeMQTTWebInit();

}
