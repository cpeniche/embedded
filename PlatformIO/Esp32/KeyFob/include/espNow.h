
#ifndef ESPNOW_H
#define ESPNOW_H

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
#if CONFIG_ESPNOW_WIFI_MODE_STATION
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_AP
#endif

#define ESPNOW_QUEUE_SIZE           6
#define ESP_NOW_ETH_ALEN            6
#define CONFIG_ESPNOW_CHANNEL       1
#define CONFIG_ESPNOW_SEND_COUNT    100
#define CONFIG_ESPNOW_SEND_DELAY    1000
#define CONFIG_ESPNOW_SEND_LEN      2
#define CONFIG_ESPNOW_PMK "pmk1234567890123"
#define CONFIG_ESPNOW_LMK "lmk1234567890123"
#define configESPNOW_RETRANSMIT_MAX_RETRIES 3

extern uint8_t uBroadCastMAC[ESP_NOW_ETH_ALEN];
extern uint8_t uDriverControlMAC[ESP_NOW_ETH_ALEN];
extern uint8_t uPassengerControlMAC[ESP_NOW_ETH_ALEN];

//#define IS_BROADCAST_ADDR(addr) (memcmp(addr, prvBroadCastMAC, ESP_NOW_ETH_ALEN) == 0)

typedef enum {
    eESPNOW_INIT_DONE,
    eESPNOW_SEND_CB,
    eESPNOW_RECV_CB, 
    eESPNOW_TRANSMIT_ERROR   
} espnow_event_id_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} espnow_event_send_cb_t;

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t *data;
    int data_len;
} espnow_event_recv_cb_t;

typedef union {
    espnow_event_send_cb_t send_cb;
    espnow_event_recv_cb_t recv_cb;
} espnow_event_info_t;

/* When ESPNOW sending or receiving callback function is called, post event to ESPNOW task. */
typedef struct {
    espnow_event_id_t id;
    espnow_event_info_t info;
} espnow_event_t;

enum {
    ESPNOW_DATA_BROADCAST,
    ESPNOW_DATA_UNICAST,
    ESPNOW_DATA_MAX,
};

/* User defined field of ESPNOW data in this example. */
typedef struct {
    uint8_t type;                         //Broadcast or unicast ESPNOW data.
    uint8_t state;                        //Indicate that if has received broadcast ESPNOW data or not.
    uint16_t seq_num;                     //Sequence number of ESPNOW data.
    uint16_t crc;                         //CRC16 value of ESPNOW data.
    uint32_t magic;                       //Magic number which is used to determine which device to send unicast ESPNOW data.
    uint8_t payload[0];                   //Real payload of ESPNOW data.
} __attribute__((packed)) espnow_data_t;



extern void vWifiConfigureESPNow(void);
extern QueueHandle_t xESPNowQueue;
extern EventGroupHandle_t xESPnowEventGroupHandle;

#endif