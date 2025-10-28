#pragma once

class LIN
{

public:
  typedef enum{
    TXDONE,
    RXDONE,
    RXERROR,
  }eFlags;

  LIN(struct device *, uint8_t *, size_t);
  ~LIN(){};
  int8_t Transmit(uint8_t *, size_t);
  int8_t Receive(uint8_t *, size_t);
  void setCallback(uart_callback_t);
  void setDevice(struct device *dev){this->dev = dev;}
  struct device *getDevice(){return dev;}
  int16_t getError(){return error;}
  void setFlag(eFlags flag);
  void clearFlag(eFlags flag);
  uint8_t getFlags(void){return flags;}

private:

  int16_t error;
  void (*callback)(const struct device *dev, struct event *evt, void *user_data);
  struct device *dev;
  uint8_t flags;

};