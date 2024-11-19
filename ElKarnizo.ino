//НАСТРОЙКИ БИБЛИОТЕКИ EncButton.h
#define EB_HOLD_TIME 800  //таймаут удержания кнопки
#define EB_NO_COUNTER     // отключить счётчик энкодера (экономит 4 байта памяти)

//Настройки EEPROM
#define INIT_ADDR 1023  // номер резервной ячейки
#define INIT_KEY 50     // ключ первого запуска

//БИБЛИОТЕКИ
#include <EncButton.h>          //библиотека энкодера и кнопки
#include <FastLED.h>            //библиотека rgb светодиода для индикации
#include <EEPROM.h>             //библиотека энергонезависимой памяти
#include <Wire.h>               //библиотека для работы по I2C
#include <Adafruit_Sensor.h>    //библиотека для работы с датчиками
#include <Adafruit_BME280.h>    //библиотека для работы с bme280
#include <WiFi.h>               //библиотека для подключения к интернету
#include <AsyncTCP.h>           //библиотека для асинхронной работы с протоколом TCP
#include <ESPAsyncWebServer.h>  //библиотека для создания веб-сервера на ESP32
#include <NTPClient.h>          //библиотека для получения реального времени
#include <WiFiUdp.h>            //библиотека для работы с протоколом UDP


//НАСТРОЙКИ КОНТАКТОВ (ПИН)
#define LED_PIN 13          //rgb светодиод
#define LED_KOL 1           //количество светодиодов
Button btn1(4);             //подключение кнопки
EncButton enc(16, 17, 18);  //подключение энкодера
const int stepPin = 26;     // Пин для шагового двигателя
const int dirPin = 33;      // Пин для направления движения
const int enablePin = 27;   // Пин для включения/выключения драйвера


//ОБЪЕКТЫ И ПЕРЕМЕННЫЕ
CRGB leds[LED_KOL];  //массив цветов

uint32_t tmr, tmr2, ledTmr, stopTmr, encTmr, startTime;  // таймеры
uint32_t currentMs = millis();                           // cохраняем текущее время
uint32_t timerPhotoresistor = 0;                         // предыдущее время обновления показаний

int specPosition = 0;     // заданная позиция
int currentPosition = 0;  // текущая позиция
int difPos = 0;           // разница между текущей и заданной позицией

const int defaultR = 10000;  // коэф резистора делителя-напряжения
#define gamma 0.6            // гамма
#define R1 90000             // сопротивление при 1 люксе
const uint32_t interval = 30000;
bool flagLux = false;

// Переменные проверки подключения фоторезисторов
static bool pin36Connected = false;  //изначально фоторезистор на gpio36 не подключен
static bool pin39Connected = false;  //изначально фоторезистор на gpio39 не подключен
bool checkPhotoresistors = true;     //изначально проверка фоторезисторов включена

// Настройка BME280
Adafruit_BME280 bme;
bool bmeStatus = false;  //проверка подключения bme280

// Переменные управления двигателем
int stepsRotate360 = 48;      //кол-во шагов на полный оборот
int startPositionStep = 0;    //начальная позиция мотора
int finalPositionStep = 348;  //конечная позиция мотора
int currentStep = 0;          //текущая позиция мотора
bool flagPosition = false;    //флаг движения мотора - изначально не двигается
bool flagStop = false;        //флаг принудительной остановки

// переменные времени для автоматического открытия и закрытия
int openHour = 8;                 //час открытия
int openMinute = 0;               //минуты открытия
int closeHour = 21;               //час закрытия
int closeMinute = 0;              //минуты закрытия
bool timeControlEnabled = false;  //флаг активации режима

//Переменные для калибровки положения шторы
int encoderPos = 0;                 //поворот энкодера
bool rightEnc = false;              //флаг направления движения энкодера
bool setStartPositionStep = true;   //флаг калибровки начальной позиции
bool setFinalPositionStep = false;  //флаг калибровки конечной позиции

//Режимы работы
enum { BY_HAND,
       AUTOMATIC,
       BY_HAND_NETWORK,
       CALIBRATION } currentMode;


//НАСТРОЙКИ WI-FI
const char* ssid = "Keenetic-9043";  // Имя сети
const char* password = "HGVeaTBd";   // Пароль сети
byte attempt = 5;                    // 5 попыток на подключение

//Объявление объектов для работы с веб-сервером и NTP
AsyncWebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000);  //часовой пояс +7


void setup() {
  Serial.begin(115200);
  analogReadResolution(10);                              // 10-битный режим АЦП
  Serial.setTimeout(50);                                 //таймаут ожидания приема данных
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_KOL);  //информация о светодиоде
  FastLED.setBrightness(128);                            //яркость светодиода 50%

  pinMode(13, OUTPUT);         //пин светодиода, как выход
  pinMode(enablePin, OUTPUT);  // пин включения драйвера, как выход
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  //Инициализация BME280
  bmeStatus = bme.begin(0x76);  // Адрес датчика
  if (!bmeStatus) {
    Serial.println("Не удалось найти датчик BME280");
  }

  // Инициализация NTP клиента
  timeClient.begin();
  // Настройка и запуск веб-сервера
  byHandNetwork();

  EEPROM.begin(1024);
  if (EEPROM.read(INIT_ADDR) != INIT_KEY) {  // первый запуск
    EEPROM.write(INIT_ADDR, INIT_KEY);       // записали ключ
    EEPROM.put(0, specPosition);
    EEPROM.put(1, currentPosition);
    EEPROM.put(2, pin36Connected);
    EEPROM.put(3, pin39Connected);
    EEPROM.put(4, checkPhotoresistors);
    EEPROM.put(5, startPositionStep);
    EEPROM.put(6, finalPositionStep);
    EEPROM.commit();
  }

  EEPROM.get(0, specPosition);
  EEPROM.get(1, currentPosition);
  EEPROM.get(2, pin36Connected);
  EEPROM.get(3, pin39Connected);
  EEPROM.get(4, checkPhotoresistors);
  EEPROM.get(5, startPositionStep);
  EEPROM.get(6, finalPositionStep);

  Serial.println("Karniz version 1.0");
  currentMode = BY_HAND;        // Режим по умолчанию
  leds[0].setRGB(0, 234, 255);  // голубой
  FastLED.show();               // отобразить изменения светодиода
  ledTmr = millis();
}