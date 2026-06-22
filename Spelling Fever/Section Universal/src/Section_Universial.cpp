/*
  Shift OUT 75HC595
  //PINS 1-7, 15	Q0 " Q7	Output Pins
  //PIN 8	GND	Ground, Vss
  //PIN 9	Q7"	Serial Out                                      ARDUINO D11
  //PIN 10	MR	Controller Reclear, active low              Goes to VCC
  //PIN 11	SH_CP	Shift register clock pin                ARDUINO D13
  //PIN 12	ST_CP	Storage register clock pin (latch pin)  ARDUNIO D4
  //PIN 13	OE	Output enable, active low               Tied to ground
  //PIN 14	DS	Serial data input                       Goes to next shift chip (Q7)
  //PIN 16	Vcc	Positive supply voltage


  //   11/15/2016  - added debounce to supress any noise triggering buttons seen last year from last code

  //  08/17/18 - port to adafruit 32u4, hardware SPI
  // 12/6/23 - Removed TWBR as it is deprecated and replaced with setClock(in KHZ);
             - Converted Seperate Section code to a single Universial code that changes timers based on section variable

        
*/

#include <SPI.h>

#include <Wire.h>

#define number_of_74hc595s 4

#define totalLights number_of_74hc595s * 8

#define mins * 60000
#define freqMulti  100

const byte shiftInLatch = 5;
const byte shiftOutLatch = 7;

boolean registers[32];  //Shift out states
bool printOnce;

//Define variables to hold the data
//for each shift register.
byte switchVar1;
byte OLDswitchVar1;
byte switchVar2;
byte OLDswitchVar2;
byte switchVar3;
byte OLDswitchVar3;
byte switchVar4;
byte OLDswitchVar4;


byte bytesToSend[4] = {0, 0, 0, 0};

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
long lastDebounceTime2 = 0;  //added to supress any noise triggering letters
long lastDebounceTime3 = 0;  //added to supress any noise triggering letters
long lastDebounceTime4 = 0;  //added to supress any noise triggering letters





/******************************************************************************************************************
  Change Section number based on what section this is being uploaded to.
  The code will automattically adjust timers (each section needs an adjusted timer to account for lag)

  Sections start counting from the podium. The Finish line has slighty differnt code, this universial sketch can't be used for it.

******************************************************************************************************************
  //Set section address here
  //1-3 (finishing line has seperate code)*/

byte sectionNumber = 2;



#define wireFreq 50 * freqMulti

const long gameTimeout = 2 mins;

//do not change this value here. Change the value in the if statment in setup
int senseTimer = 0;



void receiveEvent(int howMany)  //executes when Controller sends rows (letters to be lit)
{
  if (Wire.available() > 3) {
    for (int i = 0; i < 4; i++) {  //read 4 bytes - rows 1-4
      rows[i] = Wire.read();
    }
    Serial.print(sectionNumber);
    Serial.println(" received 4 rows from Controller 1,2,3,4:");
    Serial.print(rows[0]);
    Serial.print(",");
    Serial.print(rows[1]);
    Serial.print(",");
    Serial.print(rows[2]);
    Serial.print(",");
    Serial.print(rows[3]);
    Serial.println("");
    Serial.println("");
  }
}


void requestEvent()  //sends switch stepped on to Controller when requested from Controller
{
  Wire.write(bytesToSend, 4);
  Serial.print(sectionNumber);
  Serial.println(" sending 4 rows to Controller rows 1,2,3,4: ");
  Serial.print(bytesToSend[0]);
  Serial.print(",");
  Serial.print(bytesToSend[1]);
  Serial.print(",");
  Serial.print(bytesToSend[2]);
  Serial.print(",");
  Serial.print(bytesToSend[3]);
  Serial.println("");
  bytesToSend[0] = 0;
  bytesToSend[1] = 0;
  bytesToSend[2] = 0;
  bytesToSend[3] = 0;
}


void clearLights()
{
  for (int i = 0; i < totalLights; i++)
  {
    lights[i] = LOW;
  }
}

void writeLights()  //lights of letters
{
  for (int i = 0; i < totalLights; i++)
  {
    boolean val = lights[i];
    if (i < 8)
    {
      bitWrite(rows[0], i, val);
    }
    if (i >= 8 && i < 16)
    {
      bitWrite(rows[1], i - 8, val);
    }
    if (i >= 16 && i < 24)
    {
      bitWrite(rows[2], i - 16, val);
    }
    if (i >= 24)
    {
      bitWrite(rows[3], i - 24, val);
    }
  }
  digitalWrite (shiftOutLatch, LOW);
  SPI.transfer (rows[3]);
  SPI.transfer (rows[2]);
  SPI.transfer (rows[1]);
  SPI.transfer (rows[0]);
  digitalWrite (shiftOutLatch, HIGH);
}

