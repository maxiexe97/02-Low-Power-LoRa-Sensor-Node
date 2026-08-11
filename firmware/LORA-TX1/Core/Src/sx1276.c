#include "sx1276.h"

// ================= REGISTROS =================
#define REG_FIFO                0x00
#define REG_OPMODE              0x01
#define REG_FRF_MSB             0x06
#define REG_FRF_MID             0x07
#define REG_FRF_LSB             0x08
#define REG_PA_CONFIG           0x09
#define REG_LNA                 0x0C
#define REG_FIFO_ADDR_PTR       0x0D
#define REG_FIFO_TX_BASE_ADDR   0x0E
#define REG_FIFO_RX_BASE_ADDR   0x0F
#define REG_FIFO_RX_CURRENT     0x10
#define REG_IRQ_FLAGS           0x12
#define REG_MODEM_CONFIG1       0x1D
#define REG_MODEM_CONFIG2       0x1E
#define REG_PREAMBLE_MSB        0x20
#define REG_PREAMBLE_LSB        0x21
#define REG_PAYLOAD_LENGTH      0x22
#define REG_MODEM_CONFIG3       0x26
#define REG_DIO_MAPPING1        0x40
#define REG_VERSION             0x42

#define MODE_SLEEP              0x80
#define MODE_STDBY              0x81
#define MODE_TX                 0x83

// =============================================
// ================= VARIABLES =================
volatile uint8_t tx_done_flag = 0;
// =============================================

static void SX_Write(uint8_t addr, uint8_t value)
{
    uint8_t buffer[2];
    buffer[0] = addr | 0x80;
    buffer[1] = value;

    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&SX_SPI_HANDLE, buffer, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);
}

static uint8_t SX_Read(uint8_t addr)
{
    uint8_t tx = addr & 0x7F;
    uint8_t rx;

    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&SX_SPI_HANDLE, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&SX_SPI_HANDLE, &rx, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);

    return rx;
}

static void SX_Reset(void)
{
    HAL_GPIO_WritePin(SX_RST_PORT, SX_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(SX_RST_PORT, SX_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(10);
}

void SX1276_Init(void)
{
    SX_Reset();

    // Sleep
    SX_Write(REG_OPMODE, MODE_SLEEP);
    HAL_Delay(10);

    // Standby
    SX_Write(REG_OPMODE, MODE_STDBY);
    HAL_Delay(10);

    // Frecuencia 915 MHz
    // FRF = (915000000 / 61.03515625)
    SX_Write(REG_FRF_MSB, 0xE4);
    SX_Write(REG_FRF_MID, 0xC0);
    SX_Write(REG_FRF_LSB, 0x00);

    // Potencia 14 dBm
    SX_Write(REG_PA_CONFIG, 0x8F);

    // LNA boost
    SX_Write(REG_LNA, 0x23);

    // FIFO base
    SX_Write(REG_FIFO_TX_BASE_ADDR, 0x00);
    SX_Write(REG_FIFO_RX_BASE_ADDR, 0x00);

    // BW=125kHz CR=4/5
    SX_Write(REG_MODEM_CONFIG1, 0x72);

    // SF7
    SX_Write(REG_MODEM_CONFIG2, 0x74);

    // LowDataRateOptimize off
    SX_Write(REG_MODEM_CONFIG3, 0x04);

    // Preamble 8
    SX_Write(REG_PREAMBLE_MSB, 0x00);
    SX_Write(REG_PREAMBLE_LSB, 0x08);

    // DIO0 = TxDone
    SX_Write(REG_DIO_MAPPING1, 0x40);

    //sync word de LoRaWAN
    SX_Write(0x39, 0x34);
}

void SX1276_Send(uint8_t *data, uint8_t len)
{
    SX_Write(REG_OPMODE, MODE_STDBY);

    SX_Write(REG_FIFO_ADDR_PTR, 0x00);

    for (uint8_t i = 0; i < len; i++)
        SX_Write(REG_FIFO, data[i]);

    SX_Write(REG_PAYLOAD_LENGTH, len);

    SX_Write(REG_IRQ_FLAGS, 0xFF);

    //SX_Write(REG_OPMODE, MODE_TX);

    // Esperar TxDone
    //while (HAL_GPIO_ReadPin(SX_DIO0_PORT, SX_DIO0_PIN) == GPIO_PIN_RESET);
    ////////////////////
    tx_done_flag = 0;

    SX_Write(REG_OPMODE, MODE_TX);

    // Esperar interrupción
    while (!tx_done_flag)
    {
        __WFI();  // Wait For Interrupt (bajo consumo mientras espera)
    }
    ////////////////////

    //SX_Write(REG_IRQ_FLAGS, 0xFF);

    SX_Write(REG_OPMODE, MODE_SLEEP);
}
