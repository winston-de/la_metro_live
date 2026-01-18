/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/param.h>
#include "esp_log.h"
#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "typedefs.h"
#include "esp_http_client.h"
#include "trainmanager.h"

// #include "gtfs-realtime.pb-c.h"
#include "secrets.h"

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
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {

            content_len = esp_http_client_get_content_length(evt->client);
            if (output_buffer == NULL)
            {
                // printf("buffer here of len %d\n", content_len);
                // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                output_buffer = (char *)calloc(content_len + 1, sizeof(char));

                output_len = 0;
                if (output_buffer == NULL)
                {
                    ESP_LOGI(TAG_htt, "Failed to allocate memory for output buffer");
                    return ESP_FAIL;
                }
            }
            int copy_len = MIN(evt->data_len, (content_len - output_len));
            if (copy_len)
            {
                memcpy(output_buffer + output_len, evt->data, copy_len);
            }
            output_len += copy_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG_htt, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            parse_train_data((uint8_t *)output_buffer, content_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
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
        if (output_buffer != NULL)
        {
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

void https_with_url()
{
    esp_http_client_config_t config = {
        .url = "https://api.goswift.ly/real-time/lametro-rail/gtfs-rt-vehicle-positions",
        .event_handler = _http_event_handler,
        .method = HTTP_METHOD_GET,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    ESP_LOGI(TAG_htt, "HTTPS request with url =>");
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // esp_http_client_set_method(client, HTTP_METHOD_HEAD);
    esp_http_client_set_header(client, "Accept", "application/json, application/json; charset=utf-8");
    esp_http_client_set_header(client, "Authorization", API_KEY);
    esp_err_t err = esp_http_client_perform(client);

    UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
    printf("Current Task Stack High Watermark (words): %u\n", watermark);

    if (err == ESP_OK)
    {
        int64_t len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG_htt, "HTTPS Status = %d, content_length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 len);
    }

    esp_http_client_cleanup(client);
}
