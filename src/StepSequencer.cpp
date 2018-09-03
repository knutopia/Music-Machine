//Arduino for Musicians
//StepSequencer: A container class for StepSequence objects

#include "StepSequencer.h"
#include "Track.h"
#include "LinkedNoteList.h"
#include "StepClickList.h"
#include "Timebase.h"
#include "InOutHelper.h"

//#define DEBUG true

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
//  m_currentSequence = 0;
//  m_recall_buffer_active = false;
    g_activeGlobalStep = 0;

#ifdef DEBUG
    Serial.print("Size of m_sequence is ");
    Serial.print(sizeof(m_sequence));
    Serial.print(" for ");
    Serial.print(max_sequences);
    Serial.println(" sequences ");
#endif
}

void StepSequencer::begin(CbWhenStuck panicCbPointer)
{
    PanicCb = panicCbPointer;
    
    int forceInitForSequence = m_sequence[0].getLength();

    for(int s=0; s < max_sequences; s++)
    {
        int len = m_beat_sequence[s].getLength();
        for(int n = 0; n < len; n++)
        {
            m_beat_sequence[s].setDuration(n, 0.1);
            m_beat_sequence[s].setNote(n, 20+random(12));
        }
        for(int n = 0; n < len; n+=4)
            m_beat_sequence[s].setDuration(n, 0.5);
    }

    tmpTrack1.begin(m_sequence, m_sequence_root, max_sequences, (byte)1);
    tmpTrack2.begin(m_beat_sequence, m_beat_sequence_root, max_sequences, (byte)2);
    m_beat_sequence[0].copySeqTo(m_beat_sequence_root[0]);

    m_activeTracks.appendTrack(1, &tmpTrack1);
    m_activeTracks.appendTrack(2, &tmpTrack2);

    activeEditTrack = &tmpTrack1;
    activeEditTrack->setCurrentSequenceIndex(0);
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

    if(&m_activeTracks == NULL) inout.ShowErrorOnLCD("updateNL: m_ac NULL");

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("updateNL: m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        Track *cur_trackRef = m_activeTracks.getTrackRef();

        if (cur_trackRef != NULL)
        {
//          cur_note = cur_trackRef->getNoteParams(stepInPattern, (byte)m_currentSequence);
            cur_note = cur_trackRef->getNoteParams();
            cur_track = m_activeTracks.getTrackNumber();
            activeNotes.appendNote(g_activeGlobalStep, cur_track, cur_note);
        }
        else
        {
            Serial.println("updateNoteList: NULL cur_trackRef");
            inout.ShowErrorOnLCD("updateNoteList: NULL");
        }

#ifdef DEBUG
        Serial.print("updateNoteList note ");
        Serial.print(cur_note.pitchVal);
        Serial.print(" on track ");
        Serial.print(cur_track);
        Serial.print(" for step ");
        Serial.print(g_activeGlobalStep);
//      Serial.print(" from sequence ");
//      Serial.println(m_currentSequence);
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
//  activeStepClicks.dropNotesBeforeStepAndRewind(g_activeGlobalStep);
}

void StepSequencer::AdvanceStepPositions()
{
    if(&m_activeTracks == NULL) inout.ShowErrorOnLCD("advSP: m_ac NULL");

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("advSP: m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        m_activeTracks.getTrackRef()->advanceStepPosition();
        m_activeTracks.next();
    }
}

void StepSequencer::resetStepPositions()
{
   if(&m_activeTracks == NULL) inout.ShowErrorOnLCD("resetSP: m_ac NULL");

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("resetSP: m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        m_activeTracks.getTrackRef()->resetStepPosition();
        m_activeTracks.next();
    }
}

void StepSequencer::prepFirstStep()
{
   if(&m_activeTracks == NULL) inout.ShowErrorOnLCD("pFStep: m_ac NULL");

    m_activeTracks.rewind();

    if ( !m_activeTracks.hasValue())
        inout.ShowErrorOnLCD("pFStep: m_ac noval");

    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        m_activeTracks.getTrackRef()->prepFirstStep();
        m_activeTracks.next();
    }
}



