//Programa: NodeMCU ESP8266 com display Oled I2C
//Autor: Arduino e Cia

#include <Wire.h>
#include "SSD1306Wire.h"

//Pinos do NodeMCU
// SDA => D1 -> D5
// SCL => D2 -> D6

// Inicializa o display Oled
SSD1306Wire  display(0x3c, D5, D6);

void setup()
{
  Serial.begin(115200);
  display.init();
  display.flipScreenVertically();
}

void telainicial()
{
  //Apaga o display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  //Seleciona a fonte
  display.setFont(ArialMT_Plain_16);
  display.drawString(63, 0, "TITLE");
  display.setFont(ArialMT_Plain_10);
  display.drawString(63, 13, "This");
  display.drawString(63, 23, "is my");
  display.drawString(63, 33, "TEST");
  display.display();
}

void graficos()
{
  display.clear();
  //Desenha um quadrado
  display.drawRect(12, 12, 30, 30);
  //Desenha um quadrado cheio
  display.fillRect(20, 20, 35, 35);
  //Desenha circulos
  for (int i = 1; i < 8; i++)
  {
    display.setColor(WHITE);
    display.drawCircle(92, 32, i * 3);
  }
  display.display();
}

void ProgressBar()
{
  for (int counter = 0; counter <= 100; counter++)
  {
    display.clear();
    //Desenha a barra de progresso
    display.drawProgressBar(0, 32, 120, 10, counter);
    //Atualiza a porcentagem completa
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, String(counter) + "%");
    display.display();
    delay(10);
  }
}

void loop()
{
  telainicial();
  delay(10000);
  graficos();
  delay(3000);
  ProgressBar();
  delay(2000);
}