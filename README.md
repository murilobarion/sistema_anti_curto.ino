# Sistema de Proteção de Tomada com Arduino
## Visão Geral do Projeto

Este projeto consiste em um protótipo de proteção elétrica, desenvolvido com Arduino Uno. O sistema simula a detecção de picos de tensão e alerta o usuário por meio de LEDs e buzzer, protegendo os aparelhos conectados à tomada. Todas as informações são exibidas em tempo real em um display OLED I2C, indicando o status do sistema e possíveis alertas.

O dispositivo é totalmente protótipo e didático, sem conexão direta com a rede elétrica real, garantindo segurança durante testes.

## Funcionalidades Principais

Detecção de Pico de Energia: O sistema utiliza um potenciômetro para simular variações de tensão. Quando o valor máximo é atingido, o sistema aciona um alarme sonoro e visual.

Alerta Visual:
LED Vermelho: Acende quando ocorre um pico de energia, permanecendo aceso durante o alarme.
LED Verde (Tomada): Representa a energia disponível na tomada; é desligado durante o alarme e volta a acender quando o sistema retorna ao estado normal.

Alerta Sonoro: Um buzzer emite um sinal de alarme por 4 segundos quando detectado o pico.
Interface OLED: Exibe mensagens centralizadas no display, informando o status atual do sistema, como "Sistema pronto", "AVISO: Pico de energia!" ou "Sistema resetado".

Reset Manual: Ao girar o potenciômetro para baixo durante o alarme, o sistema entra em modo de reset por 3 segundos, mantendo o LED vermelho aceso, antes de voltar ao estado normal.

## Componentes de Hardware

1x Arduino Uno
1x Potenciômetro (para simular pico de tensão)
1x Buzzer Passivo
1x LED Vermelho (5mm)
1x LED Verde (5mm) – Representa a tomada
2x Resistores de 220Ω (para LEDs)
1x Display OLED 128x64 I2C (Adafruit)
1x Protoboard
Jumpers (macho-macho)

## Bibliotecas de Software

As seguintes bibliotecas precisam ser instaladas na IDE Arduino através do "Gerenciador de Bibliotecas":
U8g2 – Para controle do display OLED
Wire – Para comunicação I2C com o display

## Instruções de Uso
Montagem: Monte o circuito conforme especificado no arquivo esquematicos/guia_de_conexoes.txt.
Alimentação: Conecte o Arduino Uno a uma fonte USB de 5V (carregador de celular ou computador). O LED verde acenderá indicando que a tomada está “ativa”.
Simulação de Pico: Gire o potenciômetro para o máximo. O sistema acionará o LED vermelho e o buzzer, desligando temporariamente o LED verde. Após 4 segundos, o buzzer para e o LED vermelho permanece por mais 10 segundos antes de voltar ao estado normal.
Reset Manual: Gire o potenciômetro para baixo durante o alarme ou LED fixo. O sistema entra em modo reset por 3 segundos com o LED vermelho aceso e, em seguida, retorna ao estado normal com o LED verde aceso.
Visualização do Status: O display OLED exibirá mensagens centralizadas sobre o estado atual do sistema, atualizando automaticamente conforme mudanças no potenciômetro.
