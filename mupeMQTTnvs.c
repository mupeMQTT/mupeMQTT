
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

#include "nvs_flash.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "string.h"
#include "mupeMQTTnvs.h"
#include "mupeMdnsNtp.h"
#include "mupeMQTTflow.h"
#include "mupeMQTTtools.h"

#define MQTTDATA "MQTTData"
#define MQTTDATATOPIC "MQTTDataTopic"


static const char *TAG = "mupeMQTTnvs";

nvs_iterator_t itra = NULL;

void mupeMQTTnvsInit(void) {
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
}

uint8_t* messageGet( char *key, uint8_t *buffer) {
	nvs_handle_t my_handle;
	nvs_open(MQTTDATA, NVS_READWRITE, &my_handle);
	size_t required_size = 0;
	nvs_get_blob(my_handle, key, NULL, &required_size);
	if (required_size == 0) {
		return NULL;
	}
	ESP_LOGV(TAG, "sendPublish size %i", required_size);
	nvs_get_blob(my_handle, key, buffer, &required_size);
	nvs_close(my_handle);
	return buffer;
}

size_t messageGetSize( char *key) {
	nvs_handle_t my_handle;
	nvs_open(MQTTDATA, NVS_READWRITE, &my_handle);
	size_t required_size = 0;
	nvs_get_blob(my_handle, key, NULL, &required_size);
	nvs_close(my_handle);
	return required_size;
}

