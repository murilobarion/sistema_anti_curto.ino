#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <U8g2lib.h>

#define POT_PIN 34
#define LED_VERDE 27
#define LED_AMARELO 14
#define LED_VERMELHO 25
#define BUZZER_PIN 26

int LIMITE_NORMAL = 700;
int LIMITE_CRITICO = 1000;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C lcd(U8G2_R0, U8X8_PIN_NONE, 22, 21);

int potValor = 0;
bool aparelhoLigado = true;
String statusTensao = "---";

unsigned long tempoInicioAlarme = 0;
unsigned long tempoBeep = 0;
bool estadoBeep = false;

bool precisaReiniciar = false; 
unsigned long tempoFimReboot = 0; 
const long DURACAO_REBOOT_MS = 3000; 

const char* ssid = "projeto_esp32";
const char* password = "12345";

WebServer server(80);

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

String paginaHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Painel de Pico de Energia</title>
<style>
/* Adicionando uma keyframe pra fazer o alerta piscar! */
@keyframes blink {
  0% { opacity: 1; }
  50% { opacity: 0.3; }
  100% { opacity: 1; }
}

body { 
  font-family: Arial, sans-serif; 
  text-align: center; 
  background: #2c3e50; 
  color: #ecf0f1; 
  padding: 20px; 
}
.container { 
  max-width: 400px; 
  margin: 40px auto; 
  background: #34495e; 
  padding: 25px; 
  border-radius: 10px; 
  /* A sombra chique pra dar profundidade */
  box-shadow: 0 10px 25px rgba(0,0,0,0.3);
}
h1 { color: #1abc9c; margin-top: 0; }
button { 
  padding: 12px 25px; 
  border: none; 
  border-radius: 5px; 
  cursor: pointer; 
  font-size: 1em; 
  font-weight: bold;
  background: #00d9ff; 
  color: #111; 
  margin: 10px 0; 
  transition: all 0.2s ease;
}
button:hover { 
  background: #00b4d8; 
  transform: scale(1.05);
  box-shadow: 0 4px 10px rgba(0, 217, 255, 0.2);
}
p { font-size: 1.2em; }

/* Onde a mágica acontece */
#status {
  font-weight: bold;
  font-size: 1.3em;
  padding: 5px 10px;
  border-radius: 5px;
  transition: all 0.3s ease;
}
/* Classes que o JS vai adicionar */
.status-normal { background: #2ecc71; color: #fff; }
.status-perigo { 
  background: #e74c3c; 
  color: #fff; 
  /* AQUI! Ele vai piscar! */
  animation: blink 1s linear infinite; 
}
.status-off { background: #95a5a6; color: #2c3e50; }

/* Dando um up no "Valor" */
#valor {
  font-size: 1.5em;
  font-weight: bold;
  color: #1abc9c;
  background: #2c3e50;
  padding: 5px 15px;
  border-radius: 5px;
}

/* Deixando o histórico mais com cara de "log" */
#historico {
  text-align: left;
  margin-top: 20px;
  max-height: 150px;
  overflow-y: auto;
  background: #2c3e50;
  padding: 10px;
  border-radius: 5px;
  font-family: 'Courier New', Courier, monospace;
  font-size: 0.9em;
  border: 1px solid #4a637c;
}
</style>
</head>
<body>
<div class="container">
<h1>Monitor Anti-Pico</h1>
<p>Status: <span id="status">--</span></p>
<p>Valor (Pot): <span id="valor">--</span></p>
<button onclick="toggleAparelho()">Desligar/Ligar Aparelho</button>
<div id="historico"></div>
</div>

<script>
function addHistorico(msg){
  const hist = document.getElementById("historico");
  const agora = new Date().toLocaleTimeString();
  hist.innerHTML = `[${agora}] ${msg}<br>` + hist.innerHTML;
}

function toggleAparelho(){
  fetch('/toggle').then(r=>r.text()).then(msg=>addHistorico(msg));
}

setInterval(()=>{
  fetch("/dados").then(r=>r.json()).then(data=>{
    document.getElementById("valor").innerText = data.valor;
    
    // --- AQUI ESTÁ A LÓGICA NOVA ---
    const statusEl = document.getElementById("status");
    statusEl.innerText = data.status;
    
    // 1. Limpa as classes de cor antigas
    statusEl.className = ''; 
    
    // 2. Adiciona a classe certa baseada no texto
    if (data.status.includes("NORMAL")) {
      statusEl.classList.add('status-normal');
    } else if (data.status.includes("PICO")) {
      statusEl.classList.add('status-perigo');
    } else {
      // Pra qualquer outro status, tipo "Desligado"
      statusEl.classList.add('status-off');
    }
    // --- FIM DA LÓGICA NOVA ---

    if(data.evento) addHistorico(data.evento);
  });
}, 1000); // 1000ms = 1 segundo
</script>
</body>
</html>
)rawliteral";
  return html;
}

