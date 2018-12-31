#include "Timebase.h"
#include "InOutHelper.h"
#include "SynthEngine.h"
#include "NoteOffList.h"
#include "NotePerClick.h"

#define MIDION true

extern SynthEngine synth;
extern InOutHelper inout;
extern StepClickList activeStepClicks;
extern NoteOffList playingNotes;
//extern PerClickNoteList notesToTrig;
extern volatile notePerClick notesToPlay[];
extern elapsedMicros clickTrack;


#ifdef DEBUG
extern volatile unsigned long timeTracker;
#endif

extern volatile unsigned long v_note_off_time;
extern volatile unsigned long v_note_trigger_time;
//extern volatile bool vb_prep_next_step;
extern volatile bool vb_clickHappened;
extern int g_midiClickCount;

extern note nextNote;

extern long g_step_duration;

unsigned long Timebase::midiClickInterval;
unsigned long Timebase::loopCutoff;

bool Timebase::bMidiTimerOn = false;
volatile int Timebase::midiClickCount;
volatile int Timebase::swingCountdown = -1;
volatile int Timebase::retrigCountdown = -1;

int Timebase::midiSteps;
retrigDivisions Timebase::retrigClickDivider;
// IntervalTimer Timebase::midiTimer; // NOTIMER

Timebase::Timebase()
{
    reset();
};

void Timebase::reset()
{
    swingMidiClicks = 0;
    remainingRetrigCount = 0;
    referenceStepDuration = BPMCONSTANT / bpm / speedMultiplier;
    g_step_duration = referenceStepDuration;
    midiClickInterval = BPMCONSTANT / bpm / speedMultiplier / MIDICLOCKDIVIDER;
    effectiveBpm = bpm * speedMultiplier;
    loopCutoff = midiClickInterval - constrain(midiClickInterval / 4, 1000, 15000);

    resetMidiTimer();
}

// React to input ("Setters")

void Timebase::updateTempo(int newBPM) 
{
    // calculate the intervals when tempo changes
    if (bpm != newBPM)
    {
        bpm = newBPM;
        effectiveBpm = bpm * speedMultiplier;
        inout.ShowBPMOnLCD(effectiveBpm);
  
        resetRefTimer = true;
    }
}

