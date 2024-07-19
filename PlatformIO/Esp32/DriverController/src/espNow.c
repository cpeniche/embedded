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
#include "dictionary.h"

#define ESPNOW_MAXDELAY 512

static const char *TAG = "espnow";

QueueHandle_t xESPNowQueue;

uint8_t uBroadCastMAC[ESP_NOW_ETH_ALEN]         = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t uDriverControlMAC[ESP_NOW_ETH_ALEN]     = { 0x0a, 0xb6, 0x1f, 0x72, 0x07, 0x33 };
uint8_t uPassengerControlMAC[ESP_NOW_ETH_ALEN]  = { 0x0a, 0xb6, 0x1f, 0x72, 0x07, 0x32 };
//static uint16_t prvESPNowSequenceNumber[ESPNOW_DATA_MAX] = { 0, 0 };

static void prvESPNowDeinitilize(void);
static void prvESPNowAddPeer(uint8_t *pPeerMACAddress, uint8_t );


/******************************************************/
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

/******************************************************/

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void vESPNowSendCallback(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    
    if(ESP_NOW_SEND_SUCCESS==status)
    {
        /* Send Event to Buttons task to acknowledge that the message was sent*/
        xEventGroupSetBits(xESPnowEventGroupHandle,1<<eESPNOW_SEND_CB);
        xEventGroupClearBits(xESPnowEventGroupHandle, 1<<eESPNOW_TRANSMIT_ERROR);
        //ESP_LOGI(TAG,"CallBack send Succesfully %x",status);
    }
    else
    {
        xEventGroupSetBits(xESPnowEventGroupHandle,1<<eESPNOW_TRANSMIT_ERROR);
        ESP_LOGI(TAG,"CallBack send Error %x",status);
    }
}

/******************************************************/

static void vESPNowReceiveCallback(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    if (xSemaphoreTake(xQueueSemaphore,10 ) == pdTRUE)
    {
        if(iDictionaryAddDataToQueue((void *)data, len) == -1)
            ESP_LOGE(TAG, "Cannot Add Data to Dictionary Queue");

        else
        /* Access to the shared resource is complete, so the mutex is
            returned. */
            xSemaphoreGive( xQueueSemaphore );
    }
     else
         ESP_LOGE(TAG, "Could not Get Dictionary Queue Mutex");

}

/******************************************************/
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
    prvESPNowAddPeer(uBroadCastMAC,0);
    prvESPNowAddPeer(uDriverControlMAC,1);
    prvESPNowAddPeer(uPassengerControlMAC,1);
    xEventGroupSetBits(xESPnowEventGroupHandle,1<<eESPNOW_INIT_DONE);

    return ESP_OK;
}

/******************************************************/
static void prvESPNowAddPeer(uint8_t *prvPeerMACAddress, uint8_t upEnableEncryption)
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
        peer->encrypt = upEnableEncryption;
        memcpy(peer->lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
        memcpy(peer->peer_addr, prvPeerMACAddress, ESP_NOW_ETH_ALEN);
        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
        free(peer);
    }
}

/******************************************************/
static void prvESPNowDeinitilize()
{   
    vSemaphoreDelete(xESPNowQueue);
    esp_now_deinit();
}

/******************************************************/
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