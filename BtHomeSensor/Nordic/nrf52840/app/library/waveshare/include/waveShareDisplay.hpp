
#include <stdint.h>
#include <zephyr/drivers/gpio.h>
#include "busInterface.hpp"

#define setCmdSize(cmd, size) cmd, size
#define HIGH 1
#define LOW 0

enum class dispError
{
  None = 0,
  BufferError = -1,
};

uint8_t initSequence[] =
    {
        setCmdSize(0x4d, 0x1),
        0x78,
        setCmdSize(0x0, 0x2),
        0x0F,
        0x29,
        setCmdSize(0x01, 0x2),
        0x07,
        0x00,
        setCmdSize(0x03, 0x3),
        0x10,
        0x54,
        0x44,
        setCmdSize(0x06, 0x7),
        0x4F,
        0x0A,
        0x2F,
        0x25,
        0x22,
        0x2E,
        0x21,
        setCmdSize(0x30, 0x1),
        0x02,
        setCmdSize(0x41, 0x1),
        0x00,
        setCmdSize(0x50, 0x1),
        0x37,
        setCmdSize(0x60, 0x2),
        0x02,
        0x2,
        setCmdSize(0x61, 0x4),
        0x0,
        160,
        296 / 256,
        296 % 256,
        setCmdSize(0x65, 0x4),
        0x00,
        0x00,
        0x00,
        0x00,
        setCmdSize(0xE7, 0x1),
        0x1C,
        setCmdSize(0xE3, 0x1),
        0x22,
        setCmdSize(0xE0, 0x1),
        0x00,
        setCmdSize(0xB4, 0x3),
        0xD0,
        0xB5,
        0x03,
        setCmdSize(0xE9, 0x1),
        0x01,
        setCmdSize(0x04, 0x0)};

template <typename typeErr>
class waveShareDisplay : public displayInterface<typeErr>
{

public:
  waveShareDisplay() {};
  typeErr SetInterface(busInterface *) override;
  typeErr Init(void) override;
  typeErr Update(void) override;
  typeErr PowerOn(void) override;
  typeErr PowerOff(void) override;
  typeErr ClearScreen(void) override;
  typeErr SetBuffer(uint8_t *) override;

private:
  busInterface *_interface = nullptr;
  uint8_t *_buffer = nullptr;

  /*gpio_dt_spec dc = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(display), gpios, 0);
  gpio_dt_spec reset = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(display), gpios, 1);
  gpio_dt_spec busy = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(display), gpios, 2);
  gpio_dt_spec power = GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(display), gpios, 3);*/
};

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::SetInterface(busInterface *Interface)
{
  _interface = Interface;
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::Init(void)
{
  uint16_t idx = 0, dataIdx = 0;
  uint16_t seqSize = sizeof(initSequence);
  int8_t err = 0;

  /* gpio_pin_configure_dt(&dc, GPIO_OUTPUT_ACTIVE);
   gpio_pin_set_dt(&dc, LOW);
   gpio_pin_configure_dt(&reset, GPIO_OUTPUT_ACTIVE);
   gpio_pin_set_dt(&reset, HIGH);
   gpio_pin_configure_dt(&power, GPIO_OUTPUT_ACTIVE);
   gpio_pin_set_dt(&power, HIGH);*/

  if (_buffer == nullptr)
    return typeErr(dispError::BufferError);
  while (idx < seqSize)
  {
    /*Send Command */
    // gpio_pin_set_dt(&dc, LOW);
    err = _interface->Write(&initSequence[idx], 1);
    if (err < 0)
      return typeErr(err);

    /* Send Data */
    // gpio_pin_set_dt(&dc, HIGH);
    for (dataIdx = 0; dataIdx < initSequence[++idx]; dataIdx++)
    {
      err = _interface->Write(&initSequence[idx], 1);
      if (err < 0)
        return typeErr(err);
    }

    idx += initSequence[idx];
  }

  return typeErr(0);
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::Update()
{
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::PowerOn(void)
{
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::PowerOff(void)
{
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::ClearScreen(void)
{
  return typeErr();
}

template <typename typeErr>
inline typeErr waveShareDisplay<typeErr>::SetBuffer(uint8_t *buffer)
{
  _buffer = buffer;
  return typeErr();
}
