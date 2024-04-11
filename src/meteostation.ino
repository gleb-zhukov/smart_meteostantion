/*
  Name:        smart_socket.ino
  Created:     22.06.2022
  Author:      Zhukov Gleb (https://github.com/zbltrz)
  Description:

  в проекте аналогичные устройства в разных корпусах: на данный момент реле и розетки.
  далее все подобные устройства для удобства будут называться реле.
  и розетки, и прочие, построенные на таких же платах.

  устройства в системе работают по принципу master-slave,
    где master - устройство, выходящее в интернет и работающее с Telegram.
    slave - подчиненные устройства, принимающие команды от master
    подчиненные устройства связываются с master по протоколу ESP-NOW.

    структуры для связи между устройствами:
    to_device (на принимающем устройстве from_device)


  какие данные в структуре отправляет определенное устройство:

  ID:
  0 - master реле, розетка, метеостанция, диммер

       command:
       0 - запрос на включение/выключение (изменение состояния) (реле, розетка, диммер, rgb-контроллер)
       1 - запрос состояния прибора  (реле, розетка, диммер, rgb-контроллер)
       2 - ответ на запрос о настройке
       3 - отчёт о получении сообщения (выключатель)
       4 - запрос на включение
       5 - запрос на выключение
       6 - запрос к диммеру или rbg-контроллеру о настройке определённой яркости (связано с bright)
       7 - запрос к rgb контроллеру о настройке определенного цвета (связано с RGB)
       8 - запрос к метеостанции о получении значения датчиков

       bright:
       0-255 - общая яркость

       r:
       0-255 - яркость красного

       g:
       0-255 - яркость зеленого

       b:
       0-255 - яркость синего

  1 - slave реле

       command:
       0 - выключено
       1 - включено
       2 - запрос к master о настройке

  2 - выключатель

       command:
       0 - запрос к slave на включение/выключение
       1 - запрос к master о настройку
       2 - отчёт об успешной настройке

  3 - датчик открытия двери

       command:
       0 - сигнал тревги
       1 - запрос к master о настройке

  4 - slave диммер

       command:
       0 - выключено
       1 - включено
       2 - запрос к master о настройке

       bright:
       0-255 - яркость

  5 - rgb контроллер

       command:
       0 - выкл
       1 - вкл
       2 - запрос к master о настройке

       bright:
       0-255 - общая яркость

       r:
       0-255 - яркость красного

       g:
       0-255 - яркость зеленого

       b:
       0-255 - яркость синего

  6 - slave метеостанция

       command:
       0 - запрос к master о настройке

       humidity:
       0-100(float) - влажность в %

       temperature:
       0-100(float) - температура в С

       ppm:
       0-5000(uint16_t) - концентрация СО2

       pressure:
       600-900(uint16_t) - давление в миллиметрах ртутного столба

  7 - датчик температуры и влажности

       command:
       0 - запрос к master о настройке


       humidity:
       0-100(float) - влажность в %

       temperature:
       0-100(float) - температура в С


*/

#define FIRMWARE_VERSION "2.4"
#define NEXT_FIRMWARE_VERSION "2.5"

#include <Arduino.h>
#include <Ticker.h>
#include "FastBot.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <espnow.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <TimerMs.h>
#include <time.h>
#include "EncButton.h"
#include <BlynkSimpleEsp8266.h>


