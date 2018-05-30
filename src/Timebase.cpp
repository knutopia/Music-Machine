#include "Timebase.h"
#include "InOutHelper.h"
#include "SynthEngine.h"

extern SynthEngine synth;
extern InOutHelper inout;


extern volatile unsigned long v_note_off_time;
extern volatile bool vb_prep_next_step;

extern note nextNote;

extern long g_step_duration;

unsigned long Timebase::midiClickInterval;
bool Timebase::bMidiTimerOn = false;
volatile int Timebase::midiClickCount;
volatile int Timebase::swingCountdown = -1;

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

//  float scaledSwing = round((float) swingPercentage / (100.0 / 9.0) - .5);
//  swingMidiClicks = uint8_t(scaledSwing);

    swingMidiClicks = (swingValue-2) / 12;

    Serial.print("swingPercentage: ");
    Serial.print(swingPercentage);
    Serial.print("  swingMidiClicks: ");
    Serial.println(swingMidiClicks);
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
        Serial.println("reseting reference time");
        
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

    if (aNote.retrigs == 0) {               // cases A
        retVal = (aNote.duration * referenceStepDuration) + holdStepCount * referenceStepDuration;
//      inout.ShowInfoOnLCD("no retrigs.");
    } else 
    {                                     // case B
//      inout.ShowValueInfoOnLCD("Retrigs:", aNote.retrigs);

        unsigned long retrigStepDuration = referenceStepDuration / (aNote.retrigs + 1)*.9;      

        retVal = aNote.duration * referenceStepDuration;
        if (retVal > retrigStepDuration) retVal = retrigStepDuration;
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

void Timebase::runMidiTimer()
{
    stopMidiTimer();
    bMidiTimerOn = true;

    Serial.print("runMidiTimer midiClickInterval: ");
    Serial.println(midiClickInterval);

    midiClickCount = 0;
    swingCountdown = 0;
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
    swingCountdown = 0;
    midiSteps = 0;
    midiTimer.priority(255);

}

void Timebase::midiClick()
{
    static note currentNote;

    midiClickCount++;
//  usbMIDI.sendRealTime(usbMIDI.Clock);
//  usbMIDI.send_now();
    if (midiClickCount >= MIDICLOCKDIVIDER)
    {
        Serial.print("1 mCC: ");
        Serial.print(midiClickCount);
        Serial.print("  ");
        
        midiClickCount = 0;
        currentNote = nextNote;
        swingCountdown = currentNote.swingTicks;

/*
        recentInterruptTime = micros();
        v_note_off_time = recentInterruptTime + currentNote.durationMS;
*/      
//      vb_prep_next_step = true;
/*   
        if (currentNote.playIt) 
            synth.playNote(currentNote);
*/
    } else {
        // handle retrigs
        if (currentNote.retrigClickDivider != NORETRIGS)
        {
            if (midiClickCount % (currentNote.retrigClickDivider) 
                == currentNote.swingTicks) 
            {
                Serial.print("2 mCC: ");
                Serial.print(midiClickCount);
                Serial.print("  ");

//              v_note_off_time = recentInterruptTime + currentNote.durationMS; // FIX THIS
                v_note_off_time = micros() + currentNote.durationMS; // FIX THIS
                if (currentNote.playIt) 
                    synth.playNote(currentNote);
            }
        }
    }
    if(swingCountdown == 0)
    {
        v_note_off_time = micros() + currentNote.durationMS;
            
        vb_prep_next_step = true;
        
        if (currentNote.playIt) 
            synth.playNote(currentNote);

    }
    swingCountdown--;

    usbMIDI.sendRealTime(usbMIDI.Clock);
    usbMIDI.send_now();
}
// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
// *                 *                  *                 *              
// *                       *                        *
// *                                    *

void Timebase::advanceStepSwingIndex()
{
    stepSwingIndex++;
}

void Timebase::resetStepSwingIndex()
{
    stepSwingIndex = 0;
}
