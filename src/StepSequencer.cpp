//Arduino for Musicians
//StepSequencer: A container class for StepPattern objects

#include "StepSequencer.h"
#include "Track.h"
#include "LinkedNoteList.h"
#include "StepClickList.h"
#include "Timebase.h"
#include "InOutHelper.h"

#define DEBUG true

// LinkedNoteList StepSequencer::activeNotes;
// StepClickList StepSequencer::activeStepClicks;

extern LinkedNoteList activeNotes;
extern StepClickList activeStepClicks;
extern Timebase metro;
extern InOutHelper inout;
extern bool startFromZero;
extern bool shiftActive;

//Public constructor and methods
StepSequencer::StepSequencer()
{
//  m_currentPattern = 0;
//  m_recall_buffer_active = false;
    g_activeGlobalStep = 0;

#ifdef DEBUG
    Serial.print("Size of m_pattern is ");
    Serial.print(sizeof(m_pattern));
    Serial.print(" for ");
    Serial.print(max_patterns);
    Serial.println(" sequences ");
#endif
}

void StepSequencer::begin(CbWhenStuck panicCbPointer)
{
    PanicCb = panicCbPointer;
    
    int forceInitForPattern = m_pattern[0].getLength();

    for(int s=0; s < max_patterns; s++)
    {
        int len = m_beat_pattern[s].getLength();
        for(int n = 0; n < len; n++)
        {
            m_beat_pattern[s].setDuration(n, 0.1);
            m_beat_pattern[s].setNote(n, 20+random(12));
        }
        for(int n = 0; n < len; n+=4)
            m_beat_pattern[s].setDuration(n, 0.5);
    }

    tmpTrack1.begin(m_pattern, m_pattern_root, max_patterns, (byte)1);
    tmpTrack2.begin(m_beat_pattern, m_beat_pattern_root, max_patterns, (byte)2);
    m_beat_pattern[0].copyPatternTo(m_beat_pattern_root[0]);

    m_activeTracks.appendTrack(1, &tmpTrack1);
    m_activeTracks.appendTrack(2, &tmpTrack2);

    activeEditTrack = &tmpTrack1;
    activeEditTrack->setCurrentPatternIndex(0);
}

void StepSequencer::updateNoteList()
{
    // get notes for step, per track
    // use global step and step in pattern where appropriate

    //step through active tracks
    //get note, do processing per track type (swingticks, duration)
    //append the note to the noteList
    //take earlier steps off the noteList

    //needed for this:
    //  active track list as linkedList to step through
    //  per-track getNoteParams();
    //  per-track calcNextNoteDuration();
    //  as functions in linkedList of active tracks


    note cur_note;
    byte cur_track;

#ifdef DEBUG
    Serial.print("updateNoteList g_activeGlobalStep: ");
    Serial.println(g_activeGlobalStep);
    
    Serial.print("  NotesL: ");
    Serial.println(activeNotes.count());

    Serial.print("  TracksL: ");
    Serial.println(m_activeTracks.count());
#endif

    if(&m_activeTracks == NULL) 
    {
        inout.ShowErrorOnLCD("updateNL: m_ac NULL");
        return;
    }

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("updateNL: m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        Track *cur_trackRef = m_activeTracks.getTrackRef();

        if (cur_trackRef != NULL)
        {
            if(!cur_trackRef->isMuted())
            {
                cur_note = cur_trackRef->getTrackNoteParams();
                cur_track = m_activeTracks.getTrackNumber();
                if(&activeNotes != NULL)
                    activeNotes.appendNote(g_activeGlobalStep, cur_track, cur_note);
                else 
                {
                    inout.ShowErrorOnLCD("updateNL: aN NULL");
                    break;
                }
            }
        }
        else
        {
            inout.ShowErrorOnLCD("updateNL: NULL");
            Serial.println("updateNL: NULL cur_trackRef");
        }

#ifdef DEBUG
        Serial.print("updateNoteList note ");
        Serial.print(cur_note.pitchVal);
        Serial.print(" on track ");
        Serial.print(cur_track);
        Serial.print(" for step ");
        Serial.print(g_activeGlobalStep);
//      Serial.print(" from pattern ");
//      Serial.println(m_currentPattern);
#endif

        m_activeTracks.next();
        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("updateNL stuck");
            break;
        }
    }
}

