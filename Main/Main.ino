#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_TCS34725.h>
#include <Adafruit_MotorShield.h>
#include <HTInfraredSeeker.h>
#include <utility/imumaths.h>
#include <EEPROM.h>

//-----------
//Definitions
//-----------
// PINS
#define OLED_RESET 6
#define MODE_PIN 2
#define OFFENSE 1
#define DEFENSE 2
// I2C addresses
#define MULTIPLEXER 0x70
#define SEEKER 0x8
#define OLED 0x70
#define COMPASS 0x70

#define WHYTE 25

extern "C" {
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

//----------------
//Global Variables
//----------------
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BNO055 bno = Adafruit_BNO055(55);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_1X);
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

boolean irSensors[4] = {false, false, false, false};
int irSensorCount = 0;

int soccerMode = OFFENSE;

double front = 0;
boolean setFrontPressed = false;

//-------------------
//Function prototypes
//-------------------
void setup(void);
void loop(void);

boolean initOLED(void);
boolean initColorSensor(void);
boolean initBallSensor(void);
boolean initCompass(void);
boolean initSonar(void);
void calibrateCompass(void);

void splashScreen(int);
void line1(String);
void line2(String);
void line3(String);

PROGMEM const unsigned char rambot_logo[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x8f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x7f, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0x3f, 0xff, 0x00, 0x0e, 0x00, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xbf, 0xfe, 0x7f, 0xff, 0x81, 0xff, 0x01, 0xfc, 0x1f, 0xe0, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xbf, 0xfe, 0x7f, 0xff, 0xc7, 0xff, 0x87, 0xfd, 0xff, 0xe1, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xbf, 0xfe, 0xfd, 0xdf, 0xe7, 0xff, 0x8f, 0xfd, 0xff, 0xe7, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xbf, 0xfe, 0xf7, 0xef, 0xef, 0xff, 0x1f, 0xfd, 0xff, 0xc7, 0xff,
  0x00, 0x00, 0x00, 0x03, 0xbf, 0xbf, 0xfe, 0xef, 0xf7, 0xef, 0xdf, 0x3f, 0xfd, 0xfe, 0x0f, 0xff,
  0x00, 0x00, 0x00, 0x3f, 0xdf, 0xbf, 0xfe, 0xec, 0xf7, 0xff, 0x9f, 0x3e, 0x7f, 0xbe, 0x1f, 0xb0,
  0x00, 0x00, 0x03, 0xff, 0xdb, 0xbf, 0xfe, 0xe9, 0x7f, 0xff, 0x9f, 0x3e, 0xfc, 0x7e, 0x1f, 0x00,
  0x00, 0x00, 0x7f, 0xff, 0xdb, 0xff, 0xfe, 0xe6, 0xf7, 0xff, 0xbf, 0x7c, 0xf8, 0x7c, 0x1f, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xfb, 0x7f, 0xfe, 0x7f, 0xf7, 0xff, 0xbe, 0x7c, 0xf8, 0x7c, 0x1f, 0x00,
  0x00, 0x03, 0xff, 0xff, 0xfb, 0x7f, 0xff, 0x7f, 0xef, 0xff, 0x3e, 0x7c, 0xf8, 0x7c, 0x3f, 0x00,
  0x00, 0x03, 0xff, 0x7f, 0xb7, 0x79, 0xff, 0x3f, 0xcf, 0xdf, 0xfc, 0x7c, 0xf8, 0x7c, 0x3f, 0xf8,
  0x00, 0x0f, 0xfe, 0x1f, 0xb7, 0x7b, 0xbe, 0xef, 0xbf, 0x9f, 0xfc, 0x7c, 0xf8, 0x7c, 0x3f, 0xfc,
  0x00, 0x03, 0xfe, 0x3f, 0x37, 0x7b, 0x05, 0xf8, 0x7f, 0x1f, 0xfe, 0xfd, 0xf8, 0xfc, 0x3f, 0xfc,
  0x00, 0x03, 0xfc, 0x3f, 0x67, 0x78, 0xf9, 0xff, 0xff, 0x3f, 0xfe, 0xf9, 0xf0, 0xf8, 0x3f, 0xfc,
  0x00, 0x03, 0xfc, 0xff, 0x7f, 0x7b, 0xfb, 0xff, 0xff, 0x3f, 0xfc, 0xf9, 0xf0, 0xf8, 0x1f, 0xfc,
  0x00, 0x03, 0xff, 0xfe, 0xff, 0x7b, 0xb9, 0xff, 0xfe, 0x3e, 0x7c, 0xf9, 0xf0, 0xf8, 0x00, 0xfc,
  0x00, 0x07, 0xff, 0xfd, 0xee, 0x77, 0x03, 0xff, 0xf8, 0x3e, 0x7d, 0xfb, 0xf1, 0xf8, 0x00, 0xf8,
  0x00, 0x07, 0xff, 0xfb, 0xce, 0x77, 0x83, 0xff, 0x80, 0x3e, 0x7d, 0xfb, 0xe1, 0xf8, 0x00, 0xf8,
  0x00, 0x0f, 0xff, 0xe7, 0xde, 0xf3, 0xf0, 0xfc, 0x00, 0x7e, 0xf9, 0xf3, 0xe1, 0xf0, 0x00, 0xf8,
  0x00, 0x0f, 0xe7, 0xcf, 0x9e, 0xf3, 0xfc, 0xc0, 0x00, 0x7c, 0xf9, 0xf3, 0xe1, 0xf0, 0x01, 0xf0,
  0x00, 0x1f, 0xc7, 0xdf, 0x9e, 0xf9, 0xfe, 0x00, 0x00, 0x7d, 0xf9, 0xf7, 0xc1, 0xf0, 0x79, 0xf0,
  0x00, 0x1f, 0xcf, 0x90, 0x3e, 0xfc, 0xff, 0x00, 0x00, 0x7f, 0xf1, 0xff, 0xc1, 0xf0, 0xff, 0xe0,
  0x00, 0x3f, 0x0f, 0x80, 0x06, 0xfe, 0x7f, 0xc0, 0x00, 0x7f, 0xe1, 0xff, 0x83, 0xe0, 0xff, 0xe0,
  0x00, 0x7e, 0x1f, 0x00, 0x00, 0xc0, 0x3f, 0xe0, 0x00, 0x7f, 0xc3, 0xff, 0x03, 0xe0, 0xff, 0xc0,
  0x01, 0xf8, 0x3c, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0xff, 0x01, 0xfc, 0x03, 0xe0, 0xff, 0x00,
  0x03, 0xe0, 0x78, 0x00, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0xfc, 0x00, 0xf0, 0x03, 0x80, 0x78, 0x00,
  0x03, 0x80, 0xf0, 0x00, 0x00, 0x0e, 0xff, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x3f, 0x03, 0xc0, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup() {
  while (!Serial); // Wait for Serial Monitor
  Serial.begin(9600);
  Wire.begin();

  // Setup screen
  if (!initOLED()) {
    while (1);
  }

  splashScreen(1500);

  // Setup Input Pins
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  display.clearDisplay();
  line1(F("Input Pins"));
  line2(F("SETUP"));
  delay(2000);

  // Setup Color Sensor
  display.clearDisplay();
  line1(F("Color Sensor"));
  if (initColorSensor()) {
    line2(F("PASS"));
  } else {
    line2(F("FAIL"));
    while (1);
  }
  delay(500);

  // Setup Ball Sensor
  display.clearDisplay();
  line1(F("Ball Sensor"));
  if (initBallSensor()) {
    line2(F("PASS"));
  } else {
    line2(F("FAIL"));
    while (1);
  }
  delay(500);

  // Setup Motor Shield
  display.clearDisplay();
  line1(F("Motor Shield"));
  AFMS.begin();
  line2(F("PASS"));
  delay(500);

  // Setup Compass
  display.clearDisplay();
  line1(F("Compass"));
  if (initCompass()) {
    display.clearDisplay();
    line1(F("Compass"));
    line2(F("PASS"));
  } else {
    display.clearDisplay();
    line1(F("Compass"));
    line2(F("FAIL"));
    while (1);
  }
  delay(500);

  display.clearDisplay();
  calibrateCompass();

  // Setup Sonar Sensors

}

// Line sensing stuff

//#define NONE -1
//
//#define RIGHT 0
//#define LEFT 1
//
//int lastXDir = NONE;
//
//#define FWD 0
//#define BKWD 1
//
//int lastYDir = NONE;
//
//boolean sawLine = false;

boolean paused = true;

void pause() {
  paused = true;
  halt();
}

int lastX = 0;
int lastY = 0;

void loop() {
  
  display.clearDisplay();

  soccerMode = (digitalRead(MODE_PIN) == HIGH) ? OFFENSE : DEFENSE;
  
  line1(soccerMode == OFFENSE ? "Striker" : "Goalie");
  
  // Set front
  boolean prevSetFrontPressed = setFrontPressed;
  setFrontPressed = digitalRead(8) == 0;
  if (!prevSetFrontPressed && setFrontPressed) {
    front = readCompass();
    pause();
  }
  

  // Read color sensor
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
//  line3(String(c) + "," + String(r) + "," + String(g) + "," + String(b));
  if (c > 100 || (digitalRead(10) == 0 && !paused)) {
    pause();
  }

  if (paused) {
    line2(F("Paused"));
    paused = digitalRead(9) == 1;
  } else {

    int angle = getReading(false);
    int power = getReading(true);
    line2(String(power));
    line3(String(angle));
      
    float rad  = (angle - 90) * PI / 180;
    float bx = 100 * cos(rad);
    float by = 100 * sin(rad) * -1;

    // Read compass
    double compOff = getCompOff();
    int rot = abs(compOff) > 9 ? (int)(-5 * compOff) : 0;
    rot = rot > 100 ? 100 : rot;
    rot = rot < -100 ? -100 : rot;
    
    if (soccerMode == OFFENSE) {

      if (c > WHYTE) {
        drive(-lastX, -lastY, 0);
        delay(700);
        return;
      }
  
  //    if (c > WHYTE) {
  //      if ((x > 0 && lastXDir == RIGHT) || (x < 0 && lastXDir == LEFT)) {
  //        x = 0;
  //      }
  //      if ((y > 0 && lastYDir == FWD) || (y < 0 && lastYDir == BKWD)) {
  //        y = 0;
  //      }
  //    }
    
  //    if (!sawLine) {
  //      if (x == 0 && c <= WHYTE) {
  //        lastXDir = NONE;
  //      } else if (x > 0) {
  //        lastXDir = RIGHT;
  //      } else if (x < 0) {
  //        lastXDir = LEFT;
  //      }
  //      
  //      if (y == 0 && c <= WHYTE) {
  //        lastYDir = NONE;
  //      } else if (y > 0) {
  //        lastYDir = FWD;
  //      } else if (y < 0) {
  //        lastYDir = BKWD;
  //      }
  //    }

      int x = 0;
      int y = 0;
  
      if (abs(angle) < 45 || power < 6) {
        x = bx;
        y = by;
      } else if (abs(angle) < 60) {
        x = bx;
        y = by * 0.4;
      } else if (angle < 90 && angle > 0) {
        x = 75;
        y = -75;
      } else if (angle > -90 && angle < 0) {
        x = -75;
        y = -75;
      } else if (abs(angle) < 110) {
        x = 0;
        y = -100;
      } else if (angle <= 135 && angle > 0) {
        x = -35;
        y = -100;
      } else if (angle >= -135 && angle < 0) {
        x = 35;
        y = -100;
      } else if (angle <= 180 && angle > 0) {
        x = -60;
        y = -50;
      } else if (angle >= -180 && angle < 0) {
        x = 60;
        y = -50;
      }

      drive(x, y, rot);

      lastX = x;
      lastY = y;
      
//      sawLine = c > WHYTE;
      
    } else {
      if (angle > 9 && angle <= 90) {
        int x = (int)min(bx * 2, 100);
        drive(x, 0, rot);
          line2(String(x));
      } else if (angle < -9 && angle >= -90) {
        int x = (int)max(bx * 2, -100);
        drive(x, 0, rot);
          line2(String(x));
      } else if (angle == 0 && power > 17) {
        drive(0, 100, 0);
        delay(500);
        halt();
        delay(100);
        drive(0, -100, 0);
        delay(500);
      } else {
        halt();
      }
    }
  }
}


