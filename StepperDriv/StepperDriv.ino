#include <LiquidCrystal.h>
#include <Wire.h>
#include <EEPROM.h>
#include "DS1307.h"
#include "AFMotor.h"

// Connect a stepper motor with 200*26.851 steps per revolution
// to motor port #2 (M3 and M4)
AF_Stepper motor(200*26.851, 2);

// initialize the lcd library with the numbers of the interface pins
LiquidCrystal lcd(11, 9, 3, 2, 1, 0);

DS1307 clock;//define a object of DS1307 class aka. RTC

// For basic motor Control
int currMotPos = 0; // the current position of the motor
int motPos = 0; // The wanted position of the motor
int stepsPTick = 10; // How far should the motor turn each tick.

// For calibration of the motor position. The 8 different positions can be assigned (>-1) or unassigned (<=-1)
int calPositions[10];

//For Input pins
int lastButton =0; //last button Pressed
int input1 = 0; //The pin to read button presses from
int numOfButtons = 3; // buttons connected to input 1
int buttonVals[3]= {289,509, 1020}; // The values that correlate to different Buttons

//For All Menus
int currMenu = 0; //Each number correlates to different Menus, as seen in the following array. 
String menuNames[4] = {"main", "move","Set Positions ","Set Time"};

//For Main Menu
uint8_t highMenu = 0; //currently highlighted Menu Option

//For Calibrating menu
uint8_t calibrating = 0; //which value is currently being calibrated? 0 if none, 1 = val 0 etc.


/*
 * EEPROM addresses are set up as seen below. To avoid rollover or EEPROM failure causing trouble, true is set to 111 and false is everything else.
 * 
 * adr. 1 : has this unit been calibrated?
 * 
 * adr. 2->22  : calPositions. (as they are longer than a byte)
 * 
 * adr. 20 -> ?? : alarms and motorPositions
 * 
 */




void setup() {

  lcd.begin(20, 4); // set up the LCD's number of columns and rows:  
  lcd.setCursor(0,0);
  
  clock.begin(); // Setup communication w. RTC 
  motor.setSpeed(4);  // Set MotorSpeed. Works w. 4 RPM
 
  //Setup input Button
  pinMode(input1, INPUT);            // Button 1 is the "turn button"
  analogWrite(input1, HIGH);        // set pullup resistor

  //Check if already calibrated
  if(EEPROM.read(1) != 111){
      currMenu = 2;
      highMenu = 1;
      calibrating = 1;
  }else{
    loadCalibration();
  }
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

    case 2: 
      CalibrationMenu();
    break;
  }
  delay(10);
}





void MainMenu(){ // The controller for the main Menu
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
      highMenu = 0;
    break;
  }
  
  highMenu %= 4; 
  
  lcd.setCursor(0,highMenu);
  lcd.print(">");
  
}


/*
 * This function control the calibration of the motor-arduino combo
 */

void CalibrationMenu(){
  if(calibrating == 0){
   //If user haven't selected a slot yet
    lcd.setCursor(4,0);
    lcd.print("Select Slot");
    
    lcd.setCursor(1,3);
    lcd.print("back");

    for(int q = 0; q < 10; q++){
      lcd.setCursor((q*2),1);
      lcd.print(" ");
      lcd.setCursor((q*2)+1,1);
      lcd.print(q);
    }

    if (highMenu < 1){
      lcd.setCursor(0,3);
      lcd.print(">");

      if(checkButtonDown() == 3){
        saveCalibration();
        lcd.clear();
        currMenu = 0;
      }
      
    }else{
      lcd.setCursor(0,3);
      lcd.print(" ");

      lcd.setCursor((highMenu - 1)*2, 1);
      lcd.print(">");

      switch(checkButtonDown()){
        case 1:
          highMenu --;
        break;
        case 2: 
          highMenu ++;
        break;
        case 3: 
         calibrating = 1; 
        break;
      }
      
    }
  }else{
    lcd.setCursor(1,2);
    lcd.print("Press L/R to move");
    if(highMenu == 1){
      switch(checkButtonDown()){
        case 1:
          motPos -= stepsPTick;
        break;
        case 2: 
          motPos += stepsPTick;
        break;
        case 3: 
          currMotPos = 0;
          motPos = 0;
          calibrating = 0;
          lcd.clear();
        break;
      }
    }else{
      switch(checkButtonDown()){
        case 1:
          motPos -= stepsPTick;
        break;
        case 2: 
          motPos += stepsPTick;
        break;
        case 3: 
          calPositions[highMenu-1] = currMotPos 
          calibrating = 0;
          lcd.clear();
        break;
      }
    }
  }
}



/*
 * These functions save and load the motor calibrations 
 */

void saveCalibration (){ 
  for(int e = 0; e < 10; e ++){
    byte saveByte1 = calPositions[e] >> 8;
    byte saveByte2 = lowbyte(calPositions[e]);

    EEPROM.write(e*2+2, saveByte1);
    EEPROM.write(e*2+3, saveByte3);
  }
}


void loadCalibration (){
  for(int e = 0; e < 10; e ++){
    calPositions[e] = EEPROM.read(e*2+2) << 8;
    calPositions[e] += EEPROM.read(e*2+3);
  }
}


/*
 * These functions check whether or not the keys have been pressed.
 */
int checkButton(){ //When a button press is detected, this value changes to said button
    for(int q = 0; q < numOfButtons; q ++){
      if(analogRead(input1) >= buttonVals[q] -30 && analogRead(input1) <= buttonVals[q] +30){
        return q+1;
      }
  }
    
  return 0;
}



int checkButtonDown(){ //When a button press is detected, this value changes to said button for 1 tick
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