void messageSet( char *key, uint8_t *buffer,size_t size) {
	nvs_handle_t my_handle;
	nvs_open(MQTTDATA, NVS_READWRITE, &my_handle);
	nvs_set_blob(my_handle, key, buffer, size);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

void messageDel( char *key) {
	nvs_handle_t my_handle;
	nvs_open(MQTTDATA, NVS_READWRITE, &my_handle);
	nvs_erase_key(my_handle, key);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

size_t messageGetSizeI(int sock) {
	char key[20];
	sprintf(key, "%i", sock);
	return messageGetSize(key);
}

uint8_t* messageGetI(int sock, uint8_t *message) {

	char key[20];
	sprintf(key, "%i", sock);
	messageGet(key, message);
	return message;
}

size_t topicGetSize( char *key) {
	nvs_handle_t my_handle;
	nvs_open(MQTTDATATOPIC, NVS_READWRITE, &my_handle);
	size_t required_size = 0;
	nvs_get_str(my_handle, key, NULL, &required_size);
	nvs_close(my_handle);
	return required_size;
}

char* topicGet( char *key, char *topic) {
	nvs_handle_t my_handle;
	nvs_open(MQTTDATATOPIC, NVS_READWRITE, &my_handle);
	size_t required_size = 0;
	nvs_get_str(my_handle, key, NULL, &required_size);
	nvs_get_str(my_handle, key, topic, &required_size);
	nvs_close(my_handle);

	return topic;
}

void topicSet( char *key, char *topic) {
	nvs_handle_t my_handle;
	nvs_open(MQTTDATATOPIC, NVS_READWRITE, &my_handle);
	nvs_set_str(my_handle, key, topic);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

void topicDel( char *key) {
	nvs_handle_t my_handle;
	nvs_open(MQTTDATATOPIC, NVS_READWRITE, &my_handle);
	nvs_erase_key(my_handle, key);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

bool topicItraInit() {
	if (itra == NULL) {
		nvs_release_iterator(itra);
		nvs_entry_find(NVS_DEFAULT_PART_NAME, MQTTDATATOPIC, NVS_TYPE_ANY,
				&itra);
		return false;
	}
	return true;
}

char* topicItraGetKey(char* key) {
	nvs_entry_info_t info;
	nvs_entry_info(itra, &info);
	strcpy(key,info.key);
	return key;
}
bool topicItraNext(){
	nvs_entry_next(&itra);
	if (itra==NULL){
		nvs_release_iterator(itra);
		return true;

	}
	return false;
}

uint64_t getTopckeyMQTTData(char *topic) {

	ESP_LOGD(TAG, "getTopckeyMQTTData %s", topic);
	nvs_iterator_t it = NULL;
	uint64_t ret = 0;
	nvs_entry_find(NVS_DEFAULT_PART_NAME, MQTTDATATOPIC, NVS_TYPE_ANY, &it);
	while (it != NULL) {
		nvs_entry_info_t info;
		nvs_entry_info(it, &info);
		char a[topicGetSize(info.key)];
		topicGet(info.key, a);

		ESP_LOGV(TAG, "getTopckey = %s %s %lld", topic, a, ret);
		if (strcmp(topic, a) == 0 && atoll(info.key) > 1000) {
			ret = atoll(info.key);
		}
		nvs_entry_next(&it);
	}
	nvs_release_iterator(it);
	ESP_LOGD(TAG, "ret %lld", ret);
	return ret;
}

void storeMQTTData(uint8_t *rx_buffer, int storeMQTTDataTyp, uint8_t protocolLevel){
	ESP_LOGI(TAG, "storeMQTTData  storeMQTTDataTyp  %i protocolLevel %i", storeMQTTDataTyp,protocolLevel);
	uint8_t propLenght=0;

	FixHeaderS *fixHeader = (FixHeaderS *)rx_buffer;
	char topic[getTopicLenght(rx_buffer) + 1];
	getTopicName(rx_buffer, topic, getTopicLenght(rx_buffer) + 1);

	if(protocolLevel==5){
		propLenght=rx_buffer[getTopicLenght(rx_buffer)+ getSizeOfRemainingLength(rx_buffer)+3+((fixHeader->qoSlevel > 0) ? 2 : 0)];

		ESP_LOGD(TAG, "propLenght  %i", propLenght);
		propLenght++;

	}

	uint8_t *message = &rx_buffer[getTopicLenght(rx_buffer) + 2 + getSizeOfRemainingLength(rx_buffer) + 1 + ((fixHeader->qoSlevel > 0) ? 2 : 0)+propLenght];

	uint16_t ms = getRemainingLength(rx_buffer) - getTopicLenght(rx_buffer) - 2 - ((fixHeader->qoSlevel > 0) ? 2 : 0)-propLenght;

	ESP_LOGD(TAG, "storeMQTTData %s %.*s %i", topic, ms, message, storeMQTTDataTyp);

	uint64_t t;
	if (storeMQTTDataTyp < 1000 && storeMQTTDataTyp != 0) {
		t = storeMQTTDataTyp;
	} else {
		t = getTopckeyMQTTData(topic);
		if (t == 0) {
			t = getNowMs();
		}
	}
	char key[20];
	sprintf(key, "%llu", t);
	ESP_LOGD(TAG, "storeMQTTData key %s ", key);
	topicSet(key, topic);
	messageSet(key, message,ms);
}

void removeMQTTData(uint64_t sock) {


	ESP_LOGI(TAG, "removeMQTTData key %llu ", sock);
		char key[20];
		sprintf(key, "%llu", sock);
		messageDel(key);
		topicDel(key);

}

uint32_t topicGetSizeI(int sock) {
	nvs_handle_t my_handle;
	size_t required_size = 0;
	nvs_open(MQTTDATATOPIC, NVS_READWRITE, &my_handle);
	char key[20];
	sprintf(key, "%i", sock);
	nvs_get_str(my_handle, key, NULL, &required_size);
	if (required_size) {
		ESP_LOGD(TAG, "getMQTTDataTopicSize sock : %i   size %i", sock,
				required_size);
	}
	nvs_close(my_handle);
	return required_size;
}

uint8_t topicGetI(int sock, char *topic) {

	ESP_LOGD(TAG, "getMQTTDataTopic sock : %i", sock);
	char key[20];
	sprintf(key, "%i", sock);
	topicGet(key, topic);
	if (strlen(key) == 0) {
		ESP_LOGV(TAG, "no Data");
		return 0;
	}
	return 1;
}

void removeNVAMQTTAll() {
	nvs_handle_t my_handle;
	ESP_LOGD(TAG, "removeNVAMQTTAll");

	nvs_open(MQTTDATATOPIC, NVS_READWRITE, &my_handle);
	nvs_erase_all(my_handle);
	nvs_commit(my_handle);
	nvs_close(my_handle);
	nvs_open(MQTTDATA, NVS_READWRITE, &my_handle);
	nvs_erase_all(my_handle);
	nvs_commit(my_handle);
	nvs_close(my_handle);

}

void topicListToWeb(httpd_req_t *req) {
	ESP_LOGD(TAG, "getNVAtopicList");
	nvs_iterator_t it = NULL;

	nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY, &it);

	while (it != NULL) {
		nvs_entry_info_t info;
		nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
		if (strcmp(info.namespace_name, "nvs.net80211")
				&& strcmp(info.namespace_name, "phy") && strcmp(info.namespace_name, "WIFIcfg"))
						{
			char a[72];
			sprintf(a, "k '%s', t '%d', n '%s' <br>", info.key, info.type,
					info.namespace_name);
			ESP_LOGV(TAG, "topic key: %s", a);
			httpd_resp_send_chunk(req, a, strlen(a));
		}
		nvs_entry_next(&it);
	}
	nvs_release_iterator(it);
}

void messageGetAllToWeb(httpd_req_t *req) {
	nvs_handle_t my_handle;
	ESP_LOGD(TAG, "getNVAtopicList");
	nvs_iterator_t it = NULL;

	nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY, &it);

	while (it != NULL) {

		nvs_entry_info_t info;
		nvs_entry_info(it, &info);
		if (strcmp(info.namespace_name, "nvs.net80211")
				&& strcmp(info.namespace_name, "phy") && strcmp(info.namespace_name, "WIFIcfg"))
						{
			char a[72];
			sprintf(a, "<br> k '%s', t '%d', n '%s' <br>", info.key, info.type,
					info.namespace_name);
			ESP_LOGV(TAG, "topic key: %s", a);
			httpd_resp_send_chunk(req, a, strlen(a));
			httpd_resp_send_chunk(req, "<pre>", strlen("</pre>"));
			if (info.type == 66) {

				nvs_open(info.namespace_name, NVS_READWRITE, &my_handle);
				size_t required_size = 0;
				char key[20];
				sprintf(key, "%s", info.key);
				nvs_get_blob(my_handle, info.key, NULL, &required_size);
				ESP_LOGV(TAG, "sendPublish size %i", required_size);
				uint8_t *blob = malloc(required_size);
				nvs_get_blob(my_handle, key, blob, &required_size);
				httpd_resp_send_chunk(req, (const char*) blob, required_size);
				free(blob);
				nvs_close(my_handle);
			}
			if (info.type == 33) {
				nvs_open(info.namespace_name, NVS_READWRITE, &my_handle);
				size_t required_size = 0;
				char key[20];
				sprintf(key, "%s", info.key);
				nvs_get_str(my_handle, key, NULL, &required_size);
				ESP_LOGV(TAG, "sendPublish size %i", required_size);
				char *blobc = malloc(required_size);
				nvs_get_str(my_handle, key, blobc, &required_size);
				httpd_resp_send_chunk(req, blobc, required_size);
				free(blobc);
				nvs_close(my_handle);
			}
			httpd_resp_send_chunk(req, "</pre>", strlen("</pre>"));
		}
		nvs_entry_next(&it);
	}
	nvs_release_iterator(it);
}
