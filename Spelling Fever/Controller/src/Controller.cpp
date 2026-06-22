#include <Arduino.h>
#include <Wire.h>

//   2/1/2015 - turned OFF Controller transmitting to Section 4
//   2/1/2015 - ignore sending as 255 for rows 1,5,6,7,8,9... (it just looks buggy when they show up)
//   8/5/2015 - consider sending new data to Sections every 150ms (immediately after reading from PC)...... then perhaps requsting data from Sections
//   8/14/2015 - tried out some code with a relay to cycle power to the Section devices.
//   8/18/2015 - using the WSWire library: https://github.com/steamfire/WSWireLib
//   11/2/2016 - updated some comments... ADN having issues after power supply failed.. seems like i2c Sections are freezing

//    11/10/2016 - added i2c extenders, simplified the code
//               - set PC interval to update every 20ms
//    11/16/2016 - added a counters for each section so that more than 8-10 letters stepped on sends a Finish flag to the PC. This reduces power spikes in the system

//    8/16/18 - ported to leonardo for rebuild
//    8/18/18 - working pretty well.. all Sections returning accept finish line
//    8/19/18 - set the computer fresh rate to 0.02 (20ms) and the serial read time to 200ms.. this gives 5-6 good serial reads from the PC each pass and seems to send the letters successfully to the pc
//   12/6/13 - updated TWBR to setClock due to deprecation


//INDICATOR LIGHTS
const int section2IndicatorLed = A0; //- receiving from Section 2
const int section3IndicatorLed = A1; // receiving from Section 3
const int finishLineIndicatorLed = A2; // receiving from finish line
const int testModeIndicatorLed = A3; // Test mode indicator near button
const int section1IndicatorLed = A4; // receiving form Section 1
const int receivingPcDataIndicatorLed = A5; // receiving from PC

long lastReceivedFromPc = 0;
long lastReceivedFromSection1 = 0;
long lastReceivedFromSection2 = 0;
long lastReceivedFromSection3 = 0;
long lastReceivedFromFinishLine = 0;

long lastNewLetter = 0;


//Test Mode Button Input
const int testModeButtonPin = 4; // connected to test mode button... LOW indicates button pressedlast1Pressed


byte testRows[14]; //initially gets rows to be lit from PC and tests whether unique
int logByte = 0; // logs each byte from Serail as available
int newMatrixFromPc = 0;

byte rows[14];  //rows to be lit (from PC via SerialRead)


byte pressed [14];  //to send to PC

byte pressed1[4];  //letters pressed on from Sections
byte pressed2[4];  //letters pressed on from Sections
byte pressed3[4];  //letters pressed on from Sections
byte pressed4;  //letters pressed on from Sections

byte last1Pressed[4];
byte last2Pressed[4];
byte last3Pressed[4];
byte last4Pressed;

long previousMillisPc = 0;
long intervalPc = 200;  //lets collect PC data for 200 ms


int readingSerial = 0;
int readNumberOfSerialPrints = 0;

int printReport = 0;  //prints faults report to Serial1
int communicationErrors = 0;  //keeps track of the errors communicating with the Section device


int finishFlag = 0;
long lastFinishTime = 0;
int finishFlagdelay = 0;
int finishFlagdelayTime = 3500;  //normal //delay time

int finishedFromTooManyLetters = 0; //finishes early if too many letters have been stepped on


byte counter = 0;

int byteToSend = 0;

boolean inButtonTestMode = 0;
int leavingButtonTestMode = 0;
long testModeDuration = 60000;
long testModeTimer = testModeDuration;

boolean pcEndedGame = 0;

//#define debug 


