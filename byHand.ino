//Управление в ручном режиме от энкодера

//Основная функция режима (обработка энкодера) - НАЧАЛО
void byHandMode() {
  unsigned long currentMillis = millis();  // получить текущее время

  //Если двигатель не движется, опрашивать энкодер
  if (flagPosition == false) {
    flagStop = false;
    if (enc.right()) {
      enc.fast() ? specPosition += 5 : specPosition += 1;  //быстрый поворот ? 5% : нет 1%
      if (specPosition > 100) specPosition = 100;
      encTmr = currentMillis;  // установить текущее время как время последнего изменения
      Serial.println(specPosition);
    }
    if (enc.left()) {
      enc.fast() ? specPosition -= 5 : specPosition -= 1;
      if (specPosition <= 0) specPosition = 0;
      encTmr = currentMillis;  // установить текущее время как время последнего изменения
      Serial.println(specPosition);
    }

    //Проверка времени с последнего изменения
    if (currentMillis - encTmr >= 1500) {  // 1500 миллисекунд
      obrabotka();
    }

    //быстрое открытие шторы из любой позиции
    if (enc.click() && specPosition != 0) {
      specPosition = 0;
      Serial.println(specPosition);
      moveToPosition(0);
    }
    //быстрое закрытие шторы из любой позиции
    if (enc.releaseHold() && specPosition != 100) {
      specPosition = 100;
      Serial.println(specPosition);
      moveToPosition(100);
    }
  }

  // Остановка движения мотора
  if (flagPosition == true && enc.click()) {
    flagStop = true;  // остановить движение
    stopTmr = currentMillis;
    if (flagStop && currentMillis - stopTmr >= 500) {
      flagPosition = false;  // мотор не движется
    }
  }
}
//Основная функция режима (обработка энкодера) - КОНЕЦ

//Функция обработки положений энкодера и активация функции вычисления позиции - НАЧАЛО
void obrabotka() {
  if (currentPosition != specPosition) {
    flagPosition = true;  // Начато движение мотора
    EEPROM.put(0, specPosition); // запись позиции
    moveToPosition(specPosition);
  }
}
//Функция обработки - КОНЕЦ

//Функция для достижения заданной позиции - НАЧАЛО
void moveToPosition(int targetPosition) {
  flagPosition = true;
  digitalWrite(enablePin, HIGH); //включаем драйвер
  float stepSize = (float)finalPositionStep / 100.0;  // размер шага
  int majorStep = (int)stepSize;                      // основной шаг
  float remainder = stepSize - majorStep;             // оставшаяся часть шага
  int minorStep = majorStep;                          // дополнительный шаг
  if (remainder >= 0.5) {
    minorStep = majorStep + 1;
  }
  float walkSteps = abs((float)(targetPosition - currentPosition) * finalPositionStep / 100);
  int stepsToMove = 0;
  for (int i = 0; i < walkSteps; i++) {
    if (i % 2 == 0) {
      stepsToMove += minorStep;
    } else {
      stepsToMove += majorStep;
    }
  }

  bool direction = targetPosition > currentPosition;
  if (direction) {
    //ЗАКРЫТИЕ
    digitalWrite(dirPin, HIGH);  // Устанавливаем направление по часовой стрелке
  } else {
    //ОТКРЫТИЕ
    digitalWrite(dirPin, LOW);  // Устанавливаем направление против часовой стрелки
  }

  for (int i = 0; i < stepsToMove; i++) {
    if (flagStop == true) {
      FastLED.clear();  // выключаем индикатор
      FastLED.show();
      digitalWrite(stepPin, LOW);
      Serial.print("СТОП! текущий процент: ");
      Serial.print(currentPosition);
      Serial.print(", текущий шаг:");
      Serial.println(currentStep);
      return;
    } else {
      // Сделать шаг в направлении
      if (direction) {
        //Если закрытие, то прибавить шаги
        currentStep++;
        if (currentStep > finalPositionStep) currentStep = finalPositionStep;
      } else {
        //Если открытие, то отнять шаги
        currentStep--;
        if (currentStep < startPositionStep) currentStep = startPositionStep;
      }
      // обновить текущий процент
      currentPosition = round((float)currentStep * 100.0 / finalPositionStep);

      // обновить индикацию светодиода
      if (direction) {
        leds[0].setRGB(0, 0, 255);  // синий цвет - закрытие
        FastLED.show();
        digitalWrite(stepPin, HIGH);
        delay(50);
        digitalWrite(stepPin, LOW);

      } else {
        leds[0].setRGB(255, 86, 0);  // оранжевый цвет - открытие
        FastLED.show();
        digitalWrite(stepPin, HIGH);
        delay(50);
        digitalWrite(stepPin, LOW);
      }

      Serial.print("текущий процент: ");
      Serial.print(currentPosition);
      Serial.print(", текущий шаг:");
      Serial.println(currentStep);

      // Остановить мотор при достижении цели
      if (currentPosition == targetPosition) {
        if (direction) {
          currentStep++;
          if (currentStep > finalPositionStep) currentStep = finalPositionStep;
          leds[0].setRGB(0, 0, 255);  // синий цвет - закрытие
          FastLED.show();
          digitalWrite(stepPin, HIGH);
          delay(50);
          digitalWrite(stepPin, LOW);
        } else {
          currentStep--;
          if (currentStep < startPositionStep) currentStep = startPositionStep;
          leds[0].setRGB(255, 86, 0);  // оранжевый цвет - открытие
          FastLED.show();
          digitalWrite(stepPin, HIGH);
          delay(50);
          digitalWrite(stepPin, LOW);
        }
        currentPosition = round((float)currentStep * 100.0 / finalPositionStep);

        Serial.print("Процент: ");
        Serial.print(currentPosition);
        Serial.print(", шаги:");
        Serial.println(currentStep);
        digitalWrite(enablePin, LOW); //выключаем драйвер
        flagPosition = false;  // мотор не движется
        EEPROM.put(1, currentPosition); // запись значения текущей позиции
        FastLED.clear();       //выключается светодиод
        FastLED.show();
        break;
      }
    }
  }
}
//Функция для достижения заданной позиции - НАЧАЛО