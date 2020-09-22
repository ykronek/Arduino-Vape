//--------------------------------<Ver.1.2.0.beta>------------------------------------
//designed by ykronek
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
//ускорена регулировка зажатием
//04.08.2019/19:29
//датчик напряжения работает
//09.08.2019/00:58
//06.10.2019/21:00
//дисплей включаеться стабильно с первого раза
//06.10.2019/21:28
//переделал датчик напряжения
//11.10.2019/23:02
//добавил DEBUG_MODE
//инициализировал среднюю кнопку
//выводит расчетную выходную мощность
//26.06.2020/01:47
//исправил Debug mode
//ардуино запоминает значение 'value' в EEPROM
//20.09.2020/22:25
//добавил фильтр для считывания напряжения

#include<SPI.h>
#include <avr/eeprom.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define FIRE 8 //кнопка фаер
#define MOSFET 6 // пин мосфета
#define VBAT_SENS A3 // пин измерения напряжения
#define PowerDisplay A4 // пин питания дисплея

    // #define DEBUG //comment to disable

#define VERSION "1.2.0"
#define STABorBETA "beta"
#define LowerVbat   3.3      // нижний попрог допустимого напряжения для вейпа

#define VoltageTrue 5.187    // когда все собрали, меряем напряжение на 5вольтах ардуины и вписываем сюда
#define coilRes 0.3          // итоговое сопротивление койлов

float k = 0.1;             // коефициент для фильтра считывания напряжения

#define button1B A0  // пин кнопки button1(left)
#define button2B A1   //(right)
#define button3B A2  //selectButton(center)

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

boolean button3S;   // храним состояния кнопок (S - State)
boolean button3F;   // флажки кнопок (F - Flag)
boolean button3R;   // флажки кнопок на отпускание (R - Release)
boolean button3P;   // флажки кнопок на нажатие (P - Press)
boolean button3H;   // флажки кнопок на удержание (многократный вызов) (H - Hold)
boolean button3HO;  // флажки кнопок на удержание (один вызов при нажатии) (HO - Hold Once)
boolean button3D;   // флажки кнопок на двойное нажатие (D - Double)
boolean button3DP;  // флажки кнопок на двойное нажатие и отпускание (D - Double Pressed)


#define double_timer 100   // время (мс), отведённое на второе нажатие
#define hold 500           // время (мс), после которого кнопка считается зажатой
#define debounce 80        // (мс), антидребезг
#define step_del 20       // задержка между шагами изменения value
boolean lowbat;             // флаг разряда аккумулятора


unsigned long button1_timer; // таймер последнего нажатия кнопки
unsigned long button1_double; // таймер двойного нажатия кнопки

unsigned long button2_timer; // таймер последнего нажатия кнопки
unsigned long button2_double; // таймер двойного нажатия кнопки

unsigned long button3_timer; // таймер последнего нажатия кнопки
unsigned long button3_double; // таймер двойного нажатия кнопки

uint32_t leftbtt_timer;    //таймеры для уменьшения валуе
uint32_t rightbtt_timer;   //таймеры для увеличения валуе



Adafruit_PCD8544 display = Adafruit_PCD8544(7, 13, 5, 4, 3);


double vbat;
int value;
float current;
int power;

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

void buttons3() {
  //-------------------------button3--------------------------
  // нажали (с антидребезгом)
  if (button3S && !button3F && millis() - button3_timer > debounce) {
    button3F = 1;
    button3HO = 1;
    button3_timer = millis();
  }
  // если отпустили до hold, считать отпущенной
  if (!button3S && button3F && !button3R && !button3DP && millis() - button3_timer < hold) {
    button3R = 1;
    button3F = 0;
    button3_double = millis();
  }
  // если отпустили и прошло больше double_timer, считать 1 нажатием
  if (button3R && !button3DP && millis() - button3_double > double_timer) {
    button3R = 0;
    button3P = 1;
  }
  // если отпустили и прошло меньше double_timer и нажата снова, считать что нажата 2 раз
  if (button3F && !button3DP && button3R && millis() - button3_double < double_timer) {
    button3F = 0;
    button3R = 0;
    button3DP = 1;
  }
  // если была нажата 2 раз и отпущена, считать что была нажата 2 раза
  if (button3DP && millis() - button3_timer < hold) {
    button3DP = 0;
    button3D = 1;
    button3_timer = millis();
  }
  // Если удерживается более hold, то считать удержанием
  if (button3F && !button3D && !button3H && millis() - button3_timer > hold) {
    button3H = 1;
  }
  // Если отпущена после hold, то считать, что была удержана
  if (!button3S && button3F && millis() - button3_timer > hold) {
    button3F = 0;
    button3H = 0;
    button3_timer = millis();
  }
  //-------------------------button3--------------------------
}
//------------------------ОТРАБОТКА КНОПОК-------------------------

void dispinit(){

  display.begin();
  display.setContrast(50);
  display.display();
  display.clearDisplay();

}

void drawmainpageframes() {
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

  display.drawFastVLine(45, 25, 26, BLACK);// разделяющая стенка
  display.drawFastVLine(46, 25, 26, BLACK);//vertical line
  display.drawFastVLine(47, 25, 26, BLACK);

  display.display();
}