void StepSequencer::updateStepClickList()
{
    // prepare notes per click to be used in interrupt
    //
    // challenge here or elsewhere: deal with ticks...

#ifdef DEBUG
    Serial.println("updateStepClickList");
#endif

    if (&activeNotes == NULL)
    {
        inout.ShowErrorOnLCD("updateSCL: aN NULL");
        return;
    }

    activeNotes.rewind();

    int sentry = 0;
    while( activeNotes.hasValue())
    {
        note aNote = activeNotes.getNote();

#ifdef DEBUG
        activeNotes.printActiveNote();
#endif

#ifdef DEBUG
        Serial.print("  track: ");
        Serial.println(activeNotes.getTrack());
        Serial.print("  step: ");
        Serial.println(activeNotes.getStep());
        Serial.print("  Note pitch before assign: ");
        Serial.println(aNote.pitchVal);
        Serial.print("  ");
#endif
        int swingOffset = 0;
        if(activeNotes.getStep() % 2 == 0)
        {
            swingOffset = aNote.swingTicks;

            // truncate the note duration if there is another one behind
            aNote.durationMS = metro.truncateSwingStepDuration(aNote);
        }

        activeStepClicks.addClickNote(  aNote, 
                                        activeNotes.getTrack(),
                                        aNote.durationMS, 
                                        activeNotes.getStep(),
                                        swingOffset);

#ifdef DEBUG
        Serial.print("Beat adds- ");
        Serial.print("Step: ");
        Serial.print(activeNotes.getStep());
        Serial.print(" Pos: ");
        Serial.print("0");
        Serial.print(" from retrigClickDivider: ");
        Serial.print(aNote.retrigClickDivider);
        Serial.print(" with swingTicks: ");
        Serial.println(aNote.swingTicks);
#endif

        if (aNote.retrigClickDivider != NORETRIGS)
        {
            bool retrigLegato = false;
            bool retrigSoundOut = false;

            if(aNote.holdsAfter > 0)
                retrigLegato = true;
            if(aNote.mutesAfter > 0)
                retrigSoundOut = true;

            for(uint8_t count = 0; count < aNote.retrigs; count++)
            {
                int clickPos = (count + 1) * aNote.retrigClickDivider 
                                + swingOffset;
                long duration = aNote.durationMS;

                // for legato, tuck on a step duration to last retrig...
                if(retrigLegato && (count+1 == aNote.retrigs))
                {
                    duration += aNote.duration * metro.getReferenceStepDurationMS();
                    // ...but shorten it if the click was pushed over to next step
                    if(clickPos > 24)
                        duration -= (clickPos - 24.0) / 24.0 * metro.getReferenceStepDurationMS();
                }
/*
                if(clickPos > 24) {
                    Serial.print("updateStepClickList: clickPos out of bounds: ");        
                    Serial.print(clickPos);
                    Serial.print(" from retrigClickDivider ");        
                    Serial.print(aNote.retrigClickDivider);
                    Serial.print(" with swingOffset ");        
                    Serial.print(swingOffset);
                    Serial.print(" at count ");
                    Serial.println(count);
                }
*/
                bool noteQualifies = false;

                if(clickPos <= 24 || retrigLegato || retrigSoundOut) 
                    noteQualifies = true;

                if(noteQualifies)
                    activeStepClicks.addClickNote(  aNote, 
                                                    activeNotes.getTrack(),
                                                    duration, 
                                                    activeNotes.getStep(), 
                                                    clickPos);
#ifdef DEBUG
                Serial.print("Retrig adds- ");
                Serial.print("Step: ");
                Serial.print(activeNotes.getStep());
                Serial.print(" Pos: ");
                Serial.print(clickPos);
                Serial.print(" from retrigClickDivider: ");
                Serial.print(aNote.retrigClickDivider);
                Serial.print(" with swingTicks: ");
                Serial.println(aNote.swingTicks);
#endif
            }
        }
        activeNotes.next();
        
        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("updateSCL stuck");
            break;
        }
    }
}

void StepSequencer::AdvanceStepPositions()
{
    if(&m_activeTracks == NULL) 
    {
        inout.ShowErrorOnLCD("advSP: m_ac NULL");
        return;
    }

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("advSP: m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        m_activeTracks.getTrackRef()->advanceStepPosition();
        m_activeTracks.next();

        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("advSP stuck");
            break;
        }
    }
}

void StepSequencer::resetStepPositions()
{
   if(&m_activeTracks == NULL) 
   {
       inout.ShowErrorOnLCD("resetSP: m_ac NULL");
       return;
   }

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("resetSP: m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        m_activeTracks.getTrackRef()->resetStepPosition();
        m_activeTracks.next();

        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("resetSP stuck");
            break;
        }
    }
}

void StepSequencer::prepFirstStep()
{
   if(&m_activeTracks == NULL)
   {
        inout.ShowErrorOnLCD("pFStep: m_ac NULL");
        return;
   }

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("pFStep: m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        m_activeTracks.getTrackRef()->prepFirstStep();
        m_activeTracks.next();

        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("pFStep stuck");
            break;
        }
    }
}


/*
bool StepSequencer::playItOrNot(int _step) //make obsolete
{
    // take step probability into account 
    // and check if the step isn't holding from the previous step
    
    bool retVal = false;

    StepPattern* curSeq = activeEditTrack->getCurrentPatternRef();
    if (curSeq == NULL)
    {
        inout.ShowErrorOnLCD("playION curSeq NULL");
        return retVal;
    }

    if (!curSeq->getHold(_step))
    {
        int stepProbability = (int)activeEditTrack->getCurrentPatternRef()->getProbability(_step);
        switch (stepProbability) {
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
        }
  }
  return retVal;
}
*/