void setup() {
  Serial.begin(9600);

  Serial1.begin(9600);
  Wire.begin();
  Wire.setClock(50000);

  pinMode(section2IndicatorLed, OUTPUT);
  pinMode(section3IndicatorLed, OUTPUT);
  pinMode(finishLineIndicatorLed, OUTPUT);
  pinMode(testModeIndicatorLed, OUTPUT);
  pinMode(section1IndicatorLed, OUTPUT);
  pinMode(receivingPcDataIndicatorLed, OUTPUT);

  digitalWrite(section2IndicatorLed, HIGH);
  digitalWrite(section3IndicatorLed, HIGH);
  digitalWrite(finishLineIndicatorLed, HIGH);
  digitalWrite(testModeIndicatorLed, HIGH);
  digitalWrite(section1IndicatorLed, HIGH);
  digitalWrite(receivingPcDataIndicatorLed, HIGH);
}


void clearAllArrays()
{
  for (int i = 0; i < 4; i++)
  {
    pressed1[i] = (char)0;;
    last1Pressed[i] = (char)0;;
    pressed2[i] = (char)0;;
    last2Pressed[i] = (char)0;;
    pressed3[i] = (char)0;;
    last3Pressed[i] = (char)0;;
  }
  pressed4 = (char)0;
  for (int i = 0; i < 14; i++)
  {
    pressed[i] = (char)0;
  }
}




//------------------------------------------------------- SENDS BYTES to PC

void sendBytesToPc(int byteSending)
{
  
  if (byteSending > 0)
  {
    #ifdef debug
    Serial.print("Sending to PC row:");
    Serial.println(byteToSend);
    Serial.println(byteSending);
    #endif
    //delay(100);
  }
  pressed[0] = 33;

  switch (byteSending)
  {
      
    case 0:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      break;
    case 1:
      Serial1.write(pressed[0]);
      Serial1.write(pressed[1]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);      
     Serial.print("Print case:");
      Serial.println(byteSending);
      Serial.println(pressed[1]);  
      break;
    case 2:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(pressed[2]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);      
      break;
    case 3:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[3]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);      
      break;
    case 4:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[4]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);      
      break;
    case 5:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[5]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);      
      break;
    case 6:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[6]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);      
      break;
    case 7:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[7]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);      
      break;
    case 8:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[8]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);      
      break;
    case 9:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[9]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);      
      break;
    case 10:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[10]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
            Serial.print("Print case:");
      Serial.println(byteSending);
      break;
    case 11:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[11]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);
      break;
    case 12:
      Serial1.write(pressed[0]);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(pressed[12]);
      Serial1.write(0x0);
      Serial.print("Print case:");
      Serial.println(byteSending);
      break;
    case 13:
      Serial1.write(33);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);

      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);
      Serial1.write(0x0);

      Serial1.write(10);
      Serial.println("Print case 13");
      break;
  }
}

//
 void countPresses()
 {
    counter++;
    Serial.print("Button Count:");
    Serial.println(counter);
    if(counter == 13)
    {
      counter = 0;
    }

 }

void allLedsOff() { //this doesn't seem to be working

  Wire.beginTransmission(1); // transmit to device #3
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.endTransmission ();
  //delay(5);
  Wire.beginTransmission(2); // transmit to device #3
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.endTransmission ();
  //delay(5);
  Wire.beginTransmission(3); // transmit to device #3
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.endTransmission ();
  //delay(5);
}

void turnOnSafeSquares() { //this doesn't seem to be working

  Wire.beginTransmission(1); // transmit to device #3
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x32);
  Wire.endTransmission ();
  //delay(5);
  Wire.beginTransmission(2); // transmit to device #3
  Wire.write(0x0);
  Wire.write(0x4);
  Wire.write(0x0);
  Wire.write(0x16);
  Wire.endTransmission ();
  //delay(5);
  Wire.beginTransmission(3); // transmit to device #3
  Wire.write(0x0);
  Wire.write(0x4);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.endTransmission ();
  //delay(5);
}






