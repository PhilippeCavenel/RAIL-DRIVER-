#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

// --- Broches HX711 ---
#define DOUT  3
#define CLK   2

HX711 scale;
LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- Paramètres de calibration ---
float calibration_factor = 10000.0;  
float mechanical_factor = 2.0;       // Correction mécanique (force divisée par 2)
float offset = 14.0;

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(5, 0);  
  lcd.print("BONJOUR !");
  lcd.setCursor(0, 2);
  lcd.print(" Poids Max : 200Kg");
  scale.begin(DOUT, CLK);
  delay(4000);
  lcd.clear();
}

void loop() {
  if (scale.is_ready()) {
    long reading = scale.read_average(20);

    // Conversion brute -> poids maquette (kg)
    float weight = round(((float)reading / calibration_factor) * mechanical_factor)-offset;

    if (weight<0)weight=0.0;

    lcd.clear();

    // Ligne 0 : titre centré
    lcd.setCursor(3, 0);
    lcd.print("Poids maquette");

    // AFfichage poids
    if(weight >= 200) {
        lcd.setCursor(0, 1);
        lcd.print("ATTENTION SURCHARGE");
        lcd.setCursor(8, 2);
        lcd.print(weight, 0);
        lcd.print(" kg");
    }
    else {  
      lcd.setCursor(8, 2);
      lcd.print(weight, 0);
      lcd.print(" kg");
    }
  } else {
    lcd.clear();
    lcd.setCursor(3, 1);
    lcd.print("Mesure en cours...");
  }

  delay(500);
}