bool StepSequencer::playItOrNot(int _step) //make obsolete
{
    // take step probability into account 
    // and check if the step isn't holding from the previous step
    
    bool retVal = false;
//  if (!m_sequence[m_currentSequence].getHold(_step))
    if (!activeEditTrack->getCurrentSequenceRef()->getHold(_step))
    {
//      int stepProbability = (int)m_sequence[m_currentSequence].getProbability(_step);
        int stepProbability = (int)activeEditTrack->getCurrentSequenceRef()->getProbability(_step);
        switch (stepProbability) {
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
  return retVal;
}

/*
void StepSequencer::prime_edit_buffers() // UNUSED - ADDRESS ?
{
    for(int i = 0; i < max_sequences; i++) 
      reset_edit_seq(i);            
}

void StepSequencer::reset_edit_seq(int seqnum) // ADDRESS
{
    m_sequence_root[seqnum].copySeqTo(m_sequence[seqnum]);            
}
*/

void StepSequencer::save_sequence(int destination) // TODO // ADDRESSed
{
//  m_sequence[m_currentSequence].copySeqTo(m_sequence_root[destination]);
//  m_sequence[m_currentSequence].copySeqTo(m_sequence[destination]);

    StepSequence* curSeq = activeEditTrack->getCurrentSequenceRef();
    StepSequence* indexedRootSeq = activeEditTrack->getRootSequenceRef(destination);
    if(curSeq != NULL && indexedRootSeq != NULL)
    {
        curSeq->copySeqTo(indexedRootSeq);
    } else {
        Serial.print("save_sequence NULL error: curSeq = ");
        Serial.print((int)curSeq);
        Serial.print("  indexedRootSeq = ");
        Serial.println((int)indexedRootSeq);
    }
}

void StepSequencer::save_edit_seq_to_root(int seqnum) // ADDRESSed
{
//  m_sequence[seqnum].copySeqTo(m_sequence_root[seqnum]);

    StepSequence* indexedSeq = activeEditTrack->getSequenceRef(seqnum);
    StepSequence* indexedRootSeq = activeEditTrack->getRootSequenceRef(seqnum);
    if(indexedSeq != NULL && indexedRootSeq != NULL)
    {
        indexedSeq->copySeqTo(indexedRootSeq);
    } else {
        Serial.print("save_edit_seq_to_root NULL error: indexedSeq = ");
        Serial.print((int)indexedSeq);
        Serial.print("  indexedRootSeq = ");
        Serial.println((int)indexedRootSeq);
    }

}

void StepSequencer::copy_edit_buffers_to_roots()
{
    Track* bufTrack = activeEditTrack;

    m_activeTracks.rewind();
    int sentry = 0;
    while( m_activeTracks.hasValue())
    {
        activeEditTrack = m_activeTracks.getTrackRef();
        if(activeEditTrack != NULL)
        {
            byte maxSeqIndex = activeEditTrack->getMaxSequenceIndex();
            for(int i = 0; i < maxSeqIndex; i++)
            {
                save_edit_seq_to_root(i);
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

//  for(int i = 0; i < max_sequences; i++) {}
/*
    Serial.println("EDIT:");
    m_sequence[i].printSequence();
    Serial.println("ROOT:");
    m_sequence_root[i].printSequence();
*/          
}

void StepSequencer::swap_edit_root_seqs(int seqnum) // ADDRESSed
{
    StepSequence swap_buffer;
    
//  m_sequence_root[seqnum].copySeqTo(swap_buffer);
//  m_sequence[seqnum].copySeqTo(m_sequence_root[seqnum]);
//  swap_buffer.copySeqTo(m_sequence[seqnum]);

    StepSequence* indexedSeq = activeEditTrack->getSequenceRef(seqnum);
    StepSequence* indexedRootSeq = activeEditTrack->getRootSequenceRef(seqnum);
    if(indexedSeq != NULL && indexedRootSeq != NULL)
    {
        indexedRootSeq->copySeqTo(swap_buffer);
        indexedSeq->copySeqTo(indexedRootSeq);
        swap_buffer.copySeqTo(indexedSeq);
    } else {
        Serial.print("swap_edit_root_seqs NULL error: indexedSeq = ");
        Serial.print((int)indexedSeq);
        Serial.print("  indexedRootSeq = ");
        Serial.println((int)indexedRootSeq);
    }
}

bool StepSequencer::toggle_pattern_recall() // TODO ...handle m_sequence_root
{
    static StepSequence recall_buffer;

    byte curIndex = activeEditTrack->getCurrentSequenceIndex();
    StepSequence* curSeq = activeEditTrack->getCurrentSequenceRef();
    StepSequence* indexedRootSeq = activeEditTrack->getRootSequenceRef(curIndex);

    if(curSeq != NULL && indexedRootSeq != NULL)
    {
//      if (!m_recall_buffer_active) {
//          m_sequence[m_currentSequence].copySeqTo(recall_buffer);
        if (!activeEditTrack->recallBufferIsActive()) {
            curSeq->copySeqTo(recall_buffer);
            indexedRootSeq->copySeqTo(curSeq);

            activeEditTrack->setRecallBufferActive(true);
            activeEditTrack->updatePath();
            Serial.print(" Restoring root ");          
            Serial.println(curIndex);          
        } else {
//          recall_buffer.copySeqTo(m_sequence[m_currentSequence]);
            recall_buffer.copySeqTo(curSeq);

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
//  return m_sequence[m_currentSequence].getNote(_step);
    return activeEditTrack->getCurrentSequenceRef()->getNote(_step);
}

byte StepSequencer::getTransposition()
{
//  return m_sequence[m_currentSequence].getTransposition();
    return activeEditTrack->getCurrentSequenceRef()->getTransposition();
}

byte StepSequencer::getLength()
{
//  return m_sequence[m_currentSequence].getLength();
    return activeEditTrack->getCurrentSequenceRef()->getLength();
}

byte StepSequencer::getMaxLength()
{
//  return m_sequence[m_currentSequence].getMaxLength();
    return activeEditTrack->getCurrentSequenceRef()->getMaxLength();
}

float StepSequencer::getDuration(int _step) // kg
{
//  return m_sequence[m_currentSequence].getDuration(_step);
    return activeEditTrack->getCurrentSequenceRef()->getDuration(_step);
}

byte StepSequencer::getProbability(int _step) // kg
{
//  return m_sequence[m_currentSequence].getProbability(_step);
    return activeEditTrack->getCurrentSequenceRef()->getProbability(_step);
}

byte StepSequencer::getTicks(int _step) // kg
{
//  return m_sequence[m_currentSequence].getTicks(_step);
    return activeEditTrack->getCurrentSequenceRef()->getTicks(_step);
}

bool StepSequencer::getMute(int _step) // kg
{
//  return m_sequence[m_currentSequence].getMute(_step);
    return activeEditTrack->getCurrentSequenceRef()->getMute(_step);
}


bool StepSequencer::getHold(int _step) // kg
{
//  return m_sequence[m_currentSequence].getHold(_step);
    return activeEditTrack->getCurrentSequenceRef()->getHold(_step); 
}


byte StepSequencer::getAccent(int _step) // kg
{
//  return m_sequence[m_currentSequence].getAccent(_step);      
    return activeEditTrack->getCurrentSequenceRef()->getAccent(_step); 
}


byte StepSequencer::getRetrig(int _step) // kg
{
//  return m_sequence[m_currentSequence].getRetrig(_step);      
    return activeEditTrack->getCurrentSequenceRef()->getRetrig(_step);      
}


byte StepSequencer::getVelocity(int _step) // kg
{
//  return m_sequence[m_currentSequence].getVelocity(_step);      
    return activeEditTrack->getCurrentSequenceRef()->getVelocity(_step);      
}

note StepSequencer::getNoteParams(int _step) // kg
{
//  return m_sequence[m_currentSequence].getNoteParams(_step);
//  return activeEditTrack->getCurrentSequenceRef()->getNoteParams(_step);
    return activeEditTrack->getNoteParams(_step);
}

byte StepSequencer::getPath() // kg
{
//  return m_sequence[m_currentSequence].getPath();
    return activeEditTrack->getCurrentSequenceRef()->getPath();
}

char* StepSequencer::getPathName()
{
    return activeEditTrack->getPathName();
}

int StepSequencer::getCurrentSequence() // TODO
{
//  return m_currentSequence;
    return (int)activeEditTrack->getCurrentSequenceIndex();
}

byte StepSequencer::getCurrentTrack()
{
    return activeEditTrack->getNumber();
}

byte StepSequencer::getPreviousStep()
{
    return activeEditTrack->getPrevPlaybackStep();
}

byte StepSequencer::getLowestSelectedNote(boolean selectedNotes[])
{
  byte lowestNote = 127;
  byte currentNote;

  if (notesArrayEmpty(selectedNotes)) {
    return 0;
  } else {
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) {
//        currentNote = m_sequence[m_currentSequence].getNote(i);
          currentNote = activeEditTrack->getCurrentSequenceRef()->getNote(i);
          lowestNote = (lowestNote > currentNote) ? currentNote : lowestNote;
      }
    }
    return lowestNote;      
  }
}
      
byte StepSequencer::getHighestSelectedNote(boolean selectedNotes[])
{
  byte highestNote = 0;
  byte currentNote;
  
  if (notesArrayEmpty(selectedNotes)) {
    return 0;
  } else {
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) {
//        currentNote = m_sequence[m_currentSequence].getNote(i);
          currentNote = activeEditTrack->getCurrentSequenceRef()->getNote(i);
          highestNote = (highestNote < currentNote) ? currentNote : highestNote; 
      }
    }
    return highestNote; 
    }     
}

byte StepSequencer::getLowestSelectedVelocity(boolean selectedNotes[])
{
  byte lowestVel = 255;
  byte currentVel;

  if (notesArrayEmpty(selectedNotes)) {
    return 0;
  } else {
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) {
//        currentVel = m_sequence[m_currentSequence].getVelocity(i);
          currentVel = activeEditTrack->getCurrentSequenceRef()->getVelocity(i);
          lowestVel = (lowestVel > currentVel) ? currentVel : lowestVel;
      }
    }
    return lowestVel;      
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
//        currentVel = m_sequence[m_currentSequence].getVelocity(i);
          currentVel = activeEditTrack->getCurrentSequenceRef()->getVelocity(i);
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
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) {
//        currentNote = m_sequence[m_currentSequence].getDuration(i);
          currentNote = activeEditTrack->getCurrentSequenceRef()->getDuration(i);
          shortestNote = (shortestNote > currentNote) ? currentNote : shortestNote;
      }
    }
    return shortestNote;      
  }
}
      
