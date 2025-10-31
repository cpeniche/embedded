#pragma once

#define SynchData 0x55 

class LIN
{

public:
  typedef enum{
    TXDONE,
    RXDONE,
    RXERROR,
  }eFlags;

  LIN(struct device *, uint8_t *, size_t, uint8_t);
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
  void setProtectedID(uint8_t id){protectedId=id;}
  uint8_t *getRxBuffer(void){return &rxBuffer[2];}
  void callBack(const struct device *dev, struct uart_event *evt, void *user_data);

private:
  uint8_t IdentifierFieldParity(uint8_t );
  //void Callback(const struct device *dev, struct uart_event *evt, void *user_data);
  int16_t error;
  void (*callback)(const struct device *dev, struct event *evt, void *user_data);
  struct device *dev;
  uint8_t flags;
  uint8_t protectedId;
  uint8_t txBuffer[10];
  uint8_t *rxBuffer;

};