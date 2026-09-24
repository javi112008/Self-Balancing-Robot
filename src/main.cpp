#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <EEPROM.h>

void handleCommands();
void moveForward(int speed);
void moveBackward(int speed);
void motorStop();

MPU6050 mpu;

// Motor control pins
const int IN1 = 2;
const int IN2 = 3;
const int ENA = 10;
const int IN3 = 6;
const int IN4 = 7;
const int ENB = 11;

// PID tuning constants (defaults, will be overwritten by EEPROM if saved!!)
float Kp = 49.0;
float Ki = 0.2;
float Kd = 0.35;

// Target angle
float setpoint = 6.9;

// EEPROM addresses ( float takes 4 bytes)
int addrKp = 0;
int addrKi = 4;
int addrKd = 8;
int addrSetpoint = 12;

// State variables
double angle = 0.0;
double accAngle = 0.0;
double gyroRate = 0.0;
double error = 0.0, prevError = 0.0;
double integral = 0.0;
double derivative = 0.0;
double output = 0.0;
unsigned long prevTime;

// MPU6050 raw data
int16_t ax, ay, az, gx, gy, gz;

// Filter constant
const double alpha = 0.98;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu.initialize();

  // Set offsets for the MPU6050
  mpu.setXGyroOffset(-479);
  mpu.setYGyroOffset(84);
  mpu.setZGyroOffset(15);
  mpu.setZAccelOffset(1638);

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  delay(10); 
  prevTime = millis();

  // Load saved values from EEPROM
  EEPROM.get(addrKp, Kp);
  EEPROM.get(addrKi, Ki);
  EEPROM.get(addrKd, Kd);
  EEPROM.get(addrSetpoint, setpoint);

  Serial.println("Loaded saved values from EEPROM:");
  Serial.print("Kp="); Serial.print(Kp);
  Serial.print(" Ki="); Serial.print(Ki);
  Serial.print(" Kd="); Serial.print(Kd);
  Serial.print(" Target Angle="); Serial.println(setpoint);
}

void loop() {  // Read MPU6050 data and compute PID output
  handleCommands();

  unsigned long currTime = millis();
  double dt = (currTime - prevTime) / 1000.0;
  prevTime = currTime;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  accAngle = atan2(-ax, az) * 180.0 / PI; //CONVERT TO DEGREES
  gyroRate = gy / 131.0;
  // Complementary filter to combine accelerometer and gyroscope data
  angle = alpha * (angle + gyroRate * dt) + (1 - alpha) * accAngle;

  // PID calculations
  error = setpoint - angle;
  integral += error * dt;
  integral = constrain(integral, -100, 100);

  derivative = (error - prevError) / dt;
  prevError = error;

  output = Kp * error + Ki * integral + Kd * derivative;
  //clamp output to motor speed range
  output = constrain(output, -255, 255);

  if (abs(output) < 10) {
    motorStop();
  } else if (angle > -30 && angle < 30) {
    if (output > 0) moveBackward(output);
    else moveForward(-output);
  } else {
    motorStop();
    integral = 0;
  }

  Serial.print("Target angle:");
  Serial.print(setpoint);
  Serial.print("    | Angle: ");
  Serial.print(angle);
  Serial.print(" | Output: ");
  Serial.print(output);
  Serial.print("    | Current PID Values:");
  Serial.print("|Kp="); Serial.print(Kp);
  Serial.print(" |Ki="); Serial.print(Ki);
  Serial.print(" |Kd="); Serial.println(Kd);
  delay(30);
}

void handleCommands() {
  if (Serial.available() > 0) {
    String userIn = Serial.readStringUntil('\n'); 
    userIn.trim();

    if (userIn == "pid") {
      motorStop();
      Serial.println("Enter Kp, Ki, Kd:");
      while (Serial.available() == 0);
      Kp = Serial.parseFloat();
      Ki = Serial.parseFloat();
      Kd = Serial.parseFloat();

      // Save to EEPROM
      EEPROM.put(addrKp, Kp);
      EEPROM.put(addrKi, Ki);
      EEPROM.put(addrKd, Kd);

      Serial.println("PID values updated and saved!");
    }
    else if (userIn == "angle") {
      motorStop();
      Serial.println("Enter new target angle:");
      while (Serial.available() == 0);
      setpoint = Serial.parseFloat();

      // Save to EEPROM
      EEPROM.put(addrSetpoint, setpoint);

      Serial.print("New target angle saved: ");
      Serial.println(setpoint);
    }
  }
}

// control motor functions
void moveForward(int speed) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); analogWrite(ENA, speed);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENB, speed);
}

void moveBackward(int speed) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); analogWrite(ENA, speed);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); analogWrite(ENB, speed);
}

void motorStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
}
