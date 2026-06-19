# SkySight — Drone para recreação e educação

Projeto de PAP (Prova de Aptidão Profissional) do Curso Técnico de Gestão de
Equipamentos Informáticos — ETPR, ano letivo 2025/2026.

Sistema composto por duas partes construídas e configuradas de raiz: um **comando
transmissor** e um **drone quadricóptero**, que comunicam por rádio a 2.4GHz.

## Estrutura do repositório

- `comando/` — Código do comando transmissor (Arduino Nano)
- `drone/` — Firmware do drone recetor (Arduino Pro Micro, baseado em MultiWii)

## Hardware

**Comando (transmissor)**
- Arduino Nano (ATmega328P)
- Módulo de rádio NRF24L01
- Ecrã LCD I2C 16x2
- 2 joysticks analógicos + 1 rotary encoder

**Drone (recetor)**
- Arduino Pro Micro (ATmega32U4)
- Módulo de rádio NRF24L01
- Sensor MPU-6050 (giroscópio + acelerómetro)
- 4 motores coreless controlados por MOSFETs SI2300 (configuração QuadX)

## Atribuição / créditos

O firmware do **drone** tem como base o projeto open-source
[MultiWii](https://github.com/multiwii/multiwii-firmware) (licença GPL), com a
parte de rádio NRF24 baseada num exemplo de MultiWii brushed do ELECTRONOOBS.
O meu trabalho consistiu na adaptação e integração ao meu hardware: porte para
o Arduino Pro Micro (ATmega32U4), integração do NRF24L01, configuração QuadX,
mapeamento de pinos e canais, e correção da orientação do MPU-6050.

O **comando** foi desenvolvido por mim em C++/Arduino.

## Bibliotecas necessárias

- RF24 (TMRh20)
- LiquidCrystal_I2C (Frank de Brabander) — comando
- TimerFreeTone (Tim Eckel) — drone

## Autor

Guilherme Mendes — ETPR, 2025/2026
