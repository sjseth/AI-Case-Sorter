#include <Wire.h>
#include <SoftwareSerial.h>

//sends 5v to stepper enable, pul and dir pins
#define TBPOWERPIN 10

//TB6600 PINS (STEPPIN IS PUL+ ON TB6000)
//Stepper controller is set to 32 Microsteps
#define FEED_DIRPIN 8
#define FEED_STEPPIN 9  
#define FEED_TB6600Enable 2
#define FEED_MICROSTEPS 36


#define SORT_MICROSTEPS 32 
#define SORT_DIRPIN 5
#define SORT_STEPPIN 6
#define SORT_TB6600Enable 4

#define SORTER_CHUTE_SEPERATION 20 //number of steps between chutes

//not used but could be if you wanted to specify exact positions. 
//referenced in the commented out code in the run sorter method
int sorterChutes[] ={0,17, 33, 49, 66, 83, 99, 116, 132}; 


//inputs which can be set via serial console like:  feedspeed:50 or sortspeed:60
int feedSpeed = 50; //range: 1..100
int feedSteps = 100; //range: 1..1000
int sortSpeed = 60; //range: 1..100
int sortSteps = 20; //range: 1..500 //20 default
int feedPauseTime = 100; //range: 1.2000


//tracking variables
int sorterMotorCurrentPosition = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(TBPOWERPIN, OUTPUT);
  digitalWrite(TBPOWERPIN, HIGH);
  pinMode(FEED_TB6600Enable, OUTPUT);
  pinMode(SORT_TB6600Enable, OUTPUT);
  pinMode(FEED_DIRPIN, OUTPUT);
  pinMode(FEED_STEPPIN, OUTPUT);
  digitalWrite(FEED_TB6600Enable, HIGH);
  digitalWrite(SORT_TB6600Enable, LOW);
  digitalWrite(FEED_DIRPIN, HIGH);
}

void loop() {
digitalWrite(TBPOWERPIN, HIGH);
    if(Serial.available() > 0 )  
    {
      
      String input = Serial.readStringUntil('\n');
      
      //normal input would just be the tray # you want to sort to which would be something like "1\n" or "9\n" (\n)if you are using serial console, the \n is already included in the writeline
      //so just the number 1-10 would suffice. 
      //the software on the other end should be listening for  "done\n". 
      
      //set feed speed. Values 1-100. Def 60
      if(input.startsWith("feedspeed:")){
         input.replace("feedspeed:","");
         feedSpeed= input.toInt();
          Serial.print("ok\n");
         return;
      }
      
      //set sort speed. Values 1-100. Def 60
      if(input.startsWith("sortspeed:")){
         input.replace("sortspeed:","");
         sortSpeed= input.toInt();
          Serial.print("ok\n");
         return;
      }

      //set sort steps. Values 1-100. Def 20
      if(input.startsWith("sortsteps:")){
         input.replace("sortsteps:","");
         sortSteps= input.toInt();
          Serial.print("ok\n");
         return;
      }

      //set feed steps. Values 1-1000. Def 100
      if(input.startsWith("feedsteps:")){
         input.replace("feedsteps:","");
         feedSteps= input.toInt();
          Serial.print("ok\n");
         return;
      }

      //set feed steps. Values 1-1000. Def 100
      if(input.startsWith("feedpausetime:")){
         input.replace("feedpausetime:","");
         feedPauseTime= input.toInt();
          Serial.print("ok\n");
         return;
      }

      //to change sorter arm position, send x1, x2.. x10, etc. 
      if(input.startsWith("x")){
         input.replace("x","");
         int msortsteps= input.toInt();
         //runSorterMotorSteps(msortsteps);
         runSortMotorManual(msortsteps);
         Serial.print("done\n");
         return;
      }
      
      //to run feeder, send  any string starting with f:
       if(input.startsWith("f")){
         input.replace("f","");
         runFeedMotorManual();
         Serial.print("done\n");
         return;
      }
      
      //to run feeder, send  any string starting with f:
       if(input.startsWith("f")){
         input.replace("f","");
         runFeedMotorManual();
         Serial.print("done\n");
         return;
      }

      
      int sortPosition = input.toInt();
      runSorterMotor(sortPosition);
      runFeedMotorManual();
      delay(100);//allow for vibrations to calm down for clear picture
      Serial.print("done\n");
    }

}

//moves the sorter arm. Blocking operation until complete
void runSorterMotor(int chute){

   //you can uncomment to use the array sorterChutes for exact positions of each slot
   //int newStepsPos = sorterChutes[chute]; //the steps position from zero of the "slot"

   //rather than use exact positions, we are assuming uniform seperation betweeen slots specified by sorter_chute_seperation constant
   int newStepsPos = chute * SORTER_CHUTE_SEPERATION;

   //calculate the amount of movement and move.
   int nextmovement = newStepsPos - sorterMotorCurrentPosition; //the number of +-steps between current position and next position
   int movement = nextmovement * SORT_MICROSTEPS; //calculate the number of microsteps required
   
   runSortMotorManual(movement);
   sorterMotorCurrentPosition = newStepsPos;
}

void runFeedMotorManual(){
  
  int steps=0;
  
  digitalWrite(FEED_DIRPIN, LOW);
  
  steps = abs(feedSteps) * FEED_MICROSTEPS;
  
  int delayTime = 120 - feedSpeed; //assuming a feedspeed variable between 0 and 100. a delay of less than 20ms is too fast so 20mcs should be minimum delay for fastest speed.
  
  for(int i=0;i<steps;i++){
      digitalWrite(FEED_STEPPIN, HIGH);
      delayMicroseconds(60);   //pulse. i have found 60 to be very consistent with tb6600. Noticed that faster pulses tend to drop steps. 
      digitalWrite(FEED_STEPPIN, LOW);
      delayMicroseconds(delayTime); //speed 156 = 1 second per revolution
  }
  delay(feedPauseTime);
  
  for(int i=0;i<steps;i++){
      digitalWrite(FEED_STEPPIN, HIGH);
      delayMicroseconds(60);   //pulse
      digitalWrite(FEED_STEPPIN, LOW);
      delayMicroseconds(delayTime); //speed 156 = 1 second per revolution
  }
 
}


void runSortMotorManual(int steps){
  int delayTime = 120 - sortSpeed;
     
  if(steps>0){
    digitalWrite(SORT_DIRPIN, HIGH);
  }else{
    digitalWrite(SORT_DIRPIN, LOW);
  }
  steps = abs(steps) ;
  for(int i=0;i<steps;i++){
      digitalWrite(SORT_STEPPIN, HIGH);
      delayMicroseconds(60);   //pulse //def 60
      digitalWrite(SORT_STEPPIN, LOW);
      delayMicroseconds(delayTime); //speed 156 = 1 second per revolution //def 20
  }
  //Serial.print(steps);Serial.print("\n");
}
