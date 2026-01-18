/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/param.h>
// #include <stdlib.h>
// #include <ctype.h>
#include "esp_log.h"
// #include "esp_event.h"
// #include "esp_netif.h"
// #include "protocol_examples_common.h"
// #include "protocol_examples_utils.h"
#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "station.h"
#include "esp_http_client.h"
#include "justinjson.h"

#include "gtfs-realtime.pb-c.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG_htt = "HTTP_CLIENT";

/* Root cert for howsmyssl.com, taken from howsmyssl_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/

extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[] asm("_binary_howsmyssl_com_root_cert_pem_end");

void do_stuff(uint8_t *buffer, size_t len)
{
    printf("Doing the stuff\n");
    FeedMessage *m = feed_message__unpack(NULL, len, buffer);
    if (m != NULL)
    {
        printf("Blah %ld\n", (long int)m->header->timestamp);

        for (int i = 0; i < m->n_entity; i++)
        {
            VehiclePosition *v = m->entity[i]->vehicle;
            if (v)
            {
                printf("V pointers: %p\n", v);
                TripDescriptor *t = v->trip;
                // bool dir = t->direction_id;
                printf("T pointers: %p\n", t);
                if (t)
                {
                    printf("Train: %s %s %d\n", t->route_id, v->stop_id, (int)t->direction_id);
                }
                else
                {
                    printf("Invalid train info!\n");
                }
                // int route = strtol(t->route_id);
                // int stop = strtol(t->route_id);
            } else {
                printf("Invalid vehicle!");
            }
        }
    }
    // printf("pointer %p", response_buffer);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    static int content_len;
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG_htt, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG_htt, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG_htt, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG_htt, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG_htt, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // Clean the buffer in case of a new request
        printf("arrr we made it here\n");

        if (output_len == 0 && evt->user_data)
        {
            // we are just starting to copy the output data into the use
            memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
        }
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            int copy_len = 0;
            if (evt->user_data)
            {
                // should not be in use
                // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                content_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    // printf("buffer here of len %d\n", content_len);
                    // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                    output_buffer = (char *)calloc(content_len + 1, sizeof(char));
                    printf("content len: %d\n", content_len);

                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG_htt, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (content_len - output_len));
                printf("data len: %d\n", copy_len);
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
            printf("output len: %d\n", copy_len);
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        // ESP_LOGD(TAG_htt, "HTTP_EVENT_ON_FINISH");
        // if (output_buffer != NULL)
        // {
        //     free(output_buffer);
        //     output_buffer = NULL;
        // }
        // output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_htt, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG_htt, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG_htt, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        printf("Pointer %p length %d\n", output_buffer, content_len);
        if (output_buffer != NULL)
        {
            do_stuff((uint8_t *)output_buffer, content_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
        
    case HTTP_EVENT_REDIRECT:
        break;
    }
    return ESP_OK;
}

uint8_t parse_json(int32_t start_indx, char *response_buffer, VehicleData *vehicles)
{
    char field[64];
    printf("start buffer %d\n", (int)start_indx);
    int32_t id;
    char vbuffer[32];

    // printf("start: %s", response_buffer + start_indx);
    printf("\n");
    uint8_t vehicle_index = 0;

    if (!json_read(response_buffer, start_indx))
    {
        printf("Failed to parse\n");
    }
    else
    {
        // return printf("failed to read output", 0;
        json_obj_iter(field, 64)
        {
            printf("iterating main obj\n");
            if (!strcmp(field, "entity"))
            {
                // if (json_arr())
                // {
                printf("found entity\n");
                json_arr_iter()
                {
                    printf("iterating v arr\n");
                    // if (json_obj())
                    // {
                    json_obj_iter(field, sizeof(field))
                    {
                        printf("\titerating v obj\n");
                        if (!strcmp(field, "vehicle"))
                        {
                            VehicleData v = {};

                            json_obj_iter(field, sizeof(field))
                            {
                                if (!strcmp(field, "trip"))
                                {
                                    // if (json_obj())
                                    // {
                                    json_obj_iter(field, sizeof(field))
                                    {
                                        printf("\t\titerating trip\n");
                                        if (!strcmp(field, "routeId"))
                                        {
                                            if (json_str(vbuffer, sizeof(vbuffer)))
                                                v.routeId = strtol(field, 0, 10);
                                            // v.routeId = 8181;
                                            // printf("\t\t\troute id %s\n", vbuffer);
                                        }
                                        else if (!strcmp(field, "directionId"))
                                        {
                                            if (json_int(&id))
                                                v.dir = (bool)id;
                                            //     printf("\t\t\tdir id %ld\n", id);
                                            // else
                                            //     printf("\t\t\tmalformed dir\n");
                                        }
                                        else
                                        {
                                            json_skip(false);
                                        }
                                    }
                                    // }
                                    // else
                                    //     printf("\t\ttrip not an obj\n");
                                }
                                else if (!strcmp(field, "stopId"))
                                {
                                    if (json_str(vbuffer, sizeof(vbuffer)))
                                    {
                                        v.stopId = strtol(field, 0, 10);
                                        // v.stopId = 10;
                                        if (v.stopId > 0)
                                        {
                                            v.stopId %= 10000;
                                        }
                                    }
                                    //     printf("\t\t\tstopId %s\n", vbuffer);
                                    // else
                                    //     printf("\t\t\tmalformed stopId\n");
                                }
                                else
                                {
                                    json_skip(false);
                                }
                            }
                            vehicles[vehicle_index] = v;
                            vehicle_index++;
                        }
                        else
                        {
                            json_skip(false);
                        }
                    }
                }
            }
            else
            {
                json_skip(false);
            }
        }
    }
    return vehicle_index;
}

void https_with_url()
{
    // char* url = ";
    // char buff[512];

    esp_http_client_config_t config = {
        .url = "https://api.goswift.ly/real-time/lametro-rail/gtfs-rt-vehicle-positions",
        .event_handler = _http_event_handler,
        .method = HTTP_METHOD_GET,

        .crt_bundle_attach = esp_crt_bundle_attach,
        // .buffer_size = 5,
        .timeout_ms = 20000,
        .buffer_size_tx = 4096,
        // .buffer_size = 16384,
        // .timeout_ms = 1000,
        // .user_data = buff,
    };

    ESP_LOGI(TAG_htt, "HTTPS request with url =>");
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // esp_http_client_set_method(client, HTTP_METHOD_HEAD);
    esp_http_client_set_header(client, "Accept", "application/json, application/json; charset=utf-8");
    esp_http_client_set_header(client, "Authorization", "");
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int64_t len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG_htt, "HTTPS Status = %d, content_length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 len);

        // char buffer[128];

        if (len > 0)
        {
            // uint8_t *response_buffer = malloc((size_t)len);
            uint8_t *response_buffer = NULL;
            // memset(response_buffer, 0, (size_t)len);
            printf("\n\\allocated response buffer\n");
            if (response_buffer != NULL)
            {
                printf("Read length: %d\n", esp_http_client_read_response(client, (char *)(response_buffer), len));
                printf("First char: %d\n", (int)response_buffer[0]);
                printf("Last char: %d\n", (int)response_buffer[len - 1]);

                FeedMessage *m = feed_message__unpack(NULL, len, response_buffer);
                if (m != NULL)
                {
                    printf("Blah %ld", (long int)m->header->timestamp);
                }
                printf("pointer %p", response_buffer);
                // for(int i = 0; i < m->n_entity; i++) {
                //     // VehiclePosition *v = m->entity[i]->vehicle;
                //     // TripDescriptor *t = v->trip;
                //     // bool dir = t->direction_id;
                //     // printf("Train: %s %s %d\n", t->route_id, v->stop_id, (int)t->direction_id);
                //     // int route = strtol(t->route_id);
                //     // int stop = strtol(t->route_id);
                // }
                feed_message__free_unpacked(m, NULL);

                // esp_http_client_cleanup(client);
                // free(response_buffer);
                // response_buffer[len] = '\0'; // Null-terminate the string

                int32_t i;
                int32_t start_indx = 0;
                UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
                printf("Current Task Stack High Watermark (words): %u\n", watermark);

                for (i = 0; i < len + 4; i++)
                {
                    printf("%02x ", (int)(response_buffer[i]));
                    // char *he = "{\"header";
                    // if (strncmp(response_buffer + i, he, strlen(he)) == 0)
                    // {
                    //     start_indx = i;
                    //     break;
                    // }
                    // printf("%c", response_buffer[i]);
                }
                // printf("start: %s", response_buffer + start_indx);
                // printf("\n");

                // printf("\n");
                // VehicleData vehicles[256];
                // uint8_t num_trains = parse_json(start_indx, response_buffer, vehicles);

                // for (uint8_t i = 0; i < num_trains; i++)
                // {
                //     printf("Vehicle:\tdir: %d\troute: %d\tstop: %d\n", vehicles[i].dir, vehicles[i].routeId, vehicles[i].stopId);
                // }
                // printf("Current Task Stack High Watermark (words): %u\n", watermark);

                // ESP_LOGI(TAG_htt, "HTTP Response: %d %s", i, response_buffer);

                // Process the response data here (e.g., using cJSON)
                // int index = strind
            }
            else
            {
                esp_http_client_cleanup(client);
                ESP_LOGI(TAG_htt, "mallloooccc failed");
            }

            // if (esp_http_client_get_header(client, "Content-Type", buffer) == ESP_OK)
            // {
            //     printf("Content-Type: %s\n", buffer);
            // }
            // while(esp_http_client_read(client, buff, 512) > 0) {
            // }
            // ESP_LOG_BUFFER_HEX(TAG_htt, buff, strlen(buff));
        }
        else
        {
            esp_http_client_cleanup(client);
            ESP_LOGE(TAG_htt, "Error perform http request %s", esp_err_to_name(err));
        }
    }
    while (true)
    {
        printf(".");
        sleep(1);
    }
}

// void app_main(void)
// {
//     // esp_err_t ret = nvs_flash_init();
//     // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
//     // {
//     //     ESP_ERROR_CHECK(nvs_flash_erase());
//     //     ret = nvs_flash_init();
//     // }
//     // ESP_ERROR_CHECK(ret);

//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//      */
//     //     ESP_ERROR_CHECK(example_connect());
//     //     ESP_LOGI(TAG, "Connected to AP, begin http example");

//     // #if CONFIG_IDF_TARGET_LINUX
//     //     http_test_task(NULL);
//     // #else
//     //     xTaskCreate(&http_test_task, "http_test_task", 8192, NULL, 5, NULL);
//     // #endif
// }
