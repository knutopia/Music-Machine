#ifndef __TIMEBASE
#define __TIMEBASE

extern IntervalTimer myTimer;
extern unsigned long recentInterruptTime;
extern long g_step_duration;

class Timebase
{
    public:
    
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

    public:
    //Public constructor and methods
    Timebase()
    {
       reset();
    };
    
    void reset()
    {
        swingValue = 0;
        swingOffset = 0;
        retrigCount = 0;
        remainingRetrigCount = 0;
        referenceStepDuration = 60000000 / bpm / speedMultiplier;
        g_step_duration = referenceStepDuration;
    }

    void prepPlay()
    {

      Serial.println("Starting play.");

      // precalculate the timings
      initReferenceTime();

      // kick off interrupt
    }

    // React to input ("Setters")
    
    void updateTempo(int newBPM) 
    {
      // calculate the intervals when tempo changes
      bpm = newBPM;
//    recalcTimings();
      resetRefTimer = true;
    }

    void updateSpeedMultiplier(speedFactor mult) 
    {

      Serial.print("uSM remainingRetrigCount ");
      Serial.println(remainingRetrigCount);
      
      // calculate the intervals when tempo changes
      speedMultiplier = mult;
//    recalcTimings();
      resetRefTimer = true;
    }

    void updateSwing(int swingPercentage) 
    {
      swingValue = swingPercentage;
      // calculate the intervals when tempo changes
      // max swing is 1/3 step duration offset
      swingOffset = referenceStepDuration / 300 * swingValue;
    }

    void setRetrigCount(int count) 
    {
      retrigCount = count;
      remainingRetrigCount = retrigCount;
      timeRetrigStep();
    }

    byte getRetrigs()
    {
      return remainingRetrigCount;
    }
    
    byte getAndCountdownRetrigs()
    {
      byte retVal = remainingRetrigCount;
      if (remainingRetrigCount > 0) remainingRetrigCount--;
      return retVal;
    }

    void resetRemainingRetrigs()
    {
      remainingRetrigCount = retrigCount;      
    }

    //"Getters"

    int getBPM()
    {
      return bpm;
    }

    int getSwing()
    {
      return swingValue;
    }

    void updateTimingIfNeeded()
    {
      if(resetRefTimer) 
      {

        Serial.println("reseting reference time");
        
        resetRefTimetoMostRecentNote();   // if speed or multiplier changed
        resetRefTimer = false;
      }      
      recalcTimings();
    }
    
    long getNoteStartTime(int stepIndex) 
    {  
/*      
       // Test of time: how stable / variable ?
       static long older_interval;
       static long newer_interval;
       static unsigned long prev_interrupt_time;

       older_interval = newer_interval;
       newer_interval = (long)recentInterruptTime - (long)prev_interrupt_time;
       prev_interrupt_time = recentInterruptTime;

       Serial.print("Interval jitter:");
       Serial.println(newer_interval - older_interval);
*/       

      unsigned long nextStart = 0;
      
      if(stepIndex >=0 && stepIndex < max_steps) {
/*
          Serial.print("gT resetRefTimer ");
          Serial.print(resetRefTimer);
          Serial.print(" remainingRetrigCount ");
          Serial.print(remainingRetrigCount);
          Serial.print(" retrigCount ");
          Serial.println(retrigCount);
*/
          // calculate when the next step should trigger
          // and substract the current time
          // to get the delay until the interrupt that plays the step

          // possible conditions:
          // -A next note is a new beat
          // -B next note is a new tick within a beat
          // -C next note is a retrig during a beat
          // -D next note is a retrig during a tick
          
          if (remainingRetrigCount == 0) // cases A or B
          {              
              partialStepsSinceLast = 0;
              nextStart = referenceTime + (stepsSinceReferenceTime + 1) * referenceStepDuration
                          - referenceStepOffset * referenceStepDuration / 100;  // PROBABLY MINUS
              if (swingSteps[stepIndex]) nextStart += swingOffset;
              stepsSinceReferenceTime++;

//            Serial.println("gNST 1");

/*
              Serial.print("gT A stepDelay ");
              Serial.print(stepDelay);
              Serial.print("  stepsSinceReferenceTime ");
              Serial.print(stepsSinceReferenceTime);
              Serial.print("  referenceStepDuration ");
              Serial.println(referenceStepDuration);
*/
          } else // cases C or D
          {              
              partialStepsSinceLast = 100 * (retrigCount + 1 - remainingRetrigCount) / (retrigCount + 1);
              nextStart = referenceTime + (stepsSinceReferenceTime) * referenceStepDuration
                         - referenceStepOffset * referenceStepDuration / 100
                         + referenceStepDuration * partialStepsSinceLast / 100;
//            nextStart = referenceTime + (stepsSinceReferenceTime) * referenceStepDuration
//                       + retrigStepDuration * (retrigCount + 1 - remainingRetrigCount);
              if (swingSteps[stepIndex]) nextStart += swingOffset;

//            Serial.println("gNST 2");
              
          }
      } else {
         Serial.print("OOPSIE: getNoteStartTime stepIndex ");
         Serial.println(stepIndex);        
      }

      if (nextStart < micros()) {
         Serial.print("OOPSIE: nextStart behind time ");
         Serial.println(micros()-nextStart);                
      }
      noteStartTime = nextStart;
      return nextStart - micros();
    }

