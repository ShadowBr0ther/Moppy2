/*
 * CNCShieldV3.cpp
 * @author : ShadowBrother
 * Output for controlling EasyDrivers.
 */
#include "MoppyInstrument.h"
#include "CNCShieldV3.h"


namespace instruments {
// This is used for calculating step and direction pins.
const byte FIRST_DRIVER = 1;
const byte LAST_DRIVER = 3;  // This sketch can handle only up to 3 drivers (the max for Arduino Uno)


// Maximum note number to attempt to play on easydrivers.  It's possible higher notes may work,
// but they need to be added in "MoppyInstrument.h".
const byte MAX_DRIVER_NOTE = 119;


/*NOTE: The arrays below contain unused zero-indexes to avoid having to do extra
 * math to shift the 1-based subAddresses to 0-based indexes here.  Unlike the previous
 * version of Moppy, we *will* be doing math to calculate which driver maps to which pin,
 * so there are as many values as drivers (plus the extra zero-index)
 */


// Microstep Resolution of each stepper motor: Fixed Full step

/*Array to keep track of state of each pin.  Even indexes track the step-pins for toggle purposes.  Odd indexes
 track direction-pins.  LOW = forward, HIGH=reverse <- This depends on the wiring of the stepper motor with the EasyDriver.
 */
int CNCShieldV3::currentState[] = {0,0,LOW,LOW,LOW,LOW,LOW,LOW};

// Current period assigned to each driver.  0 = off.  Each period is two-ticks (as defined by
// TIMER_RESOLUTION in MoppyInstrument.h) long.
unsigned int CNCShieldV3::currentPeriod[] = {0,0,0,0,0};

// Tracks the current tick-count for each driver (see EasyDrivers::tick() below)
unsigned int CNCShieldV3::currentTick[] = {0,0,0,0,0};

// The period originally set by incoming messages (prior to any modifications from pitch-bending)
unsigned int CNCShieldV3::originalPeriod[] = {0,0,0,0,0};


bool CNCShieldV3::getCurStatByNumb(byte driverNumber, bool isDirPin){
  if (!isDirPin){
    return currentState[1+(2*(driverNumber-1))]; //1,3,5 
  }
  else{
    return currentState[2+(2*(driverNumber-1))];  //2,4,6 
  }
}

void CNCShieldV3::setCurStatByNumb(byte driverNumber, bool state, bool isDirPin) {    
  if (!isDirPin) {
    currentState[1+(2*(driverNumber-1))] = state;}
 else
  {
    currentState[2+(2*(driverNumber-1))] = state;
  } 
}

void CNCShieldV3::setPinByNumb(byte driverNumber, bool state, bool isDirPin){
  byte pin;

  if (isDirPin) { 
    driverNumber = driverNumber + 3;
  }

  switch (driverNumber)
  {
    case 1:pin = M1_PIN_STEP; break;
    case 2:pin = M2_PIN_STEP; break;
    case 3:pin = M3_PIN_STEP; break;
    case 4:pin = M1_PIN_DIR; break;
    case 5:pin = M2_PIN_DIR; break;
    case 6:pin = M3_PIN_DIR; break;
  }
  digitalWrite(pin, state);
  setCurStatByNumb(driverNumber, state, isDirPin);
}


byte CNCShieldV3::getPinByNumb(byte driverNumber, bool isDirPin){
  byte Pin; 

  if (isDirPin) { 
    driverNumber = driverNumber + 3;
  }

  switch (driverNumber)
  {
    case 1: Pin = M1_PIN_STEP; break;
    case 2: Pin = M2_PIN_STEP; break;
    case 3: Pin = M3_PIN_STEP; break;
    case 4: Pin = M1_PIN_DIR; break;
    case 5: Pin = M2_PIN_DIR; break;
    case 6: Pin = M3_PIN_DIR; break;
  }
  return Pin;
}


void CNCShieldV3::setup() {

  // Prepare pins (0 and 1 are reserved for Serial communications)
  pinMode(M1_PIN_STEP, OUTPUT); // Step pin 1
  pinMode(M1_PIN_DIR, OUTPUT); // Direction pin 1

  pinMode(M2_PIN_STEP, OUTPUT); // Step pin 2
  pinMode(M2_PIN_DIR, OUTPUT); // Direction pin 2

  pinMode(M3_PIN_STEP, OUTPUT); // Step pin 3
  pinMode(M3_PIN_DIR, OUTPUT); // Direction pin 3

  // With all pins setup, let's do a first run reset
  resetAll();

  delay(500); // Wait a half second for safety

  // Setup timer to handle interrupts for drivers driving
  MoppyTimer::initialize(TIMER_RESOLUTION, tick);

  // If MoppyConfig wants a startup sound, play the startupSound on the
  // first driver.
  if (PLAY_STARTUP_SOUND) {
    startupSound(FIRST_DRIVER);
    delay(500);
    resetAll();
  }
}

// Play startup sound to confirm driver functionality
void CNCShieldV3::startupSound(byte driverNum) {
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
      currentPeriod[driverNum] = chargeNotes[i++];
    }
  }
}

