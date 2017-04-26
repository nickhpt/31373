#include <LiquidCrystal.h>
#include <Wire.h>
#include "DS1307.h"
#include "AFMotor.h"
// include the library code:


// Connect a stepper motor with 48 steps per revolution (7.5 degree)
// to motor port #2 (M3 and M4)
AF_Stepper motor(200*26.851, 2);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(11, 9, 3, 2, 1, 0);

DS1307 clock;//define a object of DS1307 class aka. RTC

// For motor Control
int currMotPos = 0;
int motPos = 5400;
int stepsPTick = 10;

//For Input pins
int lastButton =0;
int input1 = 0;
int numOfButtons = 3; // buttons connected to input 1
int buttonVals[3]= {289,509, 1020};

//For All Menus
int currMenu = 0;
String menuNames[4] = {"main", "men1","men2","calibration"};

//For Main Menu
uint8_t highMenu = 0; 

void setup() {

  lcd.begin(20, 4); // set up the LCD's number of columns and rows: 
  
  lcd.setCursor(0,0);
  lcd.print("Test2");
  
  clock.begin(); // Setup communication w. RTC 
  motor.setSpeed(4);  // Set MotorSpeed. Works w. 4 RPM
 
 
  //Setup input Button
  pinMode(input1, INPUT);            // Button 1 is the "turn button"
  analogWrite(input1, HIGH);        // set pullup resistor

}

void loop() {

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
 
  MotorUpdate();
  
  //printTime();
  
  lcd.setCursor(0,0);
  
  switch(currMenu){
    case 0: 
      MainMenu();
    break;

    case 3: 
      CalibrationMenu();
    break;
  }
  delay(10);
}

void MainMenu(){
  for(int i = 0; i < 3; i++){
    lcd.setCursor(1,i);
    lcd.print(menuNames[i+1]);
  }

  switch(checkButtonDown()){
    case 1:
      lcd.setCursor(0,highMenu);
      lcd.print(" ");
      highMenu --;
    break;

    case 2: 
      lcd.setCursor(0,highMenu);
      lcd.print(" ");
      highMenu ++;
    break;

    case 3: 
      lcd.clear();
      currMenu = highMenu-1;
    break;
  }
  
  highMenu %= 4; 
  
  lcd.setCursor(0,highMenu);
  lcd.print(">");
  
}

void CalibrationMenu(){
  for(int i = 0; i < 3; i++){
    lcd.setCursor(1,i);
    lcd.print(menuNames[i+1]);
  }

  switch(checkButtonDown()){
    case 1:
      lcd.setCursor(0,highMenu);
      lcd.print(" ");
      highMenu --;
    break;

    case 2: 
      lcd.setCursor(0,highMenu);
      lcd.print(" ");
      highMenu ++;
    break;

    case 3: 
      lcd.clear();
      currMenu = highMenu-1;
    break;
  }
  
  highMenu %= 4; 
  
  lcd.setCursor(0,highMenu);
  lcd.print(">");
  
}

int checkButton(){ //When a button press is detected, this value changes to said button for 1 step
    for(int q = 0; q < numOfButtons; q ++){
      if(analogRead(input1) >= buttonVals[q] -30 && analogRead(input1) <= buttonVals[q] +30){
        return q+1;
      }
  }
    
  return 0;
}

int checkButtonDown(){ //When a button press is detected, this value changes to said button for 1 step
  if(lastButton !=checkButton()){
    lastButton = checkButton();
    return checkButton();
  }else {
   return 0; 
  }
  
}

void MotorUpdate(){ //This updates the motor each tick, to match with the motPos value.
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
