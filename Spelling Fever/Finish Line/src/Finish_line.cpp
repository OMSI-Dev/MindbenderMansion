/*

//   11/15/2016  - added debounce to supress any noise triggering buttons seen last year from last code

//  08/17/18 - port to adafruit 32u4, hardware SPI
//  08/19/18 - reduced the send window to 250ms to keep the game from ending early.. didn't work
*/

#include <SPI.h>

#include <Wire.h>

#define number_of_74hc595s 4 

#define totalLights number_of_74hc595s * 8

#define freqMulti 100



const byte shiftInLatch = 5;
const byte shiftOutLatch = 7;

boolean registers[32];  //Shift out states

//Define variables to hold the data 
//for each shift register.
byte switchVar1; 
byte OLDswitchVar1;

byte byteToSend = 0;

volatile byte rows[4];

long previousMillis = 0;
long timeSenseNewLetter = 0; //last instance a letter was stepped on

long attractorTime = 0;
int attractorModeFlag = 1;

byte mask = 1; //bitmask for going from rows to lights
boolean lights[32];  //lights, letters to be lit

int buttonState = 0;

unsigned long currentMillis = 0;

long lastRandomTime = 0;
int randomByte = 0;

long lastDebounceTime1 = 0;  //added to supress any noise triggering letters

//section number here:
byte sectionNumber = 4;
#define wireFreq 50 * freqMulti

void requestEvent()  //sends switch stepped on to Controller when requested from Controller
{
  Wire.write(byteToSend); 
  Serial.println("finish line: ");
  Serial.println(byteToSend);

}




void clearLights()
{
  digitalWrite (shiftOutLatch, LOW);
  SPI.transfer (0);
  digitalWrite (shiftOutLatch, HIGH); 
} 

void writeLights()  //lights of letters
{  
  for(int i = 0; i < totalLights; i++)
  {
    boolean val = lights[i];
    if(i < 8)
    {      
      bitWrite(rows[0],i,val);
    }
    if(i >= 8 && i < 16)
    {      
      bitWrite(rows[1],i-8,val);
    }
    if(i >= 16 && i < 24)
    {      
      bitWrite(rows[2],i-16,val);
    }
    if(i >= 24)
    {      
      bitWrite(rows[3],i-24,val);
    }
  }
  digitalWrite (shiftOutLatch, LOW);
  SPI.transfer (rows[3]);
  digitalWrite (shiftOutLatch, HIGH);  
}

void attractorMode()
{
  if(currentMillis - attractorTime > 100)
  {
    memset(lights, 0, 32); // clears the array
    int j = random(0, 32);
    lights[j] = 1;  
    writeLights();  //lights letter
    attractorTime = currentMillis; 
  }
}


void setup()
{
  
  Serial.println ("Begin switch test on Section 4");

  //-------------------SET ADDRESS
  Wire.begin(sectionNumber);               
  
  Wire.setClock(wireFreq);
  Wire.onRequest(requestEvent);
  

  SPI.begin ();
  pinMode (shiftInLatch, OUTPUT);
  pinMode(shiftOutLatch, OUTPUT);
  digitalWrite (shiftInLatch, HIGH);
  digitalWrite (shiftOutLatch, HIGH);

  clearLights();
  

}


void loop()
{
  currentMillis = millis();
  
   
    digitalWrite (shiftInLatch, LOW); 
    digitalWrite (shiftInLatch, HIGH);
    
    switchVar1 = SPI.transfer (0);

  if (switchVar1 != OLDswitchVar1)
  {
    OLDswitchVar1 = switchVar1;
    lastDebounceTime1 = currentMillis;
  }
     
  
  // if ((currentMillis - lastDebounceTime1) > 20) 
  // {
    if(switchVar1 > 0)
    {
      timeSenseNewLetter = currentMillis;
      byteToSend = 1;
      digitalWrite (shiftOutLatch, LOW);
      SPI.transfer (255);
      digitalWrite (shiftOutLatch, HIGH);
    }
  //}
 

  if(currentMillis - timeSenseNewLetter > 50){ 
    digitalWrite (shiftOutLatch, LOW);
    SPI.transfer (0);
    digitalWrite (shiftOutLatch, HIGH);

  }  
  
  if(currentMillis - timeSenseNewLetter > 100){
    byteToSend = 0;
  } 

}


  
  