void drawmainpage(){

   display.clearDisplay();

  display.setCursor(5, 3);
  display.setTextSize(3);
  display.print(value);
  display.setCursor(58, 16);
  display.setTextSize(1);
  display.print("val");

  
  if(lowbat){
    display.setTextSize(1);
    display.setCursor(3, 29);
    display.print("lowBatt");
  }else{
  display.setTextSize(1);
  display.setCursor(4, 29);
  display.print(vbat);
  display.print("v");
  }

  display.setCursor(49, 29);
  display.setTextSize(1);
  display.print(VERSION);

  display.setCursor(50, 37);
  display.setTextSize(1);
  #ifdef DEBUG
  display.print("debug");
  #else
  display.print(STABorBETA);
  #endif

  display.setTextSize(1);
  display.setCursor(4, 37);
  display.print(power);
  display.print("w");
}

float expRunningAverage(float newVal) {
  static float filVal = 0;
  filVal += (newVal - filVal) * k;
  return filVal;
}


void setup() {
  value = eeprom_read_word(0);
  
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.print("Debug mode, firmware version is ");
  Serial.print(VERSION);
  Serial.print('.');
  Serial.println(STABorBETA);
  #endif

  pinMode(PowerDisplay,OUTPUT);
  digitalWrite(PowerDisplay,HIGH);

  dispinit();

  //analogReference(INTERNAL);

  pinMode(button1B, INPUT_PULLUP);
  pinMode(button2B, INPUT_PULLUP);
  pinMode(button3B, INPUT_PULLUP);

  pinMode(FIRE, INPUT_PULLUP);
  pinMode(MOSFET, OUTPUT);

  analogWrite(MOSFET, 0);
}


void loop() {
  if(vbat < LowerVbat){
      lowbat = 1;
  } else{
    lowbat = 0;
  }

  drawmainpageframes();
  drawmainpage();

  int fire = !digitalRead(FIRE);

  //-------опрос кнопок управления--------//
  button1S = !digitalRead(button1B);
  buttons1(); //отработка кнопок
  button2S = !digitalRead(button2B);
  buttons2();
  button3S = !digitalRead(button3B);
  buttons3();
  //-------опрос кнопок управления--------//

  //----------------------------------------button1
  if (button1P || button1D) {
    #ifdef DEBUG
    Serial.println("LEFT-button pressed");
    #endif
    value--;
    if (value <= 0) {
      value = 0;
    }
    button1P = 0;
    button1D = 0;
  }
  if (button1H && button1HO) {
    #ifdef DEBUG
    Serial.println("LEFT-button hold once");
    #endif
    button1HO = 0;
    }
  if (button1H) {
    #ifdef DEBUG
    Serial.println("LEFT-button hold");
    #endif

    if (millis() - leftbtt_timer > step_del){
    leftbtt_timer = millis();
    value--;  
    }

    if (value <= 0) {
      value = 0;
    }

    button1H = 0;
  }
  //----------------------------------------button1
  //----------------------------------------button2
  if (button2P || button2D) {
    #ifdef DEBUG
    Serial.println("RIGHT-button pressed");
    #endif
    value++;
    if (value >= 255) {
      value = 255;
    }
    button2P = 0;
    button2D = 0;
  }
  if (button2H && button2HO) {
    #ifdef DEBUG
    Serial.println("RIGHT-button hold once");
    #endif
    button2HO = 0;
    }
    if (button2H) {
    #ifdef DEBUG
    Serial.println("RIGHT-button hold");
    #endif
    
    if (millis() - rightbtt_timer > step_del){
    rightbtt_timer = millis();
    value++;  
    }

    if (value >= 255) {
      value = 255;
    }

    button2H = 0;
  }
  //----------------------------------------button2
 //-----------------------------------------button3
  if (button3P || button3D) {
    #ifdef DEBUG
    Serial.println("CENTER_button pressed");
    #endif
    button3P = 0;
    button3D = 0;
  }
  if (button3H && button3HO) {
    #ifdef DEBUG
    Serial.println("CENTER_button hold once");
    #endif
    button3HO = 0;
    }
  if (button3H) {
    #ifdef DEBUG
    Serial.println("CENTER_button hold");
    #endif
    button3H = 0;
  }  
   //---------------------------------------button3

//----------------------------------------voltage 
 vbat = (analogRead(VBAT_SENS) * VoltageTrue) / 1024;
 vbat = expRunningAverage(vbat);
 current = vbat / coilRes;
 power = current * vbat * value / 255;
//----------------------------------------voltage

  if (fire == 1) {
    if (!lowbat){
      #ifdef DEBUG
      Serial.println("power on coil is ON!");
      #endif
      analogWrite(MOSFET, value);
    }else if(lowbat){
    #ifdef DEBUG
    Serial.print("FIRE-button pressed, power on coli is OFF!");
    Serial.println(" Low Voltage");
    
    #endif
    }
  
  } else {
    analogWrite(MOSFET, 0);
  }


  eeprom_update_word(0, value);

}

