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

struct timeStruct{
  uint8_t wd;
  uint8_t hr;
  uint8_t mi;
  uint8_t pos;
  };

//The main alarms
timeStruct alarms[10];

// the Timestruct used for simplicity in menus.
timeStruct menuTime; 


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
String menuNames[5] = {"main","Move to Position", "Set Alarm","Set Positions ","Set Time"};

//For Main Menu
uint8_t highMenu = 0; //currently highlighted Menu Option
uint8_t highMenu2 = 0; //currently highlighted Menu Option (extra for validation menu)


//For Set Alarm Menu
uint8_t settingA = 0; //is an alarm currently being set? 0 if none, 1 = val 0 etc.

//For Calibrating Menu
uint8_t calibrating = 0; //is positions currently being calibrated? 0 if none, 1 = val 0 etc.

//For Set Time Menu
uint8_t timeSet = 0; // Setting: 0 = hour, 1 = minute, 2 = weekday.
uint8_t timeSetValue = 0;


/*
 * EEPROM addresses are set up as seen below. To avoid rollover or EEPROM failure causing trouble, true is set to 111 and false is everything else.
 * 
 * adr. 1 : has this unit been calibrated?
 * 
 * adr. 2->22  : calPositions. (as they are longer than a byte)
 * 
 * adr. 40 -> ?? : alarms and motorPositions
 * 
 */

int alarmAddr = 40;




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
    LoadCalibration();
  }

  LoadAlarms();
}





void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  AlarmUpdate();
  MotorUpdate();
  
  //PrintTime();
  
  lcd.setCursor(0,0);
  
  switch(currMenu){
    case 0: 
      MainMenu();
    break;

    case 1: 
      MoveToPosMenu();
    break;

    case 2: 
      SetAlarmMenu();
    break;
    
    case 3: 
      CalibrationMenu();
    break;

    case 4:
      SetTimeMenu();
    break;
  }
  delay(10);
}



/*
 * Main Menu Function
 */


void MainMenu(){ // The controller for the main Menu
  for(int i = 0; i < 4; i++){
    lcd.setCursor(1,i);
    lcd.print(menuNames[i+1]);
  }
  switch(CheckButtonDown()){
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
      currMenu = highMenu+1;
      highMenu = 0;
      return;
    break;
  }
  
  highMenu %= 4; 
  
  lcd.setCursor(0,highMenu);
  lcd.print(">");
  
}

/*
 * Move To predefined Position Menu
 */
void MoveToPosMenu(){
    lcd.setCursor(4,0);
    lcd.print("Select  Slot");
    
    lcd.setCursor(1,3);
    lcd.print("back");

    for(int q = 0; q < 10; q++){
      lcd.setCursor((q*2),1);
      lcd.print(" ");
      lcd.setCursor((q*2)+1,1);
      lcd.print(q);
    }

    if (highMenu == 0){
      lcd.setCursor(0,3);
      lcd.print(">"); 
    }else{
      lcd.setCursor(0,3);
      lcd.print(" ");
      lcd.setCursor((highMenu - 1)*2, 1);
      lcd.print(">");
    }


      switch(CheckButtonDown()){
        case 1:
          highMenu --;
          highMenu %= 11;
        break;
        case 2: 
          highMenu ++;
          highMenu %= 11;
        break;
        case 3: 
          if(highMenu == 0){
            SaveCalibration();
            lcd.clear();
            currMenu = 0;
            return;
          }
          motPos = calPositions[highMenu-1];
        break; 
      }
}

/*
 * Set alarm Menu
 */