static const char serverCACert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFMDCCBBigAwIBAgISBNG02QSCDx60y4HsGSAPu9w7MA0GCSqGSIb3DQEBCwUA
MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD
EwJSMzAeFw0yMjA1MTYwNjUwMzJaFw0yMjA4MTQwNjUwMzFaMBcxFTATBgNVBAMT
DGdyaWItdGVjaC5ydTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOPW
qDVWrwwt9CTQncV3a/KwQdshxna2wvKCaKfwogrzrD+PsIfFJVrV91lz/8IV9/d+
uDSb63sASxvl8oPuxHKaqHxr4nqCbEOIEyyy6pd7AvBXK3aWATnf5N2l8DdBlOMi
arRbag6U3I8quIFX79RkW98jmfE+BKHrIlq4HMIjaLHuh714wLLv5a1qUu/Nz0bC
cE6dxjKAlPczJDL8DM7U73S7F8TVJlKuFLNd2zJhY1jbBEo+n8UKR3wrnoUDrGlb
rqRJa2mSDnpKKd3WkU0fn8Dr8Y1JSZEYNuOW0UGeHH4dmG9EnQFYFZpxFzBBQB8a
kvDb954UL3TVONiDsHcCAwEAAaOCAlkwggJVMA4GA1UdDwEB/wQEAwIFoDAdBgNV
HSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4E
FgQUBX2HRLeU1UrzD/mPmxd+eMKhccQwHwYDVR0jBBgwFoAUFC6zF7dYVsuuUAlA
5h+vnYsUwsYwVQYIKwYBBQUHAQEESTBHMCEGCCsGAQUFBzABhhVodHRwOi8vcjMu
by5sZW5jci5vcmcwIgYIKwYBBQUHMAKGFmh0dHA6Ly9yMy5pLmxlbmNyLm9yZy8w
KQYDVR0RBCIwIIIMZ3JpYi10ZWNoLnJ1ghB3d3cuZ3JpYi10ZWNoLnJ1MEwGA1Ud
IARFMEMwCAYGZ4EMAQIBMDcGCysGAQQBgt8TAQEBMCgwJgYIKwYBBQUHAgEWGmh0
dHA6Ly9jcHMubGV0c2VuY3J5cHQub3JnMIIBBAYKKwYBBAHWeQIEAgSB9QSB8gDw
AHYAQcjKsd8iRkoQxqE6CUKHXk4xixsD6+tLx2jwkGKWBvYAAAGAy9juEQAABAMA
RzBFAiEA9y6HoJ1Wu9W9Xo2hnAlWnvxKaV3YkIEZn7gO9xFjO8oCIDmH4k3mgNCh
+gylONt8hgCz7w6YVJr1IBDrPkq4HPwMAHYARqVV63X6kSAwtaKJafTzfREsQXS+
/Um4havy/HD+bUcAAAGAy9juIAAABAMARzBFAiBVDQAKe5sokZM0gkAezyfhD9RJ
qFrtUS6idpATyAZ+MQIhAKp3Q1XGm5DdYxrxE+1zXafCW6q/rXUA7xO1/r06SviF
MA0GCSqGSIb3DQEBCwUAA4IBAQA/DsuHgVbU//0KezIcctG+z+UM3v0YyZYK7awx
zZolPTiBQhCIAxbPVxBtbRnTkJllZ55fTFTLVoVR6HgH74qi22tBHo3Yp9LRm/M4
i7qXLiH1ZyvPVaFHNhDtn+L4BrLIZqap+nrcn3hV7XEZjcGQaaMogBHH7sw3zugN
wqtotE1YWMARhmil6vM8F1V9RWkq0OV1K0emLSmvXGIuK1T173BSkdJ0/2H7gs0W
ygujuoyEjQyQd2WCZ+eZZw9BN98lsR18gOt9fcQE+nN44+4HjWgBDCiuFwTlvOJ/
xtrpyKpz/BNmYBm2wtuug6nbeqzg6oLHTDqlCpzO/jLjZaZt
-----END CERTIFICATE-----
)EOF";

EncButton<EB_TICK, 4> butt(INPUT_PULLUP); //для работы кнопки


  ////////////////////BLYNK////////////////////
  //#define BLYNK_AUTH_TOKEN            "fqt7bklhTwl_1IJCa0yqcOq2maLa4SW-"
    #define BLYNK_AUTH_TOKEN            "uoUJu2wTViTX5tQVkekWIjSBcVGzB-jr"
#define BLYNK_PRINT //Serial

  char auth[] = BLYNK_AUTH_TOKEN;
////////////////////SERVER////////////////////

