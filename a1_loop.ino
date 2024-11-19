void loop() {
  enc.tick();
  // Вызов соответствующих функций в зависимости от текущего режима
  switch (currentMode) {
    case BY_HAND:
      if ((millis() - ledTmr >= 10000)) {
        FastLED.clear();  // отключить
        FastLED.show();
      }
      byHandMode();
      break;
    case AUTOMATIC:
      if ((millis() - ledTmr >= 10000)) {
        FastLED.clear();  // отключить
        FastLED.show();
      }
      currentMs = millis();  //Сохраняем текущее время
      photoresistors();
      break;
    case BY_HAND_NETWORK:
      if ((millis() - ledTmr >= 10000)) {
        FastLED.clear();  // отключить
        FastLED.show();
      }
      byHandNetworkMode();
      if (timeControlEnabled) {
        //принудительное получение времени
        while (!timeClient.update()) {
          timeClient.forceUpdate();
        }
        // Проверка текущего времени и выполнение действий по расписанию
        int currentHour = timeClient.getHours();
        int currentMinute = timeClient.getMinutes();

        if (currentHour == openHour && currentMinute == openMinute) {
          moveToPosition(0);  // Открыть
          Serial.println("Открытие по времени");
        }
        if (currentHour == closeHour && currentMinute == closeMinute) {
          moveToPosition(100);  // Закрыть
          Serial.println("Закрытие по времени");
        }
      }
      break;
    case CALIBRATION:
      if ((millis() - ledTmr >= 10000)) {
        FastLED.clear();  // отключить
        FastLED.show();
      }
      calibrationMode();
      break;
  }

  //Опрос кнопки - НАЧАЛО
  btn1.tick();
  // Нажатие кнопки
  // Переключение режимов
  if (btn1.click()) {
    switch (currentMode) {
      case BY_HAND:
        currentMode = AUTOMATIC;
        leds[0].setRGB(0, 255, 0);  // зеленый
        FastLED.show();
        Serial.println("Режим: Автоматический");
        ledTmr = millis();
        break;
      case AUTOMATIC:
        currentMode = BY_HAND_NETWORK;
        leds[0].setRGB(255, 0, 220);  // розовый
        FastLED.show();
        Serial.println("Режим: Вручную по сети");
        ledTmr = millis();
        connectWiFi();  //Установить соединение с wi-fi для следующего режима
        break;
      case BY_HAND_NETWORK:
        WiFi.disconnect();  //Разорвать соединение с wi-fi при переключении режима
        currentMode = BY_HAND;
        leds[0].setRGB(0, 234, 245);  // голубой
        FastLED.show();               //отобразить изменения светодиода
        Serial.println("Режим: Вручную");
        ledTmr = millis();
        break;
    }
  }

  // Удержание кнопки
  // Кнопка отпущена, считаем через сколько
  if (btn1.release()) {
    //от 7 до 15 секунд - Калибровка
    if (btn1.pressFor() > 7000 && btn1.pressFor() <= 15000) {
      currentMode = CALIBRATION;
      leds[0].setRGB(255, 230, 0);  // желтый
      FastLED.show();
      Serial.println("Режим: Калибровка");
      ledTmr = millis();
    }
  }
  //Опрос кнопки - КОНЕЦ
}