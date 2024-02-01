#include "Wire.h" 

 //Barometer:
  #include <Adafruit_BMP085.h>    // BMP-Barometer Bibliothek
  #define seaLevelPressure_hPa 1013.25

  // Datenspeicherung:
  #include <SD.h>   //Datenspeicherungsbibliothek
  int CS_pin = 10;
  int pow_pin = 8;  // Power Pin für die Stromversorgung der SD-Karte
  // Deklarierung des Sensors
  Adafruit_BMP085 bmp;
void setup() {
  Serial.begin(9600); // Für das Debugging || Serieller Monitor
  Wire.begin();
  //CS Pin ist ein Ausgang
  pinMode(CS_pin, OUTPUT);
  //Karte wird ihren Strom vom Pin 10 ziehen. 
  pinMode(pow_pin, OUTPUT); // Daher wird dieser hier HIGH gesetzt
  // Überprüfe, ob die SD Karte bereit ist
  if(!SD.begin(CS_pin)){
    Serial.println("Card Failed");  // Wenn nicht, schreibe "Card Failed"
    return;
  }
  Serial.println("Card Ready");   // Wenn ja, schreibe "Card Ready"

  //Barometer Set-Up:
  if (!bmp.begin()) {   // Wenn das Barometer nicht verbunden ist, schreibe...
  Serial.println("Could not find a valid BMP085 sensor, check wiring!"); 
  while (1) {}
  }
}
void loop() {
  
// Datenspeicherung:
  //Schreibe:
  //Schreibe die Sensorendaten auf den Datenstring
  String dataString = String(bmp.readPressure());
  //Öffne eine Datei auf der SD-Karte
  //Es kann nur eine Datei zur selben Zeit geöffnet werden
  File dataFile = SD.open("data.csv", FILE_WRITE);    //Der Dateiname muss EINE zusammenhängende Zeichenkette mit Dateityp am Ende durch einen Punkt abgetrennt

  if(dataFile)
  {
    dataFile.println(dataString);
    dataFile.close();
  } 
  else 
  {
    Serial.println("Couldn't access file");
  }
  delay(10);    // Wiederhole das oben stehende alle 0,01 Sekunden
}