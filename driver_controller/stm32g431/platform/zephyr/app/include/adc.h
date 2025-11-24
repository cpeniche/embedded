#pragma once

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
  ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

class adc : public inputInterface
{

public:
  adc();
  ~adc() {};
  int8_t readInput(uint8_t *, size_t) override;
  uint8_t *getInput(void) override;
  bool getDataReady(void) override;
  int getError(void){return err;}

private:
  uint32_t count = 0;
	uint16_t buf;
  int err;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};

 

};