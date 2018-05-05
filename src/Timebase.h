#ifndef __TIMEBASE
#define __TIMEBASE

#include <Arduino.h>
#include "Enum.h"

class Timebase
{    
    public:

      //Public constructor and methods
      Timebase();    
      void reset();
      void prepPlay();

      // React to input ("Setters")
      
      void updateTempo(int newBPM);
      void updateSpeedMultiplier(speedFactor mult);
      void updateSwing(int swingPercentage);
      void setRetrigCount(int count);
      byte getRetrigs();
      byte getAndCountdownRetrigs();
      void resetRemainingRetrigs();

      //"Getters"

      int getBPM();
      int getSwing();
      void updateTimingIfNeeded();
      long getNoteStartTime(int stepIndex);
      long getStepDurationMS(float durationAsNoteFraction, byte holdStepCount);
      long getStepDurationRetrigHoldMS(float durationAsNoteFraction, byte holdStepCount);
      
    private:

      enum{max_steps = 16};

      //Class data members:
      int bpm = 120; 
      speedFactor speedMultiplier = NORMAL; // DEGLOBALIZE
      
      unsigned long referenceTime;              // starting timestamp to calculate step time target from
      unsigned long stepsSinceReferenceTime;    // count of steps since reference time
      unsigned long referenceStepDuration;      // normal step duration
      unsigned long swingOffset;                // additional time for a swing step
      unsigned long retrigStepDuration;         // duration for a step with retriggering active
      unsigned long noteStartTime;              // note start time - updated whenever a start time is calculated
      int partialStepsSinceLast;                // partial step completion
      int referenceStepOffset;                    // delta from reference time to next step start
      
      byte nextStepIndex;
      int swingValue;
      bool resetRefTimer = false;
      byte retrigCount;
      byte remainingRetrigCount;
      const bool swingSteps[max_steps] = {false, true, false, true, 
                                          false, true, false, true, 
                                          false, true, false, true, 
                                          false, true, false, true};
    
      //Helper methods
      void initReferenceTime();
      void resetRefTimetoMostRecentNote();
      void recalcTimings();
      void timeRetrigStep();
};

#endif
