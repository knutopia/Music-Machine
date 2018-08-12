#ifndef __TIMEBASE
#define __TIMEBASE

#include <Arduino.h>
#include "Enum.h"
#include "Note.h"
#include "StepSequencer.h"

class Timebase
{    
    public:

      //Public constructor and methods
      Timebase();    
      void reset();
//    void prepPlay();

      // React to input ("Setters")
      
      void updateTempo(int newBPM);
      void updateSpeedMultiplier(speedFactor mult);
      void updateSwing(int swingPercentage);

      //"Getters"

      int getBPM();
      int getSwing();
      u_int8_t getSwingMidiClicks();
      void updateTimingIfNeeded();
      long getStepDurationMS(note aNote);
      long getReferenceStepDurationMS();
//    long getStepDurationRetrigHoldMS(note aNote, byte holdStepCount);
      u_int8_t getSwingTicks();
//    u_int8_t getSwingTicks(int step);

      void startPlayingRightNow();
      void runMidiTimer();
      void stopMidiTimer();

      //"Setters"
      void updateMidiTimer();
      void resetMidiTimer();

    private:

      enum{max_steps = 16};

      //Class data members:
      int bpm = 120; 
      speedFactor speedMultiplier = NORMAL; // DEGLOBALIZE
      
      unsigned long referenceTime;              // starting timestamp to calculate step time target from
      unsigned long stepsSinceReferenceTime;    // count of steps since reference time
      unsigned long referenceStepDuration;      // normal step duration
      unsigned long noteStartTime;              // note start time - updated whenever a start time is calculated
      int partialStepsSinceLast;                // partial step completion
      int referenceStepOffset;                    // delta from reference time to next step start
      
      byte stepSwingIndex;
      byte swingMidiClicks;
      int swingValue;
      bool resetRefTimer = false;
      byte remainingRetrigCount;
      const bool swingSteps[max_steps] = {false, true, false, true, 
                                          false, true, false, true, 
                                          false, true, false, true, 
                                          false, true, false, true};

      static unsigned long midiClickInterval;
      static bool bMidiTimerOn;
      volatile static int midiClickCount;
      volatile static int swingCountdown;
      volatile static int retrigCountdown;

      static retrigDivisions retrigClickDivider;
      static int midiSteps;
      static IntervalTimer midiTimer;
    
      //Helper methods
      void recalcTimings();

      static void midiClick();

};

#endif