IPAddress local_IP(10, 10, 10, 10);
IPAddress gateway(10, 10, 10, 10);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer webServer(80);

////////////////////TELEGRAM BOT////////////////////

FastBot bot;

////////////////////FLAGS////////////////////

bool answer_flag; //для ответа пользователю в tg
bool esp_now_flag; //для отправки данных по esp-now
bool set_channel_flag; //для настройки канала (slave)

String answer_ID; //ID пользователя для ответа ему в Telegram

byte mode_flag; //0 - режим master-реле, 1 - режим сервера, 2 - режим slave-реле;
byte device_type_global; //тип устройства 1-розетка, 2-rgb лента и тд
byte room_id; //для хранения номера комнаты при обращении в tg
byte tg_mode; //для выбора case в обработчике сообщений tg
byte answer_command; //для ответа пользователю


////////////////////WIFI////////////////////

#define AP_SSID "grib socket"
#define AP_PASS "iloveyou"

#define WIFI_SSID "grib socket" //для настройки канала (режим slave-реле)

////////////////////TIME////////////////////

TimerMs firmware_tmr(1000 * 60 * 30, 1, 0); //для обновления прошивки
TimerMs reset_tmr(5000, 0, 1); //для перезагрузки при сохранении параметров

TimerMs tmr_sync_channel(1000*60*5, 1, 0); //для синхронизации канала (режим slave-реле)
TimerMs tmr_wi_fi(1000 * 60 * 30, 1, 0); //для периодического включения wi-fi, чтобы проверить доступность обновления прошивки (режим slave-реле)


////////////////////FUNCTIONS////////////////////



////////////////////ESP-NOW////////////////////

uint8_t broadcast_address[6]; //для хранения mac-адреса