//
//// Message Handlers
//

void CNCShieldV3::sys_reset() {
    resetAll();
}

void CNCShieldV3::sys_sequenceStop() {
    haltAllDrivers();
}

void CNCShieldV3::dev_reset(uint8_t subAddress) {
    if (subAddress == 0x00) {
        resetAll();
    } else {
        reset(subAddress);
    }
}

void CNCShieldV3::dev_noteOn(uint8_t subAddress, uint8_t payload[]) {
    // Set the current period to the new value to play it immediately
    // Also set the originalPeriod in-case we pitch-bend
    if (payload[0] <= MAX_DRIVER_NOTE) {
        currentPeriod[subAddress] = originalPeriod[subAddress] = noteDoubleTicks[payload[0]];
    }
}

void CNCShieldV3::dev_noteOff(uint8_t subAddress, uint8_t payload[]) {
    currentPeriod[subAddress] = originalPeriod[subAddress] = 0;
}

void CNCShieldV3::dev_bendPitch(uint8_t subAddress, uint8_t payload[]) {
    // A value from -8192 to 8191 representing the pitch deflection
    int16_t bendDeflection = payload[0] << 8 | payload[1];

    // A whole octave of bend would double the frequency (halve the the period) of notes
    // Calculate bend based on BEND_OCTAVES from MoppyInstrument.h and percentage of deflection
    //currentPeriod[subAddress] = originalPeriod[subAddress] / 1.4;
    currentPeriod[subAddress] = originalPeriod[subAddress] / pow(2.0, BEND_OCTAVES * (bendDeflection / (float)8192));
}

//
//// Driver driving functions
//

/*
Called by the timer interrupt at the specified resolution.  Because this is called extremely often,
it's crucial that any computations here be kept to a minimum!
 */
void CNCShieldV3::tick()
{
  /*
   If there is a period set for step pin 2, count the number of
   ticks that pass, and toggle the pin if the current period is reached.
   */

  if (currentPeriod[1]>0){
    currentTick[1]++;
    if (currentTick[1] >= currentPeriod[1]){
      togglePin(1,M1_PIN_STEP, M1_PIN_DIR); // Drive 1 is on pins 2 and 3
      currentTick[1]=0;
    }
  }
  if (currentPeriod[2]>0){
    currentTick[2]++;
    if (currentTick[2] >= currentPeriod[2]){
      togglePin(2,M2_PIN_STEP, M2_PIN_DIR);
      currentTick[2]=0;
    }
  }
  if (currentPeriod[3]>0){
    currentTick[3]++;
    if (currentTick[3] >= currentPeriod[3]){
      togglePin(3,M3_PIN_STEP, M3_PIN_DIR);
      currentTick[3]=0;
    }
  }
}

void CNCShieldV3::togglePin(byte driverNum, byte placeholder1, byte placeholder2) {
// Switch directions if either end has been reached.
  if (digitalRead(SWITCH_PIN_COUNTERCLOCKWISE)==LOW) { // If front direction pin is on, change direction.
    setPinByNumb(getCurStatByNumb(driverNum, true), true, true);
  }
  else { // If rear direction pin is on, change direction.
    setPinByNumb(getCurStatByNumb(driverNum, true), false, true);
  }

  // Pulse the step pin
  setPinByNumb(getCurStatByNumb(driverNum, false), getCurStatByNumb(driverNum, false), false);
  setCurStatByNumb(driverNum, !getCurStatByNumb(driverNum, false), false); //Invert
}


//
//// UTILITY FUNCTIONS
//

// Not used now, but good for debugging...
void CNCShieldV3::blinkLED(){
  digitalWrite(13, HIGH); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(13, LOW);
}

// Immediately stops all drivers
void CNCShieldV3::haltAllDrivers() {
  for (byte d=FIRST_DRIVER;d<=LAST_DRIVER;d++) {
    currentPeriod[d] = 0;
  }
}

// For a given driver number, runs e.g. the scanner-head all the way back to the rear
void CNCShieldV3::reset(byte driverNum)
{
  currentPeriod[driverNum] = 0; // Stop note
  setPinByNumb(getCurStatByNumb(driverNum, true), false, true);
  setPinByNumb(getCurStatByNumb(driverNum, false), false, false);
}

// Resets all the drivers simultaneously
void CNCShieldV3::resetAll()
{
  for (int i = FIRST_DRIVER; i < LAST_DRIVER; i++)
  {
    reset(i);
  }
}

} // namespace instruments