float StepSequencer::getLongestSelectedNote(boolean selectedNotes[])
{
  float longestNote = 0.0;
  float currentNote;
  
  if (notesArrayEmpty(selectedNotes)) {
    return 0;
  } else {
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) {
//        currentNote = m_sequence[m_currentSequence].getDuration(i);
          currentNote = activeEditTrack->getCurrentSequenceRef()->getDuration(i);
          longestNote = (longestNote < currentNote) ? currentNote : longestNote;
      }
    }
    return longestNote;    
  }  
}    

void StepSequencer::offsetSelectedNotes(boolean selectedNotes[], byte note_offset, byte rawHeldStep) // kg
{
  byte heldStep = rawHeldStep % STEPSOFFSET;
  
  if (rawHeldStep != 255 && selectedNotes[heldStep]) {

    offsetNote(heldStep, note_offset);
    byte heldStepNote = getNote(heldStep);
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) setNote(i, heldStepNote);
    }

  } else {
    
    for (int i=0; i < 16; i++) {
      if (selectedNotes[i]) offsetNote(i, note_offset);
    }
  }
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
  for (int i=0; i < 16; i++) {
//  if (selectedNotes[i]) m_sequence[m_currentSequence].setTicks(i, repetition);
    if (selectedNotes[i]) activeEditTrack->getCurrentSequenceRef()->setTicks(i, repetition);
  }
}


