/* 
OMSI Feeding Frenzy code. Adafruit ItsyBitsy


NOTES:
4/1/2015 - Added int pcToSignalSpeed = 0; because when the uController sends an S5, the PC responds with an S5 and the uC goes to 'S' slow speed

8/16/18 -Moved from Teensy 3.1 to Adafruit ItsyBitsy
          -Remove button library and replaced with Bounce2
          -updated all pins
          - added new circuit board
          - inhibit2 for lower motor is not connected (both inhibits can not share the same isolated ground

*/
#include <Arduino.h>
#include <Bounce2.h>
#include <Timer.h>

unsigned long trayScanTimer = 0;
int trayScanDuration = 3000; // milliseconds
bool scanningTrayItems = false;
int scoreSentCooldown = false;
int scoreSentCooldownDuration = 2000; // milliseconds
unsigned long scoreSentTimer = 0;

MoToTimer HeartBeat;
//instinalize button pins  
#define slow_pin 11
#define medium_pin 10
#define fast_pin 9
#define eStop_pin 12
#define beamStop 13
boolean BeamFlag = 0;
Bounce slow = Bounce();
Bounce medium = Bounce();
Bounce fast = Bounce();
Bounce eStop = Bounce();
//Bounce beamStop = Bounce();

//Define Reed switch pins
#define tray_pin A0
#define peas_pin A1
#define meat_pin A2
#define corn_pin A3
#define pie_pin A4
#define potatoes_pin A5

#define topMotor 6

const int buttonSlowLight = 5;
const int  buttonMediumLight = 4;
const int buttonFastLight = 8;

int tray = 0;
int potatoes = 0;
int peas = 0;
int pie = 0;
int corn = 0;
int meat = 0;

int pin1 = 0;
int pin2 = 0;
int pin3 = 0;
int pin4 = 0;
int pin5 = 0;
int pin6 = 0;

int lastpin1 = 0;
int lastpin2 = 0;
int lastpin3 = 0;
int lastpin4 = 0;
int lastpin5 = 0;
int lastpin6 = 0;

int counting = 0;
int sendToPcX = 0;
int inByte = 0;
int holdUntillReset = 0;  //flag that allows the speed button to reset the game after Emergency Stop has been pressed without actually sending a letter to the PC or lighting up the button LED
int gameReset = 1;  //flag that only allows the speed button to be selected once per game

//Sensor Flags
int sensor = 0;
int potatoesHold = 0;
int cornHold = 0;
int pieHold = 0;
int peasHold = 0;
int meatHold = 0;

//Flags for inhibit on the motors
const int inhibit = MOSI; //Mosi
const int inhibit2 = MISO; //Miso - not connected

//controls relay motorpower on pin D7
const int motorPower = 7;

int topMotorSpeed = 0;

long currentTime = 0;
long startSensing = 0;

int pcToSignalSpeed = 0;  //flag that only allows the speed to changed once per game

void setup()   { 
//Start Serial1 to communicate to PC             
Serial1.begin(9600);  
Serial.begin(9600);

pinMode(topMotor, OUTPUT);
pinMode(inhibit, OUTPUT);
pinMode(inhibit2, OUTPUT); //Not used but set as safety 
pinMode(beamStop, INPUT);
pinMode(motorPower, OUTPUT);
pinMode(13,OUTPUT); 
digitalWrite(inhibit, LOW);


digitalWrite(motorPower, LOW);

pinMode(tray_pin, INPUT);
pinMode(peas_pin, INPUT);
pinMode(meat_pin, INPUT);
pinMode(corn_pin, INPUT);
pinMode(pie_pin, INPUT);
pinMode(potatoes_pin, INPUT);


//Button Attachment and debouncing value
slow.attach(slow_pin);
slow.interval(50);
medium.attach(medium_pin);
medium.interval(50);
fast.attach(fast_pin);
fast.interval(50);
eStop.attach(eStop_pin);
eStop.interval(50);

  
pinMode(buttonSlowLight, OUTPUT);
pinMode(buttonMediumLight, OUTPUT);
pinMode(buttonFastLight, OUTPUT);
  
}

bool on;

void hearttick()
{
  if(!HeartBeat.running())
  {
    digitalWrite(13,on);
    on = !on;
    HeartBeat.setTime(1000);
  }
}


//--------------------------------------------------------START LOOP

