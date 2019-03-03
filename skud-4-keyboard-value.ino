#include <EEPROM.h>

// Замены
#define row1  12                               // Строки клавиатуры
#define row2  2
#define row3  3
#define row4  4
#define column1 5                              // Колонки клавиатуры
#define column2 6
#define column3 7
#define ledAccess 8                            // Светодиод - разрешающий
#define ledDenied 9                            // Светодиод - запрещающий    
#define buttonOpen 10                          // Кнопка разблокировки
#define lock 11                                // Контакт замка
#define buzzer 12                              // Контакт динамика

//переменные времени для мультизадачности
uint8_t buttonTime;                            // Переменная времени дребезга кнопки открытия
uint16_t openLockTime;                         // Переменная времени задержки открытия времени замка
uint16_t resetPassTime;                        // Переменная времени задержки открытия времени замка

// Настройки
bool lockType;                                 // Тип замка - электромеханический = 1. Электромагнитный будет = 0
uint16_t openTime;                             // Время открытия замка

//массивы для клавиатуры
byte keypadOut[4] {row1, row2, row3, row4};    // Пины строк - на них будем подавать напряжение
byte keypadIn[3] {column1, column2, column3};  // Пины колонок - отсюда будем считывать напряжение

const char keyboardValue[4][3]                 // Создаём двумерный массив
{ {'1', '2', '3'},                             // Структура клаиватуры
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// Другое
unsigned char keyboardResult;                  // Переменная, куда назначается число из массива(номер кнопки)
int password = 0;                              // Вводимый пароль
//boolean firstSymbol = true;
int correctPassword;                           // Переменная корректного пароля



void setup()
{
  Serial.begin(9600);
  Serial.println("Power On");

  pinMode(buttonOpen, INPUT);             // Конфигурируем контакт кнопки на вход с подтяжкой
  digitalWrite(buttonOpen, HIGH);
  pinMode(lock, OUTPUT);                  // Конфигурируем контакт замка на выход
  pinMode(ledAccess, OUTPUT);             // Конфигурируем разрешающий светодиод
  pinMode(ledDenied, OUTPUT);             // Конфигурируем запрещающий светодиод
  pinMode(buzzer, OUTPUT);                // Конфигурируем зуммер

  pinMode(row1, OUTPUT);                  // Инициализируем порты на выход (подают нули на столбцы)
  pinMode(row2, OUTPUT);
  pinMode(row3, OUTPUT);
  pinMode(row4, OUTPUT);

  pinMode (column1, INPUT);               // Входные порты колонок клавиатуры настроены на вход
  pinMode (column2, INPUT);
  pinMode (column3, INPUT);


  correctPassword = 5467;                 // Пароль для открытия
  lockType = 1;                           // Задаём тип замка
  openTime = 1000;                        // Задаём время открытия замка
}


void key_scan ()
{
  for (int r = 1; r <= 4; r++)                                // Цикл for, меняющий номер выхода
  {
    digitalWrite(keypadOut[r - 1], LOW);                      // Подаём низкий уровень на выход
    for (int c = 1; c <= 3; c++)                              // Цикл for, меняющий номер входа
    {
      if (digitalRead(keypadIn[c - 1]) == LOW)                // Цикл перебора строк
      {
        delay(50);
        while (digitalRead(keypadIn[c - 1]) == LOW) {}        // Пока кнопка нажата
        {
          resetPassTime = millis();                           // Сбрасываем переменную времени сброса пароля
          keyboardResult = keyboardValue[r - 1][c - 1] - 48;  // Отнимаем из массива 48, для приведения char к int
          password_input();                                   // Функция обработки пароля
        }
      }
    }
    delay(10);
    digitalWrite(keypadOut[r - 1], HIGH);               // подаём обратно высокий уровень
  }
}

void lock_open()
{
  openLockTime = millis();          // Сбрасываем переменную времени openLockTime, чтобы могла сработать функция закрытия
  if (lockType == 1)                // Если замок электромеханический, то
  {
    digitalWrite(lock, HIGH);       // Подаём напряжение на замок
    digitalWrite(ledAccess, HIGH);  // Индицируем, что дверь открыта
    Serial.println("open_electric_lock");
  }
  else if (lockType == 0)           // Если замок электромагнитный
  {
    digitalWrite(lock, LOW);        // Снимаем напряжение на замке
    digitalWrite(ledAccess, HIGH);  // Индицируем, что дверь открыта
    Serial.println("open_magnetic_lock");
  }
}

void lock_close()
{
  if (millis() - openLockTime > openTime && lockType == 1 && digitalRead(lock) == HIGH) // Задержка время открытия при электрическом замке
  {
    digitalWrite(lock, LOW);      // Снимаем напряжение замка
    digitalWrite(ledAccess, LOW); // Снимаем напряжение светодиода
    Serial.println("close");
  }
  else if (millis() - openLockTime > openTime && lockType == 0 && digitalRead(lock) == LOW) // Задержка время открытия при электромагнитном замке
  {
    digitalWrite(lock, HIGH);     // Подаём напряжение замка
    digitalWrite(ledAccess, LOW); // Снимаем напряжение светодиода
    Serial.println("close");
  }
}

void open_button()
{
  if (millis() - buttonTime > 70 && lockType == 1 && digitalRead(lock) == LOW) // Проверяем условия для электромеханического замка
  {
    buttonTime = millis();                   // Сбрасываем переменную времени сканирования кнопки
    if (digitalRead(buttonOpen) == LOW )     // Если кнопка открытия нажата
    {
      lock_open();                           // Запускаем функцию открытия замка
    }
  }
  if (millis() - buttonTime > 70 && lockType == 0 && digitalRead(lock) == HIGH) // Проверяем условия для электромагнитного замка
  {
    buttonTime = millis();                    // Сбрасываем переменную времени сканирования кнопки
    if (digitalRead(buttonOpen) == LOW )      // Если кнопка открытия нажата,
    {
      lock_open();                            // Запускаем функцию открытия замка
    }
  }
}

void check_password()                             // Функция проверки пароля
{
  if (password == correctPassword)                // Если введённый пароль совпал с тем, что сохранён в памяти
  {
    lock_open();                                  // Запускаем процедуру открытия замка
    password = 0;                                 // Сбрасываем пароль
    Serial.println("Password_OK");
  } else                                          // Во всех других случаях, если пароль не совпал
  {
    Serial.println("Password_FAIL");
    password = 0;                                 // Сбрасываем пароль
  }
}

void reset_password()                                   // Функция сброса пароля, при простое
{
  if (millis() - resetPassTime > 3000 && password > 0)  // Если пользователь начал вводить пароль, и прошло время более 3 секунд
  {
    password=0;                                         // Сбрасываем пароль
    Serial.println("Reset_password_3s");
  }
}

void password_input ()
{
  if (keyboardResult >= 0 && keyboardResult <= 9)   // Если нажата цифра от 0 до 9 включительно, то выполняем код формирования числа
  {
    password = password * 10 + keyboardResult;      // Прибавляем к значению password, новую нажатую клавишу, формируя число
    Serial.print("Password = ");
    Serial.println(password);                       // Вывод на экран число
  }
  else if (keyboardResult == 250)                   // Если нажата *
  {
    password = 0;                                   // Сбрасываем пароль
    Serial.println("Reset_password");               
  }
  else if (keyboardResult == 243)                   // Если нажата #
  {
    check_password();                               // Переходим на процедуру проверки пароля
  }
}


void loop() {
  key_scan ();
  open_button();
  lock_close();
  reset_password();
}
