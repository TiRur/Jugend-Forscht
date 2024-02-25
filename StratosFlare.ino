#include <Arduino_LPS22HB.h>
#include "Arduino_BMI270_BMM150.h"  
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SD.h> 

float x, y, z;
int degreesX = 0;
int degreesY = 0;

//----------------------

Adafruit_PWMServoDriver myServos = Adafruit_PWMServoDriver();

#define SERVOMIN  150
#define SERVOMAX  600
int numberOfServos = 2;

//---------------------
int CS_pin = 10;

//---------------------
#define CYAN 22
#define MAG 23
#define YEL 24
#define r "r"
#define g "g"
#define b "b"

void setup() {
  pinMode(MAG, OUTPUT);
  pinMode(CYAN, OUTPUT);
  pinMode(YEL, OUTPUT);

  //----------------------------------------------------
  if (!BARO.begin()) {
    LED_R();
  }
  //-------------------------------------------------------
  if (!IMU.begin()) {
    LED_R();
  }

  //------------------------------------------------------
  myServos.begin();
  myServos.setPWMFreq(60);
  delay(5);

  //-----------------------------------------------------
  Wire.begin();
  pinMode(CS_pin, OUTPUT);
  if(!SD.begin(CS_pin)){
    LED_R();
  }
  LED_G();
}

void loop() {
  // read the sensor value
  float pressure = BARO.readPressure();
  float altitude = 44330 * ( 1 - pow(pressure/101.325, 1/5.255) );

  //-----------------------------------------
  float x, y, z;

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
  }

  y = position(y);
  z = position(z);
  y = y * -1.0;

  //-------------------------------------------------
  if (y > 30.0) {y = 30.0;}
  if (y < -30.0) {y = -30.0;}
  if (z > 25.0) {z = 25.0;}
  if (z < -25.0) {z = -25.0;}
  int Y = map(y, -90, 90, SERVOMIN, SERVOMAX);
  int Z = map(z, -90, 90, SERVOMIN, SERVOMAX);
  int pos[]={Y, Z};
  for (int i=0; i<numberOfServos; i++){
    myServos.setPWM(i, 0, pos[i]);
  }
  delay(10);  // delay zum Schonen der Servomotoren 
  for (int i=0; i<numberOfServos; i++){
    myServos.setPWM(i, 0, pos[i]);
  }

  //--------------------------------------------
  if(!SD.begin(CS_pin)){
    LED_R();
  } else {
    LED_G();
  }

  String dataString = String(altitude) + "," + String(x) + "," + String(y);
  File dataFile = SD.open("data.csv", FILE_WRITE);   
  if(dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  } else {
    LED_R();
  }
  delay(10);   // Zeit zwischen den Iterationen
}

float position(float pos){
  if(pos > 0.1){
    pos = 100*pos;
    pos = map(pos, 0, 100, 0, 90);      // positiver Bereich: LINKS
  }

  if(pos < -0.1){
    pos = 100*pos;
    pos = map(pos, 0, -100, 0, -90);    // negativer Bereich: RECHTS
  }
  return pos;
}

void LED_R() {
  digitalWrite(YEL, HIGH);
  digitalWrite(MAG, HIGH);
  digitalWrite(CYAN, LOW);
}

void LED_G() {
  digitalWrite(YEL, HIGH);
  digitalWrite(MAG, LOW);
  digitalWrite(CYAN, HIGH);
}