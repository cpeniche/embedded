#pragma once
#include "inputInterface.h"

#define SynchData 0x55

class LIN : public inputInterface
{

public:
  typedef enum
  {
    TXDONE,
    RXDONE,
    RXERROR,
  } eFlags;

  LIN(struct device *, uint8_t *, size_t, uint8_t);
  ~LIN() {};
  int8_t Transmit(uint8_t *, size_t);
  int8_t Receive(uint8_t *, size_t);
  void setCallback(uart_callback_t);
  void setDevice(struct device *dev) { this->dev = dev; }
  struct device *getDevice() { return dev; }  
  void setFlag(eFlags flag);
  void clearFlag(eFlags flag);
  uint8_t getFlags(void) { return flags; }
  void setProtectedID(uint8_t id) { protectedId = id; }
  uint8_t *getRxBuffer(void) { return rxBuffer; }
  void callBack(const struct device *dev, struct uart_event *evt, void *user_data);
  int8_t readInput(uint8_t *, size_t) override;
  int8_t getInput(uint8_t *) override;
  bool isDataReady(void) override;
  int8_t getError(void) override;

private:
  uint8_t IdentifierFieldParity(uint8_t);
  int8_t error;
  void (*callback)(const struct device *dev, struct event *evt, void *user_data);
  struct device *dev;
  uint8_t flags;
  uint8_t protectedId;
  uint8_t txBuffer[10];
  uint8_t *rxBuffer;
  uint8_t buffLength;
  uint8_t idxBuffer=0;
};

typedef void (LIN::*callBackPtr)(const struct device *, struct uart_event *, void *);