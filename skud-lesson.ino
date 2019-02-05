#define row1  12                          // Строки клавиатуры
#define row2  2
#define row3  3
#define row4  4
#define column1 5                        // Колонки клавиатуры
#define column2 6
#define column3 7

byte keypadOut[4] {row1, row2, row3, row4};    // пины строк - на них будем подавать напряжение
byte keypadIn[3] {column1, column2, column3};  // пины колонок - отсюда будем считывать напряжение

const char keyboardValue[4][3]          //Создаём двумерный массив
{ {'1', '2', '3'},                      //структура клаиватуры
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

char keyboardResult; // переменная, куда назначается число из массива(номер кнопки)



void setup() {
  Serial.begin(9600);
  Serial.println("Test Power On");

  pinMode(row1, OUTPUT);                  // инициализируем порты на выход (подают нули на столбцы)
  pinMode(row2, OUTPUT);
  pinMode(row3, OUTPUT);
  pinMode(row4, OUTPUT);

  pinMode (column1, INPUT);               // входные порты колонок клавиатуры настроены на вход
  pinMode (column2, INPUT);
  pinMode (column3, INPUT);
}


void key_scan () {
  for (int r = 1; r <= 4; r++) // цикл for, меняющий номер выхода
  {
    digitalWrite(keypadOut[r - 1], LOW); // подаём низкий уровень на выход
    for (int c = 1; c <= 3; c++) // цикл for, меняющий номер входа
    {
      if (digitalRead(keypadIn[c - 1]) == LOW) { //
        delay(50);
        while (digitalRead(keypadIn[c - 1]) == LOW) {}
        {
          Serial.print(keyboardValue[r - 1][c - 1]); //
        }

      }
    }
    delay(10);
    digitalWrite(keypadOut[r - 1], HIGH); // подаём обратно высокий уровень
  }
}

void loop() {
  key_scan ();
}
