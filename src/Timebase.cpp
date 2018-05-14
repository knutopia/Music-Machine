#include "Timebase.h"
#include "SynthEngine.h"

extern SynthEngine synth;

extern volatile unsigned long recentInterruptTime;
extern volatile unsigned long v_note_off_time;
extern volatile bool vb_prep_next_step;

extern note nextNote;

extern long g_step_duration;

unsigned long Timebase::midiClickInterval;
bool Timebase::bMidiTimerOn = false;
volatile int Timebase::midiClickCount;
int Timebase::midiSteps;
retrigDivisions Timebase::retrigClickDivider;
IntervalTimer Timebase::midiTimer;

Timebase::Timebase()
{
    reset();
};

void Timebase::reset()
{
    swingValue = 0;
    swingOffset = 0;
    retrigCount = 0;
    remainingRetrigCount = 0;
    referenceStepDuration = BPMCONSTANT / bpm / speedMultiplier;
    retrigStepDuration = referenceStepDuration / (retrigCount + 1);
    g_step_duration = referenceStepDuration;
    midiClickInterval = BPMCONSTANT / bpm / speedMultiplier / MIDICLOCKDIVIDER;
    resetMidiTimer();
}

/*
void Timebase::prepPlay()
{

    Serial.println("Starting play.");

    // precalculate the timings
    initReferenceTime();

    // kick off interrupt
}
*/

// React to input ("Setters")

void Timebase::updateTempo(int newBPM) 
{
    // calculate the intervals when tempo changes
    if (bpm != newBPM)
    {
        bpm = newBPM;
//      recalcTimings();
        resetRefTimer = true;
    }
}

void Timebase::updateSpeedMultiplier(speedFactor mult) 
{
    if (mult != speedMultiplier)
    {
        Serial.print("uSM remainingRetrigCount ");
        Serial.println(remainingRetrigCount);
        
        // calculate the intervals when tempo changes
        speedMultiplier = mult;
//      recalcTimings();
        resetRefTimer = true;
    }
}

void Timebase::updateSwing(int swingPercentage) 
{
    swingValue = swingPercentage;
    // calculate the intervals when tempo changes
    // max swing is 1/3 step duration offset
    swingOffset = referenceStepDuration / 300 * swingValue;
}
/*
void Timebase::setRetrigCount(int count) // USE retrigCount in NOTE
{
    retrigCount = count;
    remainingRetrigCount = retrigCount;

    switch (count)
    {
        case 0:
            Timebase::retrigClickDivider = NORETRIGS;
            break;
        case 1:
            Timebase::retrigClickDivider = ONERETRIG;
            break;
        case 2:
            Timebase::retrigClickDivider = TWORETRIGS;
            break;
        case 3:
            Timebase::retrigClickDivider = THREERETRIGS;
            break;
        default:
            break;
    }

    timeRetrigStep();
}
*/
/*
byte Timebase::getRetrigs()
{
    return remainingRetrigCount;
}
*/
/*
byte Timebase::getAndCountdownRetrigs()
{
    byte retVal = remainingRetrigCount;
    if (remainingRetrigCount > 0) remainingRetrigCount--;
    return retVal;
}
*/
/*
void Timebase::resetRemainingRetrigs()
{
    remainingRetrigCount = retrigCount;      
}
*/

//"Getters"

int Timebase::getBPM()
{
    return bpm;
}

int Timebase::getSwing()
{
    return swingValue;
}

void Timebase::updateTimingIfNeeded()
{
    if(resetRefTimer) 
    {
        Serial.println("reseting reference time");
        
//    resetRefTimetoMostRecentNote();   // if speed or multiplier changed
        resetRefTimer = false;
        recalcTimings();
        updateMidiTimer();
    }      
}

/*
long Timebase::getNoteStartTime(int stepIndex) 
{        
    unsigned long nextStart = 0;
    
    if(stepIndex >=0 && stepIndex < max_steps) {
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
*/