void Timebase::updateSpeedMultiplier(speedFactor mult) 
{
    if (mult != speedMultiplier)
    {
#ifdef DEBUG
        Serial.print("updateSpeedMultiplier speedMultiplier ");
        Serial.print(speedMultiplier);
        Serial.print(" newval ");
        Serial.println(mult);
#endif

        // calculate the intervals when tempo changes
        speedMultiplier = mult;
        effectiveBpm = bpm * speedMultiplier;
        inout.ShowBPMOnLCD(effectiveBpm);
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

int Timebase::getEffectiveBPM()
{
    return effectiveBpm;
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
//#ifdef DEBUG
        Serial.println("updateTimingIfNeeded triggered");
//#endif

//    resetRefTimetoMostRecentNote();   // if speed or multiplier changed
        resetRefTimer = false;
        recalcTimings();
        updateMidiTimer();
    }      
}

long Timebase::truncateSwingStepDuration(const note aNote)
{
    long retVal = aNote.durationMS;

    if(aNote.holdsAfter == 0 && aNote.mutesAfter == 0 && aNote.swingTicks > 0)
    {
        float factor = ((float)(MIDICLOCKDIVIDER - aNote.swingTicks - 3.0) / (float)MIDICLOCKDIVIDER);
        long durAvail = referenceStepDuration * factor;
        if(durAvail < retVal)
            retVal = durAvail;

//      inout.ShowValueInfoOnLCD("factor ", factor);

    }

    if(retVal == 0)
    {
//      inout.ShowErrorOnLCD("truncSSD 0");
        Serial.print("truncateSwingStepDuration retval 0  ");
        Serial.print("aNote.swingTicks ");
        Serial.println(aNote.swingTicks);
    }   

    return retVal;
}

long Timebase::getStepDurationMS(const note aNote) // USE NOTE
{
    // Using microseconds (not milliseconds)
    unsigned long retVal = 0;

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

    if(retVal == 0)
        inout.ShowErrorOnLCD("getStepDurationMS 0");

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
    loopCutoff = midiClickInterval - constrain(midiClickInterval / 4, 1000, 15000);

    Serial.println("");
    Serial.print("midiClickInterval ");
    Serial.println(midiClickInterval);
}

void Timebase::startPlayingRightNow()
{
    midiClickCount = MIDICLOCKDIVIDER;
    g_midiClickCount = MIDICLOCKDIVIDER;
//  midiClick();
//  shortMidiClick();
    
    clickTrack = 0;

    arrayMidiClick();
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
//  midiTimer.begin(Timebase::shortMidiClick, midiClickInterval);
//  midiTimer.begin(Timebase::arrayMidiClick, midiClickInterval); // NOTIMER
}

void Timebase::stopMidiTimer()
{
    if (bMidiTimerOn) {
#ifdef DEBUG
        Serial.println("stopMidiTimer");
#endif
        bMidiTimerOn = false;
//      midiTimer.end(); // NOTIMER
    }
}

void Timebase::updateMidiTimer()
{
    if (bMidiTimerOn) {
#ifdef DEBUG
        Serial.println("updateMidiTimer");
#endif
//      midiTimer.update(midiClickInterval); // NOTIMER
    }
}

void Timebase::resetMidiTimer()
{
#ifdef DEBUG
    Serial.println("resetMidiTimer");
#endif

    if (bMidiTimerOn) {
//      midiTimer.end(); // NOTIMER
    }
    bMidiTimerOn = false;
    midiClickCount = 0;
    g_midiClickCount = 0;
    swingCountdown = 0;
    retrigCountdown = 0;
    midiSteps = 0;
//  midiTimer.priority(255); // NOTIMER

}

/*
void Timebase::shortMidiClick()
{
//  Serial.print("trying ");
    int sentry = 0;
    while(notesToTrig.hasReadValue())
    {        
        note trigNote = notesToTrig.readNote();
        unsigned long trigDur = notesToTrig.readDurationMS();
        byte trigTrack = notesToTrig.readTrack();

#ifdef DEBUG
        Serial.print(" playing ");
        Serial.println(trigNote.pitchVal);
#endif

        unsigned long now = micros();
        //for the step indicators...
        if(trigTrack ==1)
            v_note_off_time = now + trigDur;

        v_note_trigger_time = now;

        if(trigNote.playIt)
        {
            synth.playNote(trigTrack, trigNote);
        }
        notesToTrig.readNext();

        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("shMClk stuck");
            break;
        }
    }
    vb_clickHappened = true;
}
*/

void Timebase::arrayMidiClick()
{
//  Serial.print("trying ");

    for(int i = 0; i < TRACKCOUNT; i++)
    {        
        if(notesToPlay[i].active)
        {
            unsigned long trigDur = notesToPlay[i].durationMS;
            byte trigTrack = notesToPlay[i].track;

#ifdef DEBUG
            Serial.print(" playing track ");
            Serial.print(notesToPlay[i].track);
            Serial.print(" note ");
            Serial.println(notesToPlay[i].clickNote.pitchVal);
#endif

            unsigned long now = micros();
            //for the step indicators...
            if(trigTrack ==1)
                v_note_off_time = now + trigDur;

            v_note_trigger_time = now;

            if(notesToPlay[i].clickNote.playIt)
            {
                note playNote;

                playNote.retrigClickDivider = notesToPlay[i].clickNote.retrigClickDivider;
                playNote.unmuted = notesToPlay[i].clickNote.unmuted;
                playNote.playIt = notesToPlay[i].clickNote.playIt;
                playNote.pitchVal = notesToPlay[i].clickNote.pitchVal;
                playNote.pitchFreq = notesToPlay[i].clickNote.pitchFreq;
                playNote.durationMS = notesToPlay[i].clickNote.durationMS;
                playNote.hold = notesToPlay[i].clickNote.hold;
                playNote.duration = notesToPlay[i].clickNote.duration;
                playNote.retrigs = notesToPlay[i].clickNote.retrigs;
                playNote.ticks = notesToPlay[i].clickNote.ticks;
                playNote.accent = notesToPlay[i].clickNote.accent;
                playNote.velocity = notesToPlay[i].clickNote.velocity;
                playNote.swingTicks = notesToPlay[i].clickNote.swingTicks;
                playNote.holdsAfter = notesToPlay[i].clickNote.holdsAfter;  
                playNote.mutesAfter = notesToPlay[i].clickNote.mutesAfter;  
                
                synth.playNote(trigTrack, playNote);
            }
        }
    }
    vb_clickHappened = true;

#ifdef MIDION
    usbMIDI.send_now();
#endif
}
// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
// *                 *                  *                 *              
// *                       *                        *
// *                                    *
