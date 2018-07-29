
#include "StepSequence.h"
#include "Timebase.h"
#include "Path.h"

extern Timebase metro;
extern Path playpath;
extern int midiTranspose;


StepSequence::StepSequence()
{
    m_length = max_notes;
    m_transposition = 0;
    m_path = 0;
    reset();
};

void StepSequence::reset()
{
        //Initialize sequence flat
    for(int n = 0; n < max_notes; n++)
    {
        m_notes[n] = 32;
        m_duration[n] = .5;
        m_probability[n] = 3;
        m_ticks[n] = 1;
        m_mute[n] = true;
        m_hold[n] = false;
        m_accent[n] = 0;
        m_retrig[n] = 0;
        m_velocity[n] = 128;
    }
}

unsigned long StepSequence::calcNextNoteDuration(note aNote)
{
    unsigned long retVal;

    // cases for note duration:
    // -A if simple note: 
    //   -note duration as stored
    //   -if there is a hold: full step plus hold (use legato note's duration ?)
    // -B if retrig within note:
    //   -retrig fraction
    //   -if there is a hold and retrig is last one: retrig fraction plus hold (use legato note's duration ?)
    // -C if muted: blip (to keep timer going)
    // -FUTURE: stretch retrigs across holds ?
    
        
    if (aNote.unmuted) {
        byte hold_count = assembleHolds();
        retVal = metro.getStepDurationMS(aNote, hold_count);
        
//    Serial.print("Hold count: ");
//    Serial.println(hold_count);

    } else {
        retVal = BLIP;  // huh ? this is odd
//      Serial.println("BLIP found"); // ...even a muted note needs a duration.
    }
    return retVal;
}

byte StepSequence::assembleHolds()
{
    // use getStepPosAfterNext to look ahead for holds. 
    // count consecutive forward-holds, to pass into getStepDurationMS
    byte holdStepCount = 0;
    byte stepOffset = 1;
    byte seqLength = getLength();
    bool holdNext = true;
    
    while (holdNext) {
        holdNext = getHold(playpath.getStepPosForward(stepOffset, seqLength));
        if (holdNext) {
        holdStepCount++;
        stepOffset++;
        }
    }
    return holdStepCount;
}

void StepSequence::copySeqTo(StepSequence &destination) 
{
    for(byte n = 0; n < max_notes; n++)
    {
        destination.setNote(n, m_notes[n]); 
        destination.setDuration(n, m_duration[n]);
        destination.setProbability(n, m_probability[n]);
        destination.setTicks(n, m_ticks[n]);
        destination.setMute(n, m_mute[n]);
        destination.setHold(n, m_hold[n]);
        destination.setAccent(n, m_accent[n]);
        destination.setRetrig(n, m_retrig[n]);
        destination.setVelocity(n, m_velocity[n]);
    }
    destination.setLength(m_length);
    destination.setTransposition(m_transposition);
    destination.setPath(m_path);
}


// getters

byte StepSequence::getNote(int _step)
{
    if(_step >=0 && _step < max_notes)
    {
        return m_notes[_step];
    } 
    return -1;  //error
}

float StepSequence::getDuration(int _step) // kg
{
    if(_step >=0 && _step < max_notes)
    {
        return m_duration[_step];
    } 
    return -1;  //error
}

byte StepSequence::getProbability(int _step) // kg
{
    if(_step >=0 && _step < max_notes)
    {
        return m_probability[_step];
    } 
    return -1;  //error
}
    
byte StepSequence::getTicks(int _step) // kg
{
    if(_step >=0 && _step < max_notes)
    {
        return m_ticks[_step];
    } 
    return -1;  //error
}

byte StepSequence::getTransposedNote(int _step)
{
    return getNote(_step) + m_transposition; 
}

byte StepSequence::getTransposition(){return m_transposition;};

void StepSequence::setTransposition(byte trans)
{
    m_transposition = trans; 
}

bool StepSequence::getMute(int _step)
{
    if(_step >=0 && _step < max_notes)
    {
        return m_mute[_step];
    } 
    return false;  //error
}

bool StepSequence::getHold(int _step)
{
    if(_step >=0 && _step < max_notes)
    {
        return m_hold[_step];
    } 
    return false;  //error
}

byte StepSequence::getAccent(int _step)
{
    if(_step >=0 && _step < max_notes)
    {
        return m_accent[_step];
    } 
    return false;  //error
}

byte StepSequence::getRetrig(int _step)
{
    if(_step >=0 && _step < max_notes)
    {
        return m_retrig[_step];
    } 
    return false;  //error
}

byte StepSequence::getVelocity(int _step)
{
    if(_step >=0 && _step < max_notes)
    {
        return m_velocity[_step];
    } 
    return false;  //error
}

bool StepSequence::playItOrNot(int _step)
{
    // take step probability into account 
    // and check if the step isn't holding from the previous step
    
    bool retVal = false;
    if (_step >=0 && _step < max_notes)
    {
        if( !m_hold[_step] && m_mute[_step] )
        {
            switch ((int)m_probability[_step]) 
            {
                case ZEROPROB:
                    retVal = false;
                    break;        
                case LOWPROB:
                    if (random(100) < 33) retVal = true;
                    else retVal = false;
                    break;        
                case HIGHPROB:
                    if (random(100) < 66) retVal = true;
                    else retVal = false;
                    break;        
                case FULLPROB:
                    retVal = true;
                    break;
            }
        }
    }
    return retVal;
}

