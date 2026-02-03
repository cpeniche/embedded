#pragma once

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
  ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

class adc : public inputInterface
{

public:
  adc();
  ~adc() {};
  int8_t  readInput(uint8_t *, size_t) override;
  uint8_t *getInput(void) override;
  bool getDataReady(void) override;
  int getError(void) { return err; }

private:
  zephyrSpiBuilder<uint8_t, int16_t> spibuilder;
  spiInterface<uint8_t, int16_t> *spi = spibuilder.factoryMethod(&spi_cfg);
  int err;
  struct spi_config spi_cfg =
      {
          .frequency = 1000000U,
          .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB |
                       SPI_WORD_SET(8) | SPI_MODE_CPOL | SPI_MODE_CPHA,
          .slave = 0x0};
  uint8_t rxBuffer[3] = {0};
  float voltage;
};