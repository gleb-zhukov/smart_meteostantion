/*
  Name:        meteo.ino
  Created:     11.09.2022
  Author:      Zhukov Gleb (https://github.com/zbltrz)
  Description:


   данный проект - метеостанция, позволяющая мониторить различные показатели воздуха (температура, влажность, давление, концентрация со2)

  устройства в системе работают по принципу master-slave,
    где master - устройство, выходящее в интернет и работающее с Telegram.
    slave - подчиненные устройства, принимающие команды от master.
    подчиненные устройства связываются с master по протоколу ESP-NOW.

    структуры для связи между устройствами:

  struct {
    uint8_t ID; //ID устройства (розетка, выключатель или сигнализация)
    uint8_t command; //команда устройства
  } from_device;


    struct {
    uint8_t ID; //ID устройства (розетка, выключатель или сигнализация)
    uint8_t command; //команда устройства
    uint8_t bright; //яркость (для диммера и rgb-контроллера)
    uint8_t r;
    uint8_t g;
    uint8_t b;
    float humidity;
    float temperature;
    uint16_t ppm; //(co2)
    uint16_t pressure;
  } to_device;

  какие команды или ответы отправляет это устройство:

  6 - slave метеостанция

       command:
       1 - ответ OK вместе с показаниями датчиков

       2 - запрос к master о настройке


       humidity:
       0-100(float) - влажность в %

       temperature:
       0-100(float) - температура в С

       ppm:
       0-5000(uint16_t) - концентрация СО2

       pressure:
       600-900(uint16_t) - давление в миллиметрах ртутного столба

*/

#define FIRMWARE_VERSION "1.2"
#define NEXT_FIRMWARE_VERSION "1.3"


#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <TimerMs.h>
#include "EncButton.h"
#include <DHT.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>

TFT_eSPI tft = TFT_eSPI();

#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

#include "jpeg1.h"
#include "jpeg2.h"
#include "jpeg3.h"
#include "jpeg4.h"
#include "jpeg5.h"



#define DHTPIN 12 //пин датчика
#define DHTTYPE DHT22 //тип датчика

DHT dht(DHTPIN, DHTTYPE); //для работы библиотеки датчика DHT
EncButton<EB_TICK, 4> butt(INPUT_PULLUP); //для работы кнопки
////////////////////WIFI////////////////////

#define WIFI_SSID "OCISLY" //для настройки канала

////////////////////TIME////////////////////
TimerMs tmr_check_channel(1000 * 60 * 5, 1, 0); //для синхронизации канала
TimerMs tmr_wi_fi(1000 * 60 * 2, 1, 0); //для периодического включения wi-fi, чтобы проверить доступность обновления прошивки
TimerMs tmr_check_sensor(1000 * 3, 1, 0);


bool set_channel_flag; //для настройки канала
bool read_sensors_flag; //для получения инфы с датчиков

uint8_t broadcast_address[6]; //для хранения mac-адреса

byte lcd_mode; //режим экрана (температура, влажность и тд)


void setup() {



  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(4096);
  dht.begin(); //инициализация датчика

tft.begin();
tft.setRotation(1);  // portrait

tft.fillScreen(TFT_BLACK);
drawArrayJpeg(logo_jpg, sizeof(logo_jpg), 0, 0); // Draw a jpeg image stored in memory
  delay(3000);
lcd_get_temp();

  configTime("MSK-3MSD-3", "time.google.com", "time.windows.com", "pool.ntp.org");




  set_channel();



  if (esp_now_init() != 0) {
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(data_recv);
  esp_now_register_send_cb(data_sent);

  char ssid[35];
  char pass[65];
  EEPROM.get(3430, ssid);
  EEPROM.get(3467, pass);
  WiFi.begin(ssid, pass);


  butt.setHoldTimeout(3000);
}




void loop() {

  butt.tick(); //обработка кнопки

  if (tmr_check_channel.tick()) { //если сработал таймер проверки канала

    uint8_t broadcast_address[6];

    EEPROM.get(150, broadcast_address);

    send_packet(broadcast_address, 7); //отправляем пакет c несуществующей командой, проверяем дошёл ли он в void data_send
  }

  if (read_sensors_flag) {
    read_sensors_flag = false;
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    Serial.println("send packet");
    send_packet(broadcast_address, temp, hum); //ответ успешно с показаниями датчиков
  }

  if (set_channel_flag) {
    set_channel_flag = false;
    set_channel();
  }

  if (tmr_wi_fi.tick()) {
    char ssid[35];
    char pass[65];
    EEPROM.get(3430, ssid);
    EEPROM.get(3467, pass);
    WiFi.begin(ssid, pass);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("update");
    Serial.println(setClock());
    update_firm();
  }

if (tmr_check_sensor.tick()) { //если сработал таймер обновления показания датчиков
    switch (lcd_mode) {
      case 0:
        {
          byte temp = dht.readTemperature();
          String mes;
          mes += "+";
          mes += temp;
          mes += " C    ";
          tft.drawString(mes, 64, 29, 4);
        }
        break;
      case 1:
        {
          byte hum = dht.readHumidity();
          String mes;
          mes += hum;
          mes += " %    ";
          tft.drawString(mes, 70, 29, 4);
        }
        break;
      case 2:
        {
          int co2 = random(300, 5000);
          String mes;
          mes += co2;
          mes += "    ";
          tft.drawString(mes, 70, 29, 4);
        }
        break;
      case 3:
        {
          int pres = random(600, 900);
          String mes;
          mes += pres;
          mes += " mm    ";
          tft.drawString(mes, 55, 29, 4);
        }
        break;
    }
  }

  if (butt.click()) { //если кнопка была нажата
    lcd_mode++;
    if (lcd_mode > 3) lcd_mode = 0;
    switch (lcd_mode) {
      case 0:
        lcd_get_temp();
        break;
      case 1:
        lcd_get_hum();
        break;
      case 2:
        lcd_get_co2();
        break;
      case 3:
        lcd_get_pres();
        break;
    }
  }
  
  if (butt.held()) { //если кнопка была зажата
    uint8_t broadcast_address[6];
    for (int i = 0; i < 6; i++) {
      broadcast_address[i] = 0xFF;
    }
    send_packet(broadcast_address, 2);
  }
}
