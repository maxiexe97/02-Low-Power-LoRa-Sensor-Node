#ifndef SX1276_H
#define SX1276_H

#include "stm32l0xx_hal.h"
#include <stdint.h>

extern volatile uint8_t tx_done_flag;

// ====== DEFINIR ESTOS SEGÚN TU HARDWARE ======
#define SX_SPI_HANDLE hspi1   // Cambiar si usás otro SPI
extern SPI_HandleTypeDef SX_SPI_HANDLE;

#define SX_NSS_PORT GPIOA     // CAMBIAR
#define SX_NSS_PIN  GPIO_PIN_15

#define SX_RST_PORT GPIOB     // CAMBIAR
#define SX_RST_PIN  GPIO_PIN_0

#define SX_DIO0_PORT GPIOC    // CAMBIAR
#define SX_DIO0_PIN  GPIO_PIN_13
// ============================================

void SX1276_Init(void);
void SX1276_Send(uint8_t *data, uint8_t len);

#endif
