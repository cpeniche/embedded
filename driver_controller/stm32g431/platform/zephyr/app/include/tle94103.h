#pragma once
#define W_R(BIT) (BIT << 7)
#define LABT(BIT) (BIT << 1)
#define ADDRESS(x) (uint8_t)(x << 2)

#define HB3HSEN 1 << 5
#define HB3LSEN 1 << 4
#define HB2HSEN 1 << 3
#define HB2LSEN 1 << 2
#define HB1HSEN 1 << 1
#define HB1LSEN 1 << 5

class Tle94103 : public motorInterface
{
public:
  enum
  {
    HBCTRL = 0x0,
    FMCLKCTRL = 0xC,
    OLBLKCTRL = 0x1A,
    CONFIGCTRL = 0x19,
    STATUSREG = 0x6,
    SYSDIAG2 = 0x16,
    SYSDIAG3 = 0x1,
  
  };
  Tle94103(){};  
  void Right(void) override;
  void Left(void) override;
  void Down(void) override;
  void Up(void) override;  
  void Idle(void) override;
  void WriteRegister(uint8_t, uint8_t *);

private:
  
  zephyrSpiBuilder<uint8_t, int16_t> spibuilder;
  spiInterface<uint8_t, int16_t> *spi = spibuilder.factoryMethod();
};