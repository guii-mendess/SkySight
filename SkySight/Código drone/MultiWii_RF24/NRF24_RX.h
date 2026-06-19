/* =====================================================================
   SkySight - Projeto de PAP (ETPR, 2025/2026)
   Adaptação e configuração: Guilherme Mendes

   Firmware base: MultiWii (https://github.com/multiwii/multiwii-firmware)
   A parte de rádio foi baseada num exemplo de MultiWii brushed do
   ELECTRONOOBS, que adaptei ao meu hardware.

   O que fiz neste módulo:
     - Porte para Arduino Pro Micro (ATmega32U4) com o NRF24L01
     - Estrutura de dados de 7 canais, igual à que o meu comando envia
     - Mapeamento dos canais AUX (armar / buzzer / modo de voo)
   =====================================================================

   Ligação do NRF24 ao Pro Micro (nRF24 -> Pro Micro):
     CE      7
     CSN     8
     MOSI   16
     MISO   14
     SCK    15
   (o CE e o CSN podem ser mudados em NRF24_RX.cpp)
*/

#ifndef NRF24_RX_H_
#define NRF24_RX_H_

#include "config.h"

#if defined(NRF24_RX)

// O tamanho desta struct não pode passar dos 32 bytes.
// São 7 bytes, exatamente pela mesma ordem que o comando usa para enviar.
struct RF24Data {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;   // Armar / Desarmar  (0 ou 255)
  byte AUX2;   // Buzzer            (0 ou 255)
  byte AUX3;   // Modo de voo       (0 ou 255)
};

// Estrutura preparada para telemetria de volta (ainda não a uso - ver NRF24_RX.cpp)
struct RF24AckPayload {
  float lat;
  float lon;
  int16_t heading;
  int16_t pitch;
  int16_t roll;
  int32_t alt;
  byte flags;
};

extern RF24Data nrf24Data;
extern RF24AckPayload nrf24AckPayload;
extern int16_t nrf24_rcData[RC_CHANS];

void NRF24_Init();
void NRF24_Read_RC();

#endif

#endif /* NRF24_RX_H_ */
