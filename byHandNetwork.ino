//Режим управления через веб-сервер
void byHandNetworkMode() {
  connectWiFi();
  // Настройка основного обработчика веб-страницы
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Создание HTML-кода для веб-страницы
    String html = "<!DOCTYPE html><body>";
    html += "<h1>Электрокарниз</h1>";
    html += "<p>Время: " + timeClient.getFormattedTime() + "</p>";
    html += "<p>Дата: " + getFormattedDate() + "</p>";
    html += "<p>Текущая позиция электрокарниза: <span id='currentPosition'>0</span>%</p>";
    html += "<input type='range' min='0' max='100' value='0' id='slider' oninput='updateSlider(this.value)'><span id='sliderValue'>0</span>%";
    html += "<br><button onclick='setManualMode()'>Ручное управление</button><button onclick='setAutoMode()'>Автоматическое управление</button>";

    // Добавление отображения температуры и влажности, если BME280 успешно инициализирован
    if (bmeStatus) {
      float temperature = bme.readTemperature();
      float humidity = bme.readHumidity();
      html += "<h2>Показания датчика BME280</h2>";
      html += "<p>Температура: " + String(temperature) + " &#8451;</p>";
      html += "<p>Влажность: " + String(humidity) + " %</p>";
    } else {
      html += "<p>BME280 sensor not found</p>";
    }

    html += "<h2>Управление по времени</h2>";
    html += "<label>Открыть в: </label>";
    html += "<input type='number' id='openHour' min='0' max='23' value='" + String(openHour) + "'> : ";
    html += "<input type='number' id='openMinute' min='0' max='59' value='" + String(openMinute) + "'><br>";
    html += "<label>Закрыть в: </label>";
    html += "<input type='number' id='closeHour' min='0' max='23' value='" + String(closeHour) + "'> : ";
    html += "<input type='number' id='closeMinute' min='0' max='59' value='" + String(closeMinute) + "'><br>";
    html += "<button onclick='toggleTimeControl()'>" + String(timeControlEnabled ? "Неактивировано" : "Активировано") + " управление по времени</button>";

    html += "<script>";
    // Функция для обновления позиции шторы
    html += "function updateSlider(value) {";
    html += "document.getElementById('sliderValue').innerText = value;";
    html += "var xhr = new XMLHttpRequest();";
    html += "xhr.open('GET', '/setPosition?value=' + value, true);";
    html += "xhr.send();";
    html += "}";
    // Функция для переключения в ручной режим
    html += "function setManualMode() {";
    html += "var xhr = new XMLHttpRequest();";
    html += "xhr.open('GET', '/setMode?mode=manual', true);";
    html += "xhr.send();";
    html += "}";
    // Функция для переключения в автоматический режим
    html += "function setAutoMode() {";
    html += "var xhr = new XMLHttpRequest();";
    html += "xhr.open('GET', '/setMode?mode=automatic', true);";
    html += "xhr.send();";
    html += "}";
    // Функция для включения/выключения контроля времени
    html += "function toggleTimeControl() {";
    html += "var xhr = new XMLHttpRequest();";
    html += "xhr.open('GET', '/toggleTimeControl', true);";
    html += "xhr.send();";
    html += "}";
    // Функция для периодического обновления текущей позиции шторы
    html += "setInterval(function() {";
    html += "var xhr = new XMLHttpRequest();";
    html += "xhr.open('GET', '/getCurrentPosition', true);";
    html += "xhr.onreadystatechange = function() {";
    html += "if (xhr.readyState == 4 && xhr.status == 200) {";
    html += "document.getElementById('currentPosition').innerText = xhr.responseText;";
    html += "}";
    html += "};";
    html += "xhr.send();";
    html += "}, 1000);";
    html += "</script>";
    html += "</body></html>";
    // Отправка HTML-страницы клиенту
    request->send(200, "text/html", html);
  });
  // Запуск веб-сервера
  server.begin();
}

// Функция форматирования времени
String getFormattedDate() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&rawtime);
  char buffer[80];
  strftime(buffer, 80, "%d-%m-%Y", timeinfo);
  return String(buffer);
}
