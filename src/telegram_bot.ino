void newMsg(FB_msg& msg) {
  /////////////////////////////////////////////////////////////////////////ДОБАВЛЕНИЕ ПЕРВОГО ПОЛЬЗОВАТЕЛЯ/////////////////////////////////////////////////////////////////////////
  /* tg_mode:
    0 - пропуск
    1 - если добавляем первого пользователя
    2 - если добавляем нового пользователя
    3 - если удаляем пользователя
    4 - если добавляем комнату
    5 - выбор комнаты (если добавляем устройство)
    6 - выбор комнаты (если удаляем устройство)
    7 - выбор комнаты (если переименовываем устройство)
    8 - выбор комнаты (если настраиваем выключатель)
    9 - если настраиваем выключатель
    10 - если настраиваем устройство
    11 - если переименовываем устройство
    12 - если удаляем устройство
    13 - если сбрасываем настройки
  */
  if  (strcmp(msg.text.c_str(), "Назад") == 0) {
    tg_mode = 0;
    bot.showMenuText(F("\xF0\x9F\x8C\xBF"), main_menu(), msg.chatID);
    goto bailout;
  }
  switch (tg_mode) {
    case 0: break;

    /////////////////////////////////////////////////////////////////////////ДОБАВЛЕНИЕ ПЕРВОГО ПОЛЬЗОВАТЕЛЯ/////////////////////////////////////////////////////////////////////////
    case 1: {
        uint32_t userid[10]; //объявляем массив с ID добавленных пользователей
        EEPROM.get(3580, userid); //считываем из EEPROM
        userid[0] = atoll(msg.chatID.c_str()); //добавляем первого пользователя в массив
        //записываем в EEPROM
        EEPROM.put(3580, userid);
        EEPROM.commit();
        bot.chatIDs = "0,";
        for (int i = 0; i < 10; i++) {
          if (userid[i] != 0) {
            bot.chatIDs += userid[i];
            bot.chatIDs += ",";
          }
        }
        bot.showMenuText(F("Добро пожаловать в систему управления умным домом!"), main_menu(), msg.chatID);
        tg_mode = 0;
        goto bailout; //для выхода из ветки if else
      }
      break;

    /////////////////////////////////////////////////////////////////////////ДОБАВЛЕНИЕ НОВОГО ПОЛЬЗОВАТЕЛЯ/////////////////////////////////////////////////////////////////////////
    case 2: {
        uint32_t x = 0;
        x = atoll(msg.text.c_str());
        uint32_t userid[10];
        EEPROM.get(3580, userid);
        for (int i = 1; i < 10; i++) { //отсчет с 1, т.к. 0 зарезерверирован под первого пользователя (владельца)
          if (userid[i] == 0) {
            userid[i] = x;
            EEPROM.put(3580, userid);
            EEPROM.commit();
            bot.chatIDs = "0,";
            for (int i = 0; i < 10; i++) {
              if (userid[i] != 0) {
                bot.chatIDs += userid[i];
                bot.chatIDs += ",";
              }
            }
            bot.showMenuText(F("Пользователь успешно добавлен!"), main_menu(), msg.chatID);
            String newUser = "";
            newUser += x;
            // bot.sendMessage(F("Теперь вы можете управлять системой умного дома.%0A%0AНапишите любое сообщение и мы начнём..."), newUser);
            bot.sendMessage(F("Прошивка обновлена до версии 2.0%0A%0AИз обновлений:%0A%0A1.Более стабильная работа (надеюсь..)%0A%0A2.Изменена структура меню. Вместо двух групп"
                              " 'Освещение' и 'Розетки' присутствуют комнаты, находящиеся в доме.%0A%0A3.Добавлена возможность управления новым типом устройства - RGB контроллером."
                              "%0A%0A4.Всем похуй, но соединение между ботом и сервером стало более надежным: теперь прошивки обновляются с сервера, защищённого SSL-сертификатом."), newUser);

            bot.sendMessage(F("Большая просьба сообщать Глебке о всех найденных ошибках, нестабильности в работе и прочей хуйне.%0A%0AПо возможности максимально подробно и прикрепляя скрины.%0A%0AМерси!"), newUser);
            tg_mode = 0;
            goto bailout;
          }
          else if (i == 9) {
            bot.sendMessage(F("Все места заняты."), msg.chatID);
            tg_mode = 0;
            goto bailout;
          }
        }
      }
      break;

    /////////////////////////////////////////////////////////////////////////УДАЛЕНИЕ ПОЛЬЗОВАТЕЛЯ/////////////////////////////////////////////////////////////////////////
    case 3: {
        uint32_t userid[10];
        uint32_t x = 0;
        x = atoll(msg.text.c_str());
        EEPROM.get(3580, userid);
        for (int i = 1; i < 10; i++) { //отсчет с 1, т.к. 0 зарезерверирован под первого пользователя (владельца)
          if ((x == userid[i]) && (x != 0)) {
            userid[i] = 0;
            EEPROM.put(3580, userid);
            EEPROM.commit();
            bot.chatIDs = "0,";
            for (int i = 0; i < 10; i++) {
              if (userid[i] != 0) {
                bot.chatIDs += userid[i];
                bot.chatIDs += ",";
              }
            }
            bot.showMenuText("Пользователь успешно удалён.", main_menu(), msg.chatID);
            tg_mode = 0;
            goto bailout;
          }
          else if (i == 9) {
            bot.sendMessage("Пользователь не найден. Проверьте ID.", msg.chatID);
            goto bailout;
          }
        }
      }
      break;

    /////////////////////////////////////////////////////////////////////////ДОБАВЛЕНИЕ КОМНАТЫ/////////////////////////////////////////////////////////////////////////
    case 4: {
        if (strlen(msg.text.c_str()) < 42)
        {
          char room_name[10][41]; //название комнаты
          EEPROM.get(2454, room_name);
          for (int i = 0; i < 10; i++) {
            if (!strlen(room_name[i])) { //если поле пустое, strlen возвращает 0. нам как раз и нужно это условие. инвертируем через !
              strcpy(room_name[i], msg.text.c_str());
              EEPROM.put(2454, room_name);
              EEPROM.commit();
              bot.showMenuText("Комната успешно добавлена. Теперь вы можете поместить в неё устройство.", main_menu(), msg.chatID);
              tg_mode = 0;
              goto bailout;
            }
            else if (i == 9) {
              bot.showMenuText("Всё место занято. Если хотите добавить ещё, воспользуйтесь кнопкой "
                               "'У меня проблема' в главном меню и свяжитесь с разработчиком.", main_menu(), msg.chatID);
              tg_mode = 0;
              goto bailout;
            }
          }
        }
        else {
          bot.sendMessage("Слишком длинное название. Введите менее 20 символов.", msg.chatID);
          goto bailout;
        }
      }
      break;

    /////////////////////////////////////////////////////////////////////////ВЫБОР КОМНАТЫ/////////////////////////////////////////////////////////////////////////
    case 5: //выбор комнаты (если добавляем устройство)
    case 6: //выбор комнаты (если удаляем устройство)
    case 7: //выбор комнаты (если переименовываем устройство)
    case 8: { //выбор комнаты (если настраиваем выключатель)

        if (strcmp(msg.text.c_str(), "Создать новую комнату") == 0)  {
          bot.closeMenuText(F("Придумайте название для комнаты:"), msg.chatID);
          tg_mode = 4; //если добавляем комнату
          goto bailout;
        }
        else {

          char room_name[10][41];
          EEPROM.get(2454, room_name);
          for (byte i = 0; i < 10; i++) {
            if (strcmp(msg.text.c_str(), room_name[i]) == 0) {
              room_id = i; //оставляем в глобальной переменной номер комнаты, в которую добавим устройство.
              if (tg_mode == 5) { //выбор комнаты (если добавляем устройство)
                bot.sendMessage(F("Зажмите и удерживайте кнопку на устройстве, которое хотите настроить."), msg.chatID);
                goto bailout;
              }
              else {
                byte room_device[10][20];
                EEPROM.get(2866, room_device);
                bot.showMenuText(F("Выберите устройство:"), device_menu(room_device[room_id]), msg.chatID);
                switch (tg_mode) {
                  case 6: { //выбор комнаты (если удаляем устройство)
                      tg_mode = 12;
                      goto bailout;
                    }
                    break;
                  case 7: { //выбор комнаты (если переименовываем устройство)
                      tg_mode = 11;
                      goto bailout;
                    }
                    break;
                  case 8: { //выбор комнаты (если настраиваем выключатель)
                      tg_mode = 9;
                      goto bailout;
                    }
                    break;
                  default: break;
                }
              }
            }
            else if (i == 9) {
              bot.showMenuText(F("Комната не найдена. Попробуйте сначала создать её."), main_menu(), msg.chatID);
              tg_mode = 0;
              goto bailout;
            }
          }
        }
      }
      break;

    /////////////////////////////////////////////////////////////////////////НАСТРОЙКА ВЫКЛЮЧАТЕЛЯ/////////////////////////////////////////////////////////////////////////
    case 9: {
        char device_name[20][41]; //название устройства освещения
        uint8_t device_mac_addr[20][6]; //mac-адрес устройства освещения
        EEPROM.get(100, device_name);
        EEPROM.get(2152, device_mac_addr);
        for (int i = 0; i < 20; i++) {
          if (strcmp(msg.text.c_str(), device_name[i]) == 0) {
            if (i == 0) { //если нужно данное устройство (резерв под данное устройство в масиве на 0)
              for (int a = 0; a < 6; a++) {
                WiFi.macAddress(broadcast_address); //получаем mac-адресс этого устройства
              }
            }
            else {
              for (int a = 0; a < 6; a++) {
                broadcast_address[a] = device_mac_addr[i][a];
              }
            }
            bot.sendMessage(F("Зажмите кнопку сзади выключателя. Продолжая удерживать, нажмите на сенсорную кнопку, которую хотите настроить."), msg.chatID);
            goto bailout;
          }
          else if (i == 19) {
            bot.showMenuText(F("Устройство не найдено. Проверьте название."), main_menu(), msg.chatID);
            tg_mode = 0;
            goto bailout;
          }
        }
      }
      break;

    /////////////////////////////////////////////////////////////////////////НАСТРОЙКА УСТРОЙСТВА/////////////////////////////////////////////////////////////////////////
    case 10: {
        if (strlen(msg.text.c_str()) < 42)
        {
          byte device_type[10]; //флаг включения/отключения пункта меню в клавиатуре розеток
          char device_name[20][41]; //название розетки
          uint8_t device_mac_addr[20][6]; //mac-адрес розетки
          uint8_t room_device[10][20];
          EEPROM.get(2152, device_mac_addr);
          EEPROM.get(100, device_name);
          EEPROM.get(87, device_type);
          EEPROM.get(2866, room_device);
          for (int i = 1; i < 10; i++) { //отсчёт с 1 т.к. нулевой в резерве под данное устройство

            if (device_type[i] == 0) {
              strcpy(device_name[i], msg.text.c_str());
              room_device[room_id][i] = 1;
              device_type[i] = device_type_global; //из глобальной переменной получаем тип устройства

              for (int a = 0; a < 6; a++) {
                device_mac_addr[i][a] = broadcast_address[a];
              }
              room_device[room_id][i] = 1;
              EEPROM.put(2152, device_mac_addr);
              EEPROM.put(100, device_name);
              EEPROM.put(87, device_type);
              EEPROM.put(2866, room_device);
              EEPROM.commit();
              bot.showMenuText(F("Устройство успешно добавлено."), main_menu(), msg.chatID);
              tg_mode = 0;
              goto bailout;
            }
            else if (i == 9) {
              bot.showMenuText(F("Всё место занято. Если хотите добавить ещё, воспользуйтесь кнопкой"
                                 "'У меня проблема' в главном меню и свяжитесь с разработчиком."), main_menu(), msg.chatID);
              tg_mode = 0;
              goto bailout;
            }
          }
        }
        else {
          bot.sendMessage(F("Слишком длинное название. Введите менее 20 символов."), msg.chatID);
          goto bailout;
        }
      }
      break;

    /////////////////////////////////////////////////////////////////////////ПЕРЕИМЕНОВАНИЕ УСТРОЙСТВА/////////////////////////////////////////////////////////////////////////
    case 11: {
        static bool set_name_flag = false;
        static uint8_t index = 0;
        char device_name[20][41]; //название устройства
        EEPROM.get(100, device_name);

        if (set_name_flag) {
          if (strlen(msg.text.c_str()) < 42)
          {
            strcpy(device_name[index], msg.text.c_str());
            EEPROM.put(100, device_name);
            EEPROM.commit();
            bot.showMenuText(F("Устройство успешно переименовано"), main_menu(), msg.chatID);
            set_name_flag = false;
            tg_mode = 0;
            goto bailout;
          }
          else {
            bot.sendMessage(F("Слишком длинное название. Введите менее 20 символов."), msg.chatID);
            goto bailout;
          }
        }
        for (int i = 0; i < 20; i++) {
          if (strcmp(msg.text.c_str(), device_name[i]) == 0) {
            bot.closeMenuText(F("Напишите новое название:"), msg.chatID);
            index = i;
            set_name_flag = true;
            goto bailout;
          }
          else if (i == 19) {
            bot.sendMessage(F("Устройство не найдено"), msg.chatID);
            goto bailout;
          }
        }
      }
      break;

    /////////////////////////////////////////////////////////////////////////УДАЛЕНИЕ УСТРОЙСТВА/////////////////////////////////////////////////////////////////////////
    case 12: {

        char device_name[20][41]; //название устройства
        EEPROM.get(100, device_name);

        for (int i = 1; i < 20; i++) { //отсчёт с 1 т.к. нулевой в резерве под данное устройство
          if (strcmp(msg.text.c_str(), device_name[i]) == 0) {

            byte device_type[10]; //флаг включения/отключения пункта меню в клавиатуре
            uint8_t device_mac_addr[20][6]; //mac-адрес устройства
            uint8_t room_device[10][20];
            EEPROM.get(87, device_type);
            EEPROM.get(2152, device_mac_addr);
            EEPROM.get(2866, room_device);

            strcpy(device_name[i], "");
            room_device[room_id][i] = 0;
            device_type[i] = 0;
            for (int a = 0; a < 6; a++) {
              device_mac_addr[i][a] = 0;
            }
            EEPROM.put(2152, device_mac_addr);
            EEPROM.put(100, device_name);
            EEPROM.put(87, device_type);
            EEPROM.put(2866, room_device);
            EEPROM.commit();
            bot.showMenuText("Устройство успешно удалено.", main_menu(), msg.chatID);
            tg_mode = 0;
            goto bailout;
          }
          else if (i == 19) {
            bot.sendMessage("Невозможно удалить данное устройство", msg.chatID);
            tg_mode = 0;
            goto bailout;
          }
        }
      }
      break;

    /////////////////////////////////////////////////////////////////////////СБРОС НАСТРОЕК/////////////////////////////////////////////////////////////////////////
    case 13: {
        if (strcmp(msg.text.c_str(), "Да") == 0) {
          EEPROM.write(1, 0); // key for writing start data
          EEPROM.commit();
          bot.showMenuText("Все настройки сброшены. Перезагрузка...", main_menu(), msg.chatID);
          delay(100);
          ESP.restart();
        }
      }
      break;
    default: break;
  }

  /////////////////////////////////////////////////////////////////////////УПРАВЛЕНИЕ ЯРКОСТЬЮ И ЦВЕТОМ/////////////////////////////////////////////////////////////////////////
  if (strcmp(msg.text.c_str(), "Выключить") == 0) {
    // send_packet(mac, command)
    send_packet(broadcast_address, 5);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Режим огня") == 0) {
    // send_packet(mac, command)
    send_packet(broadcast_address, 8);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "10%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 10);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "20%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 20);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "30%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 30);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "40%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 40);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "50%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 50);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "60%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 60);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "70%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 70);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "80%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 80);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "90%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 90);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "100%") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 6, 100);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Зелёный") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 7, 0, 255, 0);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Синий") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 7, 0, 0, 255);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Красный") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 7, 255, 0, 0);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Жёлтый") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 7, 255, 255, 0);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Оранжевый") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 7, 255, 140, 0);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Голубой") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 7, 0, 191, 255);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Фиолетовый") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 7, 128, 0, 128);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Белый") == 0) {
    // send_packet(mac, command, bright)
    send_packet(broadcast_address, 7, 255, 255, 255);
    answer_flag = true; //поднимаем флаг, чтобы получить ответ
    answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
    goto bailout;
  }


  /////////////////////////////////////////////////////////////////////////ПРИ ВЫБОРЕ КОМНАТЫ/////////////////////////////////////////////////////////////////////////

  {
    char room_name[10][41]; //название комнаты
    EEPROM.get(2454, room_name);
    for (byte i = 0; i < 10; i++) {
      if (strcmp(msg.text.c_str(), room_name[i]) == 0) {
        byte room_device[10][20];
        EEPROM.get(2866, room_device);
        bot.showMenuText(room_name[i], device_menu(room_device[i]), msg.chatID);
        goto bailout;
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////УПРАВЛЕНИЕ УСТРОЙСТВАМИ/////////////////////////////////////////////////////////////////////////
  {
    char device_name[20][41];
    EEPROM.get(100, device_name);
    for (byte i = 0; i < 20; i++) {
      if (strcmp(msg.text.c_str(), device_name[i]) == 0) {
        if (i == 0) { //если управляем розеткой на данном устройстве
          set_relay();
          answer_ID = msg.chatID;
          answer_command = digitalRead(12); //читаем значение на выводе и отправляем код ответа если 0 - выключено, если 1 - включено
          esp_now_flag = true; //поднимаем флаг чтобы функция ответа была доступна
          goto bailout;
        }
        else {
          byte device_type[10];
          uint8_t device_mac_addr[20][6]; //mac-адрес устройства
          EEPROM.get(87, device_type);
          EEPROM.get(2152, device_mac_addr); //получаем в переменную адрес всех розеток
          if (device_type[i] == 1) {
            // send_packet(mac получателя, команда)
            send_packet(device_mac_addr[i], 0);
            answer_flag = true; //поднимаем флаг, чтобы получить ответ
            answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
            goto bailout;
          }

          else if (device_type[i] == 2) {
            bot.showMenuText(F("\xF0\x9F\x8E\xA8"), F("Зелёный \t Синий \n"
                             "Красный \t Жёлтый \n"
                             "Оранжевый \t Голубой \n"
                             "Фиолетовый \t Белый \n"
                             "Выключить \n"
                             "Режим огня \n"
                             "10% \t 20% \t 30% \n"
                             "40% \t 50% \t 60% \n"
                             "70% \t 80% \t 90% \n"
                             "100% \n"
                             "Назад \n"), msg.chatID);
            for (int a = 0; a < 6; a++) {
              broadcast_address[a] = device_mac_addr[i][a];
            }
            answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
            goto bailout;
          }
          else if (device_type[i] == 3) {

            bot.showMenuText(F("\xF0\x9F\x8E\xA8"), F("10% \t 20% \t 30% \n"
                             "40% \t 50% \t 60% \n"
                             "70% \t 80% \t 90% \n"
                             "100% \n"
                             "Назад \n"), msg.chatID);
            uint8_t device_mac_addr[20][6]; //mac-адрес устройства
            EEPROM.get(2152, device_mac_addr); //получаем в переменную адрес всех розеток
            for (int a = 0; a < 6; a++) {
              broadcast_address[a] = device_mac_addr[i][a];
            }
            answer_ID = msg.chatID; //записываем в переменную ID пользователя для ответа ему
            goto bailout;
          }
        }
      }
    }
  }
  /////////////////////////////////////////////////////////////////////////ОСНОВНОЕ МЕНЮ/////////////////////////////////////////////////////////////////////////

  if (strcmp(msg.text.c_str(), "Настройки \xE2\x9A\x99") == 0) {
    bot.showMenuText(F("\xE2\x9A\x99"), F("Настроить новое устройство \n"
                                          "Отвязать устройство \n"
                                          "Переименовать устройство \n"
                                          "Настроить выключатель \n"
                                          "Добавить пользователя \n"
                                          "Удалить пользователя \n"
                                          "Сбросить все настройки \n"
                                          "Назад"), msg.chatID);
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "У меня проблема \xF0\x9F\x91\x80") == 0) {
    bot.sendMessage(F("Бот-помощник @grib_tech_bot"), msg.chatID);
    goto bailout;
  }

  /////////////////////////////////////////////////////////////////////////НАСТРОЙКИ/////////////////////////////////////////////////////////////////////////
  else if (strcmp(msg.text.c_str(), "Настроить новое устройство") == 0)  {
    bot.showMenuText(F("В какую комнату поместить устройство?"), rooms() + F("Создать новую комнату \n Назад \n"), msg.chatID);
    tg_mode = 5;
    answer_ID = msg.chatID;
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Отвязать устройство") == 0) {
    bot.showMenuText(F("В какой комнате находится устройство, которое хотите удалить?"), rooms(), msg.chatID);
    tg_mode = 6;
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Переименовать устройство") == 0)  {
    bot.showMenuText(F("В какой комнате находится устройство, которое хотите переименовать?"), rooms(), msg.chatID);
    tg_mode = 7;
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Настроить выключатель") == 0)  {
    bot.showMenuText(F("В какой комнате находится устройство, которое хотите связать с выключателем?"),  rooms(), msg.chatID);
    tg_mode = 8;
    answer_ID = msg.chatID;
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Добавить пользователя") == 0) {
    bot.sendMessage(F("Введите ID пользователя:"), msg.chatID);
    tg_mode = 2;
    goto bailout;
  }
  else if (strcmp(msg.text.c_str(), "Удалить пользователя") == 0) {
    String message;
    message += F("Введите ID пользователя, которого хотите удалить. %0A%0AДобавленные ID:%0A");
    uint32_t userid[10];
    EEPROM.get(3580, userid);
    bool val = false;
    for (int i = 1; i < 10; i++) {
      if (userid[i] != 0) {
        message += userid [i];
        message += "%0A";
        val = true;
      }
      else if (i == 9) {
        val = false;
      }
    }
    if (!val) { //если ни одного пользователя
      bot.sendMessage(F("Никто не имеет прав доступа к этому боту."), msg.chatID);
      goto bailout;
    }
    else {
      bot.sendMessage(message, msg.chatID);
      tg_mode = 3;
      goto bailout;
    }
  }
  else if (strcmp(msg.text.c_str(), "Сбросить все настройки") == 0) {
    bot.showMenuText(F("ВНИМАНИЕ! Все настройки будут сброшены, информация о токене бота, логине и пароле Wi-Fi"
                       "и добавленных устройствах будет удалена.%0AСбросить все настройки?"), F("Да \n Назад"), msg.chatID);
    tg_mode = 13; //если сбрасываем настройки
    goto bailout;
  }
  else
  {
    bot.showMenuText("\xF0\x9F\x8C\xBF", main_menu(), msg.chatID);
  }
bailout:
  {
  }
}
