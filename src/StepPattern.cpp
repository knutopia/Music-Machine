
#include "StepPattern.h"
#include "Timebase.h"
#include "Path.h"
#include "InOutHelper.h"

extern Timebase metro;
//extern Path playpath;
extern int midiTranspose;
extern InOutHelper inout;

StepPattern::StepPattern()
{
    m_length = MAXNOTES;
    m_transposition = 0;
    m_path = 0;
    reset();
};

void StepPattern::begin()
{
    reset();
}

void StepPattern::reset()
{
        //Initialize pattern flat
    for(int n = 0; n < MAXNOTES; n++)
    {
        m_notes[n] = 32;
        m_duration[n] = .5;
        m_probability[n] = 3;
        m_ticks[n] = 1;
        m_unmuted[n] = true;
        m_hold[n] = false;
        m_accent[n] = 0;
        m_retrig[n] = 0;
        m_velocity[n] = 128;
    }
}

unsigned long StepPattern::calcNextNoteDuration(const note aNote)
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
        retVal = metro.getStepDurationMS(aNote);
        
//    Serial.print("Hold count: ");
//    Serial.println(hold_count);

    } else {
        retVal = BLIP;  // huh ? this is odd
//      Serial.println("BLIP found"); // ...even a muted note needs a duration.
    }
    return retVal;
}

byte StepPattern::assembleHolds(const note aNote, Path aPath)
{
    // use getStepPosAfterNext to look ahead for holds. 
    // count consecutive forward-holds, to pass into getStepDurationMS
    byte holdStepCount = 0;
    if(aNote.unmuted)
    {
        byte stepOffset = 1;
        byte seqLength = getLength();
        bool holdNext = true;
        
        while (holdNext) {
            holdNext = getHold(aPath.getStepPosForward(stepOffset, seqLength));
            if (holdNext) {
                holdStepCount++;
                stepOffset++;
                if(holdStepCount > 100)
                {
                    inout.ShowErrorOnLCD("assembleHolds stuck");
                    break;
                }
            }
        }
    }
    return holdStepCount;
}

byte StepPattern::assembleMutes(const note aNote, Path aPath)
{
    // use getStepPosAfterNext to look ahead for holds. 
    // count consecutive forward-holds, to pass into getStepDurationMS
    byte muteStepCount = 0;
    if(aNote.unmuted)
    {
        byte stepOffset = 1;
        byte seqLength = getLength();
        bool muteNext = true;
        
        while (muteNext) {
            muteNext = !getMute(aPath.getStepPosForward(stepOffset, seqLength));
            if (muteNext) {
                muteStepCount++;
                stepOffset++;

                if(muteStepCount > 100)
                {
                    inout.ShowErrorOnLCD("assembleMutes stuck");
                    break;
                }
            }
        }
    }
    return muteStepCount;
}

void StepPattern::copyPatternTo(StepPattern &destination) 
{
    for(byte n = 0; n < MAXNOTES; n++)
    {
        destination.setNote(n, m_notes[n]); 
        destination.setDuration(n, m_duration[n]);
        destination.setProbability(n, m_probability[n]);
        destination.setTicks(n, m_ticks[n]);
        destination.setMute(n, m_unmuted[n]);
        destination.setHold(n, m_hold[n]);
        destination.setAccent(n, m_accent[n]);
        destination.setRetrig(n, m_retrig[n]);
        destination.setVelocity(n, m_velocity[n]);
    }
    destination.setLength(m_length);
    destination.setTransposition(m_transposition);
    destination.setPath(m_path);
}

void StepPattern::copyPatternTo(StepPattern *destination) 
{
    for(byte n = 0; n < MAXNOTES; n++)
    {
        destination->setNote(n, m_notes[n]); 
        destination->setDuration(n, m_duration[n]);
        destination->setProbability(n, m_probability[n]);
        destination->setTicks(n, m_ticks[n]);
        destination->setMute(n, m_unmuted[n]);
        destination->setHold(n, m_hold[n]);
        destination->setAccent(n, m_accent[n]);
        destination->setRetrig(n, m_retrig[n]);
        destination->setVelocity(n, m_velocity[n]);
    }
    destination->setLength(m_length);
    destination->setTransposition(m_transposition);
    destination->setPath(m_path);
}


// getters

byte StepPattern::getNote(int _step)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_notes[_step];
    } 
    return -1;  //error
}

float StepPattern::getDuration(int _step) // kg
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_duration[_step];
    } 
    return -1;  //error
}

byte StepPattern::getProbability(int _step) // kg
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_probability[_step];
    } 
    return -1;  //error
}
    
byte StepPattern::getTicks(int _step) // kg
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_ticks[_step];
    } 
    return -1;  //error
}