long Timebase::getStepDurationMS(float durationAsNoteFraction, byte holdStepCount) // USE NOTE
{
    // Using microseconds (not milliseconds)
    unsigned long retVal;

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
        retrigStepDuration = referenceStepDuration / (retrigCount + 1);      

        retVal = durationAsNoteFraction * referenceStepDuration;
        if (retVal > retrigStepDuration) retVal = retrigStepDuration;
    }
    return retVal;
}

long Timebase::getStepDurationRetrigHoldMS(float durationAsNoteFraction, byte holdStepCount)
{
    // called if note is last retrig and there are holds coming up

    // Using microseconds (not milliseconds)
    unsigned long retVal;

    retrigStepDuration = referenceStepDuration / (retrigCount + 1);      

    retVal = durationAsNoteFraction * referenceStepDuration;
    if (retVal > retrigStepDuration) retVal = retrigStepDuration;
    retVal += holdStepCount * referenceStepDuration;

    return retVal;
}

// private:

//Helper methods
/*
void Timebase::initReferenceTime()
{
    referenceTime = micros();
    stepsSinceReferenceTime = 0;      
}
*/

/*
void Timebase::resetRefTimetoMostRecentNote()
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
*/

void Timebase::recalcTimings()
{
    referenceStepDuration = BPMCONSTANT / bpm / speedMultiplier;
    swingOffset = referenceStepDuration / 300 * swingValue;  
    retrigStepDuration = referenceStepDuration / (retrigCount + 1);
    g_step_duration = referenceStepDuration;
    midiClickInterval = BPMCONSTANT / bpm / speedMultiplier / MIDICLOCKDIVIDER;
}

/*
void Timebase::timeRetrigStep()
{
    retrigStepDuration = referenceStepDuration / (retrigCount + 1);      
}
*/

void Timebase::runMidiTimer()
{
    stopMidiTimer();
    bMidiTimerOn = true;

    Serial.print("runMidiTimer midiClickInterval: ");
    Serial.println(midiClickInterval);

    midiClickCount = 0;
    midiTimer.begin(Timebase::midiClick, midiClickInterval);
}

void Timebase::stopMidiTimer()
{
    if (bMidiTimerOn) {
        Serial.println("stopMidiTimer");
        bMidiTimerOn = false;
        midiTimer.end();
    }
}

void Timebase::updateMidiTimer()
{
    if (bMidiTimerOn) {
        Serial.println("updateMidiTimer");
        midiTimer.update(midiClickInterval);
    }
}

void Timebase::resetMidiTimer()
{
    Serial.println("resetMidiTimer");
    if (bMidiTimerOn) {
        midiTimer.end();
    }
    bMidiTimerOn = false;
    midiClickCount = 0;
    midiSteps = 0;
    midiTimer.priority(255);

}

void Timebase::midiClick()
{
    static note currentNote;

    midiClickCount++;
//    usbMIDI.sendRealTime(usbMIDI.Clock);
//    usbMIDI.send_now();
    if (midiClickCount >= MIDICLOCKDIVIDER)
    {
        Serial.print("1 mCC: ");
        Serial.print(midiClickCount);
        Serial.print("  ");
        
        midiClickCount = 0;
        currentNote = nextNote;
        recentInterruptTime = micros();
        v_note_off_time = recentInterruptTime + currentNote.durationMS;
            
        vb_prep_next_step = true;
//      b_timer_on = true;
        
        if (currentNote.playIt) 
            synth.playNote(currentNote);
    } else {
        // handle retrigs
        if (currentNote.retrigClickDivider != NORETRIGS)
        {
            if (midiClickCount % currentNote.retrigClickDivider == 0) 
            {
                Serial.print("2 mCC: ");
                Serial.print(midiClickCount);
                Serial.print("  ");

                v_note_off_time = recentInterruptTime + currentNote.durationMS; // FIX THIS
                if (currentNote.playIt) 
                    synth.playNote(currentNote);
            }
        }
    }
}
// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
// *                 *                  *                 *              
// *                       *                        *
// *                                    *