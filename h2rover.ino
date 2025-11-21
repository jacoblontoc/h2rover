#include <DHT11.h>
#include "Adafruit_TCS34725.h"


// ------------------ DHT11 Sensor ------------------
//CHANGE CODE
DHT11 dht11(10);


// ================== PINOUT (TB6612FNG) ==================


//CHANGE CODE
const uint8_t PWMA = 9;
const uint8_t AIN1 = 8;
const uint8_t AIN2 = 7;


const uint8_t PWMB = 3;
const uint8_t BIN1 = 5;
const uint8_t BIN2 = 4;


const uint8_t STBY = 6;


// ================== USER CONFIGURATION ==================

// *** CHANGE THESE BEFORE EACH RUN ***
// Set your assigned team color: "RED", "BLUE", or "GREEN"
String ASSIGNED_TEAM_COLOR = "BLUE";

// Set your terrain difficulty choice: "PINK" (Easy), "BLUE" (Moderate), or "YELLOW" (Severe)
String TERRAIN_DIFFICULTY = "BLUE";
// *** END USER CONFIGURATION ***

// State machine stages
enum Stage {
  STAGE_1_BLACK_START,      // Follow BLACK to first intersection
  STAGE_2_TEAM_COLOR,       // Follow assigned team color
  STAGE_3_TERRAIN_COLOR,    // Follow terrain difficulty color
  STAGE_4_BLACK_FINAL       // Follow BLACK to sample
};

Stage currentStage = STAGE_1_BLACK_START;
String currentFollowColor = "BLACK";
String LEFT_BOUNDARY_COLOR  = "WHITE";
String RIGHT_BOUNDARY_COLOR = "ORANGE";


// Intersection colors (handled by navigation logic)
const String FIRST_INTERSECTION[] = {"RED", "BLUE", "GREEN"};
const String SECOND_INTERSECTION[] = {"PINK", "BLUE", "YELLOW"};


// ================== SPEED CONTROLS ==================


//CHANGE CODE
const uint8_t FWD_PWM = 30;
uint8_t PIVL_LEFT_PWM  = 40;
uint8_t PIVL_RIGHT_PWM = 40;
uint8_t PIVR_LEFT_PWM  = 40;
uint8_t PIVR_RIGHT_PWM = 40;


// ================== COLOR SENSOR ==================
Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);


// ================== MOTOR HELPERS ==================
inline uint8_t clampPWM(int v){ return (uint8_t)constrain(v,0,255); }


void leftMotorForward(uint8_t pwm){ digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); analogWrite(PWMA, clampPWM(pwm)); }
void leftMotorBackward(uint8_t pwm){ digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); analogWrite(PWMA, clampPWM(pwm)); }
void leftMotorStop(){ digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW); analogWrite(PWMA, 0); }


void rightMotorForward(uint8_t pwm){ digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); analogWrite(PWMB, clampPWM(pwm)); }
void rightMotorBackward(uint8_t pwm){ digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); analogWrite(PWMB, clampPWM(pwm)); }
void rightMotorStop(){ digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW); analogWrite(PWMB, 0); }


void motorsBrake(){
  leftMotorStop();
  rightMotorStop();
}


void motorsForward(uint8_t l, uint8_t r){
  leftMotorBackward(l);
  rightMotorBackward(r);
}
void pivotLeft(uint8_t l, uint8_t r){
  leftMotorBackward(l);
  rightMotorForward(r);
}
void pivotRight(uint8_t l, uint8_t r){
  leftMotorForward(l);
  rightMotorBackward(r);
}


// ================== COLOR DETECTION ==================
String detectColor(float r, float g, float b) {

  //CHANGE CODE
  if ((r >= 100 && r <= 120) && (g >= 75 && g <= 90) && (b >= 60 && b <= 75)) return "BLACK"; // low
  if ((r >= 95 && r <= 110) && (g >= 80 && g <= 95) && (b >= 50 && b <= 70)) return "WHITE"; // low
  if ((r >= 130 && r <= 145) && (g >= 60 && g <= 75) && (b >= 50 && b <= 65)) return "PINK"; // low
  if ((r >= 110 && r <= 175) && (g >= 50 && g <= 65) && (b >= 40 && b <= 55))   return "RED"; // low
  if ((r >= 110 && r <= 140) && (g >= 80  && g <= 115) && (b >= 30 && b <= 50))  return "YELLOW"; // low
  if ((r >= 70  && r <= 90) && (g >= 100 && g <= 140) && (b >= 55 && b <= 70))  return "GREEN"; // low
  if ((r >= 70  && r <= 110) && (g >= 70  && g <= 110) && (b >= 65 && b <= 120)) return "BLUE"; // low
  if ((r >= 120 && r <= 160) && (g >= 65  && g <= 85)  && (b >= 25 && b <= 55)) return "ORANGE"; // low
  return "UNKNOWN";

}

