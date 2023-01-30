#ifndef MUPEWEBHandler_H
#define MUPEWEBHandler_H
#include "esp_http_server.h"


void mupeMQTTWebInit();


 esp_err_t root_get_handler(httpd_req_t *req);
 esp_err_t default_get_handler(httpd_req_t *req);
 esp_err_t favicon_get_handler(httpd_req_t *req);
 esp_err_t clist_get_handler(httpd_req_t *req);
 esp_err_t topc_get_handler(httpd_req_t *req);
 esp_err_t topcD_get_handler(httpd_req_t *req);
 esp_err_t removeNVAtopicList(httpd_req_t *req);
 esp_err_t clist_get_nData(httpd_req_t *req);
 esp_err_t mqtt_get_handler(httpd_req_t *req);
 esp_err_t root_post_handler(httpd_req_t *req);

#endif

