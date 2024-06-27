#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "espNow.h"

#define ESPNOW_MAXDELAY 512

static const char *TAG = "espnow";

QueueHandle_t xESPNowQueue;

//static uint8_t prvBroadCastMAC[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t uDriverControlMAC[ESP_NOW_ETH_ALEN] = { 0x0a, 0xb6, 0x1f, 0x72, 0x07, 0x33 };
uint8_t uPassengerControlMAC[ESP_NOW_ETH_ALEN] = { 0x0a, 0xb6, 0x1f, 0x72, 0x07, 0x32 };
static uint16_t prvESPNowSequenceNumber[ESPNOW_DATA_MAX] = { 0, 0 };

static void prvESPNowDeinitilize(void);
static void prvESPNowAddPeer(uint8_t *pPeerMACAddress);

/* WiFi should start before using ESPNOW */
static void vWirelessInterfaceInitialize(void)
{
    
    uint8_t uprvMACAddress[6];
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));
    esp_wifi_get_mac(ESPNOW_WIFI_MODE,uprvMACAddress);
    ESP_LOGE(TAG,"My MAC Address : " MACSTR, MAC2STR(uprvMACAddress));

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
}

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void vESPNowSendCallback(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    
    if(ESP_NOW_SEND_SUCCESS==status)
    {
        xEventGroupSetBits(xESPnowEventGroupHandle,1<<eESPNOW_SEND_CB);
        xEventGroupClearBits(xESPnowEventGroupHandle, 1<<eESPNOW_TRANSMIT_ERROR);
        ESP_LOGI(TAG,"CallBack send Succesfully %x",status);
    }
    else
    {
        xEventGroupSetBits(xESPnowEventGroupHandle,1<<eESPNOW_TRANSMIT_ERROR);
        ESP_LOGI(TAG,"CallBack send Error %x",status);
    }

#if 0    
    espnow_event_t evt;
    espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = eESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(xESPNowQueue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send send queue fail");
    }
#endif
}

static void vESPNowReceiveCallback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    espnow_event_t evt;
    espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
    uint8_t * mac_addr = recv_info->src_addr;
    uint8_t * des_addr = recv_info->des_addr;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

#if 0
    if (IS_BROADCAST_ADDR(des_addr)) {
        /* If added a peer with encryption before, the receive packets may be
         * encrypted as peer-to-peer message or unencrypted over the broadcast channel.
         * Users can check the destination address to distinguish it.
         */
        ESP_LOGD(TAG, "Receive broadcast ESPNOW data");
    } else {
        ESP_LOGD(TAG, "Receive unicast ESPNOW data");
    }

#endif
    evt.id = eESPNOW_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(xESPNowQueue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}

/* Parse received ESPNOW data. */
int espnow_data_parse(uint8_t *data, uint16_t data_len, uint8_t *state, uint16_t *seq, int *magic)
{
    espnow_data_t *buf = (espnow_data_t *)data;
    uint16_t crc, crc_cal = 0;

    if (data_len < sizeof(espnow_data_t)) {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return -1;
    }

    *state = buf->state;
    *seq = buf->seq_num;
    *magic = buf->magic;
    crc = buf->crc;
    buf->crc = 0;
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, data_len);

    if (crc_cal == crc) {
        return buf->type;
    }

    return -1;
}

#if 0
/* Prepare ESPNOW data to be sent. */
void vESPNowPreparePayload(espnow_send_param_t *send_param)
{
    espnow_data_t *buf = (espnow_data_t *)send_param->buffer;

    assert(send_param->len >= sizeof(espnow_data_t));

    buf->type = ESPNOW_DATA_UNICAST;//IS_BROADCAST_ADDR(send_param->dest_mac) ? ESPNOW_DATA_BROADCAST : ESPNOW_DATA_UNICAST;
    buf->state = send_param->state;
    buf->seq_num = prvESPNowSequenceNumber[buf->type]++;
    buf->crc = 0;
    buf->magic = send_param->magic;
    /* Fill all remaining bytes after the data with random values */
    esp_fill_random(buf->payload, send_param->len - sizeof(espnow_data_t));
    buf->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);
}

