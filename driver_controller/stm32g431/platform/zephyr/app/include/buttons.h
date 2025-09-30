#pragma once


#define  maskLOCK           0x01
#define  maskUNLOCK         0x02
#define  maskRIGHWINDOWUP   0x04
#define  maskRIGHWINDOWDWON 0x08
#define  maskLEFTWINDOWUP   0x10
#define  maskLEFTWINDOWDWON 0x20s
#define  maskMIRRORUP       0x40
#define  maskMIRRORDOWN     0x80


class Buttons{

public:
    Buttons(spiInterface<uint8_t, int16_t> *);
    void Read(void);
    void setCs(uint8_t value){gpio_pin_set(gpioa,10,value);}
    void setLoad(uint8_t value){gpio_pin_set(gpioa,0,value);}
    void Task(void);
    
private:
  spiInterface<uint8_t, int16_t> *spi=nullptr;
  uint8_t rxBuffer; 
  const struct device* gpioa = DEVICE_DT_GET(DT_NODELABEL(gpioa));
};