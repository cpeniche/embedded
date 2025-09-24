#pragma once
#define W_R(BIT) (BIT<<7)
#define LABT(BIT) (BIT<<1)

class Tle94103{

public:
  enum
  {
    HBCTRL = 0x0,
    FMCLKCTRL = 0xC,
    OLBLKCTRL = 0x1A,
    CONFIGCTRL = 0x19,
    STATUSREG =0x6,
    SYSDIAG2  =0x16,
    SYSDIAG3  =0x1,
  };

  Tle94103(){
    /* Create Spi Device */
    spibuilder= new zephyrSpiBuilder<uint8_t, int16_t>();
	  spi = spibuilder->factoryMethod(); 
  }
  void WriteRegister(uint8_t, uint8_t *);


private:  
 
 spiInterface<uint8_t, int16_t> *spi=nullptr; 
 SpiBuilder<uint8_t, int16_t> *spibuilder= nullptr;

};