/*
void StepSequencer::prime_edit_buffers() // UNUSED - ADDRESS ?
{
    for(int i = 0; i < max_patterns; i++) 
      reset_edit_seq(i);            
}

void StepSequencer::reset_edit_seq(int seqnum) // ADDRESS
{
    m_pattern_root[seqnum].copyPatternTo(m_pattern[seqnum]);            
}
*/

void StepSequencer::save_pattern(int destination) // TODO // ADDRESSed
{
//  m_pattern[m_currentPattern].copyPatternTo(m_pattern_root[destination]);
//  m_pattern[m_currentPattern].copyPatternTo(m_pattern[destination]);

    StepPattern* curSeq = activeEditTrack->getCurrentPatternRef();
    byte curSeqIndex = activeEditTrack->getCurrentPatternIndex();
    StepPattern* curRootSeq = activeEditTrack->getRootPatternRef(curSeqIndex);
    StepPattern* destRootSeq = activeEditTrack->getRootPatternRef(destination);
    StepPattern* destEditSeq = activeEditTrack->getPatternRef(destination);
    if(curRootSeq != NULL && curSeq != NULL && destRootSeq != NULL && destEditSeq != NULL)
    {
        curSeq->copyPatternTo(destRootSeq);
        curSeq->copyPatternTo(destEditSeq);
        if( curSeqIndex != destination)
        {
            curRootSeq->copyPatternTo(curSeq);
            activeEditTrack->setCurrentPatternIndex(destination);
        }
    } else {
        inout.ShowErrorOnLCD("save_seq NULL");
        Serial.print("save_pattern NULL error: curSeq = ");
        Serial.print((int)curSeq);
        Serial.print("  curRootSeq = ");
        Serial.print((int)curRootSeq);
        Serial.print("  destEditSeq = ");
        Serial.print((int)destEditSeq);
        Serial.print("  destRootSeq = ");
        Serial.println((int)destRootSeq);
    }
}

void StepSequencer::save_all_patterns()
{
    byte curTrackNumBuf = activeEditTrack->getNumber();

    if(&m_activeTracks == NULL)
    {
        inout.ShowErrorOnLCD("s_a_s: m_ac NULL");
        return;
    }

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("s_a_s m_ac noval");

    while(m_activeTracks.hasValue())
    {        
        activeEditTrack = m_activeTracks.getTrackRef();
        if(activeEditTrack != NULL)
        {
            byte curSeqNumBuf = activeEditTrack->getCurrentPatternIndex();

            for(int f = 0; f < max_patterns; f++)
            {
                activeEditTrack->setCurrentPatternIndex(f);
                save_pattern(f);

                Serial.print("  saved seq ");
                Serial.print(f);
                Serial.print("  on track ");
                Serial.print(m_activeTracks.getTrackNumber());
                Serial.print(" to root");
            }
            setCurrentPattern(curSeqNumBuf);
        } else
            inout.ShowErrorOnLCD("s_a_s aET NULL");
        m_activeTracks.next();
    }
    setCurrentTrack(curTrackNumBuf);
}

void StepSequencer::save_edit_pattern_to_root(int seqnum) // ADDRESSed
{
//  m_pattern[seqnum].copyPatternTo(m_pattern_root[seqnum]);

    StepPattern* indexedSeq = activeEditTrack->getPatternRef(seqnum);
    StepPattern* indexedRootSeq = activeEditTrack->getRootPatternRef(seqnum);
    if(indexedSeq != NULL && indexedRootSeq != NULL)
    {
        indexedSeq->copyPatternTo(indexedRootSeq);
    } else {
        Serial.print("save_edit_pattern_to_root NULL error: indexedSeq = ");
        Serial.print((int)indexedSeq);
        Serial.print("  indexedRootSeq = ");
        Serial.println((int)indexedRootSeq);
    }

}

void StepSequencer::copy_edit_buffers_to_roots()
{
    Track* bufTrack = activeEditTrack;

    if(&m_activeTracks == NULL)
    {
        inout.ShowErrorOnLCD("copyEBtR: m_ac NULL");
        return;
    }

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("copyEBtR m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        activeEditTrack = m_activeTracks.getTrackRef();
        if(activeEditTrack != NULL)
        {
            byte maxSeqIndex = activeEditTrack->getMaxPatternIndex();
            for(int i = 0; i < maxSeqIndex; i++)
            {
                save_edit_pattern_to_root(i);
            }
        } else
            Serial.println("copy_edit_buffers_to_roots activeEditTrack is NULL");
        m_activeTracks.next();
        
        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("copyEBtR stuck");
            break;
        }
    }
    activeEditTrack = bufTrack;

//  for(int i = 0; i < max_patterns; i++) {}
/*
    Serial.println("EDIT:");
    m_pattern[i].printPattern();
    Serial.println("ROOT:");
    m_pattern_root[i].printPattern();
*/          
}