byte StepPattern::getTransposedNote(int _step)
{
    return getNote(_step) + m_transposition; 
}

byte StepPattern::getTransposition(){return m_transposition;};

void StepPattern::setTransposition(byte trans)
{
    m_transposition = trans; 
}

bool StepPattern::getMute(int _step)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_unmuted[_step];
    } 
    return false;  //error
}

bool StepPattern::getHold(int _step)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_hold[_step];
    } 
    return false;  //error
}

byte StepPattern::getAccent(int _step)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_accent[_step];
    } 
    return false;  //error
}

byte StepPattern::getRetrig(int _step)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_retrig[_step];
    } 
    return false;  //error
}

byte StepPattern::getVelocity(int _step)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        return m_velocity[_step];
    } 
    return false;  //error
}

bool StepPattern::playItOrNot(int _step)
{
    // take step probability into account 
    // and check if the step isn't holding from the previous step
    
    bool retVal = false;
    if (_step >=0 && _step < MAXNOTES)
    {
        if( !m_hold[_step] && m_unmuted[_step] )
        {
            switch ((int)m_probability[_step]) 
            {
                case LOWPROB:
                    if (random(100) < 20) retVal = true;
                    else retVal = false;
                    break;        
                case MEDPROB:
                    if (random(100) < 50) retVal = true;
                    else retVal = false;
                    break;
                case HIGHPROB:
                    if (random(100) < 80) retVal = true;
                    else retVal = false;
                    break;        
                case FULLPROB:
                    retVal = true;
                    break;
                default:
                    inout.ShowErrorOnLCD("playItOrNot default");
                    retVal = true;
            }
        }
    }
    return retVal;
}

retrigDivisions StepPattern::getRetrigDivider(int retrigs) 
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

note StepPattern::getPatternNoteParams(int _step, Path aPath)
{
    note thisNote;

    if(_step >=0 && _step < MAXNOTES)
    {
        thisNote.notEmpty = true;
        thisNote.retrigClickDivider = getRetrigDivider(m_retrig[_step]);
        thisNote.unmuted = m_unmuted[_step];
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
        thisNote.holdsAfter = assembleHolds(thisNote, aPath);
        thisNote.mutesAfter = assembleMutes(thisNote, aPath);
        thisNote.durationMS = calcNextNoteDuration(thisNote);
    }
    else {
        inout.ShowErrorOnLCD("gNPar out of range");
        Serial.print("getNoteParams out of range, step ");
        Serial.print(_step);
        Serial.print(" path ");
        Serial.println((int)&aPath);
    }
    return thisNote;
}

byte StepPattern::getLength(){return m_length;};

int StepPattern::getMaxLength(){return MAXNOTES;};

byte StepPattern::getPath(){return m_path;}


// setters

void StepPattern::setNote(int _step, byte note)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_notes[_step] = note;  
    }
}

void StepPattern::setLength(byte _length)
{
    if(_length <= MAXNOTES && _length >0)
    {
        m_length = _length;
    } 
}

void StepPattern::setDuration(int _step, float duration) // kg...
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_duration[_step] = duration;  
    }      
}

void StepPattern::setProbability(int _step, byte probabil) // kg...
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_probability[_step] = probabil;  
    }      
}
    
void StepPattern::setTicks(int _step, float repetition) // kg...
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_ticks[_step] = repetition;  
    }      
}

void StepPattern::setMute(int _step, bool muteFlag)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_unmuted[_step] = muteFlag;  
    }
}

void StepPattern::setHold(int _step, bool holdFlag)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_hold[_step] = holdFlag;  
    }
}

void StepPattern::setAccent(int _step, byte accent)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_accent[_step] = accent;  
    }
}

void StepPattern::setRetrig(int _step, byte retrig)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_retrig[_step] = retrig;  
    }
}

void StepPattern::setVelocity(int _step, byte velocity)
{
    if(_step >=0 && _step < MAXNOTES)
    {
        m_velocity[_step] = velocity;  
    }
}

void StepPattern::setPath(byte path)
{
    m_path = path;
}

// Utility

void StepPattern::printPattern()
{
    Serial.println("Note\tDuratn\tProb\tRepeat\tNotMute\tHold\tAccent\tRetrig\tVelocity");
    for(int n = 0; n < MAXNOTES; n++)
    {
        Serial.print(m_notes[n]); 
        Serial.print("\t"); 
        Serial.print(m_duration[n]);
        Serial.print("\t"); 
        Serial.print(m_probability[n]);
        Serial.print("\t"); 
        Serial.print(m_ticks[n]);
        Serial.print("\t"); 
        Serial.print(m_unmuted[n]);
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
