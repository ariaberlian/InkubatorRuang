#include <TimerOne.h>

#include <DallasTemperature.h>
#include <OneWire.h>
#include <PID_v1.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define TRIMPOT_PIN A7
#define SENSOR_PIN 2
#define PELTIER_PIN 3
#define HEATER_PIN 6
#define CIRC_FAN_PIN 11

OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 16, 2);

// const float refMaxTemp = 99.91;
// const float refMinTemp = 0;
// const float rawMaxTemp = 96.81;
// const float rawMinTemp = 0.06;

double realTemp = 22.0;
double targetTemp = 25.0;
double outputPWM = 0;

double Kp = 2.0, Ki = 5.0, Kd = 1.0;
PID myPID(&realTemp, &outputPWM, &targetTemp, Kp, Ki, Kd, DIRECT);

volatile bool updateFlag = false;
bool error = false;
bool peltierOn = false;

void timerCallback() {
  updateFlag = true;
}

float getTemp() {
  sensors.requestTemperatures();
  float raw = sensors.getTempCByIndex(0);
  if (raw != DEVICE_DISCONNECTED_
          } else if (peltierOn && (realTemp <= (targetTemp - 0.2))) {
            peltierOn = false;
          }

          digitalWrite(PELTIER_PIN, peltierOn);
          analogWrite(HEATER_PIN, 0);
          analogWrite(CIRC_FAN_PIN, 0);
        } else {
          analogWrite(HEATER_PIN, abs(outputPWM));
          analogWrite(CIRC_FAN_PIN, 100);
          digitalWrite(PELTIER_PIN, LOW);
          peltierOn = false;
        }
      } else {
        analogWrite(HEATER_PIN, 0);C) {
    error = false;
    // return (((raw - rawMinTemp) * (refMaxTemp - refMinTemp)) / (rawMaxTemp - rawMinTemp)) + refMinTemp;
  } else {
    error = true;
  }
  return raw;
}

void setup() {
  pinMode(PELTIER_PIN, OUTPUT);
  pinMode(CIRC_FAN_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(TRIMPOT_PIN, INPUT);

  Serial.begin(9600);
  sensors.begin();
  lcd.init();
  lcd.backlight();

  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);
  analogWrite(CIRC_FAN_PIN, 0);

  targetTemp = 20.0 + (analogRead(TRIMPOT_PIN) / 1023.0) * (35.0 - 20.0);
  realTemp = getTemp();

  // // TimerOne setup: 1 detik interval
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(timerCallback);
}

void loop() {
  if (updateFlag) {
    updateFlag = false;

    realTemp = getTemp();
    targetTemp = 20.0 + (analogRead(TRIMPOT_PIN) / 1023.0) * (35.0 - 20.0);

    if (!error) {
      myPID.Compute();

      if (abs(realTemp - targetTemp) > 0.2) {  // Deadband
        if (outputPWM < 0) {
          // === HISTERESIS UNTUK PELTIER ===
          if (!peltierOn && (realTemp > (targetTemp + 0.2))) {
            peltierOn = true;
        analogWrite(CIRC_FAN_PIN, 0);
        digitalWrite(PELTIER_PIN, peltierOn);
      }

      // Display
      Serial.print(realTemp);
      Serial.print(",");
      Serial.print(targetTemp);
      Serial.print(",");
      Serial.println(outputPWM);

      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(realTemp, 1);
      lcd.print((char)223);
      lcd.print("C   ");

      lcd.setCursor(0, 1);
      lcd.print("Target: ");
      lcd.print(targetTemp, 1);
      lcd.print((char)223);
      lcd.print("C   ");
    } else {
      Serial.print("Sensor Disconnected: ");
      Serial.println(realTemp);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error!");
      lcd.setCursor(0, 1);
      lcd.print("Cant Read Temperature");
    }
  }
}
