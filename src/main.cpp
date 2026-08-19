#include <Arduino.h>
#include <LiquidCrystal.h> // استخدام المكتبة العادية

// تعريف الأطراف: (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int pinDC = A0; 
const int pinAC = A1; // الإشارة بعد الـ HPF، النوتش، الداود، ومرحلة التكبير

const float Gain_DC = 6.0; 
const float Gain_AC = 40.0; 
const float DiodeDrop = 0.44; // جهد الانخفاض على الداود (عدله حسب نوع الداود المستخدم: 0.7 أو 0.2)

unsigned long lastBeatTime = 0;
int bpm = 0;
float currentPeakAC = 0;
float detectedPeakAC = 0;
bool crossingZero = true;

void setup() {
lcd.begin(16, 2); // التهيئة الصحيحة لشاشة LCD عادية (أعمدة، صفوف)
  // تم حذف سطر lcd.backlight() لأنه غير موجود في الشاشة العادية
  lcd.setCursor(0, 0);
  lcd.print("Pulse Oximeter");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  // 1. قراءة الـ DC الحقيقي
  int rawDC = analogRead(pinDC);
  float trueDC = (rawDC * (5.0 / 1023.0)) / Gain_DC;

  // 2. قراءة إشارة الـ AC الموجبة (بعد الداود)
  int rawAC = analogRead(pinAC);
  float voltageAC = rawAC * (5.0 / 1023.0);

  // تتبع القمة (Peak Detection) للإشارة الموجبة فقط
  if (voltageAC > 0.1) { // عتبة التشويش
    if (voltageAC > currentPeakAC) {
      currentPeakAC = voltageAC; 
    } else {
      detectedPeakAC = currentPeakAC; // الوصول للقمة البدء بالتناقص
    }
    
    if (crossingZero) {
      unsigned long currentTime = millis();
      unsigned long deltaInterval = currentTime - lastBeatTime;
      if (deltaInterval > 300) {
        bpm = 60000 / deltaInterval;
        lastBeatTime = currentTime;
      }
      crossingZero = false;
    }
  } else {
    currentPeakAC = 0;
    crossingZero = true;
  }

  // 3. حساب القيمة الحقيقية للـ AC (تعويض جهد الداود ثم قسمة التكبير)
  float restoredPeakAC = detectedPeakAC + DiodeDrop; 
  float trueAC_Peak = restoredPeakAC / Gain_AC;
  if (trueAC_Peak < 0) trueAC_Peak = 0;

  // 4. حساب نسبة الأكسجين SpO2
// 4. حساب نسبة الأكسجين SpO2 (مع تعديل معامل الحساسية ليظهر تباين واقعي)
  float ratio = 0;
  if (trueDC > 0) {
    ratio = trueAC_Peak / trueDC;
  }
  
  // قم بزيادة الرقم المضروب في ratio (مثلاً إلى 150 أو 200) لترى التغيير الفعلي على الشاشة
  int spo2 = constrain(99 - (int)(150 * ratio), 90, 100);

  // 5. العرض على الشاشة
  static unsigned long displayTimer = 0;
  if (millis() - displayTimer > 500) {
    displayTimer = millis();
    
    lcd.setCursor(0, 0);
    lcd.print("BPM: ");
    if (bpm > 40 && bpm < 180) {
      lcd.print(bpm);
      lcd.print("   ");
    } else {
      lcd.print("-- ");
    }
    
    lcd.setCursor(0, 1);
    lcd.print("SpO2: ");
    lcd.print(spo2);
    lcd.print("%  ");
  }
  
  delay(5);
}