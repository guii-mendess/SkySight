/* =====================================================================
   SkySight - Projeto de PAP (ETPR, 2025/2026)
   Adaptação e configuração: Guilherme Mendes

   Firmware base: MultiWii. A receção de rádio foi baseada num exemplo
   de MultiWii brushed do ELECTRONOOBS, adaptado por mim ao meu hardware.
   ===================================================================== */

#include "Arduino.h"
#include "config.h"
#include "def.h"
#include "types.h"
#include "MultiWii.h"
#include <RF24.h>
#include "NRF24_RX.h"

#if defined(NRF24_RX)

int16_t nrf24_rcData[RC_CHANS];

// Endereço do canal de rádio - tem mesmo de ser igual ao do comando.
static const uint64_t pipe = 0xE8E8F0F0E1LL;

// CE = D7, CSN = D8. Foi aqui que acertei os pinos para a minha montagem.
RF24 radio(7, 8); // CE, CSN

RF24Data MyData;
RF24AckPayload nrf24AckPayload;

// Valores seguros por defeito: motor parado, eixos ao centro e tudo desarmado.
void resetRF24Data()
{
  MyData.throttle = 0;
  MyData.yaw   = 128;
  MyData.pitch = 128;
  MyData.roll  = 128;

  MyData.AUX1 = 0;   // Desarmado (por segurança)
  MyData.AUX2 = 0;   // Buzzer desligado
  MyData.AUX3 = 0;   // Modo Angle (o mais seguro)
}

void resetRF24AckPayload()
{
  nrf24AckPayload.lat = 0;
  nrf24AckPayload.lon = 0;
  nrf24AckPayload.heading = 0;
  nrf24AckPayload.pitch = 0;
  nrf24AckPayload.roll = 0;
  nrf24AckPayload.alt = 0;
  nrf24AckPayload.flags = 0;
}

void NRF24_Init() {

  resetRF24Data();
  resetRF24AckPayload();

  radio.begin();
  // Potência no mínimo: evita os picos de corrente que me travavam a USB na bancada.
  // Para voar a sério, com alimentação por bateria, isto pode subir (ex.: RF24_PA_HIGH).
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);
  //radio.enableAckPayload();   // só se quiser telemetria de volta um dia

  radio.openReadingPipe(1, pipe);
  radio.startListening();
}

void NRF24_Read_RC() {

  static unsigned long lastRecvTime = 0;

  // Deixo a telemetria preparada para ir de volta (só usada se ativar o ackPayload).
  nrf24AckPayload.lat = 35.62;
  nrf24AckPayload.lon = 139.68;
  nrf24AckPayload.heading = att.heading;
  nrf24AckPayload.pitch = att.angle[PITCH];
  nrf24AckPayload.roll = att.angle[ROLL];
  nrf24AckPayload.alt = alt.EstAlt;
  memcpy(&nrf24AckPayload.flags, &f, 1);

  unsigned long now = millis();
  while ( radio.available() ) {
    radio.writeAckPayload(1, &nrf24AckPayload, sizeof(RF24AckPayload));
    radio.read(&MyData, sizeof(RF24Data));
    lastRecvTime = now;
  }
  if ( now - lastRecvTime > 1000 ) {
    // Failsafe: se perco o sinal por mais de 1 segundo, volto a valores seguros.
    resetRF24Data();
  }

  // Conversão dos eixos para o formato do MultiWii (1000 a 2000).
  // Se algum eixo ficar invertido, basta trocar os limites (1000<->2000).
  nrf24_rcData[THROTTLE] = map(MyData.throttle, 0, 255, 1000, 2000);
  nrf24_rcData[ROLL]     = map(MyData.yaw,      0, 255, 2000, 1000);
  nrf24_rcData[PITCH]    = map(MyData.pitch,    0, 255, 1000, 2000);
  nrf24_rcData[YAW]      = map(MyData.roll,     0, 255, 2000, 1000);

  // Canais auxiliares. Tive de corrigir o range para 0..255 (estava 0..1,
  // e por isso o armamento nunca chegava a funcionar como devia).
  nrf24_rcData[AUX1] = map(MyData.AUX1, 0, 255, 1000, 2000);  // Armar
  nrf24_rcData[AUX2] = map(MyData.AUX2, 0, 255, 1000, 2000);  // Buzzer
  // O modo de voo (AUX3) nem sequer estava a ser lido - acrescentei esta linha.
  nrf24_rcData[AUX3] = map(MyData.AUX3, 0, 255, 1000, 2000);  // Angle / Acro
}

#endif
