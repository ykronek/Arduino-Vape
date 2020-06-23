//--------------------------------<Ver.1.1.2.beta>------------------------------------
//designed_by_ykronek
//just fire on max power with battery volts on disp(work incorrectly)
//28.07.2019
//renamed COIL to MOSFET
//soldered MOSFET pin to D6(can PWM but not used now)
//soldered display pin Din to D13
//soldered display's BL pin on 5v
//30.07.2019/20:05
//added pwm regulation power(measuring units is value)
//30.07.2019/23:38
//added constrain on values(0-255)
//добавлен коефициент к формуле считывания напряжения
//идут тесты делителя напряжения
//31.07.2019/22:05

#include<SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define FIRE 8
#define MOSFET 6

#define button1B A0  // пин кнопки button1
#define button2B A1

boolean button1S;   // храним состояния кнопок (S - State)
boolean button1F;   // флажки кнопок (F - Flag)
boolean button1R;   // флажки кнопок на отпускание (R - Release)
boolean button1P;   // флажки кнопок на нажатие (P - Press)
boolean button1H;   // флажки кнопок на удержание (многократный вызов) (H - Hold)
boolean button1HO;  // флажки кнопок на удержание (один вызов при нажатии) (HO - Hold Once)
boolean button1D;   // флажки кнопок на двойное нажатие (D - Double)
boolean button1DP;  // флажки кнопок на двойное нажатие и отпускание (D - Double Pressed)

boolean button2S;   // храним состояния кнопок (S - State)
boolean button2F;   // флажки кнопок (F - Flag)
boolean button2R;   // флажки кнопок на отпускание (R - Release)
boolean button2P;   // флажки кнопок на нажатие (P - Press)
boolean button2H;   // флажки кнопок на удержание (многократный вызов) (H - Hold)
boolean button2HO;  // флажки кнопок на удержание (один вызов при нажатии) (HO - Hold Once)
boolean button2D;   // флажки кнопок на двойное нажатие (D - Double)
boolean button2DP;  // флажки кнопок на двойное нажатие и отпускание (D - Double Pressed)

#define double_timer 100   // время (мс), отведённое на второе нажатие
#define hold 500           // время (мс), после которого кнопка считается зажатой
#define debounce 80        // (мс), антидребезг
unsigned long button1_timer; // таймер последнего нажатия кнопки
unsigned long button1_double; // таймер двойного нажатия кнопки

unsigned long button2_timer; // таймер последнего нажатия кнопки
unsigned long button2_double; // таймер двойного нажатия кнопки

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 13, 5, 4, 3);

int value = 150;

void setup() {
  //value = constrain(value, 0, 255);

  //Serial.begin(9600);

  display.begin();
  display.setContrast(50);
  display.display();
  display.clearDisplay();

  analogReference(INTERNAL);

  pinMode(button1B, INPUT_PULLUP);
  pinMode(button2B, INPUT_PULLUP);

  pinMode(FIRE, INPUT_PULLUP);
  pinMode(MOSFET, OUTPUT);

  analogWrite(MOSFET, 0);
}

void loop() {

  frames();

  int fire = !digitalRead(FIRE);

  //-------опрос кнопок управления--------
  button1S = !digitalRead(button1B);
  buttons1(); //отработка кнопок

  button2S = !digitalRead(button2B);
  buttons2();
  //-------опрос кнопок управления--------
  //-------------------------------------------button1
  if (button1P || button1D) {
    //Serial.println("1 pressed");
    value--;
    if (value <= 0) {
      value = 0;
    }
    button1P = 0;
    button1D = 0;
  }
  if (button1H) {
    //Serial.println("1 hold");
    value--;
    if (value <= 0) {
      value = 0;
    }
    delay(30);
    button1H = 0;
  }
  //----------------------------------------button1
  //----------------------------------------button2
  if (button2P || button2D) {
    //Serial.println("2 pressed");
    value++;
    if (value >= 255) {
      value = 255;
    }
    button2P = 0;
    button2D = 0;
  }
  if (button2H) {
    //Serial.println("2 hold");
    value++;
    if (value >= 255) {
      value = 255;
    }
    delay(30);
    button2H = 0;
  }
  //----------------------------------------button2

  float Vbat = (analogRead(A1) * 1.1) / 1024.0;
  float Vin = (Vbat / (1.0 / (4.0 + 1.0))) * 0.743169399;

  //Serial.println(fire);

  if (fire == 1) {
    analogWrite(MOSFET, value);
  } else {
    analogWrite(MOSFET, 0);
  }

  display.clearDisplay();

  display.setCursor(5, 3);
  display.setTextSize(3);
  display.print(value);
  display.setCursor(58, 16);
  display.setTextSize(1);
  display.print("val");

  display.setTextSize(1);
  display.setCursor(4, 29);
  display.print(Vin);
  display.print("v");

  display.setCursor(39, 29);
  display.setTextSize(1);
  display.print("1.1.2");

  display.setCursor(56, 37);
  display.setTextSize(1);
  display.print("beta");

  display.setTextSize(1);
  display.setCursor(4, 37);
  display.print("unlck");
  /*display.setCursor(5, 5);
    display.print("Ver. 1.0.1");
    display.setCursor(5,29);
    display.print("beta");*/

}

