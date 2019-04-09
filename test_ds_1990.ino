#include <OneWire.h>

OneWire ds1990(10); // выход считывателя на 10 пин
byte address[8];    // буфер приема

void setup() // стартовая инициализация
{
  Serial.begin(9600);
  delay(100);
  Serial.println("Power on");
}

void loop() {

  if (ds1990.reset())             // Проверяем, есть ли на линии устройство iButton
  {
    ds1990.write(0x33);           // отправляем команду "считать ROM"
    delay(1);                     // Задержка,для подготовки приёма данных
    for (int i = 0; i < 8; i++)   // Цикл для считывания данных в массив
    {
      address[i] = ds1990.read(); // считываем
    }

    Serial.print("HEX card =");
    for (int i = 0; i < 8; i++)       // Цикл для вывода 8 байт в терминал
    {
      Serial.write(' ');              // Выводим пробел для визуального удобства
      Serial.print(address[i], HEX);  // Выводим данные из массива в формате HEX
    }
    Serial.println();
    delay(500);
  }
}