void setup() {

  EEPROM.begin(4096);


      pinMode(12, OUTPUT);

    bool pin_state;
    EEPROM.get(3570, pin_state);
    digitalWrite(12, pin_state);
  //Serial.begin(115200);
//Serial.println();



  configTime("MSK-3MSD-3", "time.google.com", "time.windows.com", "pool.ntp.org");

  
  if (digitalRead(4) == LOW)  //если зажали кнопку при включении входим в режим сервера (mode_flag = 1)
 {
    //Serial.println(F("LOW PIN"));
    mode_flag = 1;
    first_start(); //функция, обнуляющая все переменные при первом запуске или сбросе настроек
    delay(100);
  }

  if (EEPROM.read(1) != 4) //проверка ключа на первый запуск
  {
    //Serial.println(F("FIRST START"));
    EEPROM.write(1, 4); //если запуск первый, записываем ключ

    mode_flag = 2; //т.к. запуск первый, входим в режим slave-реле

    EEPROM.put(3550, mode_flag);
    EEPROM.commit();

    first_start(); //функция, обнуляющая все переменные при первом запуске или сбросе настроек

  }



  EEPROM.get(3550, mode_flag);

  //если режим master-реле
  if (mode_flag == 0) {
//Serial.println(F("mode 0"));

char ssid[35];
  char pass[65];
  EEPROM.get(3430, ssid);
  EEPROM.get(3467, pass);
  
  
  Blynk.begin(auth, ssid, pass, "blynk.tk", 8080);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
  //  connect_wifi(); //функция соединения с Wi-Fi

    bool check_firm;
EEPROM.get(3, check_firm);
if (check_firm){
  update_firm();
}

    uint32_t userid[10];
    EEPROM.get(3580, userid);
    String massive = "0,";
    bot.chatIDs = "0,";
    for (int i = 0; i < 10; i++) {
      if (userid[i] != 0) {
        bot.chatIDs += userid[i];
        bot.chatIDs += ",";
      }
    }
    if (bot.chatIDs == "0,") {
      bot.chatIDs = "";
      tg_mode = 1;
    }
    char token[60];  //Telegram token
    EEPROM.get(3368, token); //получаем в глобальную переменную значения токена с EEPROM
    bot.setToken(token);
    bot.attach(newMsg);

    String message;
    message += F("smart socket ");
    message += String(FIRMWARE_VERSION);
bot.sendMessage(message, String(321588402));

    //запускаем esp-now (для связи с периферийными устройствами)
    if (esp_now_init() != 0) {
      return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO); //роль контроллера. комбо для двустороннего общения
    esp_now_register_send_cb(data_sent); //регистрируем функцию вызываемую при передаче данных
    esp_now_register_recv_cb(data_recv); //регистрируем функцию вызываемую при приёме данных

  }

  //если режим сервера
  else if (mode_flag == 1) {
    //Serial.println(F("mode 1"));
    WiFi.softAPConfig(local_IP, gateway, subnet); //настраиваем свою точку доступа
    WiFi.softAP(AP_SSID, AP_PASS); //создаём свою точку доступа

    webServer.on("/save", handleForm);
    webServer.on("/confirm", confirmPage);
    webServer.on("/", mainForm);
    webServer.onNotFound(mainForm);
    webServer.begin();
  }
  //если режим slave-реле
  else if (mode_flag == 2) {
//Serial.println(F("mode 2"));

    set_channel();

    if (esp_now_init() != 0) {
      return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_recv_cb(data_recv_sock);
    esp_now_register_send_cb(data_sent_sock);

    char ssid[35];
    char pass[65];
    EEPROM.get(3430, ssid);
    EEPROM.get(3467, pass);
    WiFi.begin(ssid, pass);

    butt.setHoldTimeout(3000);
  }
}

void loop() {

  switch (mode_flag) {

    ////////////////////режим master////////////////////

    case 0:
      {

        if (esp_now_flag)
        {
          esp_now_flag = false;
          send_answer(answer_command);
        }
    Blynk.run();

        bot.tick();

        if (firmware_tmr.tick() && (WiFi.status() == WL_CONNECTED)) {


  if (setClock() == 2){

bool check_firm = true;
EEPROM.put(3, check_firm);
    EEPROM.commit();
    delay(100);
    ESP.restart();
  }
  
        }

        butt.tick();

        if (butt.click()) { //если кликнули на кнопку
          set_relay();
        }

      }
      break;

    ////////////////////режим сервера////////////////////
    case 1:
      {
        webServer.handleClient(); //обработка сервера

        butt.tick(); //обработка кнопки


        if (reset_tmr.tick()) {
          delay(1000);
          ESP.restart();
        }

        if (butt.click()) { //если кликнули на кнопку
          EEPROM.write(1, 0); // key for writing start data
          EEPROM.commit();
          delay(100);
          ESP.restart();
        }
      }
      break;

    ////////////////////режим slave////////////////////
    case 2:
      {

        if (WiFi.status() == WL_CONNECTED) {
          update_firm();
        }

        if (set_channel_flag){
          set_channel_flag = false;
          set_channel();
        }

        butt.tick(); //обработка кнопки

        if (tmr_sync_channel.tick()) { //если сработал таймер

             uint8_t broadcast_address[6];

    EEPROM.get(150, broadcast_address);

    send_packet(broadcast_address, 7); //отправляем пакет c несуществующей командой, проверяем дошёл ли он в void data_send
        }

        if (tmr_wi_fi.tick()) {
          char ssid[35];
          char pass[65];
          EEPROM.get(3430, ssid);
          EEPROM.get(3467, pass);
          WiFi.begin(ssid, pass);
        }



        if (butt.click()) { //если кликнули на кнопку
          set_relay(); //функция включения/выключения реле
        }

        if (butt.held()) { //если кнопка была зажата
          for (int i = 0; i < 6; i++) {
            broadcast_address[i] = 0xFF;
          }
          struct {
            uint8_t ID; //ID устройства (розетка, выключатель или сигнализация)
            uint8_t command; //команда устройства
          } to_device;
          to_device.ID = 1; //запрос от slave
          to_device.command = 2; //запрос на сопряжение
          esp_now_send(broadcast_address, (uint8_t *) &to_device, sizeof(to_device));
        }
      }
      break;
  }
}
