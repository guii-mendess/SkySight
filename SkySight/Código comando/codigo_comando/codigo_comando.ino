/* Comando transmissor da PAP - Drone FPV Skysight
   Arduino Nano + NRF24L01 + LCD I2C 16x2 + 2 joysticks + encoder rotativo */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h>

// Pinos dos botões
const int joyLeftSW  = 7;  // botão do joystick esquerdo - liga/desliga o buzzer
const int joyRightSW = 6;  // botão do joystick direito - troca o modo de voo
const int encoderSW  = 5;  // botão do encoder - arma/desarma
const int encoderDT  = 4;  // encoder, sinal DT
const int encoderCLK = 3;  // encoder, sinal CLK

// Estados do sistema
bool isArmed        = false;
bool buzzerActive   = false;
bool flightModeAcro = false; // false = Angle (estabilizado), true = Acro (livre)

// Limite de potência controlado pela rotação do encoder
int throttleLimit = 100;
int lastClkState;

// Guardo o estado anterior dos botões para fazer o debounce
int lastEncoderSW = HIGH;
int lastLeftSW    = HIGH;
int lastRightSW   = HIGH;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Tem de ser exatamente o mesmo endereço que está no drone
const uint64_t pipeOut = 0xE8E8F0F0E1LL;

// Rádio: CE no D10, CSN no D9
RF24 radio(10, 9);

// Pacote que envio para o drone
struct MyData {
  byte throttle;
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1; // armamento (0 ou 255)
  byte AUX2; // buzzer (0 ou 255)
  byte AUX3; // modo de voo (0 ou 255)
};

MyData data;

// Controlo do tempo de atualizacao do LCD para nao usar delay no loop
unsigned long lcdUpdateTime = 0;
const unsigned long lcdUpdateInterval = 150;

// Coloca tudo em valores seguros (motores parados, sticks ao centro, desarmado)
void resetData() {
  data.throttle = 0;
  data.yaw      = 127;
  data.pitch    = 127;
  data.roll     = 127;
  data.AUX1     = 0;
  data.AUX2     = 0;
  data.AUX3     = 0;
}

void setup() {
  // Arranque do radio
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);    // potencia minima - evita picos de corrente que travam a USB
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  radio.stopListening();            // põe o modulo em modo de transmissão
  resetData();

  // Arranque do LCD com a mensagem inicial
  lcd.init();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("Comando PAP");
  lcd.setCursor(1, 1);
  lcd.print("A iniciar...");
  delay(2000);
  lcd.clear();

  // Botões com pull-up interno
  pinMode(encoderSW,  INPUT_PULLUP);
  pinMode(joyLeftSW,  INPUT_PULLUP);
  pinMode(joyRightSW, INPUT_PULLUP);

  // Pinos de rotação do encoder (o módulo já traz resistências próprias)
  pinMode(encoderCLK, INPUT);
  pinMode(encoderDT,  INPUT);

  lastClkState = digitalRead(encoderCLK);
}

// Lê o joystick (0-1023) e converte para 0-255, com o centro a cair em 127.
// Os valores lower/middle/upper sao a calibracao de cada eixo.
int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return (reverse ? 255 - val : val);
}

void loop() {
  unsigned long currentMillis = millis();

  // ---------------------------------------------------------
  // 1. Botões
  // ---------------------------------------------------------

  // Encoder: arma / desarma
  int currentEncoderSW = digitalRead(encoderSW);
  if (currentEncoderSW == LOW && lastEncoderSW == HIGH) {
    isArmed = !isArmed;
    delay(50);
  }
  lastEncoderSW = currentEncoderSW;

  // Joystick esquerdo: liga / desliga o buzzer
  int currentLeftSW = digitalRead(joyLeftSW);
  if (currentLeftSW == LOW && lastLeftSW == HIGH) {
    buzzerActive = !buzzerActive;
    delay(50);
  }
  lastLeftSW = currentLeftSW;

  // Joystick direito: troca o modo de voo
  int currentRightSW = digitalRead(joyRightSW);
  if (currentRightSW == LOW && lastRightSW == HIGH) {
    flightModeAcro = !flightModeAcro;
    delay(50);
  }
  lastRightSW = currentRightSW;

  // ---------------------------------------------------------
  // 2. Rotação do encoder ajusta o limite de potência (entre 40 e 100)
  // ---------------------------------------------------------
  int currentClkState = digitalRead(encoderCLK);
  if (currentClkState != lastClkState && currentClkState == LOW) {
    if (digitalRead(encoderDT) != currentClkState) {
      throttleLimit += 5;
    } else {
      throttleLimit -= 5;
    }
    throttleLimit = constrain(throttleLimit, 40, 100);
  }
  lastClkState = currentClkState;

  // ---------------------------------------------------------
  // 3. Preparar os dados e enviar
  // ---------------------------------------------------------
  data.AUX1 = isArmed        ? 255 : 0;
  data.AUX2 = buzzerActive   ? 255 : 0;
  data.AUX3 = flightModeAcro ? 255 : 0;

int rawThrottle = mapJoystickValues(analogRead(A2), 50, 505, 1020, false); // throttle: cima aumenta
data.yaw        = mapJoystickValues(analogRead(A0), 34, 522, 1020, true);  // yaw: direita aumenta
data.pitch      = mapJoystickValues(analogRead(A1), 12, 544, 1021, true);  // pitch: cima aumenta
data.roll       = mapJoystickValues(analogRead(A3), 13, 524, 1015, true);  // roll: direita aumenta

  // O throttle nunca passa do limite definido pelo encoder
  data.throttle = map(rawThrottle, 0, 255, 0, map(throttleLimit, 0, 100, 0, 255));

  radio.write(&data, sizeof(MyData));

  // ---------------------------------------------------------
  // 4. Atualizar o LCD sem travar o loop
  // ---------------------------------------------------------
  if (currentMillis - lcdUpdateTime >= lcdUpdateInterval) {
    updateLCD();
    lcdUpdateTime = currentMillis;
  }
}

void updateLCD() {
  // Linha de cima: estado de armamento e modo de voo (ocupa os 16 caracteres)
  lcd.setCursor(0, 0);
  lcd.print(isArmed ? "ARMED " : "DISARM");
  lcd.print(flightModeAcro ? " | M:ACRO " : " | M:ANGLE");

  // Linha de baixo: limite de potencia a esquerda, estado do buzzer a direita
  lcd.setCursor(0, 1);
  lcd.print("Lim:");
  lcd.print(throttleLimit);
  lcd.print("%");
  if (throttleLimit < 100) lcd.print(" "); // limpa o digito que sobra ao descer de 100

  // Comeco na coluna 9 para o texto do buzzer caber sem sair do ecrã
  lcd.setCursor(9, 1);
  lcd.print(buzzerActive ? "|BZ:ON " : "|BZ:OFF");
}