inline String readColorOnce(float &r, float &g, float &b) {
  tcs.setInterrupt(false); delay(60);
  tcs.getRGB(&r, &g, &b);
  tcs.setInterrupt(true);
  return detectColor(r, g, b);
}

// Update boundaries based on current stage and follow color
void updateBoundaries() {
  if (currentStage == STAGE_1_BLACK_START) {
    LEFT_BOUNDARY_COLOR = "WHITE";
    RIGHT_BOUNDARY_COLOR = "ORANGE";
  }
  else if (currentStage == STAGE_2_TEAM_COLOR) {
    if (currentFollowColor == "RED") {
      LEFT_BOUNDARY_COLOR = "GREEN";
      RIGHT_BOUNDARY_COLOR = "BLUE";
    }
    else if (currentFollowColor == "BLUE") {
      LEFT_BOUNDARY_COLOR = "RED";
      RIGHT_BOUNDARY_COLOR = "GREEN";
    }
    else if (currentFollowColor == "GREEN") {
      LEFT_BOUNDARY_COLOR = "BLUE";
      RIGHT_BOUNDARY_COLOR = "RED";
    }
  }
  else if (currentStage == STAGE_3_TERRAIN_COLOR) {
    if (currentFollowColor == "PINK") {
      LEFT_BOUNDARY_COLOR = "RED";
      RIGHT_BOUNDARY_COLOR = "GREEN";
    }
    else if (currentFollowColor == "BLUE") {
      LEFT_BOUNDARY_COLOR = "RED";
      RIGHT_BOUNDARY_COLOR = "GREEN";
    }
    else if (currentFollowColor == "YELLOW") {
      LEFT_BOUNDARY_COLOR = "WHITE";
      RIGHT_BOUNDARY_COLOR = "ORANGE";
    }
  }
  else if (currentStage == STAGE_4_BLACK_FINAL) {
    LEFT_BOUNDARY_COLOR = "WHITE";
    RIGHT_BOUNDARY_COLOR = "ORANGE";
  }
}

// Navigate intersection: pivot away from wrong colors toward target
// Layout: leftColor <- centerColor -> rightColor
void navigateIntersection(String detectedColor, String targetColor, String leftColor, String centerColor, String rightColor) {
  if (detectedColor == targetColor) {
    return; // Already on target, no action needed
  }
  
  // Determine position: left (0), center (1), right (2)
  int currentPos = -1;
  int targetPos = -1;
  
  if (detectedColor == leftColor) currentPos = 0;
  else if (detectedColor == centerColor) currentPos = 1;
  else if (detectedColor == rightColor) currentPos = 2;
  
  if (targetColor == leftColor) targetPos = 0;
  else if (targetColor == centerColor) targetPos = 1;
  else if (targetColor == rightColor) targetPos = 2;
  
  if (currentPos == -1 || targetPos == -1) return; // Invalid colors
  
  // Navigate: if current < target, pivot right; if current > target, pivot left
  if (currentPos < targetPos) {
    Serial.println("NAVIGATION: Pivot RIGHT to reach target");
    motorsBrake(); delay(150);
    pivotRight(PIVR_LEFT_PWM, PIVR_RIGHT_PWM);
    delay(400);
    motorsBrake(); delay(100);
  }
  else if (currentPos > targetPos) {
    Serial.println("NAVIGATION: Pivot LEFT to reach target");
    motorsBrake(); delay(150);
    pivotLeft(PIVL_LEFT_PWM, PIVL_RIGHT_PWM);
    delay(400);
    motorsBrake(); delay(100);
  }
}


// ================== SETUP ==================
void setup() {
  Serial.begin(9600);


  pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  motorsBrake();


  if (!tcs.begin()) {
    Serial.println("No TCS34725 found!");
    while(1);
  }


  Serial.println("System Ready: Color Sensor + DHT11");
}


