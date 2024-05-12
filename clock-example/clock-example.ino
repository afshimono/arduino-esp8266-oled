//Programa: Relogio com NodeMCU ESP8266 e display Oled - NTP
//Referencia: A Beginner's Guide to the ESP8266 - Pieter P.
//Adaptacoes e alteracoes: Arduino e Cia

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SSD1306Wire.h>

//Cria uma instancia da classe ESP8266WiFiMulti, chamada 'wifiMulti'
ESP8266WiFiMulti wifiMulti;

//Cria uma instancia da classe WifiUDP para enviar e receber dados
WiFiUDP UDP;

IPAddress timeServerIP;
//Define o servidor NTP utilizado
const char* NTPServerName = "b.ntp.br";

//Time stamp do NTP se encontra nos primeiros 48 bytes da mensagem
const int NTP_PACKET_SIZE = 48;

//Buffer para armazenar os pacotes transmitidos e recebidos
byte NTPBuffer[NTP_PACKET_SIZE];

//Pinos do NodeMCU - Interface I2C: SDA => D1, SCL => D2

//Inicializa o display Oled
SSD1306Wire  display(0x3c, D5, D6);

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println("rn");

  //Inicia a comunicacao com os hothospts configurados
  startWiFi();
  startUDP();

  if (!WiFi.hostByName(NTPServerName, timeServerIP))
  {
    //Obtem o endereco IP do servidor NTP
    Serial.println("DNS lookup failed. Rebooting.");
    Serial.flush();
    ESP.reset();
  }
  Serial.print("IP do servidor NTP:t");
  Serial.println(timeServerIP);

  Serial.println("rnEnviando requisicao NTP...");
  sendNTPpacket(timeServerIP);

  //Inicializacao do display
  display.init();
  display.flipScreenVertically();
}

//Requisita horario do servidor NTP a cada minuto
unsigned long intervalNTP = 60000;
unsigned long prevNTP = 0;
unsigned long lastNTPResponse = millis();
uint32_t timeUNIX = 0;

unsigned long prevActualTime = 0;

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - prevNTP > intervalNTP)
  {
    //Verificar se passou um minuto da ultima requisicao
    prevNTP = currentMillis;
    Serial.println("rnEnviando requisicao NTP ...");
    sendNTPpacket(timeServerIP);
  }

  uint32_t time = getTime();
  if (time)
  {
    timeUNIX = time - 10800;
    Serial.print("Resposta NTP:t");
    Serial.println(timeUNIX);
    lastNTPResponse = currentMillis;
  } else if ((currentMillis - lastNTPResponse) > 3600000) {
    Serial.println("Mais de 1 hora desde a ultima resposta NTP. Reiniciando.");
    Serial.flush();
    ESP.reset();
  }

  uint32_t actualTime = timeUNIX + (currentMillis - lastNTPResponse) / 1000;
  if (actualTime != prevActualTime && timeUNIX != 0)
  {
    //Verifica se passou um segundo desde a ultima impressao de valores no serial monitor
    prevActualTime = actualTime;
    Serial.printf("rUTC time:t%d:%d:%d   ", getHours(actualTime), getMinutes(actualTime), getSeconds(actualTime));
    Serial.println();
  }

  //Mostrando a hora no display
  //Apaga o display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  //Seleciona a fonte
  display.setFont(ArialMT_Plain_16);
  //Mostra o titulo na parte superior do display
  display.drawString(63, 0, "NTP Clock");
  //Mostra o horario atualizado
  display.setFont(ArialMT_Plain_24);
  display.drawString(29, 29, String(getHours(actualTime)));
  display.drawString(45, 28, ":");
  display.drawString(62, 29, String(getMinutes(actualTime)));
  display.drawString(78, 28, ":");
  display.drawString(95, 29, String(getSeconds(actualTime)));
  display.display();
}

void startWiFi()
{
  //Coloque aqui as redes wifi necessarias
  wifiMulti.addAP("WIFI ID 1", "WIFI PASSWORD 1");
  wifiMulti.addAP("WIFI ID 2", "WIFI PASSWORD 2");

  Serial.println("Conectando");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    //Aguarda a conexao da rede wifi
    delay(250);
    Serial.print('.');
  }
  Serial.println("rn");
  Serial.print("Conectado a rede ");
  Serial.println(WiFi.SSID());
  Serial.print("Endereco IP:t");
  Serial.print(WiFi.localIP());
  Serial.println("rn");
}

void startUDP()
{
  Serial.println("Iniciando UDP");
  //Inicializa UDP na porta 23
  UDP.begin(123);
  Serial.print("Porta local:t");
  Serial.println(UDP.localPort());
  Serial.println();
}

uint32_t getTime()
{
  if (UDP.parsePacket() == 0)
  {
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  //Combina os 4 bytes do timestamp em um numero de 32 bits
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  //Converte o horario NTP para UNIX timestamp
  //Unix time comeca em 1 de Jan de 1970. Sao 2208988800 segundos no horario NTP:
  const uint32_t seventyYears = 2208988800UL;
  //Subtrai setenta anos do tempo
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}

void sendNTPpacket(IPAddress& address)
{
  //Seta todos os bytes do buffer como 0
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);
  //Inicializa os valores necessarios para formar a requisicao NTP
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  //Envia um pacote requisitando o timestamp
  UDP.beginPacket(address, 123);
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

inline int getSeconds(uint32_t UNIXTime) 
{
  return UNIXTime % 60;
}

inline int getMinutes(uint32_t UNIXTime) 
{
  return UNIXTime / 60 % 60;
}

inline int getHours(uint32_t UNIXTime) 
{
  return UNIXTime / 3600 % 24;
}