void StepSequencer::swap_edit_root_seqs(int seqnum) // ADDRESSed
{
    StepPattern swap_buffer;
    
//  m_pattern_root[seqnum].copyPatternTo(swap_buffer);
//  m_pattern[seqnum].copyPatternTo(m_pattern_root[seqnum]);
//  swap_buffer.copyPatternTo(m_pattern[seqnum]);

    StepPattern* indexedSeq = activeEditTrack->getPatternRef(seqnum);
    StepPattern* indexedRootSeq = activeEditTrack->getRootPatternRef(seqnum);
    if(indexedSeq != NULL && indexedRootSeq != NULL)
    {
        indexedRootSeq->copyPatternTo(swap_buffer);
        indexedSeq->copyPatternTo(indexedRootSeq);
        swap_buffer.copyPatternTo(indexedSeq);
    } else {
        Serial.print("swap_edit_root_seqs NULL error: indexedSeq = ");
        Serial.print((int)indexedSeq);
        Serial.print("  indexedRootSeq = ");
        Serial.println((int)indexedRootSeq);
    }
}

bool StepSequencer::toggle_pattern_recall() // TODO ...handle m_pattern_root
{
    static StepPattern recall_buffer;

    byte curIndex = activeEditTrack->getCurrentPatternIndex();
    StepPattern* curSeq = activeEditTrack->getCurrentPatternRef();
    StepPattern* indexedRootSeq = activeEditTrack->getRootPatternRef(curIndex);

    if(curSeq != NULL && indexedRootSeq != NULL)
    {
//      if (!m_recall_buffer_active) {
//          m_pattern[m_currentPattern].copyPatternTo(recall_buffer);
        if (!activeEditTrack->recallBufferIsActive()) {
            curSeq->copyPatternTo(recall_buffer);
            indexedRootSeq->copyPatternTo(curSeq);

            activeEditTrack->setRecallBufferActive(true);
            activeEditTrack->updatePath();
            Serial.print(" Restoring root ");          
            Serial.println(curIndex);          
        } else {
//          recall_buffer.copyPatternTo(m_pattern[m_currentPattern]);
            recall_buffer.copyPatternTo(curSeq);

            activeEditTrack->setRecallBufferActive(false);
            activeEditTrack->updatePath();
            Serial.print(" Recalling edit ");          
            Serial.println(curIndex);       
        }

    } else {
        Serial.print("toggle_pattern_recall NULL error: curSeq = ");
        Serial.print((int)curSeq);
        Serial.print("  indexedRootSeq = ");
        Serial.print((int)indexedRootSeq);
        Serial.print("  curIndex = ");
        Serial.println(curIndex);
    }

    return activeEditTrack->recallBufferIsActive();
}

//"Getters" and "setters"
byte StepSequencer::getNote(int _step)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getNote(_step);
}

byte StepSequencer::getTransposition()
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getTransposition();
}

byte StepSequencer::getLength()
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getLength();
}

byte StepSequencer::getMaxLength()
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getMaxLength();
}

float StepSequencer::getDuration(int _step) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getDuration(_step);
}

byte StepSequencer::getProbability(int _step) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getProbability(_step);
}

byte StepSequencer::getTicks(int _step) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getTicks(_step);
}

bool StepSequencer::getMute(int _step) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return false;
    else
        return activeEditTrack->getCurrentPatternRef()->getMute(_step);
}


bool StepSequencer::getHold(int _step) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return false;
    else
        return activeEditTrack->getCurrentPatternRef()->getHold(_step); 
}


byte StepSequencer::getAccent(int _step) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getAccent(_step); 
}


byte StepSequencer::getRetrig(int _step) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getRetrig(_step);      
}


byte StepSequencer::getVelocity(int _step) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getVelocity(_step);      
}

note StepSequencer::getNoteParams(int _step) // kg
{
    note retVal;
    if(! &activeEditTrack == NULL)
        retVal = activeEditTrack->getTrackNoteParams(_step);
    return retVal;
}

byte StepSequencer::getPath() // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return 0;
    else
        return activeEditTrack->getCurrentPatternRef()->getPath();
}

const char* StepSequencer::getPathName()
{
    note retVal;
    if(! &activeEditTrack == NULL)
        return activeEditTrack->getPathName();
    else return("No path !");
}

int StepSequencer::getCurrentPattern() // TODO
{
    if(! &activeEditTrack == NULL)
        return (int)activeEditTrack->getCurrentPatternIndex();
    else
        return 0;
}

byte StepSequencer::getCurrentTrack()
{
    if(! &activeEditTrack == NULL)
        return activeEditTrack->getNumber();
    else
        return 0;
}

bool StepSequencer::getTrackMute(byte aTrackNum)
{
    bool retVal = false;
    byte curTrackNumBuf = activeEditTrack->getNumber();

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("SS:gTM m_ac noval");

    while(m_activeTracks.hasValue())
    {        
        Track* aTrack = m_activeTracks.getTrackRef();
        if(aTrack != NULL)
        {
            if(aTrack->getNumber() == aTrackNum)
            {
                if(aTrack->isMuted())
                    retVal = true;
                break;
            }
        } else {
            inout.ShowErrorOnLCD("SS:gTM aTrack NULL");
            break;
        }
        m_activeTracks.next();
    }
    setCurrentTrack(curTrackNumBuf);
    return retVal;
}