void loop() 
{

  unsigned long currentTime = millis();
  // Serial.print("Byte to send");
  // Serial.println(byteToSend);

  /*------------------------------------- SEND and RECEIVE DATA TO/FROM PC */
  if (currentTime - previousMillisPc < intervalPc ) //send/receive for intervalPC
  {
    readingSerial = 1; //don't i2c anything while serial is reading

    if (Serial1.available() && logByte == 0)
    {
      testRows[0] = Serial1.read();
      //Serial.println(testRows[0]);
      if (testRows[0] == 33) //if true, read all 13 bytes
      {
        logByte = 1;
        #ifdef debug
        Serial.print(logByte);
        Serial.print(":");
        Serial.println("!   Start of packet");
        #endif
      }
    }

    if (Serial1.available() >= 2 && logByte > 0)
    {
      lastReceivedFromPc = currentTime;
      testRows[logByte] = Serial1.read();
      #ifdef debug
      Serial.print(logByte);
      Serial.print(":");
      Serial.println(testRows[logByte]);
      #endif
      logByte++;
      if (logByte == 13)
      {
        #ifdef debug
        Serial.print(logByte);
        Serial.print(":");
        Serial.println(testRows[logByte]);
        Serial.println("_________End of Packet_____________");
        #endif
      }      
      if (logByte == 14) //the matrrix is complete, all 13 bytes have been received
      {
        //Serial.println("light matrix recived");


        readNumberOfSerialPrints++;
        logByte = 0;
        #ifdef debug
            for(int i = 0; i<14; i++)
            {
            Serial.print(rows[i]);
            Serial.print(",");            
            }
            Serial.println("");
         #endif   
                //configure ready to receive the 1st byte again (byte 33)
          if (memcmp(testRows, rows, 14) != 0) // && memcmp(&testRows[0],&rows[0],sizeof(testRows[0])) == 0) //is the matrix new? has the PC has sent new data?
          {
            memcpy(rows, testRows, 14); //if yes, then copy new Matrix to rows

            
            newMatrixFromPc = 1;  // sets up an event to send new data to the Sections

            if (byteToSend == 13)
            {
              Serial.println("Matrix upadated");
              newMatrixFromPc = 0;
            }
          }
    }
  }
}


  if (currentTime - previousMillisPc > intervalPc)                            // READING SERIAL time has elapsed
  {
    readingSerial = 0;
    memset(pressed, 0, 14); // clears the array
  }

//  /*------------------------------------- SENDS DATA TO SectionS */

  if ((currentTime - previousMillisPc) > intervalPc && newMatrixFromPc == 1) //after reading serial... send new data
  {
    newMatrixFromPc = 0;  //dont retransmit unless new data has arrived
   // Serial.println("updating Sections lights");
    Wire.beginTransmission(1); // transmit to device #1
    Wire.write(rows[1]);
    Wire.write(rows[2]);
    Wire.write(rows[3]);
    Wire.write(rows[4]);
    Serial.print("to Section 1:");
    Serial.print(rows[1]);
    Serial.print(",");
    Serial.print(rows[2]);
    Serial.print(",");
    Serial.print(rows[3]);
    Serial.print(",");
    Serial.println(rows[4]);
    if (Wire.endTransmission () == 0) {
      //Serial.println("Controller sent x to Section 1");
    } else {
      Serial.println("Controller did not send to Section 1");
    }
    //}

    Wire.beginTransmission(2); // transmit to device #2
    Wire.write(rows[5]);
    Wire.write(rows[6]);
    Wire.write(rows[7]);
    Wire.write(rows[8]);
    Serial.print("to Section 2:");
    Serial.print(rows[5]);
    Serial.print(",");
    Serial.print(rows[6]);
    Serial.print(",");
    Serial.print(rows[7]);
    Serial.print(",");
    Serial.println(rows[8]);
    if (Wire.endTransmission () == 0) {
      //Serial.println("Controller sent x to Section 2");
    } else {
      Serial.println("Controller did not send to Section 2");
    }
    //}
    Wire.beginTransmission(3); // transmit to device #3
    Wire.write(rows[9]);
    Wire.write(rows[10]);
    Wire.write(rows[11]);
    Wire.write(rows[12]);

    Serial.print("to Section 3:");
    Serial.print(rows[9]);
    Serial.print(",");
    Serial.print(rows[10]);
    Serial.print(",");
    Serial.print(rows[11]);
    Serial.print(",");
    Serial.println(rows[12]);
    if (Wire.endTransmission () == 0) {
      //Serial.println("Controller sent x to Section 3");
    } else {
      Serial.println("Controller did not send to Section 3");
    }
    //}
    
  }



  /*------------------------------------- REQUEST DATA FROM SectionS */

  if ((currentTime - previousMillisPc) > intervalPc) //after sending data... request data...
  {
    //Serial.println("Update buttons");
    byteToSend = 0;
    for (int i = 0; i < 14; i++)
    {
      //Serial.println("clear array");
      pressed[i] = 0; //clears the array
    }
    /*-------------------------------------REQUEST FROM Section 1 */
    if(byteToSend == 0 )
    {
      if (Wire.requestFrom(1, 4) == 4) // request letter from Section(1) consisting 4 bytes, returns 0 or # of bytes received
      {
        //Serial.println("Request buttons from 1");
        for (int i = 0; i < 4; i++)
        {        
          pressed1[i] = Wire.read();



            if (pressed1[i] != last1Pressed[i])
            {
              Serial.println("Upating button press");
              last1Pressed[i] = pressed1[i];
              if (pressed1[i] == 1 || pressed1[i] == 2 || pressed1[i] == 4 || pressed1[i] == 8 || pressed1[i] == 16 || pressed1[i] == 32 || pressed1[i] == 64 || pressed1[i] == 128)
              {
                if(pressed1[3] != 32){
                countPresses();
                } else(Serial.println("safe was pressed in Section 1----Do Not Count"));

                #ifdef debug
                Serial.print(i);
                Serial.print("-1st Section: ");
                Serial.println(pressed1[i]);
                #endif
                
                byteToSend = i + 1;
                pressed[i + 1] = pressed1[i];
                lastNewLetter = currentTime;
                
                
              }
            }
          
        }
        lastReceivedFromSection1 = currentTime;
      } else {
        Serial.println("does not recieve data from Section 1");
      }
      sendBytesToPc(byteToSend);
    }
   /*-------------------------------------REQUEST FROM Section 2 */
    if (byteToSend == 0) //skip Section if we already have something to send
    {
      if (Wire.requestFrom(2, 4)== 4) // request letter from Section(2) consisting 4 bytes, returns 0 or # of bytes received
      {
        //Serial.println("Request buttons from 2");
        for (int i = 0; i < 4; i++)
        {
          pressed2[i] = Wire.read();

          if (pressed2[i] != last2Pressed[i])
          {
            last2Pressed[i] = pressed2[i];
            if (pressed2[i] == 1 || pressed2[i] == 2 || pressed2[i] == 4 || pressed2[i] == 8 || pressed2[i] == 16 || pressed2[i] == 32 || pressed2[i] == 64 || pressed2[i] == 128)
            {
              Serial.println("section 2: ");
              Serial.print(pressed2[0]);
              Serial.print(",");
              Serial.print(pressed2[1]);
              Serial.print(",");
              Serial.print(pressed2[2]);
              Serial.print(",");
              Serial.println(pressed2[3]);

              #ifdef debug
              Serial.print(i);
              Serial.print("-2nd Section: ");
              Serial.println(pressed2[i]);
              #endif
              byteToSend = i + 5;
              pressed[i + 5] = pressed2[i];
              lastNewLetter = currentTime;
            }
          }
        }
        lastReceivedFromSection2 = currentTime;
      } else {
        Serial.println("does not recieve data from Section 2");
      }
      sendBytesToPc(byteToSend);
      
    }

    /*-------------------------------------REQUEST FROM Section 3 */
    if (byteToSend == 0) //skip Section if we already have something to send
    {
      if (Wire.requestFrom(3, 4) == 4) // request letter from Section(3) consisting 4 bytes, returns 0 or # of bytes received
      {
        //Serial.println("Request buttons from 3");
        for (int i = 0; i < 4; i++)
        {
          pressed3[i] = Wire.read();
          //Serial.print("Section 3 pressed: ");
          //Serial.println(pressed1[i]);
          if (pressed3[i] != last3Pressed[i])
          {
            last3Pressed[i] = pressed3[i];
            if (pressed3[i] == 1 || pressed3[i] == 2 || pressed3[i] == 4 || pressed3[i] == 8 || pressed3[i] == 16 || pressed3[i] == 32 || pressed3[i] == 64 || pressed3[i] == 128)
            {

               #ifdef debug
              Serial.print(i);
              Serial.print("-3rdSection: ")
              Serial.println(pressed3[i]);
              #endif
              byteToSend = i + 9;
              pressed[i + 9] = pressed3[i];
              lastNewLetter = currentTime;
            }
          }
        }
        lastReceivedFromSection3 = currentTime;
      } else {
        Serial.println("does not recieve data from Section 3");
      }
      sendBytesToPc(byteToSend);
      }

    /*-------------------------------------REQUEST FROM Section 4 */
    // Serial.print("byte send before finishline:");
    // Serial.println(byteToSend);
    if (byteToSend == 0) //skip Section if we already have something to send
    {
      if (Wire.requestFrom(4, 1) == 1) 
      {   // request letter from Section(4), SECTION 4 consisting 1 byte
      //Serial.println("Request buttons from 4");
        pressed4 = Wire.read();
        if (pressed4 == 1)
          {
            Serial.println("sending LF");
            byteToSend = 13;
            sendBytesToPc(byteToSend);
            clearAllArrays();
          }
        finishFlagdelayTime = 3000;
        
      }
 
      lastReceivedFromFinishLine = currentTime;
      
    } else {
      // Serial.print("Data from finish: ");
      // Serial.println("");
    }

  }

  if (currentTime - lastFinishTime < finishFlagdelayTime) //computer sends out a bunch of garbage after game finish.. this flags that amount of time
  {
    finishFlagdelay = 1;
  } else {
    finishFlagdelay = 0;
  }

  if ((currentTime - previousMillisPc) > intervalPc) { //finally go back into serial reading mode...
    previousMillisPc = currentTime;

    while (Serial1.read() != -1); //clears data in the PC Serial Port

    //Serial.println("PC Packet complete resetting");
    logByte = 0; //configure ready to receive the 1st byte again (byte 33)
    printReport = 0;
  }

  //------------INDICATOR FLAGS
  if (currentTime - lastReceivedFromPc < 300) {
    digitalWrite(receivingPcDataIndicatorLed, HIGH);
  } else {
    digitalWrite(receivingPcDataIndicatorLed, LOW);
  }
  if (currentTime - lastReceivedFromSection1 < 300) {
    digitalWrite(section1IndicatorLed, HIGH);
  } else {
    digitalWrite(currentTime - section1IndicatorLed, LOW);
  }
  if (currentTime - lastReceivedFromSection2 < 300) {
    digitalWrite(section2IndicatorLed, HIGH);
  } else {
    digitalWrite(section2IndicatorLed, LOW);
  }
  if (currentTime - lastReceivedFromSection3 < 300) {
    digitalWrite(section3IndicatorLed, HIGH);
  } else {
    digitalWrite(section3IndicatorLed, LOW);
  }
  if (currentTime - lastReceivedFromFinishLine < 300) {
    digitalWrite(finishLineIndicatorLed, HIGH);
  } else {
    digitalWrite(finishLineIndicatorLed, LOW);
  }
}


//---------------------------------END LOOP