void StepSequencer::setSelectedRetrigs(boolean selectedNotes[], byte retrigs) 
{
  for (int i=0; i < 16; i++) {
//  if (selectedNotes[i]) m_sequence[m_currentSequence].setRetrig(i, retrigs);
    if (selectedNotes[i]) activeEditTrack->getCurrentSequenceRef()->setRetrig(i, retrigs);
  }
}


void StepSequencer::setSelectedProbabilities(boolean selectedNotes[], byte prob)
{
  for (int i=0; i < 16; i++) {
//  if (selectedNotes[i]) m_sequence[m_currentSequence].setProbability(i, (byte)prob);
    if (selectedNotes[i]) activeEditTrack->getCurrentSequenceRef()->setProbability(i, (byte)prob);
  }
}


void StepSequencer::setNote(int _step, byte note)
{
//  m_sequence[m_currentSequence].setNote(_step, note); 
    activeEditTrack->getCurrentSequenceRef()->setNote(_step, note); 
}

void StepSequencer::setMute(int _step, bool muteFlag)
{
//  m_sequence[m_currentSequence].setMute(_step, muteFlag);
    activeEditTrack->getCurrentSequenceRef()->setMute(_step, muteFlag);
}

void StepSequencer::setHold(int _step, bool holdFlag)
{
//  m_sequence[m_currentSequence].setHold(_step, holdFlag); 
    activeEditTrack->getCurrentSequenceRef()->setHold(_step, holdFlag);
}

