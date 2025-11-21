// ===================================================
//  MOTOR FUNCTION TESTER for TB6612FNG
// ===================================================


const uint8_t PWMA = 9;    // Left motor speed (PWM)
const uint8_t AIN1 = 8;
const uint8_t AIN2 = 7;


const uint8_t PWMB = 3;    // Right motor speed (PWM)
const uint8_t BIN1 = 5;
const uint8_t BIN2 = 4;


const uint8_t STBY = 6;    // Standby pin (must be HIGH)




// ---------------- Motor Helper ----------------
inline uint8_t clampPWM(int v){ return (uint8_t)constrain(v,0,255); }


void leftMotorForward(uint8_t pwm){ digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); analogWrite(PWMA, clampPWM(pwm)); }
void leftMotorBackward(uint8_t pwm){ digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); analogWrite(PWMA, clampPWM(pwm)); }
void leftMotorStop(){ digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW); analogWrite(PWMA, 0); }


void rightMotorForward(uint8_t pwm){ digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); analogWrite(PWMB, clampPWM(pwm)); }
void rightMotorBackward(uint8_t pwm){ digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); analogWrite(PWMB, clampPWM(pwm)); }
void rightMotorStop(){ digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW); analogWrite(PWMB, 0); }


void motorsBrake(){
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
  analogWrite(PWMA, 0); analogWrite(PWMB, 0);
}


// Match your main wiring (backward == forward motion)
void motorsForward(uint8_t leftPWM, uint8_t rightPWM){
  leftMotorForward(leftPWM);
  rightMotorForward(rightPWM);
}
void motorsBackward(uint8_t leftPWM, uint8_t rightPWM){
  leftMotorBackward(leftPWM);
  rightMotorBackward(rightPWM);
}
void pivotLeft(uint8_t leftPWM, uint8_t rightPWM){
  leftMotorBackward(leftPWM);
  rightMotorForward(rightPWM);
}
void pivotRight(uint8_t leftPWM, uint8_t rightPWM){
  leftMotorForward(leftPWM);
  rightMotorBackward(rightPWM);
}




// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);
  pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  motorsBrake();


  Serial.println("=== MOTOR FUNCTION TEST ===");
  Serial.println("Sequence: Forward -> Backward -> Pivot Left -> Pivot Right -> Brake");
  delay(2000);
}




// ---------------- LOOP ----------------
void loop() {
  uint8_t testPWM = 100; // Adjust if needed


  Serial.println("FORWARD...");
  motorsForward(testPWM, testPWM);
  delay(2000);


  Serial.println("BACKWARD...");
  motorsBackward(testPWM, testPWM);
  delay(2000);


  Serial.println("PIVOT LEFT...");
  pivotLeft(testPWM, testPWM);
  delay(2000);


  Serial.println("PIVOT RIGHT...");
  pivotRight(testPWM, testPWM);
  delay(2000);


  Serial.println("BRAKE...");
  motorsBrake();
  delay(2000);


  Serial.println("Cycle complete — repeating...");
  delay(3000);
}