// ================== MAIN LOOP ==================
void loop() {


  /************** READ DHT11 SENSOR **************/
  int temp = 0, hum = 0;
  int result = dht11.readTemperatureHumidity(temp, hum);


  if (result == 0) {
    Serial.print("TEMP: "); Serial.print(temp);
    Serial.print("°C  HUM: "); Serial.print(hum);
    Serial.println("%");
  } else {
    Serial.println(DHT11::getErrorString(result));
  }


  /************** READ COLOR SENSOR **************/
  float r, g, b;
  String color = readColorOnce(r, g, b);


  Serial.print("RGB: ");
  Serial.print(int(r)); Serial.print(",");
  Serial.print(int(g)); Serial.print(",");
  Serial.print(int(b));
  Serial.print(" -> ");
  Serial.print(color);
  Serial.print(" | Stage: ");
  Serial.print(currentStage);
  Serial.print(" | Following: ");
  Serial.println(currentFollowColor);


  /************** STAGE TRANSITIONS & INTERSECTION NAVIGATION **************/
  
  // Check if we've hit first intersection (while in STAGE_1)
  if (currentStage == STAGE_1_BLACK_START) {
    if (color == "RED" || color == "BLUE" || color == "GREEN") {
      Serial.println(">>> FIRST INTERSECTION DETECTED <<<");
      navigateIntersection(color, ASSIGNED_TEAM_COLOR, "RED", "BLUE", "GREEN");
      
      // Transition to STAGE_2 after navigation
      currentStage = STAGE_2_TEAM_COLOR;
      currentFollowColor = ASSIGNED_TEAM_COLOR;
      updateBoundaries();
      Serial.print(">>> TRANSITIONED TO STAGE 2: Following ");
      Serial.println(currentFollowColor);
      return;
    }
  }
  
  // Check if we've hit second intersection (while in STAGE_2)
  else if (currentStage == STAGE_2_TEAM_COLOR) {
    if (color == "PINK" || color == "YELLOW" || 
        (color == "BLUE" && currentFollowColor != "BLUE")) {
      Serial.println(">>> SECOND INTERSECTION DETECTED <<<");
      navigateIntersection(color, TERRAIN_DIFFICULTY, "PINK", "BLUE", "YELLOW");
      
      // Transition to STAGE_3 after navigation
      currentStage = STAGE_3_TERRAIN_COLOR;
      currentFollowColor = TERRAIN_DIFFICULTY;
      updateBoundaries();
      Serial.print(">>> TRANSITIONED TO STAGE 3: Following ");
      Serial.println(currentFollowColor);
      return;
    }
  }
  
  // Check if we need to transition to final BLACK (after terrain section)
  // This could be triggered by detecting BLACK after being on terrain color for a while
  // You may need to add additional logic here based on your course layout
  else if (currentStage == STAGE_3_TERRAIN_COLOR) {
    if (color == "BLACK") {
      // Check if we've completed terrain section (may need distance/time check)
      // For now, transition immediately when BLACK is detected
      currentStage = STAGE_4_BLACK_FINAL;
      currentFollowColor = "BLACK";
      updateBoundaries();
      Serial.println(">>> TRANSITIONED TO STAGE 4: Following BLACK to sample <<<");
    }
  }


  /************** LINE FOLLOWING LOGIC **************/
  
  // Follow current color
  if (color == currentFollowColor) {
    Serial.println("ACTION: FORWARD (FOLLOWING LINE)");
    motorsForward(FWD_PWM, FWD_PWM);
    delay(50);
    return;
  }


  // Handle boundaries
  if (color == LEFT_BOUNDARY_COLOR) {
    Serial.println("ACTION: TURN RIGHT (LEFT BOUNDARY)");
    motorsBrake(); delay(150);
    pivotRight(PIVR_LEFT_PWM, PIVR_RIGHT_PWM);
    delay(400);
    motorsBrake(); delay(100);
    return;
  }


  if (color == RIGHT_BOUNDARY_COLOR) {
    Serial.println("ACTION: TURN LEFT (RIGHT BOUNDARY)");
    motorsBrake(); delay(150);
    pivotLeft(PIVL_LEFT_PWM, PIVL_RIGHT_PWM);
    delay(400);
    motorsBrake(); delay(100);
    return;
  }


  /************** RECOVERY SEQUENCE **************/
  // Try moving forward to find the line
  motorsForward(FWD_PWM, FWD_PWM); delay(350);
  motorsBrake(); delay(100);


  float rN, gN, bN;
  String colorN = readColorOnce(rN, gN, bN);
  if (colorN == currentFollowColor) {
    Serial.println("ACTION: RECOVERED → FORWARD");
    motorsForward(FWD_PWM, FWD_PWM);
    return;
  }


  // Try pivoting left
  pivotLeft(PIVL_LEFT_PWM, PIVL_RIGHT_PWM); delay(300);
  motorsBrake(); delay(100);


  float rL, gL, bL;
  String colorL = readColorOnce(rL, gL, bL);
  if (colorL == currentFollowColor) {
    Serial.println("ACTION: RECOVERED LEFT → FORWARD");
    motorsForward(FWD_PWM, FWD_PWM);
    return;
  }


  // Try pivoting right
  pivotRight(PIVR_LEFT_PWM, PIVR_RIGHT_PWM); delay(600);
  motorsBrake(); delay(100);


  float rR, gR, bR;
  String colorR = readColorOnce(rR, gR, bR);
  if (colorR == currentFollowColor) {
    Serial.println("ACTION: RECOVERED RIGHT → FORWARD");
    motorsForward(FWD_PWM, FWD_PWM);
    return;
  }


  // If all recovery attempts fail, stop
  motorsBrake();
  delay(300);
}