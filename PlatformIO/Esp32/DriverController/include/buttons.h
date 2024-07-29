#ifndef _buttons_h_
#define _buttons_h_

class Buttons
{

public:
  Buttons();
  void vReadButtons();
  bool bBottonsChanged();
  void vSendEspNowMessage(uint8_t *);
  bool bIsThereAnError();
  esp_err_t xGetError();

private:
  typedef struct
  {
    stMessageHeader xHeader;
    uint16_t data;
    /* data */
  } __attribute__((packed)) stMessageType;

  stMessageType xMessage =
  {
    /*mesageid , readwrite, datatype*/
    .xHeader = {0x55, 1, eUNSIGNED16},
    .data = {0}
  };

  uint16_t uprvButtonsState;
  uint16_t uprvPreviousButtonsState;
  esp_err_t xprvError;
  Spi *xprvSpi;
};

#ifdef __cplusplus
extern "C"
{
#endif



#define BUTTON_TASK_STACK_SIZE 8192

  /***********External Functions  **************/
extern void vButtonsTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif
