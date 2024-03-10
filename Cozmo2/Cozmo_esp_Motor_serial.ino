// Define pins for motor control
#define MOTOR1_PWM 4  // PWM pin for Motor 1
#define MOTOR1_DIR 5  // Direction pin for Motor 1
#define MOTOR2_PWM 14 // PWM pin for Motor 2
#define MOTOR2_DIR 12 // Direction pin for Motor 2

// Define variables for storing motor speed and direction
int motor1_speed = 0;
int motor2_speed = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Set motor control pins as outputs
  pinMode(MOTOR1_PWM, OUTPUT);
  pinMode(MOTOR1_DIR, OUTPUT);
  pinMode(MOTOR2_PWM, OUTPUT);
  pinMode(MOTOR2_DIR, OUTPUT);
}

void loop() {
  //demo();
  if (Serial.available() > 0) {
    // Read the incoming byte from serial port
    char command = Serial.read();

    // Handle different commands
    Serial.println(command);
    switch(command) {
      case 'F':
        // Move forward
        setMotorSpeedDirection(255, LOW, 255, LOW);
        break;
      case 'B':
        // Move backward
        setMotorSpeedDirection(0, HIGH, 0, HIGH);
        break;
      case 'L':
        // Turn left
        setMotorSpeedDirection(200, HIGH, 200, LOW);
        break;
      case 'R':
        // Turn right
        setMotorSpeedDirection(200, LOW, 200, HIGH);
        break;
      case 'S':
        // Stop
        setMotorSpeedDirection(0, LOW, 0, LOW);
        break;
      default:
        break;
    }
  }
}

void setMotorSpeedDirection(int speed1, int dir1, int speed2, int dir2) {
  // Set motor speed and direction
  analogWrite(MOTOR1_PWM, speed1);
  digitalWrite(MOTOR1_DIR, dir1);
  analogWrite(MOTOR2_PWM, speed2);
  digitalWrite(MOTOR2_DIR, dir2);
}

void demo() {
    // Move forward
    setMotorSpeedDirection(255, LOW, 255, LOW);
    Serial.println("Move forward");
    delay(5000);
    // Move backward
    
    setMotorSpeedDirection(0, HIGH, 0, HIGH);
    Serial.println("Move backward");
    delay(5000);
    
    // Turn left
    setMotorSpeedDirection(200, HIGH, 200, LOW);
    Serial.println("Turn left");
    delay(5000);
    // Turn right
    setMotorSpeedDirection(200, LOW, 200, HIGH);
    Serial.println("Turn right");
    delay(5000);
    // Stop
    setMotorSpeedDirection(0, LOW, 0, LOW);
    Serial.println("Stop");

}
