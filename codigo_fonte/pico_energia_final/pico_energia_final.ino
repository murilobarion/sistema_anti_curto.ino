// Definição dos pinos
int pinoTrig = 9;
int pinoEcho = 10;
int pinoBuzzer = 8;
long duracao;
int distancia;

void setup() {
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);
  pinMode(pinoBuzzer, OUTPUT);
}

void loop() {
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);

  duracao = pulseIn(pinoEcho, HIGH);
  distancia = duracao * 0.034 / 2;

  if (distancia < 50) {
    digitalWrite(pinoBuzzer, HIGH);
    delay(100);
    digitalWrite(pinoBuzzer, LOW);
    delay(100);
  } else {
    digitalWrite(pinoBuzzer, LOW);
  }
}