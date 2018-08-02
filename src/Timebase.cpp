#include "Timebase.h"
#include "InOutHelper.h"
#include "SynthEngine.h"

//#define MIDION true

extern SynthEngine synth;
extern InOutHelper inout;
extern StepClickList activeStepClicks;

#ifdef DEBUG
extern volatile unsigned long timeTracker;
#endif

extern volatile unsigned long v_note_off_time;
extern volatile bool vb_prep_next_step;
extern volatile bool vb_prep_retrig;

extern note nextNote;

extern long g_step_duration;

unsigned long Timebase::midiClickInterval;
bool Timebase::bMidiTimerOn = false;
volatile int Timebase::midiClickCount;
volatile int Timebase::swingCountdown = -1;
volatile int Timebase::retrigCountdown = -1;

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
    swingMidiClicks = 0;
    remainingRetrigCount = 0;
    referenceStepDuration = BPMCONSTANT / bpm / speedMultiplier;
    g_step_duration = referenceStepDuration;
    midiClickInterval = BPMCONSTANT / bpm / speedMultiplier / MIDICLOCKDIVIDER;
    resetMidiTimer();
//  resetSwingCountDown();
}

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
#ifdef DEBUG
        Serial.print("uSM remainingRetrigCount ");
        Serial.println(remainingRetrigCount);
#endif

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

    swingMidiClicks = (swingValue-2) / 12;

#ifdef DEBUG
    Serial.print("swingPercentage: ");
    Serial.print(swingPercentage);
    Serial.print("  swingMidiClicks: ");
    Serial.println(swingMidiClicks);
#endif
}

/*
void Timebase::updateSwing(int swing0to8) 
{
    swingValue = swing0to8;
    // calculate the intervals when tempo changes
    // max swing is 1/3 step duration offset

    swingMidiClicks = swing0to8
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

u_int8_t Timebase::getSwingMidiClicks()
{
    return swingMidiClicks;
}

void Timebase::updateTimingIfNeeded()
{
    if(resetRefTimer) 
    {
#ifdef DEBUG
        Serial.println("reseting reference time");
#endif

//    resetRefTimetoMostRecentNote();   // if speed or multiplier changed
        resetRefTimer = false;
        recalcTimings();
        updateMidiTimer();
    }      
}

long Timebase::getStepDurationMS(note aNote, byte holdStepCount) // USE NOTE
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

#ifdef DEBUG
    Serial.print("getStepDurationMS ");
#endif

    if (aNote.retrigs == 0) {             // cases A
        retVal = (aNote.duration * referenceStepDuration) + holdStepCount * referenceStepDuration;
   
#ifdef DEBUG
        Serial.print(" case A  ");
        Serial.println(retVal);
#endif

    } else 
    {                                     // case B
        unsigned long retrigStepDuration = referenceStepDuration / (aNote.retrigs + 1)*.9;      

        retVal = aNote.duration * referenceStepDuration;
        if (retVal > retrigStepDuration)
        {
            retVal = retrigStepDuration;
        }

#ifdef DEBUG
        Serial.print(" case B  ");
        Serial.println(retVal);
#endif

    }
    return retVal;
}

/*
long Timebase::getStepDurationRetrigHoldMS(note aNote, byte holdStepCount)
{
    // called if note is last retrig and there are holds coming up

    // Using microseconds (not milliseconds)
    unsigned long retVal;

    retrigStepDuration = referenceStepDuration / (aNote.retrigs + 1);      

    retVal = aNote.duration * referenceStepDuration;
    if (retVal > retrigStepDuration) retVal = retrigStepDuration;
    retVal += holdStepCount * referenceStepDuration;

    return retVal;
}
*/

u_int8_t Timebase::getSwingTicks()
{
    u_int8_t retVal = 0;
    if(swingSteps[stepSwingIndex])
    {
        retVal = swingMidiClicks;
    } 
    return retVal;
}


// private:

//Helper methods

void Timebase::recalcTimings()
{
    referenceStepDuration = BPMCONSTANT / bpm / speedMultiplier;
    g_step_duration = referenceStepDuration;
    midiClickInterval = BPMCONSTANT / bpm / speedMultiplier / MIDICLOCKDIVIDER;
}

void Timebase::startPlayingRightNow()
{
    midiClickCount = MIDICLOCKDIVIDER;
    midiClick();
    runMidiTimer();
}

void Timebase::runMidiTimer()
{
    stopMidiTimer();
    bMidiTimerOn = true;

#ifdef DEBUG
    Serial.print("runMidiTimer midiClickInterval: ");
    Serial.println(midiClickInterval);
#endif

//    midiClickCount = 0;
//    resetSwingCountDown();
    midiTimer.begin(Timebase::midiClick, midiClickInterval);
}

void Timebase::stopMidiTimer()
{
    if (bMidiTimerOn) {
#ifdef DEBUG
        Serial.println("stopMidiTimer");
#endif
        bMidiTimerOn = false;
        midiTimer.end();
    }
}

void Timebase::updateMidiTimer()
{
    if (bMidiTimerOn) {
#ifdef DEBUG
        Serial.println("updateMidiTimer");
#endif
        midiTimer.update(midiClickInterval);
    }
}

void Timebase::resetMidiTimer()
{
#ifdef DEBUG
    Serial.println("resetMidiTimer");
#endif

    if (bMidiTimerOn) {
        midiTimer.end();
    }
    bMidiTimerOn = false;
    midiClickCount = 0;
    swingCountdown = 0;
    retrigCountdown = 0;
    midiSteps = 0;
    midiTimer.priority(255);

}

void Timebase::midiClick()
{
    midiClickCount++;

    if (midiClickCount >= MIDICLOCKDIVIDER)
    {
        midiClickCount = 0;
        vb_prep_next_step = true;
    }    
    PerClickNoteList* notesToTrig;
    if((notesToTrig = activeStepClicks
                      .getClickNoteList(midiClickCount))
                      != NULL)
    {
        while(notesToTrig->hasValue())
        {
            note trigNote = notesToTrig->getNote();
            unsigned long trigDur = notesToTrig->getDurationMS();
            byte trigTrack = notesToTrig->getTrack();
        
            if(trigNote.playIt)
                synth.playNote(trigTrack, trigNote);

            // DIRTY
            if( trigTrack == 1)
            {            
                unsigned long now = micros();
                v_note_off_time = now + trigDur;
            }
            notesToTrig->next();
        }
    }
    activeStepClicks.rewind();
}

// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
// *                 *                  *                 *              
// *                       *                        *
// *                                    *

/*
void Timebase::advanceStepSwingIndex()
{
    stepSwingIndex++;
}

void Timebase::resetStepSwingIndex()
{
    stepSwingIndex = 0;
}

void Timebase::resetSwingCountDown()
{
    swingCountdown = -1;
    retrigCountdown = -1;
}
*/