void StepSequencer::offsetNote(int _step, byte note_offset) // kg
{
//  int current_note = m_sequence[m_currentSequence].getNote(_step);
    int current_note = activeEditTrack->getCurrentSequenceRef()->getNote(_step);
    int new_note = current_note + note_offset;
//  m_sequence[m_currentSequence].setNote(_step, new_note);
    activeEditTrack->getCurrentSequenceRef()->setNote(_step, new_note);
}

void StepSequencer::offsetDuration(int _step, float duration_offset) // kg
{
//  float current_duration = m_sequence[m_currentSequence].getDuration(_step);
    float current_duration = activeEditTrack->getCurrentSequenceRef()->getDuration(_step);
    float new_duration = current_duration + duration_offset;
//  m_sequence[m_currentSequence].setDuration(_step, new_duration);
    activeEditTrack->getCurrentSequenceRef()->setDuration(_step, new_duration);
}

void StepSequencer::setDuration(int _step, float duration) // kg
{
//  m_sequence[m_currentSequence].setDuration(_step, duration);      
    activeEditTrack->getCurrentSequenceRef()->setDuration(_step, duration);
}

void StepSequencer::setProbability(int _step, byte prob) // kg
{
//  m_sequence[m_currentSequence].setProbability(_step, prob);      
    activeEditTrack->getCurrentSequenceRef()->setProbability(_step, prob);
}

void StepSequencer::setTicks(int _step, byte repetition) {
//  m_sequence[m_currentSequence].setTicks(_step, repetition); 
    activeEditTrack->getCurrentSequenceRef()->setTicks(_step, repetition);
}

void StepSequencer::setTransposition(byte transposition)
{
//  m_sequence[m_currentSequence].setTransposition(transposition);
    activeEditTrack->getCurrentSequenceRef()->setTransposition(transposition);
}

