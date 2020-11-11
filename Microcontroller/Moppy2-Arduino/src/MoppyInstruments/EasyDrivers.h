/*
 * EasyDrivers.h
 *
 */

#ifndef SRC_MOPPYINSTRUMENTS_EASYDRIVERS_H_
#define SRC_MOPPYINSTRUMENTS_EASYDRIVERS_H_

#include <Arduino.h>
#include "MoppyTimer.h"
#include "MoppyInstrument.h"
#include "../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

    #define M1_PIN_STEP 2 // X-STEP
    #define M2_PIN_STEP 3 // Y-STEP
    #define M3_PIN_STEP 4 // Z-STEP

    #define M1_PIN_DIR 5 // X-DIR
    #define M2_PIN_DIR 6 // Y-DIR
    #define M3_PIN_DIR 7 // Z-DIR

    #define HDD1_PIN 9  //X-Endstop
    #define HDD2_PIN 10 //Y-Endstop
    #define HDD3_PIN 11 //Z-Endstop
    #define HDD4_PIN 12 //SpinEnable
    #define HDD5_PIN 13 //SpinEnable

namespace instruments {
  class EasyDrivers : public MoppyInstrument {
  public:
    void setup();
  protected:
      void sys_sequenceStop() override;
      void sys_reset() override;

      void dev_reset(uint8_t subAddress) override;
      void dev_noteOn(uint8_t subAddress, uint8_t payload[]) override;
      void dev_noteOff(uint8_t subAddress, uint8_t payload[]) override;
      void dev_bendPitch(uint8_t subAddress, uint8_t payload[]) override;
  private:
    static unsigned int MAX_POSITION[];
    static unsigned int currentPosition[];
    static int currentState[];
    static unsigned int currentPeriod[];
    static unsigned int currentTick[];
    static unsigned int originalPeriod[];

    static void resetAll();
    static void togglePin(byte driverNum, byte pin, byte direction_pin);
    static void toggleHDDPin(byte pin);
    static void haltAllDrivers();
    static void reset(byte driverNum);
    static void tick();
    static void blinkLED();
    static void startupSound(byte driverNum);
  };
}

#endif /* SRC_MOPPYINSTRUMENTS_EASYDRIVERS_H_ */
