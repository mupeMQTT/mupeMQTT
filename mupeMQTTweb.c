#include "mupeMQTTweb.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <esp_event.h>
#include "esp_http_server.h"
#include <stdio.h>
#include <string.h>
#include "mupeMQTTdata.h"
#include "mupeWeb.h"
#include "mupeMQTTnvs.h"



static const char *TAG = "mupeWebHandler";
#define STARTS_WITH(string_to_check, prefix) (strncmp(string_to_check, prefix, (strlen(prefix))))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

esp_err_t root_mqtt_get_handler(httpd_req_t *req) {
	ESP_LOGI(TAG, "HTTPS req %s ", req->uri);

	if (STARTS_WITH(req->uri, "/mqtt/clist") == 0) {
		return clist_get_handler(req);
	}
	if (STARTS_WITH(req->uri, "/mqtt/ndata") == 0) {
		return clist_get_nData(req);
	}
	if (STARTS_WITH(req->uri, "/mqtt/nlist") == 0) {
		return topc_get_handler(req);
	}
	if (STARTS_WITH(req->uri, "/mqtt/deltopic") == 0) {
		return removeNVAtopicList(req);
	}
	if (STARTS_WITH(req->uri, "/mqtt/mqtt.html") == 0) {
		return mqtt_get_handler(req);
	}
	return mqtt_get_handler(req);
}

httpd_uri_t mqtt_t = { .uri = "/mqtt/*", .method = HTTP_GET, .handler =
		root_mqtt_get_handler };

esp_err_t removeNVAtopicList(httpd_req_t *req) {
	ESP_LOGI(TAG, "removeNVAtopicList ");
	removeNVAMQTTAll();
	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, "deleted", 8);
	return ESP_OK;
}

esp_err_t clist_get_nData(httpd_req_t *req) {
	httpd_resp_set_type(req, "text/html");
	messageGetAllToWeb(req);
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

esp_err_t topc_get_handler(httpd_req_t *req) {
	httpd_resp_set_type(req, "text/html");
	topicListToWeb(req);
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

esp_err_t clist_get_handler(httpd_req_t *req) {
	httpd_resp_set_type(req, "text/html");
	topicListGet(req);
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

esp_err_t mqtt_get_handler(httpd_req_t *req) {
	extern const unsigned char mqtt_html_start[] asm("_binary_mqtt_html_start");
	extern const unsigned char mqtt_html_end[] asm("_binary_mqtt_html_end");
	const size_t mqtt_html_size = (mqtt_html_end - mqtt_html_start);
	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char*) mqtt_html_start, mqtt_html_size);
	return ESP_OK;
}
const char * l_txt="MQTT Controll";
void mupeMQTTWebInit(){
	addHttpd_uri(&mqtt_t,l_txt);
}
