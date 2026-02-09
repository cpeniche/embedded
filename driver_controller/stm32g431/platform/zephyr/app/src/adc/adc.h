#pragma once

#include "zephyrSpiBuilder.h"

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
  ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

class adc : public inputInterface
{

public:
  adc();
  ~adc() {};
  int8_t readInput(uint8_t *, size_t) override;
  int8_t getInput(uint8_t *buffer) override;
  bool isDataReady(void) override;
  int8_t getError(void) { return err; }

private:
  zephyrSpiBuilder<uint8_t, int16_t> spibuilder;
  spiInterface<uint8_t, int16_t> *spi = spibuilder.factoryMethod();
  int8_t err;  
  uint8_t rxBuffer[3] = {0};
  float voltage;
};