void frames() {
  display.drawFastHLine(0, 0, 84, BLACK);//верхний
  display.drawFastHLine(0, 1, 84, BLACK);//top

  display.drawFastVLine(0, 0, 48, BLACK);//левый
  display.drawFastVLine(1, 0, 48, BLACK);//left
  display.drawFastVLine(2, 0, 48, BLACK);

  display.drawFastHLine(0, 47, 84, BLACK);//нижний
  display.drawFastHLine(0, 46, 84, BLACK);//down
  display.drawFastHLine(0, 45, 84, BLACK);

  display.drawFastVLine(83, 0, 48, BLACK);//правый
  display.drawFastVLine(82, 0, 48, BLACK);//right
  display.drawFastVLine(81, 0, 48, BLACK);

  display.drawFastHLine(0, 25, 83, BLACK);// разделяющая горизонт
  display.drawFastHLine(0, 26, 83, BLACK);//horizontal line
  display.drawFastHLine(0, 27, 83, BLACK);

  display.drawFastVLine(35, 25, 26, BLACK);// разделяющая стенка
  display.drawFastVLine(36, 25, 26, BLACK);//vertical line
  display.drawFastVLine(37, 25, 26, BLACK);

  display.display();
}
//------------------------ОТРАБОТКА КНОПОК-------------------------
void buttons1() {
  //-------------------------button1--------------------------
  // нажали (с антидребезгом)
  if (button1S && !button1F && millis() - button1_timer > debounce) {
    button1F = 1;
    button1HO = 1;
    button1_timer = millis();
  }
  // если отпустили до hold, считать отпущенной
  if (!button1S && button1F && !button1R && !button1DP && millis() - button1_timer < hold) {
    button1R = 1;
    button1F = 0;
    button1_double = millis();
  }
  // если отпустили и прошло больше double_timer, считать 1 нажатием
  if (button1R && !button1DP && millis() - button1_double > double_timer) {
    button1R = 0;
    button1P = 1;
  }
  // если отпустили и прошло меньше double_timer и нажата снова, считать что нажата 2 раз
  if (button1F && !button1DP && button1R && millis() - button1_double < double_timer) {
    button1F = 0;
    button1R = 0;
    button1DP = 1;
  }
  // если была нажата 2 раз и отпущена, считать что была нажата 2 раза
  if (button1DP && millis() - button1_timer < hold) {
    button1DP = 0;
    button1D = 1;
    button1_timer = millis();
  }
  // Если удерживается более hold, то считать удержанием
  if (button1F && !button1D && !button1H && millis() - button1_timer > hold) {
    button1H = 1;
  }
  // Если отпущена после hold, то считать, что была удержана
  if (!button1S && button1F && millis() - button1_timer > hold) {
    button1F = 0;
    button1H = 0;
    button1_timer = millis();
  }
  //-------------------------button1--------------------------
}
//------------------------ОТРАБОТКА КНОПОК-------------------------

void buttons2() {
  //-------------------------button2--------------------------
  // нажали (с антидребезгом)
  if (button2S && !button2F && millis() - button2_timer > debounce) {
    button2F = 1;
    button2HO = 1;
    button2_timer = millis();
  }
  // если отпустили до hold, считать отпущенной
  if (!button2S && button2F && !button2R && !button2DP && millis() - button2_timer < hold) {
    button2R = 1;
    button2F = 0;
    button2_double = millis();
  }
  // если отпустили и прошло больше double_timer, считать 1 нажатием
  if (button2R && !button2DP && millis() - button2_double > double_timer) {
    button2R = 0;
    button2P = 1;
  }
  // если отпустили и прошло меньше double_timer и нажата снова, считать что нажата 2 раз
  if (button2F && !button2DP && button2R && millis() - button2_double < double_timer) {
    button2F = 0;
    button2R = 0;
    button2DP = 1;
  }
  // если была нажата 2 раз и отпущена, считать что была нажата 2 раза
  if (button2DP && millis() - button2_timer < hold) {
    button2DP = 0;
    button2D = 1;
    button2_timer = millis();
  }
  // Если удерживается более hold, то считать удержанием
  if (button2F && !button2D && !button2H && millis() - button2_timer > hold) {
    button2H = 1;
  }
  // Если отпущена после hold, то считать, что была удержана
  if (!button2S && button2F && millis() - button2_timer > hold) {
    button2F = 0;
    button2H = 0;
    button2_timer = millis();
  }
  //-------------------------button2--------------------------
}
//------------------------ОТРАБОТКА КНОПОК-------------------------
