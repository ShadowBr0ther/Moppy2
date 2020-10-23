/*
 * CNCSHIELDV3.h
 *
 */

#ifndef SRC_MOPPYINSTRUMENTS_CNCSHIELDV3_H_
#define SRC_MOPPYINSTRUMENTS_CNCSHIELDV3_H_

#include <Arduino.h>
#include "MoppyTimer.h"
#include "MoppyInstrument.h"
#include "../MoppyConfig.h"
#include "../MoppyNetworks/MoppyNetwork.h"

namespace instruments {
  class CNCShieldV3 : public MoppyInstrument {
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
    static void haltAllDrivers();
    static void reset(byte driverNum);
    static void tick();
    static void blinkLED();
    static void startupSound(byte driverNum);

    //#######################################
    static byte getPinByNumb(byte driverNumber, bool isDirPin);
    static void setPinByNumb(byte driverNumber, bool state, bool isDirPin);

    static bool getCurStatByNumb(byte driverNumber, bool isDirPin);
    static void setCurStatByNumb(byte driverNumber, bool state, bool isDirPin);
  };
}

#endif /* SRC_MOPPYINSTRUMENTS_CNCSHIELDV3_H_ */