retrigDivisions StepSequence::getRetrigDivider(int retrigs) 
{
    retrigDivisions retVal;

    switch (retrigs)
    {
        case 0:
            retVal = NORETRIGS;
            break;
        case 1:
            retVal = ONERETRIG;
            break;
        case 2:
            retVal = TWORETRIGS;
            break;
        case 3:
            retVal = THREERETRIGS;
            break;
        default:
            retVal = NORETRIGS;
            break;
    }
    return retVal;
}

/*
note StepSequence::getNoteParams(int _step)
{
    if(_step >=0 && _step < max_notes)
    {
        m_noteStruct.retrigClickDivider = getRetrigDivider(m_retrig[_step]);
        m_noteStruct.unmuted = m_mute[_step];
        m_noteStruct.playIt = playItOrNot(_step);
        m_noteStruct.pitchVal = m_notes[_step] + midiTranspose;
        m_noteStruct.pitchFreq = (float) 440.0 
                             * (float)(pow(2, ((m_notes[_step] + midiTranspose -57) 
                             / 12.0)));
        m_noteStruct.hold = m_hold[_step];
        m_noteStruct.retrigs = m_retrig[_step];

        m_noteStruct.duration = m_duration[_step];
        m_noteStruct.ticks = m_ticks[_step];
        m_noteStruct.accent = m_accent[_step];
        m_noteStruct.velocity = m_velocity[_step];

        m_noteStruct.swingTicks = metro.getSwingTicks();
        m_noteStruct.durationMS = calcNextNoteDuration(m_noteStruct);
    }
    return m_noteStruct;
}
*/

note StepSequence::getNoteParams(int _step)
{
    note thisNote;

    if(_step >=0 && _step < max_notes)
    {
        thisNote.retrigClickDivider = getRetrigDivider(m_retrig[_step]);
        thisNote.unmuted = m_mute[_step];
        thisNote.playIt = playItOrNot(_step);
        thisNote.pitchVal = m_notes[_step] + midiTranspose;
        thisNote.pitchFreq = (float) 440.0 
                             * (float)(pow(2, ((m_notes[_step] + midiTranspose -57) 
                             / 12.0)));
        thisNote.hold = m_hold[_step];
        thisNote.retrigs = m_retrig[_step];

        thisNote.duration = m_duration[_step];
        thisNote.ticks = m_ticks[_step];
        thisNote.accent = m_accent[_step];
        thisNote.velocity = m_velocity[_step];

        thisNote.swingTicks = metro.getSwingTicks();
        thisNote.durationMS = calcNextNoteDuration(thisNote);
    }
    return thisNote;
}

byte StepSequence::getLength(){return m_length;};

int StepSequence::getMaxLength(){return max_notes;};

byte StepSequence::getPath(){return m_path;}


// setters

void StepSequence::setNote(int _step, byte note)
{
    if(_step >=0 && _step < max_notes)
    {
        m_notes[_step] = note;  
    }
}

void StepSequence::setLength(byte _length)
{
    if(_length <= max_notes && _length >0)
    {
        m_length = _length;
    } 
}

void StepSequence::setDuration(int _step, float duration) // kg...
{
    if(_step >=0 && _step < max_notes)
    {
        m_duration[_step] = duration;  
    }      
}

void StepSequence::setProbability(int _step, byte probabil) // kg...
{
    if(_step >=0 && _step < max_notes)
    {
        m_probability[_step] = probabil;  
    }      
}
    
void StepSequence::setTicks(int _step, float repetition) // kg...
{
    if(_step >=0 && _step < max_notes)
    {
        m_ticks[_step] = repetition;  
    }      
}

void StepSequence::setMute(int _step, bool muteFlag)
{
    if(_step >=0 && _step < max_notes)
    {
        m_mute[_step] = muteFlag;  
    }
}

void StepSequence::setHold(int _step, bool holdFlag)
{
    if(_step >=0 && _step < max_notes)
    {
        m_hold[_step] = holdFlag;  
    }
}

void StepSequence::setAccent(int _step, byte accent)
{
    if(_step >=0 && _step < max_notes)
    {
        m_accent[_step] = accent;  
    }
}

void StepSequence::setRetrig(int _step, byte retrig)
{
    if(_step >=0 && _step < max_notes)
    {
        m_retrig[_step] = retrig;  
    }
}

void StepSequence::setVelocity(int _step, byte velocity)
{
    if(_step >=0 && _step < max_notes)
    {
        m_velocity[_step] = velocity;  
    }
}

void StepSequence::setPath(byte path)
{
    m_path = path;
}

// Utility

void StepSequence::printSequence()
{
    Serial.println("Note\tDuratn\tProb\tRepeat\tNotMute\tHold\tAccent\tRetrig\tVelocity");
    for(int n = 0; n < max_notes; n++)
    {
        Serial.print(m_notes[n]); 
        Serial.print("\t"); 
        Serial.print(m_duration[n]);
        Serial.print("\t"); 
        Serial.print(m_probability[n]);
        Serial.print("\t"); 
        Serial.print(m_ticks[n]);
        Serial.print("\t"); 
        Serial.print(m_mute[n]);
        Serial.print("\t"); 
        Serial.print(m_hold[n]);
        Serial.print("\t"); 
        Serial.print(m_accent[n]);
        Serial.print("\t"); 
        Serial.print(m_retrig[n]);
        Serial.print("\t"); 
        Serial.println(m_velocity[n]);
    }
    Serial.println("Length\tTrans\tPath"); 
    Serial.print(m_length); 
    Serial.print("\t"); 
    Serial.print(m_transposition); 
    Serial.print("\t"); 
    Serial.println(m_path);
}
