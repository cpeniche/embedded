/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* |	This version:   V2.0
* | Date        :   2018-10-30
* | Info        :
# ******************************************************************************
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "DEV_Config.h"
#include <zephyr/drivers/spi.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

struct gpio_dt_spec EPD_RST_PIN = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), display_gpios, 2);
struct gpio_dt_spec EPD_DC_PIN = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), display_gpios, 0);
struct gpio_dt_spec EPD_PWR_PIN = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), display_gpios, 3);
struct gpio_dt_spec EPD_BUSY_PIN = GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), display_gpios, 1);
struct spi_dt_spec hspi1 = SPI_DT_SPEC_GET(DT_NODELABEL(display), SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA);

void DEV_SPI_WriteByte(UBYTE value)
{
    struct spi_buf value_buf = {.buf = &value, .len = 1};

    struct spi_buf_set tx_bufs =
        {
            .buffers = &value_buf,
            .count = 1};

    spi_write_dt(&hspi1, &tx_bufs);
}

void DEV_SPI_Write_nByte(UBYTE *value, UDOUBLE len)
{
    struct spi_buf value_buf = {.buf = value, .len = len};
    struct spi_buf_set tx_bufs =
        {
            .buffers = &value_buf,
            .count = 1};

    spi_write_dt(&hspi1, &tx_bufs);
}

#if 0
void DEV_GPIO_Mode(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, UWORD Mode)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (Mode == 0)
    {
        GPIO_InitStruct.Pin = GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    else
    {
        GPIO_InitStruct.Pin = GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        // Debug (" %d OUT \r\n",Pin);
    }
}

void DEV_GPIO_Init()
{
    HAL_SPI_MspDeInit(&hspi1);

    // HAL_SPI_DeInit(&hspi1);
    //    __HAL_RCC_SPI1_CLK_DISABLE();
    //    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
}

void DEV_SPI_Init()
{
    HAL_SPI_MspInit(&hspi1);
    // HAL_SPI_DeInit(&hspi1);
    //    __HAL_RCC_SPI1_CLK_DISABLE();
    //    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_7);
}
#endif
void DEV_SPI_SendData(UBYTE Reg)
{

#if 0    
    
    
    UBYTE i, j = Reg;
    DEV_GPIO_Mode(EPD_MOSI_PIN, 1);
    DEV_GPIO_Mode(EPD_SCLK_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    for (i = 0; i < 8; i++)
    {
        DEV_Digital_Write(EPD_SCLK_PIN, 0);
        if (j & 0x80)
        {
            DEV_Digital_Write(EPD_MOSI_PIN, 1);
        }
        else
        {
            DEV_Digital_Write(EPD_MOSI_PIN, 0);
        }

        DEV_Digital_Write(EPD_SCLK_PIN, 1);
        j = j << 1;
    }
    DEV_Digital_Write(EPD_SCLK_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 1);
#endif
}

UBYTE DEV_SPI_ReadData()
{

    return 0;
#if 0    
    UBYTE i, j = 0xff;
    DEV_GPIO_Mode(EPD_MOSI_PIN, 0);
    DEV_GPIO_Mode(EPD_SCLK_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    for (i = 0; i < 8; i++)
    {
        DEV_Digital_Write(EPD_SCLK_PIN, 0);
        j = j << 1;
        if (DEV_Digital_Read(EPD_MOSI_PIN))
        {
            j = j | 0x01;
        }
        else
        {
            j = j & 0xfe;
        }
        DEV_Digital_Write(EPD_SCLK_PIN, 1);
    }
    DEV_Digital_Write(&EPD_SCLK_PIN, 0);
    //DEV_Digital_Write(EPD_CS_PIN, 1);
    return j;
#endif
}

int DEV_Module_Init(void)
{
    gpio_pin_configure_dt(&EPD_RST_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&EPD_DC_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&EPD_PWR_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&EPD_BUSY_PIN, GPIO_INPUT);

    DEV_Digital_Write(&EPD_DC_PIN, 0);
    // DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_Digital_Write(&EPD_PWR_PIN, 1);
    DEV_Digital_Write(&EPD_RST_PIN, 1);
    return 0;
}

void DEV_Module_Exit(void)
{
    DEV_Digital_Write(&EPD_DC_PIN, 0);
    // DEV_Digital_Write(EPD_CS_PIN, 0);

    // close 5V
    DEV_Digital_Write(&EPD_PWR_PIN, 0);
    DEV_Digital_Write(&EPD_RST_PIN, 0);
}
