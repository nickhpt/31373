
#include "AFMotor.h"

// Connect a stepper motor with 48 steps per revolution (7.5 degree)
// to motor port #2 (M3 and M4)
AF_Stepper motor(200*26.851, 2);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Stepper test!");

  motor.setSpeed(4);  // 10 rpm   
}

void loop() {

  Serial.println("Double coil steps");
  motor.step(5400/4, FORWARD, DOUBLE); 
  motor.step(5400/4, BACKWARD, DOUBLE);

}