byte StepSequencer::getPreviousStep()
{
    if(! &activeEditTrack == NULL)
        return activeEditTrack->getPrevPlaybackStep();
    else
        return 0;
}

byte StepSequencer::getLowestSelectedNote(boolean selectedNotes[])
{
    byte lowestNote = 127;
    byte currentNote;

    if (notesArrayEmpty(selectedNotes)) {
        return 0;
    } else {

        if(!checkActiveEditTrackRefandCurrentSeqRef())
            return 0;
        else
        {
            for (int i=0; i < 16; i++) {
                if (selectedNotes[i]) {

                    currentNote = activeEditTrack->getCurrentPatternRef()->getNote(i);
                    lowestNote = (lowestNote > currentNote) ? currentNote : lowestNote;
                }
            }
            return lowestNote;     
        } 
    }
}
      
byte StepSequencer::getHighestSelectedNote(boolean selectedNotes[])
{
    byte highestNote = 0;
    byte currentNote;

    if (notesArrayEmpty(selectedNotes)) {
        return 0;
    } else {

        if(!checkActiveEditTrackRefandCurrentSeqRef())
            return 0;
        else
        {
            for (int i=0; i < 16; i++) {
                if (selectedNotes[i]) {
            //        currentNote = m_pattern[m_currentPattern].getNote(i);
                    currentNote = activeEditTrack->getCurrentPatternRef()->getNote(i);
                    highestNote = (highestNote < currentNote) ? currentNote : highestNote; 
                }
            }
            return highestNote; 
        }
    }     
}

byte StepSequencer::getLowestSelectedVelocity(boolean selectedNotes[])
{
    byte lowestVel = 255;
    byte currentVel;

    if (notesArrayEmpty(selectedNotes)) {
        return 0;
    } else {

        if(!checkActiveEditTrackRefandCurrentSeqRef())
            return 0;
        else
        {
            for (int i=0; i < 16; i++) {
                if (selectedNotes[i]) {
            //        currentVel = m_pattern[m_currentPattern].getVelocity(i);
                    currentVel = activeEditTrack->getCurrentPatternRef()->getVelocity(i);
                    lowestVel = (lowestVel > currentVel) ? currentVel : lowestVel;
                }
            }
            return lowestVel;      
        }
    }
}
      
byte StepSequencer::getHighestSelectedVelocity(boolean selectedNotes[])
{
  byte highestVel = 0;
  byte currentVel;
  
  if (notesArrayEmpty(selectedNotes)) {
    return 0;
  } else {
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) {
//        currentVel = m_pattern[m_currentPattern].getVelocity(i);
          currentVel = activeEditTrack->getCurrentPatternRef()->getVelocity(i);
          highestVel = (highestVel < currentVel) ? currentVel : highestVel; 
      }
    }
    return highestVel; 
    }     
}

float StepSequencer::getShortestSelectedNote(boolean selectedNotes[])
{
    float shortestNote = 1.0;
    float currentNote;

    if (notesArrayEmpty(selectedNotes)) {
        return 0;
    } else {

        if(!checkActiveEditTrackRefandCurrentSeqRef())
            return 0;
        else
        {
            for (int i=0; i < 16; i++) {
                if (selectedNotes[i]) {
            //        currentNote = m_pattern[m_currentPattern].getDuration(i);
                    currentNote = activeEditTrack->getCurrentPatternRef()->getDuration(i);
                    shortestNote = (shortestNote > currentNote) ? currentNote : shortestNote;
                }
            }
            return shortestNote;     
        } 
    }
}
      
float StepSequencer::getLongestSelectedNote(boolean selectedNotes[])
{
    float longestNote = 0.0;
    float currentNote;

    if (notesArrayEmpty(selectedNotes)) {
    return 0;
    } else {

        if(!checkActiveEditTrackRefandCurrentSeqRef())
            return 0;
        else
        {
            for (int i=0; i < 16; i++) {
                if (selectedNotes[i]) {
            //        currentNote = m_pattern[m_currentPattern].getDuration(i);
                    currentNote = activeEditTrack->getCurrentPatternRef()->getDuration(i);
                    longestNote = (longestNote < currentNote) ? currentNote : longestNote;
                }
            }
            return longestNote;    
        }
    }  
}    

byte StepSequencer::offsetSelectedNotes(boolean selectedNotes[], byte note_offset, byte rawHeldStep) // kg
{
  // when holding down a step key, all other selected steps are forced to the same note
  byte heldStep = rawHeldStep % STEPSOFFSET;
  
  byte retval = 255;

  if (rawHeldStep != 255 && selectedNotes[heldStep]) {

    offsetNote(heldStep, note_offset);
    byte heldStepNote = getNote(heldStep);
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) setNote(i, heldStepNote);
    }
    retval = heldStepNote;

  } else {
    
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) {
          offsetNote(i, note_offset);
          retval = retval < getNote(i) ? retval : getNote(i);
      }
    }
  }
  return retval;
}


