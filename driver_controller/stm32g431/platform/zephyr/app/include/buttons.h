#pragma once
class Buttons{

public:
    Buttons(spiInterface<uint8_t, int16_t> *);
    void Read(void);
    void setCs(uint8_t value){gpio_pin_set(gpioa,10,value);}
    void setLoad(uint8_t value){gpio_pin_set(gpioa,0,value);}
    
private:
  spiInterface<uint8_t, int16_t> *spi=nullptr;
  uint8_t rxBuffer; 
  const struct device* gpioa = DEVICE_DT_GET(DT_NODELABEL(gpioa));
};