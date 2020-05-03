#include <Stepper.h>
#include "Nextion.h"

//setting up stepper motor
const float STEPS_PER_REV = 32;
const float GEAR_RED = 64;

Stepper sm(STEPS_PER_REV, 8, 10, 9, 11);

int position = 0;
int newPosition = 0;
int steps;

//setting up nextion elements
NexPage page0 = NexPage(0, 0, "page0");
NexPage page1 = NexPage(1, 0, "page1");
NexPage page2 = NexPage(2, 0, "page2");
NexText input = NexText(0, 14, "t0");
NexText currPos = NexText(0, 22, "t1"); 
NexButton calibration = NexButton(0, 16, "b13");
NexButton enter = NexButton(0, 13, "b15");
NexButton updateSpeed = NexButton(2, 4, "b2");
NexText speed = NexText(2, 7, "t1");

NexTouch *nex_listen_list[] = { 
  &calibration, 
  &enter,
  &updateSpeed,
  NULL
};

char inputBuffer[100] = {'0'};
char speedBuffer[100] = {'2', '0', '0'};
char fractionBuffer[100] = {'0'};

//position = newPosition
void calibrationPopCallback(void *ptr) {
  parseInput();
}

//position = newPosition & motor rotate to newPosition
void enterPopCallback(void *ptr) {
  int steps = parseInput();
  
  page1.show();
  sm.setSpeed(atoi(speedBuffer));
  sm.step(steps); 
  page0.show();
}

//parse the input from the nextion
int parseInput() {
  memset(inputBuffer, 0, sizeof(inputBuffer));
  input.getText(inputBuffer, sizeof(inputBuffer));
  
  const int fullRev = STEPS_PER_REV * GEAR_RED;
  String i = inputBuffer;
  String fraction = "";
  newPosition = 0;
  
  int spc = i.indexOf(" ");
  int div = i.indexOf("/");

  //if there are no spaces or division signs just convert the integer
  if(spc == -1 && div == -1) {
    int temp = atoi(inputBuffer);
    newPosition += (temp*fullRev);
    fraction += String(temp);
  }
  //if there is space get the integer before the space
  if(spc != -1) {
    int temp = 0;
    int temp2 = 0;
    temp = i.substring(0, spc).toInt();
    if(temp < 0){
      temp *= -1;
      fraction += "-";
    }
    newPosition += (temp*fullRev);
    
    //if there is no division but there is a integer after the space just add it to the newPosition
    if(div == -1){
      temp2 = i.substring((spc+1)).toInt();
      newPosition += (temp2*fullRev);
      temp += temp2;
    }
    fraction += (String(temp) + " ");
  }

  //if there is a division get the fraction
  if(div != -1) {
    float temp;
    int begin;

    //the fraction is after the space
    if(spc != -1)
      begin = (spc+1);
    //the fraction is afer the negative sign
    else if(inputBuffer[0] == '-'){
      begin = 1;
      fraction += String("-");
    }
    //there is nothing before the fraction
    else
      begin = 0;
        
    temp = (float) i.substring(begin, div).toInt();
    if(temp < 0)
      temp *= -1;
    temp /= (float) i.substring(div+1).toInt();
    newPosition += (fullRev*temp);

    fraction +=(i.substring(begin));
  }
  //if there is a negative sign make sure the the newPosition is a negative
  if(inputBuffer[0] == '-' && newPosition > -1)
    newPosition *= -1;

  //how many steps the motor has to rotate from the position to the newPosition
  steps = (newPosition-position);
 
  memset(fractionBuffer, 0, sizeof(fractionBuffer));
  fraction.toCharArray(fractionBuffer, sizeof(fractionBuffer));
  currPos.setText(fractionBuffer);

  position = newPosition;
  input.setText("");
  return steps;
   
}

//update the speed from the settings after the user is done with it
void updateSpeedPopCallback(void *ptr) {
  memset(speedBuffer, 0, sizeof(speedBuffer));
  speed.getText(speedBuffer, sizeof(speedBuffer));
  page0.show();
}

void setup(void) {
  Serial.begin(9600);

  //set up the nextion
  nexInit();

  calibration.attachPop(calibrationPopCallback, &calibration);
  enter.attachPop(enterPopCallback, &enter);
  updateSpeed.attachPop(updateSpeedPopCallback, &updateSpeed);
}

void loop(void) {  
  nexLoop(nex_listen_list); 
}
