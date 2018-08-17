#include "Timebase.h"
#include "InOutHelper.h"
#include "SynthEngine.h"
#include "NoteOffList.h"

//#define MIDION true

extern SynthEngine synth;
extern InOutHelper inout;
extern StepClickList activeStepClicks;
extern NoteOffList playingNotes;
extern PerClickNoteList notesToTrig;

#ifdef DEBUG
extern volatile unsigned long timeTracker;
#endif

extern volatile unsigned long v_note_off_time;
extern volatile unsigned long v_note_trigger_time;
extern volatile bool vb_prep_next_step;
extern volatile bool vb_clickHappened;
extern int g_midiClickCount;

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
//  swingValue = 0;
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


//void Timebase::updateSwing(int swingPercentage) 
void Timebase::updateSwing(int newClicks) 
{
    swingMidiClicks = newClicks;
}


//"Getters"

int Timebase::getBPM()
{
    return bpm;
}

int Timebase::getSwing()
{
    return swingMidiClicks;
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

long Timebase::truncateSwingStepDuration(note aNote)
{
    long retval = aNote.durationMS;

    if(aNote.holdsAfter == 0 && aNote.swingTicks > 0)
    {
        float factor = ((float)(MIDICLOCKDIVIDER - aNote.swingTicks - 1.0) / (float)MIDICLOCKDIVIDER);
        long durAvail = referenceStepDuration * factor;
        if(durAvail < retval)
            retval = durAvail;

//      inout.ShowValueInfoOnLCD("factor ", factor);

    }
    return retval;
}

long Timebase::getStepDurationMS(note aNote) // USE NOTE
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
        retVal = (aNote.duration * referenceStepDuration) + aNote.holdsAfter * referenceStepDuration;
   
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

long Timebase::getReferenceStepDurationMS()
{
    return referenceStepDuration;
}

u_int8_t Timebase::getSwingTicks()
{
    return swingMidiClicks;
}

/*
u_int8_t Timebase::getSwingTicks(int step)
{
    u_int8_t retVal = 0;
    if(swingSteps[step])
    {
        retVal = swingMidiClicks;
    } 
    return retVal;
}
*/

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
    g_midiClickCount = MIDICLOCKDIVIDER;
//  midiClick();
    shortMidiClick();
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

//  midiTimer.begin(Timebase::midiClick, midiClickInterval);
    midiTimer.begin(Timebase::shortMidiClick, midiClickInterval);
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
    g_midiClickCount = 0;
    swingCountdown = 0;
    retrigCountdown = 0;
    midiSteps = 0;
    midiTimer.priority(255);

}

void Timebase::midiClick()
{
    static int currentPlayingStep = 0;
    bool prep_next = false;

    midiClickCount++;

    if (midiClickCount >= MIDICLOCKDIVIDER)
    {
        midiClickCount = 0;
        currentPlayingStep = g_activeGlobalStep;
        prep_next = true;
#ifdef DEBUG                                    
        Serial.print("&&&&&& prep_next set, g_activeGlobalStep is ");
        Serial.println(g_activeGlobalStep);
#endif
    }    
    PerClickNoteList* notesToTrig;
    if((notesToTrig = activeStepClicks
                      .getClickNoteList(midiClickCount, currentPlayingStep))
                      != NULL)
    {
        notesToTrig->readRewind();
        while(notesToTrig->hasReadValue())
        {
            note trigNote = notesToTrig->readNote();
            unsigned long trigDur = notesToTrig->readDurationMS();
            byte trigTrack = notesToTrig->readTrack();
        
                unsigned long now = micros();
                //for the step indicators...
                if(trigTrack ==1)
                    v_note_off_time = now + trigDur;


            if(trigNote.playIt)
            {
                synth.playNote(trigTrack, trigNote);

                playingNotes.append(trigTrack, 
                                    trigNote.pitchVal, 
                                    (now + trigDur));
#ifdef DEBUG                                    
                Serial.print("Playing ");
//              Serial.print(g_activeGlobalStep);
//              Serial.print(" or ");
                Serial.print(currentPlayingStep);
                Serial.print(" at click ");
                Serial.print(midiClickCount);
                Serial.print(" on track ");
                Serial.println(trigTrack);
#endif
            } else 
            {
#ifdef DEBUG
                Serial.print("Don't play it ! ");
                Serial.print(g_activeGlobalStep);
                Serial.print(" on track ");
                Serial.println(trigTrack);
#endif
            }
            notesToTrig->readNext();
        }
    } 
    activeStepClicks.readRewind();

    if(prep_next)
    {
        vb_prep_next_step = true;
    }
}

void Timebase::shortMidiClick()
{
    while(notesToTrig.hasReadValue())
    {
        note trigNote = notesToTrig.readNote();
        unsigned long trigDur = notesToTrig.readDurationMS();
        byte trigTrack = notesToTrig.readTrack();

        unsigned long now = micros();
        //for the step indicators...
        if(trigTrack ==1)
            v_note_off_time = now + trigDur;

        v_note_trigger_time = now;

        if(trigNote.playIt)
        {
            synth.playNote(trigTrack, trigNote);
//          v_note_trigger_time = now;
/*
            playingNotes.append(trigTrack, 
                                trigNote.pitchVal, 
                                (now + trigDur));
*/
        }
        notesToTrig.readNext();
    }
    vb_clickHappened = true;
}
// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
// *                 *                  *                 *              
// *                       *                        *
// *                                    *