void StepSequencer::offsetSelectedDurations(boolean selectedNotes[], float duration_offset, byte rawHeldStep) // kg
{
  byte heldStep = rawHeldStep % STEPSOFFSET;
  
  if (rawHeldStep != 255 && selectedNotes[heldStep]) {
    
    offsetDuration(heldStep, (float)duration_offset);
    float heldStepDur = getDuration(heldStep);

    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) setDuration(i, heldStepDur);
    }

  } else {
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) offsetDuration(i, (float)duration_offset);
    }
  }
}


void StepSequencer::setSelectedRepetitions(boolean selectedNotes[], byte repetition) 
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else    
        for (int i=0; i < 16; i++) {
            if (selectedNotes[i]) activeEditTrack->getCurrentPatternRef()->setTicks(i, repetition);
        }
}


void StepSequencer::setSelectedRetrigs(boolean selectedNotes[], byte retrigs) 
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else    
        for (int i=0; i < 16; i++) {
            if (selectedNotes[i]) activeEditTrack->getCurrentPatternRef()->setRetrig(i, retrigs);
        }
}


void StepSequencer::setSelectedProbabilities(boolean selectedNotes[], byte prob)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else    
        for (int i=0; i < 16; i++) {
            if (selectedNotes[i]) activeEditTrack->getCurrentPatternRef()->setProbability(i, (byte)prob);
        }
}


void StepSequencer::setNote(int _step, byte note)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else    
        activeEditTrack->getCurrentPatternRef()->setNote(_step, note); 
}

void StepSequencer::setMute(int _step, bool muteFlag)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else    
        activeEditTrack->getCurrentPatternRef()->setMute(_step, muteFlag);
}

void StepSequencer::setHold(int _step, bool holdFlag)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else    
        activeEditTrack->getCurrentPatternRef()->setHold(_step, holdFlag);
}

void StepSequencer::offsetNote(int _step, byte note_offset) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else {
        int current_note = activeEditTrack->getCurrentPatternRef()->getNote(_step);
        int new_note = current_note + note_offset;
        activeEditTrack->getCurrentPatternRef()->setNote(_step, new_note);
    }
}

void StepSequencer::offsetDuration(int _step, float duration_offset) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else {
        float current_duration = activeEditTrack->getCurrentPatternRef()->getDuration(_step);
        float new_duration = current_duration + duration_offset;
    //  m_pattern[m_currentPattern].setDuration(_step, new_duration);
        activeEditTrack->getCurrentPatternRef()->setDuration(_step, new_duration);
    }
}

void StepSequencer::setDuration(int _step, float duration) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else
        activeEditTrack->getCurrentPatternRef()->setDuration(_step, duration);
}

void StepSequencer::setProbability(int _step, byte prob) // kg
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else
        activeEditTrack->getCurrentPatternRef()->setProbability(_step, prob);
}

void StepSequencer::setTicks(int _step, byte repetition) 
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else
        activeEditTrack->getCurrentPatternRef()->setTicks(_step, repetition);
}

void StepSequencer::setTransposition(byte transposition)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else
        activeEditTrack->getCurrentPatternRef()->setTransposition(transposition);
}

void StepSequencer::setLength(byte _length)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else {
        if(!shiftActive)
        {
            if(activeEditTrack != NULL)
                activeEditTrack->getCurrentPatternRef()->setLength(_length);
            else {
                Serial.println("setLength activeEditTrack is NULL");
                inout.ShowInfoOnLCD("setLength aET NULL");
                inout.SetLCDinfoTimeout();
            }
        } else {
            
            m_activeTracks.rewind();
            int sentry = 0;

            if ( !m_activeTracks.hasValue())
                inout.ShowErrorOnLCD("sL m_ac noval");

            while( m_activeTracks.hasValue())
            {
                Track* aTrack = m_activeTracks.getTrackRef();
                if(aTrack != NULL)
                    aTrack->getCurrentPatternRef()->setLength(_length);
                else {
                    Serial.println("setLength aTrack is NULL");
                    inout.ShowInfoOnLCD("setLength aT NULL");
                    inout.SetLCDinfoTimeout();
                    break;
                }
                m_activeTracks.next();
                
                if(++sentry == 100)
                {
                    inout.ShowErrorOnLCD("setLength stuck");
                    break;
                }
            }
        }
    }
}

void StepSequencer::setAccent(int _step, byte accent)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else {
        StepPattern* activeSequence = activeEditTrack->getCurrentPatternRef();
        if( activeSequence != NULL)
            activeSequence->setAccent(_step, accent);
        else
           inout.ShowInfoOnLCD("setAccent NULLREF");
    }
}

void StepSequencer::setRetrig(int _step, byte retrig)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else
        activeEditTrack->getCurrentPatternRef()->setRetrig(_step, retrig);
}

void StepSequencer::setVelocity(int _step, byte velocity)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else
        activeEditTrack->getCurrentPatternRef()->setVelocity(_step, velocity);

}