    long getStepDurationMS(float durationAsNoteFraction, byte holdStepCount)
    {
      // Using microseconds (not milliseconds)
      long retVal;
  
      // cases for note duration:
      // -A if simple note: 
      //   -note duration as stored
      //   -if there is a hold: note duration as stored plus full step
      //   -                    FUTURE: reverse the legato handling ?
      // -B if retrig:
      //   -shorter of retrig fraction or duration as stored
      // -C if last retrig, and hold coming up: handled below)
      //
      // -FUTURE: stretch retrigs across holds ?
      //
      // This is not taking ticks into account, but they are handled higher up.

      if (retrigCount == 0) {               // cases A
        retVal = (durationAsNoteFraction * referenceStepDuration) + holdStepCount * referenceStepDuration;
      } else 
      {                                     // case B
        retVal = durationAsNoteFraction * referenceStepDuration;
        if (retVal > retrigStepDuration) retVal = retrigStepDuration;
      }
      return retVal;
    }
    
    long getStepDurationRetrigHoldMS(float durationAsNoteFraction, byte holdStepCount)
    {
      // called if note is last retrig and there are holds coming up

      // Using microseconds (not milliseconds)
      long retVal;
  
      retVal = durationAsNoteFraction * referenceStepDuration;
      if (retVal > retrigStepDuration) retVal = retrigStepDuration;
      retVal += holdStepCount * referenceStepDuration;

      return retVal;
    }
    
    private:
    
    //Helper methods
    void initReferenceTime()
    {
      referenceTime = micros();
      stepsSinceReferenceTime = 0;      
/*      
      if (retrigCount > 0)
      {
        referenceTime += retrigStepDuration * remainingRetrigCount;
      }
*/
    }

    void resetRefTimetoMostRecentNote()
    {
      referenceTime = noteStartTime;
      stepsSinceReferenceTime = 0;
      referenceStepOffset = partialStepsSinceLast % 100;
//    referenceStepOffset = (100 - partialStepsSinceLast) % 100;

      Serial.print("rRTtMRN partialStepsSinceLast: ");
      Serial.print(partialStepsSinceLast);
      Serial.print(" referenceStepOffset: ");
      Serial.println(referenceStepOffset);
    }

    void recalcTimings()
    {
      referenceStepDuration = 60000000 / bpm / speedMultiplier;
      swingOffset = referenceStepDuration / 300 * swingValue;  
      retrigStepDuration = referenceStepDuration / (retrigCount + 1);
      g_step_duration = referenceStepDuration;
    }

    void timeRetrigStep()
    {
      retrigStepDuration = referenceStepDuration / (retrigCount + 1);      
    }
    
};

#endif
