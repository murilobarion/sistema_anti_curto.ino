#include <Wire.h>
#include <U8g2lib.h>

#define POT_PIN A0
#define LED_PIN 9  
#define BUZZER_PIN 5
#define LED_TOMADA 6

U8G2_SSD1306_128X64_NONAME_F_HW_I2C lcd(U8G2_R0, A5, A4, U8X8_PIN_NONE);

int potValor = 0;

enum Estado { NORMAL, ALARME, LED_FIXO, RESET };
Estado estadoAtual = NORMAL;
unsigned long tempoInicio = 0;

void mostrarMensagemCentral(const char* msg) {
  lcd.clearBuffer();
  lcd.setFont(u8g2_font_ncenB08_tr);
  int larguraTela = 128;
  int alturaTela = 64;
  int larguraMsg = lcd.getStrWidth(msg);
  int x = (larguraTela - larguraMsg) / 2;
  int y = alturaTela / 2;
  lcd.drawStr(x, y, msg);
  lcd.sendBuffer();
}

void tocarAlarme(int duracaoTotal) {
  unsigned long start = millis();
  while (millis() - start < duracaoTotal) {
    tone(BUZZER_PIN, 800); 
    delay(200);
    noTone(BUZZER_PIN);
    delay(200);
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_TOMADA, OUTPUT);
  digitalWrite(LED_TOMADA, HIGH);
  lcd.begin();
  mostrarMensagemCentral("Sistema pronto");
}

void loop() {
  potValor = analogRead(POT_PIN);

  switch(estadoAtual) {
    case NORMAL:
      digitalWrite(LED_PIN, LOW);
      digitalWrite(LED_TOMADA, HIGH);
      noTone(BUZZER_PIN);

      if (potValor > 1000) {
        estadoAtual = ALARME;
        tempoInicio = millis();
        digitalWrite(LED_PIN, HIGH); 
        digitalWrite(LED_TOMADA, LOW);
        mostrarMensagemCentral("AVISO: Pico de energia!");
        tocarAlarme(4000); 
        estadoAtual = LED_FIXO;
        tempoInicio = millis();
      }
      break;

    case LED_FIXO:
      digitalWrite(LED_PIN, HIGH); 
      digitalWrite(LED_TOMADA, LOW);
      if (millis() - tempoInicio >= 10000) {
        digitalWrite(LED_PIN, LOW);
        digitalWrite(LED_TOMADA, HIGH); 
        mostrarMensagemCentral("Sistema pronto");
        estadoAtual = NORMAL;
      }
      break;

    case RESET:
      digitalWrite(LED_PIN, HIGH);
      noTone(BUZZER_PIN);
      digitalWrite(LED_TOMADA, LOW); 
      if (millis() - tempoInicio >= 3000) { 
        digitalWrite(LED_PIN, LOW);
        digitalWrite(LED_TOMADA, HIGH); 
        estadoAtual = NORMAL;
        mostrarMensagemCentral("Sistema pronto");
      }
      break;
  }

  if (potValor < 1000 && (estadoAtual == LED_FIXO)) {
    estadoAtual = RESET;
    tempoInicio = millis();
    mostrarMensagemCentral("Sistema resetado");
  }

  delay(50);
}