void SetAlarmMenu(){
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

    if (highMenu == 0){
      lcd.setCursor(0,3);
      lcd.print(">");
      
    }else{
      lcd.setCursor(0,3);
      lcd.print(" ");
      lcd.setCursor((highMenu - 1)*2, 1);
      lcd.print(">");
    }

      switch(CheckButtonDown()){
        case 1:
          highMenu --;
          highMenu %= 11;
        break;
        case 2: 
          highMenu ++;
          highMenu %= 11;
        break;
        case 3: 
         
          if(highMenu == 0){
            lcd.clear();
            currMenu = 0;
            return;
           }
        calibrating = 1;
        lcd.clear();
        break;
      }
      
    
  }else if(calibrating == 1){
      lcd.setCursor(3,0);
      if(alarms[highMenu-1].hr < 10){
        lcd.print("0");
      }
      lcd.print(alarms[highMenu-1].hr,DEC);
      lcd.print(":"); 
      if(alarms[highMenu-1].hr < 10){
        lcd.print("0");
      }
      lcd.print(alarms[highMenu-1].mi,DEC);
      lcd.print(" "+DayOfWeek(alarms[highMenu-1].wd)); 
      lcd.print(", Pos ");
      lcd.print(alarms[highMenu-1].pos,DEC);
      lcd.setCursor(1,2);
      lcd.print("edit");
      lcd.setCursor(1,3);
      lcd.print("back");
      highMenu2 %= 2;
    
      if(highMenu2 == 0){
        lcd.setCursor(0,2);
        lcd.print(">");
        lcd.setCursor(0,3);
        lcd.print(" ");
      }else{
        lcd.setCursor(0,2);
        lcd.print(" ");
        lcd.setCursor(0,3);
        lcd.print(">");
      }

      switch(CheckButtonDown()){
          case 1:
            highMenu2 --;
          break;
          case 2: 
            highMenu2 ++;
          break;
          case 3: 
            if(highMenu2 == 0){
              calibrating = 2;
            }else{
              calibrating = 0;
              highMenu2 = 0;
            }
            lcd.clear();
          break;
        }
  }else if(calibrating == 2){
    if(timeSet == 4){
      SaveAlarms();
      lcd.clear();
      calibrating = 0;
      timeSet = 0;
      currMenu = 0;
      highMenu = 0;
      return;
    }
  
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("<");
    lcd.setCursor(17,1);
    lcd.print(">");

    switch(timeSet){
      case 0:
        timeSetValue %= 24;
        lcd.setCursor(8,1);
        lcd.print("hour");
        lcd.setCursor(9,2);
        lcd.print(timeSetValue);
        alarms[highMenu-1].hr = timeSetValue;
      break;
      case 1:
        timeSetValue %= 60;
        lcd.setCursor(7,1);
        lcd.print("minute");
        lcd.setCursor(9,2);
        lcd.print(timeSetValue);
        alarms[highMenu-1].mi = timeSetValue;
      break;
      case 2:
        timeSetValue %= 8;
        lcd.setCursor(9,1);
        lcd.print("day");
        lcd.setCursor(9,2);
        lcd.print(DayOfWeek(timeSetValue));
        alarms[highMenu-1].wd = timeSetValue;
      break;
      case 3:
        timeSetValue %= 10;
        lcd.setCursor(9,1);
        lcd.print("position");
        lcd.setCursor(9,2);
        lcd.print(timeSetValue);
        alarms[highMenu-1].pos = timeSetValue;
      break;
    }
  
    switch(CheckButtonDown()){
        case 1:
          timeSetValue --;
        break;
        case 2: 
          timeSetValue ++;
        break;
        case 3: 
          timeSet ++;
          timeSetValue = 0;
          lcd.clear();
        break;
      }
  }
}

/*
 * This function control the calibration of the motor-arduino combo
 */

void CalibrationMenu(){
  if(calibrating == 0){
   //If user haven't selected a slot yet
    lcd.setCursor(4,0);
    lcd.print("Select  Slot");
    
    lcd.setCursor(1,3);
    lcd.print("back");

    for(int q = 0; q < 10; q++){
      lcd.setCursor((q*2),1);
      lcd.print(" ");
      lcd.setCursor((q*2)+1,1);
      lcd.print(q);
    }

    if (highMenu == 0){
      lcd.setCursor(0,3);
      lcd.print(">");

      
    }else{
    
      lcd.setCursor(0,3);
      lcd.print(" ");
      lcd.setCursor((highMenu - 1)*2, 1);
      lcd.print(">");
    }


      switch(CheckButtonDown()){
        case 1:
          highMenu --;
          highMenu %= 11;
        break;
        case 2: 
          highMenu ++;
          highMenu %= 11;
        break;
        case 3: 
        if(highMenu == 0){
           SaveCalibration();
           lcd.clear();
           currMenu = 0;
           return;
         }
         lcd.clear();
         calibrating = 1;
         return;
        break;
      }
      
    
  }else{
    lcd.clear();
    lcd.setCursor(1,2);
    lcd.print("Press L/R to move");
    lcd.setCursor(10,1);
    lcd.print(highMenu-1);
    if(highMenu == 1){
      switch(CheckButton()){
        case 1:
          motPos -= stepsPTick;
        break;
        case 2: 
          motPos += stepsPTick;
        break;
      }
      if(CheckButtonDown() == 3){
          currMotPos = 0;
          motPos = 0;
          calibrating = 0;
          lcd.clear();
      }
    }else{
      switch(CheckButton()){
        case 1:
          motPos -= stepsPTick;
        break;
        case 2: 
          motPos += stepsPTick;
        break;
      }
      
      if(CheckButtonDown() == 3){
          calPositions[highMenu-1] = currMotPos;
          calibrating = 0;
          lcd.clear();
      }
    }
  }
    
}


/*
 * SetTime Menu
 */

