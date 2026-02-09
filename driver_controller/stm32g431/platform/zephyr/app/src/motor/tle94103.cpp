#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(tle94103, LOG_LEVEL_DBG);
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include "motorInterface.h"
#include "can.h"
#include "tle94103.h"
#include "main.h"

#if !DT_NODE_EXISTS(DT_NODELABEL(tle94103_cs))
#error "tle94103 cs not defined"
#endif

static constexpr struct gpio_dt_spec tle94103_cs =
    GPIO_DT_SPEC_GET(DT_NODELABEL(tle94103_cs), gpios);

struct spi_config tle94103SpiCfg = {
    .frequency = 1000000U,
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
    .slave = 0x0,
    .cs = nullptr
};

Tle94103::Tle94103(can *interface)
{

  CanDriver = interface;
  gpio_pin_configure_dt(&tle94103_cs, GPIO_OUTPUT_ACTIVE);
};

void Tle94103::Right(uint8_t side)
{
  uint8_t data = HB1HSEN | HB3LSEN;
  canTxBufer[1] = 0x0;
  canTxBufer[0] = 0x4;

  if (side == 0x0)
    WriteRegister(HBCTRL, &data);
  else
    CanDriver->sendCanMsg(canTxBufer);
}

void Tle94103::Left(uint8_t side)
{
  uint8_t data = HB1LSEN | HB3HSEN;
  canTxBufer[1] = 0x0;
  canTxBufer[0] = 0x8;

  if (side == 0x0)
    WriteRegister(HBCTRL, &data);
  else
    CanDriver->sendCanMsg(canTxBufer);
}

void Tle94103::Down(uint8_t side)
{
  uint8_t data = HB2HSEN | HB3LSEN;
  canTxBufer[1] = 0x1;
  canTxBufer[0] = 0x0;

  if (side == 0x0)
    WriteRegister(HBCTRL, &data);
  else
    CanDriver->sendCanMsg(canTxBufer);
}

void Tle94103::Up(uint8_t side)
{
  uint8_t data = HB3HSEN | HB2LSEN;
  canTxBufer[1] = 0x2;
  canTxBufer[0] = 0x0;

  if (side == 0x0)
    WriteRegister(HBCTRL, &data);
  else
    CanDriver->sendCanMsg(canTxBufer);
}

void Tle94103::Idle(uint8_t side)
{
  uint8_t data = 0x0;
  canTxBufer[1] = 0x0;
  canTxBufer[0] = 0x0;
  WriteRegister(HBCTRL, &data);
  CanDriver->sendCanMsg(canTxBufer);
}

void Tle94103::WriteRegister(uint8_t u8RegAddress, uint8_t *ptrU8Data)
{

  uint8_t u8Buffer[] = {uint8_t(W_R(1) |
                                ADDRESS(u8RegAddress) |
                                LABT(1) | 0x1),
                        *ptrU8Data};
  getSpiMutex();
  spi->Configure(&tle94103SpiCfg);
  gpio_pin_set_dt(&tle94103_cs, 0);
  spi->Write(u8Buffer, 2);
  gpio_pin_set_dt(&tle94103_cs, 1);
  releaseSpiMutex();
}