// ===================================================
//  RGB COLOR SENSOR TEST (Adafruit TCS34725)
// ===================================================


#include "Adafruit_TCS34725.h"


// Create sensor object
Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);


// ---------------- COLOR DETECTION (YOUR MAPPING) ----------------
String detectColor(float r, float g, float b) {
  if ((r >= 100 && r <= 120) && (g >= 75 && g <= 95) && (b >= 60 && b <= 75)) return "BLACK"; // low
  if ((r >= 95 && r <= 110) && (g >= 85 && g <= 95) && (b >= 50 && b <= 70)) return "WHITE"; // low
  if ((r >= 150 && r <= 170) && (g >= 55 && g <= 70) && (b >= 50 && b <= 65)) return "PINK"; // low
  if ((r >= 150 && r <= 180) && (g >= 45 && g <= 60) && (b >= 40 && b <= 55))   return "RED"; // low
  if ((r >= 115 && r <= 130) && (g >= 85  && g <= 100) && (b >= 30 && b <= 45))  return "YELLOW"; // low
  if ((r >= 95  && r <= 115) && (g >= 85 && g <= 110) && (b >= 60 && b <= 75))  return "GREEN"; // low
  if ((r >= 75  && r <= 100) && (g >= 75  && g <= 100) && (b >= 85 && b <= 110)) return "BLUE"; // low
  if ((r >= 150 && r <= 165) && (g >= 60  && g <= 70)  && (b >= 30 && b <= 50)) return "ORANGE"; // low
  return "UNKNOWN";
}


// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);
  Serial.println("=== TCS34725 RGB Sensor Test ===");


  if (!tcs.begin()) {
    Serial.println("No TCS34725 found ... check wiring and power!");
    while (1);
  }


  Serial.println("TCS34725 initialized successfully.");
  Serial.println("Move colored objects in front of the sensor...");
  Serial.println("===============================================");
}


// ---------------- LOOP ----------------
void loop() {
  float r, g, b;


  // Enable sensor LED, read color
  tcs.setInterrupt(false);
  delay(60);
  tcs.getRGB(&r, &g, &b);
  tcs.setInterrupt(true);


  // Determine color name
  String color = detectColor(r, g, b);


  // Optional: color temperature and lux (if needed)
  uint16_t c, red, green, blue;
  tcs.getRawData(&red, &green, &blue, &c);
  uint16_t colorTemp = tcs.calculateColorTemperature_dn40(red, green, blue, c);
  uint16_t lux = tcs.calculateLux(red, green, blue);


  // Print readings
  Serial.print("R="); Serial.print(int(r));
  Serial.print(" G="); Serial.print(int(g));
  Serial.print(" B="); Serial.print(int(b));
  Serial.print(" | Temp="); Serial.print(colorTemp);
  Serial.print("K | Lux="); Serial.print(lux);
  Serial.print(" -> Detected: ");
  Serial.println(color);


  delay(500);
}



