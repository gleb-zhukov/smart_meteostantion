//функция при отправке данных
void data_sent(uint8_t *mac_addr, uint8_t send_status) {
  if (send_status != 0) {
    Serial.println("Пакет не отправлен, настраиваем канал");
set_channel_flag = true;
  }
}


void send_packet(uint8_t *func_mac_addr, uint8_t func_command) {
  struct {
    uint8_t ID; //ID устройства (розетка, выключатель, датчик и т.д.)
    uint8_t command; //команда устройству
  } to_device;
  to_device.ID = 6; //от метеостанции
  to_device.command = func_command;

  esp_now_send(func_mac_addr, (uint8_t *) &to_device, sizeof(to_device));

}

void send_packet(uint8_t *func_mac_addr, float func_temperature, float func_humidity) {
    struct {
    uint8_t ID; //ID устройства (розетка, выключатель или сигнализация)
    uint8_t command; //команда устройства
    uint8_t bright; //яркость (для диммера и rgb-контроллера)
    uint8_t r;
    uint8_t g;
    uint8_t b;
    float humidity;
    float temperature;
    //uint16_t ppm; //(co2)
    //uint16_t pressure;
  } to_device;

  to_device.ID = 6; //от метеостанции
  to_device.command = 1; //ОК
  to_device.humidity = func_humidity;
  to_device.temperature = func_temperature;

    Serial.println(to_device.temperature);
    Serial.println(to_device.humidity);
  esp_now_send(func_mac_addr, (uint8_t *) &to_device, sizeof(to_device));
 Serial.println("SENDING OK");
}

void data_recv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {


  struct {
    uint8_t ID; //ID устройства (розетка, выключатель или сигнализация)
    uint8_t command; //команда устройства
    char router_ssid[35];
    char router_pass[65];
  } from_device;


  memcpy(&from_device, incomingData, sizeof(from_device));

  if (from_device.ID == 0) { //сообщение от master

    //записываем в EEPROM ssid и пароль wifi с выходом в интернет
    //put сравнивает записанные данные с новыми, и если они не отличаются, перезапись не выполняется
    EEPROM.put(3467, from_device.router_pass);
    EEPROM.put(3430, from_device.router_ssid);
    EEPROM.put(150, mac_addr); //mac адрес master устройства
    EEPROM.commit();

    if (from_device.command == 8) { //запрос показаний
            for (int i = 0; i < 6; i++) {
        broadcast_address[i] = mac_addr[i];
      }
  read_sensors_flag = true;
    }
  }
}