void handleDadosJSON() {
  String evento = "";
  int valorWeb = 0; 
  String novoStatus = "";

  if (tempoFimReboot > 0 && millis() < tempoFimReboot) {
    novoStatus = "Reiniciando...";
    valorWeb = 0; 
    if (statusTensao != "Reiniciando...") evento = "Sistema estabilizando pós-surto.";
    statusTensao = "Reiniciando...";
  }
  else if (aparelhoLigado) {
    valorWeb = analogRead(POT_PIN);
    valorWeb = map(valorWeb, 0, 4095, 0, 1023);

    if (valorWeb <= LIMITE_NORMAL) novoStatus = "Normal";
    else if (valorWeb <= LIMITE_CRITICO) novoStatus = "Atenção";
    else novoStatus = "Crítico";

    if (novoStatus != statusTensao) {
      evento = "Mudança detectada: " + novoStatus;
      statusTensao = novoStatus;
    }
  } else {
    novoStatus = "Desligado";
    valorWeb = 0;
    statusTensao = "Desligado";
  }

  String json = "{";
  json += "\"valor\":" + String(aparelhoLigado ? valorWeb : 0) + ",";
  json += "\"status\":\"" + statusTensao + "\",";
  json += "\"evento\":\"" + evento + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleToggle() {
  aparelhoLigado = !aparelhoLigado;
  if (!aparelhoLigado) {
    noTone(BUZZER_PIN);
    tempoInicioAlarme = 0;
    estadoBeep = false;
    precisaReiniciar = false;
    tempoFimReboot = 0;
  }
  server.send(200, "text/plain", aparelhoLigado ? "Aparelho LIGADO" : "Aparelho DESLIGADO");
}

void handleRoot() {
  server.send(200, "text/html", paginaHTML());
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.begin();
  mostrarMensagemCentral("Iniciando AP...");

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Acesse: http://"); Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/dados", handleDadosJSON);
  server.on("/toggle", handleToggle);
  server.begin();

  mostrarMensagemCentral(IP.toString().c_str());
  delay(2000);
}

// ==== LOOP ====
void loop() {
  server.handleClient(); // O garçom não pode parar

  if (!aparelhoLigado) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    noTone(BUZZER_PIN);
    mostrarMensagemCentral("Aparelho desligado");

    // Reseta TODAS as flags
    tempoInicioAlarme = 0;
    estadoBeep = false;
    precisaReiniciar = false;
    tempoFimReboot = 0;
    return;
  }

  // === LÓGICA DO "REBOOT" (O ANTI-PICO) ===
  // Se o timer de reboot foi ativado...
  if (tempoFimReboot > 0) {
    if (millis() < tempoFimReboot) {
      // Ainda estamos "reiniciando"...
      mostrarMensagemCentral("Reiniciando...");
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_AMARELO, LOW); // Tudo apagado
      digitalWrite(LED_VERMELHO, LOW);
      noTone(BUZZER_PIN);
      return; // Vaza do loop, não lê potenciômetro
    } else {
      // Acabou os 3s!
      tempoFimReboot = 0; // Reseta o timer do reboot
      mostrarMensagemCentral("Sistema OK!");
      delay(500); // Um mini delay SÓ pra gente ver a msg
    }
  }

  // Leitura do potenciômetro para a lógica física
  potValor = analogRead(POT_PIN);
  potValor = map(potValor, 0, 4095, 0, 1023);

  // === Lógica física NÃO-BLOQUEANTE ===

  if (potValor <= LIMITE_NORMAL) {
    // --- Estado Normal ---
    if (precisaReiniciar) {
      // *** ACIONA O REBOOT! ***
      // Ele saiu do Vermelho direto pro Verde!
      tempoFimReboot = millis() + DURACAO_REBOOT_MS; // Começa a contar 3s
      precisaReiniciar = false; // Desarma o gatilho
    } else {
      // Vida normal, nada pra reiniciar
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_AMARELO, LOW);
      digitalWrite(LED_VERMELHO, LOW);
      mostrarMensagemCentral("Sistema pronto");
    }
    noTone(BUZZER_PIN);
    tempoInicioAlarme = 0;
    estadoBeep = false;

  } else if (potValor <= LIMITE_CRITICO) {
    // --- Estado de Atenção ---
    if (precisaReiniciar) {
      // *** ACIONA O REBOOT! ***
      // Ele saiu do Vermelho pro Amarelo!
      tempoFimReboot = millis() + DURACAO_REBOOT_MS; // Começa a contar 3s
      precisaReiniciar = false; // Desarma o gatilho
    } else {
      // Vida normal, nada pra reiniciar
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_AMARELO, HIGH);
      digitalWrite(LED_VERMELHO, LOW);
      mostrarMensagemCentral("Aviso: Tensao alta!");
    }
    noTone(BUZZER_PIN);
    tempoInicioAlarme = 0;
    estadoBeep = false;

  } else {
    // --- ESTADO CRÍTICO ---
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    mostrarMensagemCentral("AVISO: Pico de energia!");

    // *** ARMA O GATILHO DO REBOOT ***
    // Avisa que a gente ENTROU no estado crítico
    precisaReiniciar = true;

    // Lógica do alarme (continua a mesma)
    if (tempoInicioAlarme == 0) {
      tempoInicioAlarme = millis();
    }
    if (millis() - tempoInicioAlarme < 4000) {
      if (!estadoBeep && (millis() - tempoBeep > 50)) {
        tone(BUZZER_PIN, 2000);
        estadoBeep = true;
        tempoBeep = millis();
      } else if (estadoBeep && (millis() - tempoBeep > 350)) {
        noTone(BUZZER_PIN);
        estadoBeep = false;
        tempoBeep = millis();
      }
    } else {
      noTone(BUZZER_PIN);
      estadoBeep = false;
    }
  }
}