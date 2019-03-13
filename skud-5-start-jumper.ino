// Библиотеки
#include <EEPROM.h>                            // Библиотека EEPROM

// Замены
#define row1  13                               // Строки клавиатуры
#define row2  2
#define row3  3
#define row4  4
#define column1 5                              // Колонки клавиатуры
#define column2 6
#define column3 7
#define ledAccess 8                            // Светодиод - разрешающий
#define ledDenied 9                            // Светодиод - запрещающий    
#define buttonOpen 10                          // Кнопка разблокировки
#define buzzer 11                              // Контакт динамика
#define lock 12                                // Контакт замка
#define jumper 14                              // Сброс и первичный запуск

// Глобальные переменные
// Переменные времени для мультизадачности
uint32_t buttonTime;                           // Переменная времени дребезга кнопки открытия
uint32_t openLockTime;                         // Переменная времени задержки открытия времени замка
uint32_t resetPassTime;                        // Переменная времени задержки открытия времени замка

// Настройки
uint8_t firstStart;                            // Первое включение из заводского состояния
bool lockType;                                 // Тип замка - электромеханический = 1. Электромагнитный будет = 0
bool newPassword;                              // Флаг нового пароля
uint16_t openTime;                             // Время открытия замка

// Массивы для клавиатуры
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
int correctPassword;                           // Переменная корректного пароля
int repeatPassword;                            // Переменная повторно вводимого пароля

void setup()                                   // Код, выполняемый один раз при старте
{
  Serial.begin(9600);
  Serial.println("Power On");

  pinMode(buttonOpen, INPUT);             // Конфигурируем контакт кнопки на вход с подтяжкой
  digitalWrite(buttonOpen, HIGH);
  pinMode(jumper, INPUT);                 // Конфигурируем контакт джампера на вход с подтяжкой
  digitalWrite(jumper, HIGH);
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

  digitalWrite (column1, HIGH);           // Внутренняя подтяжка PULLUP для входных портов колонок клавиатуры
  digitalWrite (column2, HIGH);
  digitalWrite (column3, HIGH);
  firstStart = EEPROM.read(0);            // Считываем переменную первого старта из EEPROM
  lockType = 1;                           // Задаём тип замка (1 = электромеханический, 0 - электромагнитный)
  if (lockType == 1)                      // Проверка на тип замка при инициализации, и установка закрытого состояния
  {
    digitalWrite (lock, LOW);
  }
  else
  {
    digitalWrite (lock, HIGH);
  }
  openTime = 3000;                        // Задаём время открытия замка
  first_start();                          // Функция сброса и первичного старта
  //EEPROM.write(0, 0xFF);                // Для теста - переменная новой ячейки EEPROM
}


void first_start()                                            // Первичная инициализация
{
  if (digitalRead(jumper) == LOW || firstStart == 0xFF)       // Если джампер установлен (сброс) или переменная первого старта == 1
  {
    Serial.println("Reset and Initialization");
    password = 0;                                             // Сбрасываем пароль
    set_password();                                           // Вызов функции установки пароля

  }
  else                                                        // Если не установлен джампер сброса (нормальный старт)
  {
    byte *r = (byte*)&correctPassword;                        // Указываем на байты в переменной типа int
    r[0] = EEPROM.read(1);                                    // Первый байт r[0] читаем из первой ячейки EEPROM
    r[1] = EEPROM.read(2);                                    // Второй байт r[1] читаем из второй ячейки EEPROM
    Serial.print("Password in memory = ");
    Serial.println(correctPassword);                          // Выводим в терминал пароль
  }
}

void set_password()                                             // Функции установки пароля
{
  Serial.println("Waiting new password");
  repeatPassword = 1;                                           // Назначаем переменной любое значение, отличное от нуля, чтобы заработал следующий цикл
  while (correctPassword != repeatPassword)                     // Цикл работает, пока два пароля не будут равны. Пользователь должен два раза ввести правильный пароль
  {
    newPassword = 1;                                            // Устанавливаем флаг установки нового пароля
    while (newPassword == 1)                                    // Пока новый пароль не установлен (флаг) - то обрабатываем клавиши в цикле
    {
      key_scan();                                               // Запускаем обработку кнопок
    }
    Serial.print("Password = ");
    Serial.println(correctPassword);
    Serial.println("Please repeat password");
    newPassword = 1;                                            // Сбрасываем флаг цикла опроса пароля
    repeatPassword = correctPassword;                           // Записываем первый введённый пароль в другую переменную, для последующего сравнения
    password = 0;                                               // Обнуляем пароль
    while (newPassword == 1)                                    // Цикл повтора пароля. Пока новый пароль не установлен (флаг) - то обрабатываем клавиши в цикле
    {
      key_scan();                                               // Запускаем обработку кнопок
    }
    if (correctPassword == repeatPassword)                      // Если пароли совпали
    {
      Serial.println("Everything is ok. New password setting successful");
    }
    else                                                        // Если пароли не совпали
    {
      Serial.println("Passwords error. Please try again");
    }
  }
  EEPROM.write(0, 0x01);                                        // Записываем значение в 0 ячейку, отличное от FF
}

void save_new_password()                                      // Функция сохранения нового пароля
{
  correctPassword = password;                                 // Устанавливаем новый пароль
  if (correctPassword == repeatPassword)
  {
    byte *p = (byte*)&correctPassword;                          // Указываем на байты в int переменной correctPassword
    EEPROM.write(1, p[0]);                                      // записываем в ячейку 1 старший байт
    EEPROM.write(2, p[1]);                                      // записываем в ячейку 2 младший байт
  }
  password = 0;                                               // Сбрасываем пароль
  newPassword = 0;                                            // Устанавливаем флаг нового пароля в 1, чтобы выйти из цикла
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
    digitalWrite(keypadOut[r - 1], HIGH);                     // подаём обратно высокий уровень
  }
}

void reset_password()                                   // Функция сброса пароля, при простое
{
  if (millis() - resetPassTime > 3000 && password > 0)  // Если пользователь начал вводить пароль, и прошло время более 3 секунд
  {
    password = 0;                                       // Сбрасываем пароль
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
    if (newPassword == 0)                           // Если флаг нового пароля не установлен
    {
      check_password();                             // Переходим на процедуру проверки пароля
    }
    else if (newPassword == 1 && password < 1000)   // Если флаг нового пароля установлен и введён пароль > 3 символов
    {

      Serial.println("Error. password must be more than 4 characters");
      password = 0;
    }
    else if (newPassword == 1 && password > 999)
    {
      save_new_password();                          // Функция сохранения нового пароля
    }
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

void red_led()                                         // Функция проверки запрещающего светодиода
{
  if (lockType == 1 && digitalRead(lock) == LOW)       // Если замок электромеханический, то
  {
    digitalWrite(ledDenied, HIGH);                     // Включаем запрещающий диод
  }
  else if (lockType == 0 && digitalRead(lock) == HIGH) // Если замок электромагнитный
  {
    digitalWrite(ledDenied, HIGH);                     // Включаем запрещающий диод
  }
  else
  {
    digitalWrite(ledDenied, LOW);                         // Выключаем запрещающий диод
  }
}



void loop()                               // Циклические операции
{
  key_scan ();                            // Сканирование клавиатуры
  open_button();                          // Сканирование кнопки выхода
  lock_close();                           // Алгоритм закрытия замка
  reset_password();                       // Функция сброса пароля
  red_led();                              // Алгоритм свечения красного светодиода
}
