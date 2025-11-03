#pragma once

#include <zephyr/drivers/can.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socketcan.h>
#include <zephyr/net/socketcan_utils.h>

#define CANID 0x1

class can
{

public:
  can();
  ~can() {};

  void sendCanMsg(uint8_t *);

private:
  int setupSocket();
  const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
  struct sockaddr_can canAddress;
  struct net_if *iface;
  struct can_frame zframe =
      {
          .id = CANID,
          .dlc = 2U};
  struct socketcan_frame sframe = {0};
  struct socketcan_filter sock_filter;
  struct can_filter zfilter = {
      .id = CANID,
      .mask = CAN_STD_ID_MASK,
      .flags = 0U};

  int fileDescriptor;
  int error;
};