#endif
static void vTaskESPNow(void *pvParameter)
{
    espnow_event_t evt;
    EventBits_t uxBits;
    uint8_t recv_state = 0;
    uint16_t recv_seq = 0;
    int recv_magic = 0;
    bool is_broadcast = false;
    int ret;

    ESP_LOGI(TAG, "ESP Now Task Started");
    uxBits = xEventGroupSetBits(xESPnowEventGroupHandle,1<<eESPNOW_INIT_DONE);

    while (xQueueReceive(xESPNowQueue, &evt, portMAX_DELAY) == pdTRUE) {
        switch (evt.id) {
            case eESPNOW_SEND_CB:
            {                
                break;
            }
            case eESPNOW_RECV_CB:
            {
                #if 0
                espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

                //ret = espnow_data_parse(recv_cb->data, recv_cb->data_len, &recv_state, &recv_seq, &recv_magic);
                //free(recv_cb->data);                
                if (ret == ESPNOW_DATA_BROADCAST) {
                    ESP_LOGI(TAG, "Receive %dth broadcast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

                    

                    /* Indicates that the device has received broadcast ESPNOW data. */
                    if (send_param->state == 0) {
                        send_param->state = 1;
                    }

                    /* If receive broadcast ESPNOW data which indicates that the other device has received
                     * broadcast ESPNOW data and the local magic number is bigger than that in the received
                     * broadcast ESPNOW data, stop sending broadcast ESPNOW data and start sending unicast
                     * ESPNOW data.
                     */
                    if (recv_state == 1) {
                        /* The device which has the bigger magic number sends ESPNOW data, the other one
                         * receives ESPNOW data.
                         */
                        if (send_param->unicast == false && send_param->magic >= recv_magic) {
                    	    ESP_LOGI(TAG, "Start sending unicast data");
                    	    ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(recv_cb->mac_addr));

                    	    /* Start sending unicast ESPNOW data. */
                            memcpy(send_param->dest_mac, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                            vESPNowPreparePayload(send_param);
                            if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
                                ESP_LOGE(TAG, "Send error");
                                prvESPNowDeinitilize(send_param);
                                vTaskDelete(NULL);
                            }
                            else {
                                send_param->broadcast = false;
                                send_param->unicast = true;
                            }
                        }
                    }
                }
                
                else if (ret == ESPNOW_DATA_UNICAST) {
                    ESP_LOGI(TAG, "Receive %dth unicast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

                    /* If receive unicast ESPNOW data, also stop sending broadcast ESPNOW data. */
                    send_param->broadcast = false;
                }
                else {
                    ESP_LOGI(TAG, "Receive error data from: "MACSTR"", MAC2STR(recv_cb->mac_addr));
                }
                #endif
                break;
            }
            default:
                ESP_LOGE(TAG, "Callback type error: %d", evt.id);
                break;
        }
    }
}

static esp_err_t xESPNowInitialize(void)
{

    xESPNowQueue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_event_t));
    if (xESPNowQueue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(vESPNowSendCallback) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(vESPNowReceiveCallback) );
#if CONFIG_ESPNOW_ENABLE_POWER_SAVE
    ESP_ERROR_CHECK( esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW) );
    ESP_ERROR_CHECK( esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL) );
#endif
    /* Set primary master key. */
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );
    
    prvESPNowAddPeer(uDriverControlMAC);
    prvESPNowAddPeer(uPassengerControlMAC);
    xTaskCreate(vTaskESPNow, "espnow_task", 2048, (void *)0, 4, NULL);

    return ESP_OK;
}


static void prvESPNowAddPeer(uint8_t *prvPeerMACAddress)
{

/* If MAC address does not exist in peer list, add it to peer list. */
    if (esp_now_is_peer_exist(prvPeerMACAddress) == false) {
        esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
        if (peer == NULL) {
            ESP_LOGE(TAG, "Malloc peer information fail");
            prvESPNowDeinitilize();
            vTaskDelete(NULL);
        }
        memset(peer, 0, sizeof(esp_now_peer_info_t));
        peer->channel = CONFIG_ESPNOW_CHANNEL;
        peer->ifidx = ESPNOW_WIFI_IF;
        peer->encrypt = true;
        memcpy(peer->lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
        memcpy(peer->peer_addr, prvPeerMACAddress, ESP_NOW_ETH_ALEN);
        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
        free(peer);
    }
}

static void prvESPNowDeinitilize()
{   
    vSemaphoreDelete(xESPNowQueue);
    esp_now_deinit();
}

void vWifiConfigureESPNow(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    vWirelessInterfaceInitialize();
    xESPNowInitialize();
}