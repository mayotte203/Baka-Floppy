/*
 * FloppyDrives.cpp
 *
 * Output for controlling floppy drives.  The _original_ Moppy instrument!
 */
#include "MoppyInstrument.h"
#include "FloppyDrives.h"


// First drive being used for floppies, and the last drive.  Used for calculating
// step and direction pins.
const byte FIRST_DRIVE = 1;
const byte LAST_DRIVE = 16;  // This sketch can handle only up to 9 drives (the max for Arduino Uno)

// Maximum note number to attempt to play on floppy drives.  It's possible higher notes may work,
// but they may also cause instability.
const byte MAX_FLOPPY_NOTE = 127;

/*NOTE: The arrays below contain unused zero-indexes to avoid having to do extra
 * math to shift the 1-based subAddresses to 0-based indexes here.  Unlike the previous
 * version of Moppy, we *will* be doing math to calculate which drive maps to which pin,
 * so there are as many values as drives (plus the extra zero-index)
 */

 /*An array of maximum track positions for each floppy drive.  3.5" Floppies have
 80 tracks, 5.25" have 50.  These should be doubled, because each tick is now
 half a position (use 158 and 98).
 NOTE: Index zero of this array controls the "resetAll" function, and should be the
 same as the largest value in this array
 */
unsigned int FloppyDrives::MAX_POSITION[] = {158,158,158,158,158,158,158,158,158,158,158,158,158,158,158,158,158,158};