void SetTimeMenu(){
if(calibrating == 1){
  if(timeSet == 3){
    lcd.clear();
    clock.fillByHMS(menuTime.hr,menuTime.mi,30);
    clock.fillDayOfWeek(menuTime.wd);
    clock.setTime();
    calibrating = 0;
    currMenu = 0;
    timeSet = 0;
    highMenu = 0;
    return;
  }
  
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("<");
    lcd.setCursor(17,1);
    lcd.print(">");

    switch(timeSet){
      case 0:
        timeSetValue %= 24;
        lcd.setCursor(8,1);
        lcd.print("hour");
        lcd.setCursor(9,2);
        lcd.print(timeSetValue);
        menuTime.hr = timeSetValue;
      break;
      case 1:
        timeSetValue %= 60;
        lcd.setCursor(7,1);
        lcd.print("minute");
        lcd.setCursor(9,2);
        lcd.print(timeSetValue);
        menuTime.mi = timeSetValue;
      break;
      case 2:
        timeSetValue %= 8;
        lcd.setCursor(9,1);
        lcd.print("day");
        lcd.setCursor(9,1);
        lcd.print(DayOfWeek(timeSetValue));
        menuTime.wd = timeSetValue;
      break;
    }
  
    switch(CheckButtonDown()){
          case 1:
            timeSetValue --;
          break;
          case 2: 
            timeSetValue ++;
          break;
          case 3: 
            timeSet ++;
            timeSetValue = 0;
            lcd.clear();
          break;
        }
  }else{
    PrintTime(3, 0);
    lcd.setCursor(1,2);
    lcd.print("edit");
    lcd.setCursor(1,3);
    lcd.print("back");
    highMenu2 %= 2;
    
    if(highMenu2 == 0){
      lcd.setCursor(0,2);
      lcd.print(">");
      lcd.setCursor(0,3);
      lcd.print(" ");
    }else{
      lcd.setCursor(0,2);
      lcd.print(" ");
      lcd.setCursor(0,3);
      lcd.print(">");
    }

    switch(CheckButtonDown()){
          case 1:
            highMenu2 --;
          break;
          case 2: 
            highMenu2 ++;
          break;
          case 3: 
            if(highMenu2 == 0){
              calibrating = 1;
            }else{
              currMenu = 0;
            }
            highMenu2 = 0;
            lcd.clear();
          break;
        }
  }
}



/*
 * These functions save and load the motor calibrations 
 */

void SaveCalibration (){ 
  for(int e = 0; e < 10; e ++){
    byte saveByte1 = calPositions[e] >> 8;
    byte saveByte2 = lowByte(calPositions[e]);

    EEPROM.write(e*2+2, saveByte1);
    EEPROM.write(e*2+3, saveByte2);
    EEPROM.write(1, 111);
  }
}


void LoadCalibration (){
  for(int e = 0; e < 10; e ++){
    calPositions[e] = EEPROM.read(e*2+2) << 8;
    calPositions[e] += EEPROM.read(e*2+3);
  }
}


/*
 * These functions check whether or not the keys have been pressed.
 */
int CheckButton(){ //When a button press is detected, this value changes to said button
    for(int q = 0; q < numOfButtons; q ++){
      if(analogRead(input1) >= buttonVals[q] -30 && analogRead(input1) <= buttonVals[q] +30){
        return q+1;
      }
  }
    
  return 0;
}



int CheckButtonDown(){ //When a button press is detected, this value changes to said button for 1 tick
  if(lastButton !=CheckButton()){
    lastButton = CheckButton();
    return CheckButton();
  }else {
   return 0; 
  }
  
}



void AlarmUpdate(){
  clock.getTime();

  for (int i = 0; i < 10; i++){
    if(clock.hour == alarms[i].hr && clock.minute == alarms[i].mi){
      if(alarms[i].wd == 0 || alarms[i].wd == clock.dayOfWeek){
        if(alarms[i].hr == 0 && alarms[i].mi == 0 && alarms[i].wd == 0 && alarms[i].pos == 0){
          return;
        }
        motPos = calPositions[alarms[i].pos];
      }
    }
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


void SaveAlarms(){
    for(int i = 0; i < 10; i++){
      int addrn = i*4 + alarmAddr; //The andress number, from which the positions will be loaded from. 
      EEPROM.write(addrn,alarms[i].wd); //Write the weekday (0-8)
      EEPROM.write(addrn+1,alarms[i].hr); //Write the hour (0-23)
      EEPROM.write(addrn+2,alarms[i].mi); //Write the minute (0-59)
      EEPROM.write(addrn+3,alarms[i].pos); //Write to position (0-9)
    }
}

void LoadAlarms(){

  for(int i = 0; i < 10; i++){
      int addrn = i*4+ alarmAddr; //The andress number, from which the positions will be loaded from. 
      alarms[i].wd  = EEPROM.read(addrn);    //read the weekday (0-8)
      alarms[i].hr  = EEPROM.read(addrn+1); //read the hour (0-23)
      alarms[i].mi  = EEPROM.read(addrn+2); //read the minute (0-59)
      alarms[i].pos = EEPROM.read(addrn+3); //read to position (0-9)

    }

}

void PrintTime(int x, int y)
{
	clock.getTime();
  lcd.setCursor(x,y);
	lcd.print(clock.hour, DEC);
	lcd.print(":");
	lcd.print(clock.minute, DEC); 

  lcd.print(" ");
  lcd.print(DayOfWeek(clock.dayOfWeek));
}

String DayOfWeek(int wd){
  switch(wd){
    case 0:
      return "all";
    break;
    case 1:
      return "MON";
    break;
    case 2:
      return "TUE";
    break;
    case 3:
      return "WED";
    break;
    case 4:
      return "THU";
    break;
    case 5:
      return "FRI";
    break;
    case 6:
      return "SAT";
    break;
    case 7:
      return "SUN";
    break;
  }
  return "error";
}