void StepSequencer::setLength(byte _length)
{
    if(!shiftActive)
    {
        if(activeEditTrack != NULL)
            activeEditTrack->getCurrentSequenceRef()->setLength(_length);
        else {
            Serial.println("setLength activeEditTrack is NULL");
            inout.ShowInfoOnLCD("setLength aET NULL");
            inout.SetLCDinfoTimeout();
        }
    } else {
        
        m_activeTracks.rewind();
        int sentry = 0;
        while( m_activeTracks.hasValue())
        {
            Track* aTrack = m_activeTracks.getTrackRef();
            if(aTrack != NULL)
                aTrack->getCurrentSequenceRef()->setLength(_length);
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

void StepSequencer::setAccent(int _step, byte accent)
{
//  m_sequence[m_currentSequence].setAccent(_step, accent);
    StepSequence* activeSequence = activeEditTrack->getCurrentSequenceRef();
    if( activeSequence != NULL)
//      activeEditTrack->getCurrentSequenceRef()->setAccent(_step, accent);
        activeSequence->setAccent(_step, accent);
    else
        Serial.println("setAccent NULLREF");
}

void StepSequencer::setRetrig(int _step, byte retrig)
{
//  m_sequence[m_currentSequence].setRetrig(_step, retrig);
    activeEditTrack->getCurrentSequenceRef()->setRetrig(_step, retrig);
}

void StepSequencer::setVelocity(int _step, byte velocity)
{
//  m_sequence[m_currentSequence].setVelocity(_step, velocity);
    activeEditTrack->getCurrentSequenceRef()->setVelocity(_step, velocity);

}

void StepSequencer::setPath(byte path)
{
//  m_sequence[m_currentSequence].setPath(path);
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

void StepSequencer::setCurrentSequence(int index) //TODO: SET ALL TRACKS ?? // ADDRESSed
{
    if(!shiftActive)
    {
        if(index >= 0 && index < max_sequences && index != activeEditTrack->getCurrentSequenceIndex()) 
        {
    //      m_currentSequence = index;
    //      m_recall_buffer_active = false;
            
            if(activeEditTrack != NULL)
            {
                bool seqCess = activeEditTrack->setCurrentSequenceIndex(index);
                activeEditTrack->setRecallBufferActive(false);
                if( !seqCess)
                {
                    Serial.print("setCurrentSequenceIndex failed with index ");
                    Serial.println(index);
                    inout.ShowValueInfoOnLCD("setCurSeqIfail", index);
                    inout.SetLCDinfoTimeout();
                }
            } else {
                Serial.print("setCurrentSequence activeEditTrack is NULL");
                Serial.println(index);
                inout.ShowInfoOnLCD("setCurSeq aET NULL");
                inout.SetLCDinfoTimeout();
            }
        }
    } else {

        m_activeTracks.rewind();
        int sentry = 0;
        while( m_activeTracks.hasValue())
        {
            Track* aTrack = m_activeTracks.getTrackRef();
            if(aTrack != NULL)
            {
                bool seqCess = aTrack->setCurrentSequenceIndex(index);
                aTrack->setRecallBufferActive(false);
                if( !seqCess)
                {
                    Serial.print("setCurrentSequenceIndex failed with index ");
                    Serial.println(index);
                    inout.ShowValueInfoOnLCD("setCurSeqIfail", index);
                    inout.SetLCDinfoTimeout();
                }
            } else {
                Serial.println("setCurrentSequence aTrack is NULL");
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
    Track *newRef = m_activeTracks.getTrackRef(trackNum);
    byte curSeqIndex = activeEditTrack->getCurrentSequenceIndex();

    if(newRef != NULL)
    {
//      activeEditTrack = activeEditTrack = newRef;
        activeEditTrack = newRef;
//      activeEditTrack->setCurrentSequenceIndex(curSeqIndex);
    }
    else
    {
        Serial.print("setCurrentTrack not found: ");
        Serial.println(trackNum);
    }
}


//Helper method
bool StepSequencer::playOrNot(int index)
{
  //evaluate probability...
  Serial.println("@@@@@@@@@@@@@  playOrNot is EMPTY @@@@@@@@@@@@@@");
  return false;
}


void StepSequencer::resetSequence(int index)
{
    if(index >=0 && index < max_sequences)
    {
//      m_sequence[index].reset();

        StepSequence* indexedSeq = activeEditTrack->getSequenceRef(index);
        if(indexedSeq != NULL)
            indexedSeq->reset();
        else
            Serial.println("resetSequence: indexedSeq is NULL");
    }
}

void StepSequencer::selectPreviousSequence() //TODO
{
//  if(m_currentSequence > 0)
//      m_currentSequence--;

    byte seqIndex = activeEditTrack->getCurrentSequenceIndex();
    if(seqIndex > 0)
        activeEditTrack->setCurrentSequenceIndex(seqIndex - 1);
}

void StepSequencer::selectNextSequence() //TODO
{
//  if(m_currentSequence < max_sequences -1)
//      m_currentSequence++; 

    byte seqIndex = activeEditTrack->getCurrentSequenceIndex();
    if(seqIndex < activeEditTrack->getMaxSequenceIndex())
        activeEditTrack->setCurrentSequenceIndex(seqIndex + 1);
}

void StepSequencer::printSequence()
{
    Serial.print("Sequence ");
//  Serial.print(m_currentSequence);
    Serial.print(activeEditTrack->getCurrentSequenceIndex());
    Serial.println(" on track ");
    Serial.println(activeEditTrack->getNumber());
//  m_sequence[m_currentSequence].printSequence();
    activeEditTrack->getCurrentSequenceRef()->printSequence();
}

bool StepSequencer::notesArrayEmpty(boolean notesArray[])
{
  bool retVal = true;
  for (int i=0; i < 16; i++) if (notesArray[i]) retVal = false;
  return retVal;
}
