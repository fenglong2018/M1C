idf_component_register(SRCS "User_http_ota.c" "User_WiFi.c"     
                    INCLUDE_DIRS .
                    REQUIRES console nvs_flash esp_http_client esp_https_ota app_update
EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem)