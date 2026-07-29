/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>

#define SERVICE_DATA_LEN 10
#define SERVICE_UUID 0xfcd2 /* BTHome service UUID */
#define IDX_TEMPL 4					/* Index of lo byte of temp in service data*/
#define IDX_TEMPH 5					/* Index of hi byte of temp in service data*/
#define IDX_PRESL 7
#define IDX_PRESM 8
#define IDX_PRESH 9

#define ADV_PARAM BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
																	BT_GAP_ADV_SLOW_INT_MIN,    \
																	BT_GAP_ADV_SLOW_INT_MAX, NULL)

static uint8_t service_data[SERVICE_DATA_LEN] = {
		BT_UUID_16_ENCODE(SERVICE_UUID),
		0x40,
		0x02, /* Temperature */
		0xc4, /* Low byte */
		0x00, /* High byte */
		0x04, /* Pressure */
		0xbf, /* low byte*/
		0x13, /* middle byte*/
		0x00, /* high byte*/
};

static struct bt_data ad[] = {
		BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
		BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
		BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))};

int initBluetooth(void)
{
	int err = 0;

	printk("Starting BTHome sensor template\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(ADV_PARAM, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err)
	{
		printk("Advertising failed to start (err %d)\n", err);
		return err;
	}
	return err;
}

int updateBluetoothData(uint8_t *temperature, uint8_t *pressure)
{

	int err = 0;

	service_data[IDX_TEMPL] = temperature[0];
	service_data[IDX_TEMPH] = temperature[1];
	service_data[IDX_PRESL] = pressure[0];
	service_data[IDX_PRESM] = pressure[1];
	service_data[IDX_PRESH] = pressure[2];

	err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);

	if (err)
	{
		printk("Failed to update advertising data (err %d)\n", err);
	}

	return err;
}
