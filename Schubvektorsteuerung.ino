#include "Wire.h"     // I2C Bibliothek 
#include "Servo.h"    // Servomotor Bibliothek

// Variabeln (Adressen des MPU-6050)
#define MPU6050_ADDR              0x68 // Alternativ setze AD0 HIGH  --> Address = 0x69
#define MPU6050_GYRO_CONFIG       0x1B // <-- Gyroskop spezielles Konfigurationsregister
#define MPU6050_ACCEL_CONFIG      0x1C
#define MPU6050_ACCEL_XOUT_H      0x3B
#define MPU6050_PWR_MGT_1         0x6B
#define MPU6050_SLEEP             0x06
// Implementierung der Messbereiche als Klasse
typedef enum {    // Beschleunigungsmessbereiche:
  MPU6050_ACC_RANGE_2G,  // +/- 2g (default)
  MPU6050_ACC_RANGE_4G,  // +/- 4g
  MPU6050_ACC_RANGE_8G,  // +/- 8g
  MPU6050_ACC_RANGE_16G // +/- 16g
} mpu6050_acc_range;
typedef enum {  // Gyroskopmessbereiche:
  MPU6050_GYR_RANGE_250,  // +/- 250 deg/s (default)
  MPU6050_GYR_RANGE_500,  // +/- 500 deg/s
  MPU6050_GYR_RANGE_1000, // +/- 1000 deg/s
  MPU6050_GYR_RANGE_2000  // +/- 2000 deg/s
} mpu6050_gyr_range;
// Variablen (Platzhalter für die gemessenen Werte der Sensoren)
int16_t accX, accY, accZ, gyroX, gyroY, gyroZ, tRaw; 
// Variable Messbereich
float gyr_range;
//Servo Deklarierung
Servo SerX;
Servo SerY;
// Positionsvariabeln
float c_posZ = 0; // z-Achse
float c_posX = 0; // x-Achse || Achsenwahl hängt von der Positionierung des Gyroskops in der Rakete ab. In diesem Fall ergaben die z- und x-Achse des Sensors eine Ebene, die parallel zur Erdoberfläche ist.
char result[7];
void setup() {  // Set Up
  Serial.begin(9600);   // Startet den seriellen Monitor
  Wire.begin();   // Startet die I2C Kommunikation
  MPU6050_wakeUp();   // "weckt" den MPU-6050 auf
  setAccRange(MPU6050_ACC_RANGE_16G); // ausgewählter Messbereich
  setGyrRange(MPU6050_GYR_RANGE_250); // ausgewählter Messbereich
  gyr_range = 2000.0;   // Variabel zum Anpassen des Messbereichs
  acc_range = 16.0;   // Variabel zum Anpassen des Messbereichs
  // Servo Ausgänge werden festgelegt
  SerX.attach(5);
  SerY.attach(6);
}
void loop() {   // loop
  MPU6050_wakeUp(); 
  Wire.beginTransmission(MPU6050_ADDR); // Beginne mit der Übertragung von Daten
  Wire.write(MPU6050_ACCEL_XOUT_H); // starte dabei mit dem Register ACCEL_XOUT_H
  Wire.endTransmission(false); // Der Parameter wird angeben, dass der Arduino einen Neustart macht 
                               // Als Resultat wird die Verbindung aufrecht erhalten
  Wire.requestFrom(MPU6050_ADDR, 7*2, true); // frage 7*2=14 Register an
  
  // "Wire.read()<<8 | Wire.read();" bedeutet, zwei Werte sind in derselben Variabel gespeichert
  accX = Wire.read()<<8 | Wire.read(); // lese Register: 0x3B (ACCEL_XOUT_H) und 0x3C (ACCEL_XOUT_L)
  accY = Wire.read()<<8 | Wire.read(); // lese Register: 0x3D (ACCEL_YOUT_H) und 0x3E (ACCEL_YOUT_L)
  accZ = Wire.read()<<8 | Wire.read(); // lese Register: 0x3F (ACCEL_ZOUT_H) und 0x40 (ACCEL_ZOUT_L)
  tRaw = Wire.read()<<8 | Wire.read(); // lese Register: 0x41 (TEMP_OUT_H) und 0x42 (TEMP_OUT_L) || die Temperatur wird ausgelesen, da ansonsten die Anzahl der ausgelesenen Register verändert, bzw. die nachvolgenden Register nach einer erneuten Auswahl eines Startegisters mit einer Zeitverzögerung gelesen werden würden. Um Komplikationen wie diese zu vermeiden, wird die Temperatur auch gelesen
  gyroX = Wire.read()<<8 | Wire.read(); // lese Register: 0x43 (GYRO_XOUT_H) und 0x44 (GYRO_XOUT_L)
  gyroY = Wire.read()<<8 | Wire.read(); // lese Register: 0x45 (GYRO_YOUT_H) und 0x46 (GYRO_YOUT_L)
  gyroZ = Wire.read()<<8 | Wire.read(); // lese Register: 0x47 (GYRO_ZOUT_H) und 0x48 (GYRO_ZOUT_L)
  MPU6050_sleep();  // der MPU-6050 wird in den Ruhezustand versetzt
  
  // Implementierung der Schubvektorsteuerung:

  //x-Axis:
  float r_gyroX = float(gyroX) / gyr_range + 1.0;   // current change of angle | with an average of 0 (but the noise creates a little bit of uncertainty)
  c_posX = c_posX + r_gyroX;
  // bedingt über die maximale Achsendrehung in der Schubvektorsteuerung, muss auch bei größerer Neigung als 15°, eine Neigung von 15° an den Servomotor weitergegeben werden um die Konstruktion zu schützen:
  if (c_posX < -15.0){c_posX = -15.0;}  
  if (c_posX > 15.0){c_posX = 15.0;}

  //z-Axis:
  float r_gyroZ = float(gyroZ) / gyr_range;   // current change of angle | with an average of 0 (but the noise creates a little bit of uncertainty)
  c_posZ = c_posZ + r_gyroZ;
  // bedingt über die maximale Achsendrehung in der Schubvektorsteuerung, muss auch bei größerer Neigung als 15°, eine Neigung von 15° an den Servomotor weitergegeben werden um die Konstruktion zu schützen:
  if (c_posZ < -15.0){c_posZ = -15.0;}  
  if (c_posZ > 15.0){c_posZ = 15.0;}

  // servo manuvering: x-Axis:

  // servo manuvering: y-Axis:
  // 90° bedeutet, das Triebwerk befindet sich senkrecht zum Boden
  SerY.write(90.0 - c_posZ/3.0);   // Die y-Achse der Konstruktion ist in diesem Fall die Z-Achse des Gyroskops  
  SerX.write(90.0 + c_posX/3.0); // "+" ist für Veränderung der Servoausrichtung
  delay(10);    // delay of each iteration
}
// Konfigurationsfunktionen:
char* toStr(int16_t i) { // int16 zu String und Ausgabeformat
  sprintf(result, "%6d", i);
  return result;
}
void setAccRange(mpu6050_acc_range range){
  writeRegister(MPU6050_ACCEL_CONFIG, range<<3);
}
void setGyrRange(mpu6050_gyr_range range){
  writeRegister(MPU6050_GYRO_CONFIG, range<<3);
}
void MPU6050_wakeUp(){
  writeRegister(MPU6050_PWR_MGT_1, 0); 
  delay(30); // der Sensor braucht eine gewisse Zeit, bis er einsatzbereit ist
}
void MPU6050_sleep(){
  writeRegister(MPU6050_PWR_MGT_1, 1<<MPU6050_SLEEP); 
}
void writeRegister(uint16_t reg, byte value){
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg); 
  Wire.write(value); 
  Wire.endTransmission(true);
}