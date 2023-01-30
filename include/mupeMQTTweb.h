
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