void StepSequencer::setPath(byte path)
{
//  m_pattern[m_currentPattern].setPath(path);
    if(!shiftActive)
    {
        if(activeEditTrack != NULL)
            activeEditTrack->setPath(path);
        else {
            Serial.println("setPath activeEditTrack is NULL");
            inout.ShowInfoOnLCD("setPath aET NULL");
            inout.SetLCDinfoTimeout();
        }
    } else {

        m_activeTracks.rewind();
        int sentry = 0;

        if ( !m_activeTracks.hasValue())
            inout.ShowErrorOnLCD("sP m_ac noval");

        while( m_activeTracks.hasValue())
        {
            Track* aTrack = m_activeTracks.getTrackRef();
            if(aTrack != NULL)
                aTrack->setPath(path);
            else {
                Serial.println("setPath aTrack is NULL");
                inout.ShowInfoOnLCD("setPath aT NULL");
                inout.SetLCDinfoTimeout();
                break;
            }
            m_activeTracks.next();
            
            if(++sentry == 100)
            {
                inout.ShowErrorOnLCD("setPath stuck");
                break;
            }
        }
    }
}

void StepSequencer::setCurrentPattern(int index) //TODO: SET ALL TRACKS ?? // ADDRESSed
{
    if(!shiftActive)
    {
        if(index >= 0 && index < max_patterns && index != activeEditTrack->getCurrentPatternIndex()) 
        {
    //      m_currentPattern = index;
    //      m_recall_buffer_active = false;
            
            if(activeEditTrack != NULL)
            {
                bool seqCess = activeEditTrack->setCurrentPatternIndex(index);
                activeEditTrack->setRecallBufferActive(false);
                if( !seqCess)
                {
                    Serial.print("setCurrentPatternIndex failed with index ");
                    Serial.println(index);
                    inout.ShowValueInfoOnLCD("setCurSeqIfail", index);
                    inout.SetLCDinfoTimeout();
                }
            } else {
                Serial.print("setCurrentPattern activeEditTrack is NULL");
                Serial.println(index);
                inout.ShowInfoOnLCD("setCurSeq aET NULL");
                inout.SetLCDinfoTimeout();
            }
        }
    } else {

        m_activeTracks.rewind();
        int sentry = 0;

        if ( !m_activeTracks.hasValue())
            inout.ShowErrorOnLCD("sCS m_ac noval");

        while( m_activeTracks.hasValue())
        {
            Track* aTrack = m_activeTracks.getTrackRef();
            if(aTrack != NULL)
            {
                bool seqCess = aTrack->setCurrentPatternIndex(index);
                aTrack->setRecallBufferActive(false);
                if( !seqCess)
                {
                    Serial.print("setCurrentPatternIndex failed with index ");
                    Serial.println(index);
                    inout.ShowValueInfoOnLCD("setCurSeqIfail", index);
                    inout.SetLCDinfoTimeout();
                }
            } else {
                Serial.println("setCurrentPattern aTrack is NULL");
                inout.ShowInfoOnLCD("setCS aT NULL");
                inout.SetLCDinfoTimeout();
                break;
            }
            m_activeTracks.next();
            
            if(++sentry == 100)
            {
                inout.ShowErrorOnLCD("setPath stuck");
                break;
            }
        }
    }
}

void StepSequencer::setCurrentTrack(byte trackNum)
{
    if(&m_activeTracks == NULL)
    {
        inout.ShowErrorOnLCD("setCT m_aT NULL");
        return;
    }

    Track *newRef = m_activeTracks.getTrackRef(trackNum);

    if(newRef != NULL)
        activeEditTrack = newRef;
    else
    {
        inout.ShowErrorOnLCD("setCT gTR NULL");
        Serial.print("setCurrentTrack not found: ");
        Serial.println(trackNum);
    }
}


//Helper method
bool StepSequencer::playOrNot(int index)
{
    //evaluate probability...
    inout.ShowErrorOnLCD("playOrNot EMPTY");
    return false;
}


void StepSequencer::resetPattern(int index)
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else 
    {
        if(index >=0 && index < max_patterns)
        {
    //      m_pattern[index].reset();

            StepPattern* indexedSeq = activeEditTrack->getPatternRef(index);
            if(indexedSeq != NULL)
                indexedSeq->reset();
            else
                Serial.println("resetPattern: indexedSeq is NULL");
        }
    }
}

void StepSequencer::selectPreviousPattern() //TODO
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else 
    {
        byte seqIndex = activeEditTrack->getCurrentPatternIndex();
        if(seqIndex > 0)
            activeEditTrack->setCurrentPatternIndex(seqIndex - 1);
    }
}

void StepSequencer::selectNextPattern() //TODO
{
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        return;
    else 
    {
        byte seqIndex = activeEditTrack->getCurrentPatternIndex();
        if(seqIndex < activeEditTrack->getMaxPatternIndex())
            activeEditTrack->setCurrentPatternIndex(seqIndex + 1);
    }
}

