//Управление по датчикам освещенности

//Фоторезисторы - НАЧАЛО
void photoresistors() {

  // Проверка нажатия кнопки энкодера, для запуска проверки подключения
  if (enc.hasClicks(2)) {
    checkPhotoresistors = true;
    pin36Connected = false;
    pin39Connected = false;
    startTime = millis();
  }

  // Проверка подключения фоторезисторов
  if (checkPhotoresistors == true) {
    leds[0].setRGB(255, 150, 0);  // желтый
    FastLED.show();
    checkPhotoConnection();
  }

  // Обновление показаний фоторезисторов каждые 30 секунд
  if (checkPhotoresistors == false && (currentMs - timerPhotoresistor >= interval)) {
    timerPhotoresistor = currentMs;

    float resistance36 = 0;
    float resistance39 = 0;
    float lux36 = 0.0;
    float lux39 = 0.0;
    float lux = 50.0;

    if (pin36Connected == true) {
      int aPin36 = analogRead(36);                             // Показание с датчика
      resistance36 = (aPin36 * defaultR) / (1024.0 - aPin36);  // Определение сопротивления
      lux36 = pow(10, (log10(R1 / resistance36) / gamma));     // Рассчет освещенности

      Serial.print("Люксы с 36 пина: ");
      Serial.println(lux36);
      Serial.print("Значение АЦП 36 пин: ");
      Serial.println(aPin36);
    }

    if (pin39Connected == true) {
      int aPin39 = analogRead(39);                             // Показание с датчика
      resistance39 = (aPin39 * defaultR) / (1024.0 - aPin39);  // Определение сопротивления
      lux39 = pow(10, (log10(R1 / resistance39) / gamma));     // Рассчет освещенности

      Serial.print("Люксы с 39 пина: ");
      Serial.println(lux39);
      Serial.print("Значение АЦП 39 пин: ");
      Serial.println(aPin39);
    }

    // Если оба фоторезистора подключены, считаем среднее значение люкс
    if (pin36Connected == true && pin39Connected == true) {
      lux = (lux36 + lux39) / 2;
      flagLux = true;
    } else if (pin36Connected == true && pin39Connected == false) {
      lux = lux36;
      flagLux = true;
    } else if (pin39Connected == true && pin36Connected == false) {
      lux = lux39;
      flagLux = true;
    }

    Serial.print("Среднее значение люкс: ");
    Serial.println(lux);

    // Управление шаговым двигателем в зависимости от уровня освещенности
    if (flagLux == true && currentStep != finalPositionStep && (lux < 30 || lux > 475)) {
      // Двигаться к позиции finalPositionStep, горит синий светодиод
      moveToPosition(100);
      flagLux = false;
    } else if (flagLux == true && currentStep != startPositionStep && (lux > 75 && lux < 425)) {
      // Двигаться в положение startPositionStep, горит оранжевый светодиод
      moveToPosition(0);
      flagLux = false;
    }
  }
}
//Фоторезисторы - КОНЕЦ

//Проверка подключения фоторезисторов - НАЧАЛО
void checkPhotoConnection() {
  Serial.println("Проверка подключения фоторезисторов");
  startTime = millis();
  // Начинаем проверку подключения фоторезисторов
  if (pin36Connected == false || pin39Connected == false) {
    while (millis() - startTime < 20000) {
      if (analogRead(36) != 1023) {
        pin36Connected = true;
        EEPROM.put(2, pin36Connected);
      }
      if (analogRead(39) != 1023) {
        pin39Connected = true;
        EEPROM.put(3, pin39Connected);
      }
    }
    if (startTime = 20000) {
      // Если прошло 20 секунд и фоторезисторы не подключены, вывести сообщение
      if (pin36Connected == false) {
        Serial.println("Фоторезистор на пине 36 не подключен!");
      }
      if (pin39Connected == false) {
        Serial.println("Фоторезистор на пине 39 не подключен!");
      }
      Serial.println("Проверка окончена");
      checkPhotoresistors = false;  //отключение проверки подключения
      EEPROM.put(4, checkPhotoresistors); // запись в память, что проверка произведена
      FastLED.clear();              //выключается светодиод
      FastLED.show();
    }
  }
}
//Проверка подключения фоторезисторов - КОНЕЦ