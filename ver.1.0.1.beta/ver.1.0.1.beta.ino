//--------------------------------<Ver.1.0.1.beta>------------------------------------
//designed_by_ykronek
//just fire on max power with battery volts on disp(work incorrectly)
//28.07.2019
//renamed COIL to MOSFET
//soldered MOSFET pin to D6(can PWM but not used now)
//soldered display pin Din to D13
//soldered display's BL pin on 5v
//30.07.2019

#include<SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define FIRE 8
#define MOSFET 6

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 13, 5, 4, 3);

void setup() {

  display.begin();
  display.setContrast(50);
  display.display();
  display.clearDisplay();

  analogReference(INTERNAL);

  pinMode(FIRE, INPUT_PULLUP);
  pinMode(MOSFET, OUTPUT);

  analogWrite(MOSFET,0);
}

void loop() {

  frames();

  int fire = !digitalRead(FIRE);
  Serial.println(fire);
  if (fire == 1) {
    analogWrite(MOSFET, 255);
  } else {
    analogWrite(MOSFET, 0);
  }

  float Vbat = (analogRead(A1) * 1.1) / 1024.0;
  float Vin = Vbat / (1.0 / (4.0 + 1.0));

  display.clearDisplay();
  display.setCursor(5, 15);
  display.print("Vbat= ");
  display.print(Vin);
  display.setCursor(5, 5);
  display.print("Ver. 1.0.1");
  display.setCursor(5,29);
  display.print("beta");

}

void frames() {
  display.drawFastHLine(0, 0, 84, BLACK);//верхний
  display.drawFastHLine(0, 1, 84, BLACK);

  display.drawFastVLine(0, 0, 48, BLACK);//левый
  display.drawFastVLine(1, 0, 48, BLACK);
  display.drawFastVLine(2, 0, 48, BLACK);

  display.drawFastHLine(0, 47, 84, BLACK);//нижний
  display.drawFastHLine(0, 46, 84, BLACK);
  display.drawFastHLine(0, 45, 84, BLACK);

  display.drawFastVLine(83, 0, 48, BLACK);//правый
  display.drawFastVLine(82, 0, 48, BLACK);
  display.drawFastVLine(81, 0, 48, BLACK);

  display.drawFastHLine(0, 25, 83, BLACK);// разделяющая горизонт
  display.drawFastHLine(0, 26, 83, BLACK);// разделяющая горизонт
  display.drawFastHLine(0, 27, 83, BLACK);// разделяющая горизонт

  display.drawFastVLine(35, 25, 26, BLACK);// разделяющая стенка
  display.drawFastVLine(36, 25, 26, BLACK);// разделяющая стенка
  display.drawFastVLine(37, 25, 26, BLACK);// разделяющая стенка

  display.display();
}