void StepSequencer::printPattern()
{
    Serial.print("Pattern ");
//  Serial.print(m_currentPattern);
    if(!checkActiveEditTrackRefandCurrentSeqRef())
        Serial.print("activeEditTrack->getCurrentPatternIndex() FAIL");
    else 
        Serial.print(activeEditTrack->getCurrentPatternIndex());
    Serial.print(" on track ");
    Serial.println(activeEditTrack->getNumber());
//  m_pattern[m_currentPattern].printPattern();
    activeEditTrack->getCurrentPatternRef()->printPattern();
}

bool StepSequencer::notesArrayEmpty(boolean notesArray[])
{
  bool retVal = true;
  for (int i=0; i < 16; i++) if (notesArray[i]) retVal = false;
  return retVal;
}


// TODO CONTINUE>>>
void StepSequencer::bufferAllTrackPatternIndices(bool bufOrRestore)
{
    byte curTrackNumBuf = activeEditTrack->getNumber();

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("bATSI m_ac noval");

    int i = 0;
    while(m_activeTracks.hasValue())
    {        
        activeEditTrack = m_activeTracks.getTrackRef();
        if(activeEditTrack != NULL)
        {
            if (bufOrRestore) 
                trackSeqNumBuf[i] = activeEditTrack->getCurrentPatternIndex();
            else
                setCurrentPattern(trackSeqNumBuf[i]);
        } else
            inout.ShowErrorOnLCD("bATSI aET NULL");
        m_activeTracks.next();
        i++;
    }
    setCurrentTrack(curTrackNumBuf);
}

void StepSequencer::retrieveMutedTracks(bool arrayPointer[], int arrayLength)
{
    byte curTrackNumBuf = activeEditTrack->getNumber();

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("rMTrcks m_ac noval");

    int i = 0;
    while(m_activeTracks.hasValue())
    {        
        activeEditTrack = m_activeTracks.getTrackRef();
        if(activeEditTrack != NULL)
        {
            byte trackNum = activeEditTrack->getNumber();
            bool muted = activeEditTrack->isMuted();
            if(trackNum - 1 < arrayLength)
                arrayPointer[trackNum - 1] = muted;
            else {
                inout.ShowErrorOnLCD("rMTrcks out of range");
                Serial.print("retrieveMutedTracks mismatch trackNum ");
                Serial.print(trackNum);
                Serial.print(" to arraylength ");
                Serial.println(arrayLength);
                break;
            }

        } else {
            inout.ShowErrorOnLCD("rMTrcks aET NULL");
            break;
        }
        m_activeTracks.next();
        i++;
    }
    setCurrentTrack(curTrackNumBuf);
}


bool StepSequencer::setTrackMute(byte aTrackNum, bool muteFlag)
{
    bool success = false;
    byte curTrackNumBuf = activeEditTrack->getNumber();

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("sTrMu m_ac noval");

    while(m_activeTracks.hasValue())
    {        
        Track* aTrack = m_activeTracks.getTrackRef();
        if(aTrack != NULL)
        {
            if(aTrack->getNumber() == aTrackNum)
            {
                if(muteFlag)
                    aTrack->mute();
                else
                    aTrack->unMute();
                success = true;
                break;
            }
        } else {
            inout.ShowErrorOnLCD("sTrMu aTrack NULL");
            break;
        }
        m_activeTracks.next();
    }
    setCurrentTrack(curTrackNumBuf);
    return success;
}

byte StepSequencer::toggleCurrentTrackMute()
{
    bool isMuted = activeEditTrack->isMuted();
    byte track = activeEditTrack->getNumber();

    if(isMuted)
    {
        activeEditTrack->unMute();
        inout.ShowValueInfoOnLCD("Unmuted track", track);
        inout.SetLCDinfoTimeout();
    } else {
        activeEditTrack->mute();
        inout.ShowValueInfoOnLCD("Muted track", track);
        inout.SetLCDinfoTimeout();
    }

    return track;
}

bool StepSequencer::isPatternRolloverStep()
{
    bool retVal = false;

    if(activeEditTrack != NULL)
        retVal = activeEditTrack->isTrackPatternRollOverStep();
    else
        inout.ShowErrorOnLCD("isPatRoSt aET NULL");
    return retVal;
}

bool StepSequencer::isPatternRolloverStep(byte trackNum)
{
    bool retVal = false;

    Track *checkTrack = m_activeTracks.getTrackRef(trackNum);

    if(checkTrack != NULL)
        retVal = checkTrack->isTrackPatternRollOverStep();
    else {
        inout.ShowErrorOnLCD("isPatRoSttN cT NULL");
        Serial.println(trackNum);
    }
    return retVal;
}

bool StepSequencer::checkActiveEditTrackRefandCurrentSeqRef()
{
    bool retVal = false;

    if(activeEditTrack != NULL && activeEditTrack->getCurrentPatternRef() != NULL)
        retVal = true;
    else
        inout.ShowErrorOnLCD("cAETRaCSR fail");

    return retVal;
}
