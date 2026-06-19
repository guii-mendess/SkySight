/* =====================================================================
   SkySight - Drone para recreação e educação
   Projeto de PAP - Curso Técnico de Gestão de Equipamentos Informáticos
   ETPR, Ano Letivo 2025/2026
   Autor: Guilherme Mendes

   ---------------------------------------------------------------------
   ATRIBUIÇÃO / CRÉDITOS
   Este firmware não foi escrito de raiz. Tem como base o projeto
   open-source MultiWii (https://github.com/multiwii/multiwii-firmware),
   licenciado sob GPL, com a parte de rádio NRF24 baseada num exemplo
   de MultiWii brushed do ELECTRONOOBS.

   O meu trabalho foi a adaptação e integração ao meu hardware:
     - Porte do firmware do ATmega328P para o Arduino Pro Micro (ATmega32U4)
     - Integração do recetor de rádio NRF24L01 (ficheiros NRF24_RX.*)
     - Configuração QUADX e mapeamento dos meus pinos e canais
     - Correção da orientação do MPU-6050 (montado de cabeça para baixo)
     - Declaração das variáveis de soft-PWM em falta para esta placa
     - Buzzer no pino D4 e respetiva configuração de alarmes

   Bibliotecas necessárias:
     - RF24 (TMRh20)
     - TimerFreeTone (Tim Eckel)
   ===================================================================== */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>