void attractorMode()
{
  if (currentMillis - attractorTime > 100)
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
  Serial.begin(115200);
  //-------------------SET ADDRESS
  Wire.begin(sectionNumber);

  //Lag timers are controller per section by this switch statment. Only adjust the timers here as it is compiled before the loop starts.
  //The Timers are used to check how often to look for a new button but this has a side effect of affecting how often they get updated to the controller. 
  switch (sectionNumber)
  {
    case 1:
      
      senseTimer = 5;
      break;

    case 2:
      senseTimer = 5;
      break;

    case 3:
      senseTimer  = 5;
      break;
  }
  


  Wire.setClock(wireFreq); 
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);


  SPI.begin ();
  pinMode (shiftInLatch, OUTPUT);
  pinMode(shiftOutLatch, OUTPUT);
  digitalWrite (shiftInLatch, HIGH);
  digitalWrite (shiftOutLatch, HIGH);

  clearLights();
  writeLights();

}

void loop()
{
  currentMillis = millis();

  digitalWrite (shiftInLatch, LOW); 
  digitalWrite (shiftInLatch, HIGH);

  switchVar1 = SPI.transfer (0);
  switchVar2 = SPI.transfer (0);
  switchVar3 = SPI.transfer (0);
  switchVar4 = SPI.transfer (0);


  if (switchVar1 != OLDswitchVar1)
  {
    OLDswitchVar1 = switchVar1;
    lastDebounceTime1 = currentMillis;
  }

  if (switchVar2 != OLDswitchVar2)
  {
    OLDswitchVar2 = switchVar2;
    lastDebounceTime2 = currentMillis;
  }

  if (switchVar3 != OLDswitchVar3)
  {
    OLDswitchVar3 = switchVar3;
    lastDebounceTime3 = currentMillis;
  }

  if (switchVar4 != OLDswitchVar4)
  {
    OLDswitchVar4 = switchVar4;
    lastDebounceTime4 = currentMillis;
  }
  int byteToSend = 0;

  if ((currentMillis - lastDebounceTime1) > 20)
  {
    if (switchVar1 > 0)
    {
      byteToSend = 1;
    }
  }
  if ((currentMillis - lastDebounceTime2) > 20)
  {
    if (switchVar2 > 0)
    {
      byteToSend = 2;
    }
  }
  if ((currentMillis - lastDebounceTime3) > 20)
  {
    if (switchVar3 > 0)
    {
      byteToSend = 3;
    }
  }
  if ((currentMillis - lastDebounceTime4) > 20)
  {
    if (switchVar4 > 0)
    {
      byteToSend = 4;
    }
  }

  if (byteToSend > 0)
  {
    attractorModeFlag = 0;  //turns off attractor mode
    timeSenseNewLetter = currentMillis;
    switch (byteToSend)
    {
      case 1:
        Serial.print("switch one: ");
        Serial.println(switchVar1);
        bytesToSend[0] = switchVar1;
        bytesToSend[1] = 0;
        bytesToSend[2] = 0;
        bytesToSend[3] = 0;
        break;
      case 2:
        Serial.print("switch two: ");
        Serial.println(switchVar2);
        bytesToSend[0] = 0;
        bytesToSend[1] = switchVar2;
        bytesToSend[2] = 0;
        bytesToSend[3] = 0;
        break;
      case 3:
        Serial.print("switch three: ");
        Serial.println(switchVar3);
        bytesToSend[0] = 0;
        bytesToSend[1] = 0;
        bytesToSend[2] = switchVar3;
        bytesToSend[3] = 0;
        break;
      case 4:
        Serial.print("switch four: ");
        Serial.println(switchVar4);
        bytesToSend[0] = 0;
        bytesToSend[1] = 0;
        bytesToSend[2] = 0;
        bytesToSend[3] = switchVar4;
        break;
    }
  }

  if (attractorModeFlag == 0) {
    digitalWrite (shiftOutLatch, LOW);
    SPI.transfer (rows[3]);
    SPI.transfer (rows[2]);
    SPI.transfer (rows[1]);
    SPI.transfer (rows[0]);
    digitalWrite (shiftOutLatch, HIGH);
  }


  if (currentMillis - timeSenseNewLetter > gameTimeout) //3 minutes has passed
  {
    attractorModeFlag = 1;  //flag to turn off write SAFE lights during attractor mode
    attractorMode();
  }

}



