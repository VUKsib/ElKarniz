//Калибровка - НАЧАЛО
void calibrationMode() {
  //проверка закрытия штор
  if (currentPosition != 0 && currentStep != 0) {
    moveToPosition(0);
  }

  digitalWrite(enablePin, LOW); //включаем драйвер
  if (flagPosition == false) {
    //Обработка положения энкодера
    if (enc.left()) {
      encoderPos = 1;
      digitalWrite(dirPin, LOW);  // Устанавливаем направление против часовой стрелки
      rightEnc = false;
      calibrationSteps();
      Serial.println(encoderPos);
    }
    if (enc.right()) {
      encoderPos = 1;
      digitalWrite(dirPin, HIGH);  // Устанавливаем направление по часовой стрелке
      rightEnc = true;
      calibrationSteps();
      Serial.println(encoderPos);
    }
    if (enc.rightH()) {
      encoderPos = stepsRotate360;  // сделать полный оборот вправо на закрытие
      digitalWrite(dirPin, HIGH);   // Устанавливаем направление по часовой стрелке
      rightEnc = true;
      calibrationSteps();
      Serial.println(encoderPos);
    }

    if (enc.click()) {
      if (setStartPositionStep == true) {
        setFinalPositionStep = true;
        setStartPositionStep = false;
        finalPositionStep = 0;
        Serial.println(startPositionStep);
      } else if (setFinalPositionStep == true) {
        setFinalPositionStep = false;
        Serial.println(finalPositionStep);
      }
    }

    //выход из режима калибровки по нажатию кнопки
    if (btn1.click() || (setFinalPositionStep == false && setStartPositionStep == false)) {
      currentMode = BY_HAND;
      digitalWrite(enablePin, HIGH); //выключаем драйвер
      leds[0].setRGB(0, 234, 255);  // голубой
      FastLED.show();               //отобразить изменения светодиода
      Serial.println("Режим: Вручную");
      ledTmr = millis();
    }
  }
}

void calibrationSteps() {
  flagPosition = true;
  int countStep = 0;
  if (setFinalPositionStep == true) {
    //установка конечного положения
    for (int i = 0; i < encoderPos; i++) {
      digitalWrite(stepPin, HIGH);
      delay(50);
      digitalWrite(stepPin, LOW);
      if (rightEnc == true) {
        finalPositionStep++;
      } else {
        finalPositionStep--;
      }
    }
  } else if (setStartPositionStep == true) {
    //установка начального положения
    for (int i = 0; i < encoderPos; i++) {
      digitalWrite(stepPin, HIGH);
      delay(50);
      digitalWrite(stepPin, LOW);
      startPositionStep = 0;
    }
  }
  flagPosition = false;  // мотор не движется
  EEPROM.put(5, startPositionStep);
  EEPROM.put(6, finalPositionStep);
  Serial.println(finalPositionStep);
}
//Калибровка - КОНЕЦ