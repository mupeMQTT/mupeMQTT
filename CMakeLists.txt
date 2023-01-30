idf_component_register(SRCS "mupeMQTTflow.c" "mupeMQTT.c" "mupeMQTTdata.c" "mupeMQTTtools.c" "mupeMQTTV5.c" "mupeMQTTweb.c" "mupeMQTTnvs.c"
                    INCLUDE_DIRS "include"  
                    REQUIRES  "esp_http_server" "nvs_flash" "mupeMdnsNtp" "mupeWeb"
                     EMBED_TXTFILES  "mqtt.html" )

