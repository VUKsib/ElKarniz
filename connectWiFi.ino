//Функция подключения к Wi-Fi - НАЧАЛО
void connectWiFi() {
  WiFi.begin(ssid, password);  // Инициализация в сети
  while (--attempt && WiFi.status() != WL_CONNECTED) {
    if (millis() > 500) {
      Serial.println(".");
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Подключение к Wi-Fi отсутствует..");
  } else {
    Serial.println("");
    Serial.println("Wi-Fi подключен");
    Serial.println("IP-адрес: ");
    Serial.println(WiFi.localIP());
  }
}
//Функция подключения к Wi-Fi - КОНЕЦ