//Array to track the current position of each floppy head.
unsigned int FloppyDrives::currentPosition[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*Array to keep track of state of each pin.  Even indexes track the control-pins for toggle purposes.  Odd indexes
 track direction-pins.  LOW = forward, HIGH=reverse
 */
int FloppyDrives::currentState[] = {0,0,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};

// Current period assigned to each drive.  0 = off.  Each period is two-ticks (as defined by
// TIMER_RESOLUTION in MoppyInstrument.h) long.
unsigned int FloppyDrives::currentPeriod[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Tracks the current tick-count for each drive (see FloppyDrives::tick() below)
unsigned int FloppyDrives::currentTick[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// The period originally set by incoming messages (prior to any modifications from pitch-bending)
unsigned int FloppyDrives::originalPeriod[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

const unsigned int stepPinsArray[] = {0, 24, 28, 32, 36, 40, 44, 48, 52, 25, 29, 33, 37, 41, 45, 49, 53};
const unsigned int dirPinsArray[]  = {0, 22, 26, 30, 34, 38, 42, 46, 50, 23, 27, 31, 35, 39, 43, 47, 51};

void FloppyDrives::setup() {

  // Prepare pins (0 and 1 are reserved for Serial communications)
  for(int i = 1; i <= 16; ++i)
  {
	  pinMode(stepPinsArray[i], OUTPUT); // Step control 1
	  pinMode(dirPinsArray[i], OUTPUT); // Direction 1
  }


  // With all pins setup, let's do a first run reset
  resetAll();
  delay(500); // Wait a half second for safety

  // Setup timer to handle interrupts for floppy driving
  Timer1.initialize(TIMER_RESOLUTION); // Set up a timer at the resolution defined in MoppyInstrument.h
  Timer1.attachInterrupt(tick); // Attach the tick function
  // If MoppyConfig wants a startup sound, play the startupSound on the
  // first drive.
  if (PLAY_STARTUP_SOUND) {
    startupSound(1);
    delay(500);
    resetAll();
  }
}

// Play startup sound to confirm drive functionality
void FloppyDrives::startupSound(byte driveNum) {
  unsigned int chargeNotes[] = {
      noteDoubleTicks[31],
      noteDoubleTicks[36],
      noteDoubleTicks[38],
      noteDoubleTicks[43],
      0
  };
  byte i = 0;
  unsigned long lastRun = 0;
  while(i < 5) {
    if (millis() - 200 > lastRun) {
      lastRun = millis();
      currentPeriod[driveNum] = chargeNotes[i++];
    }
  }
}

//
//// Message Handlers
//




  void FloppyDrives::noteOnHandler(byte channel, byte note, byte velocity){
    if(note <= MAX_FLOPPY_NOTE)
    {
      currentPeriod[channel] = originalPeriod[channel] = noteDoubleTicks[note];
    }
  }
 
  void FloppyDrives::noteOffHandler(byte channel, byte note, byte velocity){
    currentPeriod[channel] = originalPeriod[channel] = 0;
  }
  
  void FloppyDrives::pitchBendHandler(byte channel, int bend){

      // A whole octave of bend would double the frequency (halve the the period) of notes
      // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
      //currentPeriod[subAddress] = originalPeriod[subAddress] / 1.4;
    currentPeriod[channel] = originalPeriod[channel] / pow(2.0, BEND_OCTAVES*(bend/(float)8192));
  }
  void FloppyDrives::startHandler(){
    
  }
  void FloppyDrives::stopHandler(){
    haltAllDrives();
  }

//
//// Floppy driving functions
//

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
 */
void FloppyDrives::tick()
{
//  for(int i = 1; i <= 16; ++i)
//  {
//  if (currentPeriod[i]>0){
//    currentTick[i]++;
//    if (currentTick[i] >= currentPeriod[i]){
//      togglePin(i, stepPinsArray[i], dirPinsArray[i]); 
//      currentTick[i]=0;
//    }
//  }
//  }

    if (currentPeriod[16]>0){
      currentTick[16]++;
      if (currentTick[16] >= currentPeriod[16]){
       togglePin(16, stepPinsArray[16], dirPinsArray[16]); 
       currentTick[16]=0;
      }
    }

    if (currentPeriod[1]>0){
      currentTick[1]++;
      if (currentTick[1] >= currentPeriod[1]){
       togglePin(1, stepPinsArray[1], dirPinsArray[1]); 
       currentTick[1]=0;
      }
    }

    if (currentPeriod[2]>0){
      currentTick[2]++;
      if (currentTick[2] >= currentPeriod[2]){
       togglePin(2, stepPinsArray[2], dirPinsArray[2]); 
       currentTick[2]=0;
      }
    }

    if (currentPeriod[3]>0){
      currentTick[3]++;
      if (currentTick[3] >= currentPeriod[3]){
       togglePin(3, stepPinsArray[3], dirPinsArray[3]); 
       currentTick[3]=0;
      }
    }

    if (currentPeriod[4]>0){
      currentTick[4]++;
      if (currentTick[4] >= currentPeriod[4]){
       togglePin(4, stepPinsArray[4], dirPinsArray[4]); 
       currentTick[4]=0;
      }
    }

    if (currentPeriod[5]>0){
      currentTick[5]++;
      if (currentTick[5] >= currentPeriod[5]){
       togglePin(5, stepPinsArray[5], dirPinsArray[5]); 
       currentTick[5]=0;
      }
    }

    if (currentPeriod[6]>0){
      currentTick[6]++;
      if (currentTick[6] >= currentPeriod[6]){
       togglePin(6, stepPinsArray[6], dirPinsArray[6]); 
       currentTick[6]=0;
      }
    }

    if (currentPeriod[7]>0){
      currentTick[7]++;
      if (currentTick[7] >= currentPeriod[7]){
       togglePin(7, stepPinsArray[7], dirPinsArray[7]); 
       currentTick[7]=0;
      }
    }

    if (currentPeriod[8]>0){
      currentTick[8]++;
      if (currentTick[8] >= currentPeriod[8]){
       togglePin(8, stepPinsArray[8], dirPinsArray[8]); 
       currentTick[8]=0;
      }
    }

    if (currentPeriod[9]>0){
      currentTick[9]++;
      if (currentTick[9] >= currentPeriod[9]){
       togglePin(9, stepPinsArray[9], dirPinsArray[9]); 
       currentTick[9]=0;
      }
    }

    if (currentPeriod[10]>0){
      currentTick[10]++;
      if (currentTick[10] >= currentPeriod[10]){
       togglePin(10, stepPinsArray[10], dirPinsArray[10]); 
       currentTick[10]=0;
      }
    }

    if (currentPeriod[11]>0){
      currentTick[11]++;
      if (currentTick[11] >= currentPeriod[11]){
       togglePin(11, stepPinsArray[11], dirPinsArray[11]); 
       currentTick[11]=0;
      }
    }

    if (currentPeriod[12]>0){
      currentTick[12]++;
      if (currentTick[12] >= currentPeriod[12]){
       togglePin(12, stepPinsArray[12], dirPinsArray[12]); 
       currentTick[12]=0;
      }
    }

    if (currentPeriod[13]>0){
      currentTick[13]++;
      if (currentTick[13] >= currentPeriod[13]){
       togglePin(13, stepPinsArray[13], dirPinsArray[13]); 
       currentTick[13]=0;
      }
    }

    if (currentPeriod[14]>0){
      currentTick[14]++;
      if (currentTick[14] >= currentPeriod[14]){
       togglePin(14, stepPinsArray[14], dirPinsArray[14]); 
       currentTick[14]=0;
      }
    }

    if (currentPeriod[15]>0){
      currentTick[15]++;
      if (currentTick[15] >= currentPeriod[15]){
       togglePin(15, stepPinsArray[15], dirPinsArray[15]); 
       currentTick[15]=0;
      }
    }
}

void FloppyDrives::togglePin(byte driveNum, byte pin, byte direction_pin) {

  //Switch directions if end has been reached
  if (currentPosition[driveNum] >= MAX_POSITION[driveNum]) {
    currentState[direction_pin] = HIGH;
    digitalWrite(direction_pin,HIGH);
  }
  else if (currentPosition[driveNum] <= 0) {
    currentState[direction_pin] = LOW;
    digitalWrite(direction_pin,LOW);
  }

  //Update currentPosition
  if (currentState[direction_pin] == HIGH){
    currentPosition[driveNum]--;
  }
  else {
    currentPosition[driveNum]++;
  }

  //Pulse the control pin
  digitalWrite(pin,currentState[pin]);
  currentState[pin] = ~currentState[pin];
}


//
//// UTILITY FUNCTIONS
//

//Not used now, but good for debugging...
void FloppyDrives::blinkLED(){
  digitalWrite(13, HIGH); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(13, LOW);
}

// Immediately stops all drives
void FloppyDrives::haltAllDrives() {
  for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
    currentPeriod[d] = 0;
  }
}

void FloppyDrives::controlChangeHandler(byte channel, byte number, byte value)
{
	switch(number)
	{
		case 120:
		{
			currentPeriod[channel] = originalPeriod[channel] = 0;
			break;
		}
		case 123:
		{
			currentPeriod[channel] = originalPeriod[channel] = 0;
			break;
		}
	}
}
//For a given floppy number, runs the read-head all the way back to 0
void FloppyDrives::reset(byte driveNum)
{
  currentPeriod[driveNum] = 0; // Stop note

  byte stepPin = stepPinsArray[driveNum];
  digitalWrite(stepPin+1,HIGH); // Go in reverse
  for (unsigned int s=0;s<MAX_POSITION[driveNum];s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(stepPin,HIGH);
    digitalWrite(stepPin,LOW);
    delay(5);
  }
  currentPosition[driveNum] = 0; // We're reset.
  currentState[stepPin] = LOW;
  digitalWrite(dirPinsArray[driveNum],LOW);
  currentState[dirPinsArray[driveNum]] = LOW; // Ready to go forward.
}

// Resets all the drives simultaneously
void FloppyDrives::resetAll()
{

  // Stop all drives and set to reverse
  for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
    byte stepPin = stepPinsArray[d];
    currentPeriod[d] = 0;
    digitalWrite(dirPinsArray[d],HIGH);
  }

  // Reset all drives together
  for (unsigned int s=0;s<MAX_POSITION[0];s+=2){ //Half max because we're stepping directly (no toggle); grab max from index 0
    for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
      byte stepPin = stepPinsArray[d];
      digitalWrite(stepPin,HIGH);
      digitalWrite(stepPin,LOW);
    }
    delay(5);
  }

  // Return tracking to ready state
  for (byte d=FIRST_DRIVE;d<=LAST_DRIVE;d++) {
    byte stepPin = stepPinsArray[d];
    currentPosition[d] = 0; // We're reset.
    currentState[stepPin] = LOW;
    digitalWrite(dirPinsArray[d],LOW);
    currentState[dirPinsArray[d]] = LOW; // Ready to go forward.
  }
}
