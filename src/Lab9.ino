#define btn A0
#define temp A1
#define potentiometer A2
#define orangeLed D8
#define greenLed D7
#define blueLed D6
#define NOTIFY_EVENT_CODE "notify"
#define blynkTempC V6
#define blynkTempF V7
#define blynkLightLevel V2
#define lowLevel "Light level is now low"
#define mediumLevel "Light level is now medium"
#define highLevel "light level is now high"

SYSTEM_THREAD(ENABLED);

#include "env.h"
#include "oled-wing-adafruit.h"
#include "Wire.h"
#include "SparkFun_VCNL4040_Arduino_Library.h"
#include "blynk.h"

VCNL4040 proximitySensor;

OledWingAdafruit display;

bool tempMode = false;
bool btnState = false;
float tempC = 0.0;
float tempF = 0.0;
unsigned int lightLevel = 0;
bool clickedOnce = false;
bool firstClick = true;
bool doneClicking = false;
unsigned int lowValue = 0;
unsigned int highValue = 0;
int lowest = 4095;
int highest = 0;
String prevMode = "meow";
String ledMode = "meow";

BLYNK_WRITE(V0) {
	if (param.asInt()) {
    for (int i = 0; i < 4; i++) {
      resetLeds();
      digitalWrite(orangeLed, 1);
      delay(100);
      resetLeds();
      digitalWrite(greenLed, 1);
      delay(100);
      resetLeds();
      digitalWrite(blueLed, 1);
      delay(100);
    }
	}
}

void setup() {
  pinMode(temp, INPUT);
  pinMode(btn, INPUT);
  pinMode(potentiometer, INPUT);
  pinMode(orangeLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN);

	display.setup();

	resetDisplay();

  Wire.begin();

  proximitySensor.begin();

  proximitySensor.powerOnAmbient();
}

void loop() {
  loopBlynkAndOled();

  calibratePotentiometer();

  setTempAndLightVariablesAndUpdateBlynk();

  resetDisplay();

  displayTempOrLight();

  setHighLowLevels();

  changeModes();

  lightUpLeds();
}

void resetLeds() {
  digitalWrite(blueLed, 0);
  digitalWrite(greenLed, 0);
  digitalWrite(orangeLed, 0);
}

void resetDisplay() {
  display.clearDisplay();

	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
}

void displayTemp() {
  display.print(tempC);
  display.print(" Celsius");
  display.println();
  display.print(tempF);
  display.print(" Fahrenheit");
  display.display();
}

void displayLight() {
  display.print("Light Level: ");
  display.print(lightLevel);
  display.display();
}

void calibratePotentiometer() {
  lowest = min(lowest, analogRead(potentiometer));
  highest = max(highest, analogRead(potentiometer));
}

void setHighLowLevels() {
  if (digitalRead(btn) && !clickedOnce && !doneClicking) {
    clickedOnce = true;

    if (firstClick) {
      lowValue = map(analogRead(potentiometer), lowest, highest, 0, 65535);
      firstClick = false;
    } else {
      highValue = map(analogRead(potentiometer), lowest, highest, 0, 65535);
      doneClicking = true;
    }
  } else if (!digitalRead(btn)) {
    clickedOnce = false;
  }
}

void displayTempOrLight() {
  if (tempMode) {
    displayTemp();
  } else {
    displayLight();
  }
}

void changeModes() {
  if (display.pressedA()) {
    tempMode = !tempMode;
  }
}

void lightUpLeds() {
  if (doneClicking) {
    if (lightLevel < lowValue) {
      resetLeds();
      digitalWrite(orangeLed, 1);
      prevMode = ledMode;
      ledMode = lowLevel;
    } else if (lightLevel < highValue) {
      resetLeds();
      digitalWrite(greenLed, 1);
      prevMode = ledMode;
      ledMode = mediumLevel;
    } else {
      resetLeds();
      digitalWrite(blueLed, 1);
      prevMode = ledMode;
      ledMode = highLevel;
    }
  }

  if (prevMode != ledMode) {
    Blynk.logEvent("notify", ledMode);
  }
}

void setTempAndLightVariablesAndUpdateBlynk() {
  tempC = (analogRead(temp)/4095.0 * 3.3 - 0.5) * 100;
  tempF = tempC * 9.0 / 5.0 + 32.0;
  lightLevel = proximitySensor.getAmbient();
  Blynk.virtualWrite(blynkTempC, tempC);
  Blynk.virtualWrite(blynkTempF, tempF);
  Blynk.virtualWrite(blynkLightLevel, lightLevel);
}

void loopBlynkAndOled() {
  display.loop();
  Blynk.run();
}