void loop()                     
{
  hearttick();
unsigned long currentMillis = millis();

//updates buttons status
slow.update();
medium.update();
fast.update();
eStop.update();
BeamFlag = digitalRead(beamStop);    
    
if(sendToPcX == 1){    //not sure how often the PC wants to see this 'X' but we send it a few times
  Serial1.println("X");
  sendToPcX = 0;
  }
  
  
if (Serial1.available() > 0) {
  inByte = Serial1.read();
    if(inByte == 'T')                        //PC sends 'T' for restart signal
    {                     
      digitalWrite(buttonSlowLight, LOW);    //turn the motor and all the lights off
      digitalWrite(buttonMediumLight, LOW);
      digitalWrite(buttonFastLight, LOW);
      digitalWrite(motorPower, LOW);
      digitalWrite(inhibit, LOW);
      sendToPcX = 1;
      gameReset = 1;                         //allows the speed buttons to be active
    }

  if(inByte == 'S'){ 
    if(pcToSignalSpeed == 0){  //only allows the PC to send the signal once.... ignores all characters for speed changes until new button is pressed
    pcToSignalSpeed = 1;
    topMotorSpeed = 100;
    digitalWrite(inhibit, HIGH);
    digitalWrite(motorPower, HIGH);
    analogWrite(topMotor, topMotorSpeed); 
  }}

  if(inByte == 'M'){ 
  if(pcToSignalSpeed == 0){  //only allows the PC to send the signal once.... ignores all characters for speed changes until new button is pressed
    pcToSignalSpeed = 1;
    topMotorSpeed = 120;
    digitalWrite(inhibit, HIGH);
    digitalWrite(motorPower, HIGH);
    analogWrite(topMotor, topMotorSpeed); 
  }}
  
    if(inByte == 'F'){ 
    if(pcToSignalSpeed == 0){  //only allows the PC to send the signal once.... ignores all characters for speed changes until new button is pressed
    pcToSignalSpeed = 1;
    topMotorSpeed = 140;
    digitalWrite(inhibit, HIGH);
    digitalWrite(motorPower, HIGH);
    analogWrite(topMotor, topMotorSpeed); 
  }}
  
  
  

}

//*************************** Button Checks ************************************


  if(gameReset == 1){                
   
  if(slow.fell()){                      //------------------- SLOW Button is pressed
    Serial1.println("S");
    Serial.println("slow");
  if(holdUntillReset == 0){             //Emergency Stop button was previously pressed.... .. and now we're ready to play
    gameReset = 0;                       //holds the game at the current speed untill the PC sends a T (resets game) or e-stop button is pressed
    digitalWrite(buttonSlowLight, HIGH);
    digitalWrite(buttonMediumLight, LOW);
    digitalWrite(buttonFastLight, LOW);
    pcToSignalSpeed = 0;                 //flag allows PC to send speed signal to uController
  }
  
  if(holdUntillReset == 1){          //Emergency Stop button was previously pressed.... now we've reset the game by pushing this button
    holdUntillReset = 0;
    gameReset = 1;  
    digitalWrite(buttonSlowLight, LOW);
    digitalWrite(buttonMediumLight, LOW);
    digitalWrite(buttonFastLight, LOW);
    topMotorSpeed = 0;
    digitalWrite(inhibit, LOW);
  }
  sendToPcX = 1; 
   }
  
  if(medium.fell()){     //------------------- MEDIUM Button is pressed
                         
  Serial1.println("M");
  if(holdUntillReset == 0){                   //Emergency Stop button was previously pressed.... .. and now we're ready to play
    gameReset = 0;                               //holds the game at the current speed untill the PC sends a T (resets game) or e-stop button is pressed  
    digitalWrite(buttonSlowLight, LOW);
    digitalWrite(buttonMediumLight, HIGH);
    digitalWrite(buttonFastLight, LOW);
    pcToSignalSpeed = 0;                 //flag allows PC to send speed signal to uController
  }
  
  if(holdUntillReset == 1){           //Emergency Stop button was previously pressed.... now we've reset the game by pushing this button
    holdUntillReset = 0;
    gameReset = 1;                     
    digitalWrite(buttonSlowLight, LOW);
    digitalWrite(buttonMediumLight, LOW);
    digitalWrite(buttonFastLight, LOW);
    topMotorSpeed = 0;
    digitalWrite(inhibit, LOW);
  }
  sendToPcX = 1;
  }
  //---------------------------------------FAST Button is pressed
  if(fast.fell()){
  Serial1.println("F");
  if(holdUntillReset == 0){                  //Emergency Stop button was previously pressed.... .. and now we're ready to play
    gameReset = 0;                          //holds the game at the current speed untill the PC sends a T (resets game) or e-stop button is pressed
    digitalWrite(buttonSlowLight, LOW);
    digitalWrite(buttonMediumLight, LOW);
    digitalWrite(buttonFastLight, HIGH);
    pcToSignalSpeed = 0;                 //flag allows PC to send speed signal to uController
  }
  
  if(holdUntillReset == 1){                       //Emergency Stop button was previously pressed.... now we've reset the game by pushing this button
    holdUntillReset = 0;
    gameReset = 1;  
    digitalWrite(buttonSlowLight, LOW);
    digitalWrite(buttonMediumLight, LOW);
    digitalWrite(buttonFastLight, LOW);
    topMotorSpeed = 0;
    digitalWrite(inhibit, LOW);
  }
  sendToPcX = 1;
}
  }

//********************************************* emergency stop button

if(eStop.fell() || BeamFlag == HIGH){   
  if(holdUntillReset == 0){             
  Serial1.println("G");
  holdUntillReset = 1;}
  gameReset = 1;
  digitalWrite(buttonSlowLight, LOW);
  digitalWrite(buttonMediumLight, LOW);
  digitalWrite(buttonFastLight, LOW);
  topMotorSpeed = 0;
  digitalWrite(inhibit, LOW);
  digitalWrite(motorPower, LOW);  
  }

//Update Motor Settings
analogWrite(topMotor, topMotorSpeed);

  //********************************************READ FOOD SENSORS
  //reed switch trips to ground and sensor reads low
  
  
  lastpin1 = pin1;
  lastpin2 = pin2;
  lastpin3 = pin3;
  lastpin4 = pin4;
  lastpin5 = pin5;
  lastpin6 = pin6;
  // int tray = pin1 = digitalRead(tray_pin);  //sensor
  // int peas = pin2 = digitalRead(peas_pin);  //peas
  // int meat = pin3 = digitalRead(meat_pin);  //meat
  // int corn = pin4 = digitalRead(corn_pin);  //corn
  // int pie = pin5 = digitalRead(pie_pin);  //pie
  // int potatoes = pin6 = digitalRead(potatoes_pin);  //potatoes
  
  // the reed sensors return LOW (0) for active and HIGH (1) for inactive
  // so the digitalRead is negated
  if(!digitalRead(tray_pin)) {
    tray = 1;
  }
  if(!digitalRead(peas_pin)) {
    peas = 1;
  }
  if(!digitalRead(meat_pin)) {
    meat = 1;
  }
  if(!digitalRead(corn_pin)) {
    corn = 1;
  }
  if(!digitalRead(pie_pin)) {
    pie = 1;
  }
  if(!digitalRead(potatoes_pin)) {
    potatoes = 1;
  }
  
  if(pin1 != lastpin1) {

  Serial.print("tray: ");
  Serial.println(pin1);
  }
  if(pin2 != lastpin2) {

  Serial.print("peas: ");
  Serial.println(pin2);
  }
  if(pin3 != lastpin3) {

  Serial.print("meat: ");
  Serial.println(pin3);
  }
  if(pin4 != lastpin4) {

  Serial.print("corn: ");
  Serial.println(pin4);
  }
  if(pin5 != lastpin5) {

  Serial.print("pie: ");
  Serial.println(pin5);
  }
  if(pin6 != lastpin6) {

  Serial.print("potatoes: ");
  Serial.println(pin6);
  }

  // read tray pin1
  // start cool down
  // gather all the all pins
  // cooldown is over
  // assess if all pins were gathered
  // give out score

  if(tray == 1 && scanningTrayItems == false) { // start scanning tray
    scanningTrayItems = true;
    trayScanTimer = millis();
  }
  if(scanningTrayItems && millis() - trayScanTimer > trayScanDuration) {
    // time up 
    scanningTrayItems = false;
    peas = 0;
    meat = 0;
    corn = 0;
    pie = 0;
    potatoes = 0;
  }
  if (scanningTrayItems) {
    // check if score has already been sent for this tray
    if(!scoreSentCooldown) { 
      if (peas && meat && corn && pie && potatoes) {
        Serial1.println("S5"); // send full tray detected signal
        Serial.println("score sent to pc software"); // debug message
        scoreSentCooldown = true;
        scoreSentTimer = millis(); // start cooldown timer
      }
    } else {
      if(millis() - scoreSentTimer > scoreSentCooldownDuration) {
        scoreSentCooldown = false;
        Serial.println("scoreSentCooldown reset"); // debug message
      }
    }
  } // end scanningTrayItems if


/*
  //PIN 1 is the sensor PIN
  
  if(pin1 == 0) 
  // Serial.println("tray pin triggered!");
    {  //--------------SENSOR PIN has tripped low.... start looking for food pieces
    startSensing = currentMillis;
    sensor = 1;   //----------------flag for sensor pin
    potatoesHold = 0;  //set all food flags to zero
    cornHold = 0;
    pieHold = 0;
    peasHold = 0; 
    }
 
  if(sensor == 1 && pin6 == 0 && cornHold == 0 && pieHold == 0 && peasHold == 0)
    {  //pin6 'potatoes' should be the 1st pease... all other sensors waiting to read next food pieces
    potatoesHold = 1; //set potato flag 
    } 
    
  if(potatoesHold == 1 && pin4 == 0)
    {  //corn food piece sensor goes LOW
    cornHold = 1;//set corn flag
    }  
  
  if(cornHold == 1 && pin5 == 0)
    {  //pie food piece sensor goes LOW
    pieHold = 1; //set pie flag
    }
    
  if(pieHold == 1 && pin2 == 0)
    {  //peas food piece sensor goes LOW
    peasHold = 1;
    }
    
  if(peasHold == 1 && pin3 == 0)
    { //meat food piece sensor goes LOW
    Serial1.println("S5");
    //Serial1.println(currentMillis - startSensing);
    startSensing = 0;
    sensor = 0;
    potatoesHold = 0;
    cornHold = 0;
    pieHold = 0;
    peasHold = 0;
    }

    */
  

}
 

