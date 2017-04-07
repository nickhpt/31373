#include <LiquidCrystal.h>
#include <Wire.h>
#include "DS1307.h"
#include "AFMotor.h"
// include the library code:


// Connect a stepper motor with 48 steps per revolution (7.5 degree)
// to motor port #2 (M3 and M4)
AF_Stepper motor(200*26.851, 2);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(11, 9, 0, 1, 2, 3);

DS1307 clock;//define a object of DS1307 class aka. RTC

int currMotPos = 0;
int motPos = 5400;
int stepsPTick = 10;


void setup() {


  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.print("Hello, world!");
  
  clock.begin();
  
  motor.setSpeed(4);  // 1 rpm   

}

void loop() {

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);
 
  MotorUpdate();
  printTime();

}


void MotorUpdate(){
  int currDistance, stepsToGo;
  
  currDistance = abs(currMotPos - motPos);
  if(currDistance < stepsPTick){
    stepsToGo = currDistance;
  }else{
    stepsToGo = stepsPTick;
  }
  
  if(currMotPos < motPos){
    motor.step(stepsToGo, FORWARD, DOUBLE); 
    currMotPos += stepsToGo;
  }else if(currMotPos > motPos){
    motor.step(stepsToGo, BACKWARD, DOUBLE); 
    currMotPos -= stepsToGo;
  }
}

void printTime()
{
	clock.getTime();
        lcd.setCursor(0,3);
	lcd.print(clock.hour, DEC);
	lcd.print(":");
	lcd.print(clock.minute, DEC);
	lcd.print(":");
	lcd.print(clock.second, DEC);
	lcd.print(" ");
	lcd.print(clock.month, DEC);
	lcd.print("/");
	lcd.print(clock.dayOfMonth, DEC);
	lcd.print("/");
	lcd.print(clock.year+2000, DEC);

}
