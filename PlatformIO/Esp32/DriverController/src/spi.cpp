
#include "driver/spi_master.h"
#include "driver/gpio.h"

class SpiDriver{

public:
  
  virtual void Init()=0;
  virtual void Read()=0;
  virtual void Write()=0;

};

class SpiConfigBuilder{


};

template <typename type>
class SpiMode2_16Bits{

public:
  SpiMode2_16Bits(type xSck, type xMiso, type xMosi, xCs){
    xprvBusConfiguration.miso_io_num = xMiso;
    xprvBusConfiguration.mosi_io_num = xMosi;
    xprvBusConfiguration.mosi_io_num = xMosi;
    xprvBusConfiguration.mosi_io_num = xMosi;

  };


private:
  gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1<<PARALLEL_LOAD,
    .pull_down_en = 0,
    .pull_up_en = 0
};

spi_bus_config_t xprvBusConfiguration=
  {
    .miso_io_num=PIN_NUM_MISO,
    .mosi_io_num=-1,  
    .sclk_io_num=PIN_NUM_CLK,
    .quadwp_io_num=-1,
    .quadhd_io_num=-1,
    .max_transfer_sz=NUM_BITS
  };

};