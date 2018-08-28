#include "Track.h"
#include "InOutHelper.h"


Track::Track()
{

};

void Track::begin(byte number)
{
//    trackType;
//    instrument;
      trackNumber = number;
}

/*
void Track::begin(NoteGetter noteGetterRef, byte number)
{
      b_IsActive = false;
      getNoteCb = noteGetterRef;
      trackNumber = number;
      currentSequenceIndex = 0;
}
*/

void Track::begin(StepSequence sequencesPtr[], StepSequence rootSequencesPtr[], byte sequencesCount, byte number)
{
      trackType = STEPSEQUENCE;
      sequences = sequencesPtr;
      rootSequences = rootSequencesPtr;
      maxSequenceIndex = sequencesCount - 1;
      b_IsActive = false;
      trackNumber = number;
      currentSequenceIndex = 0;
      currentSequence = &sequences[0];
}


// Core Methods

note Track::getNoteParams(int step)
{
      return getNoteParams(step, currentSequenceIndex);
}

note Track::getNoteParams(int step, byte curSequence)
{
      note retNote;
      note retrievedNote;

      switch (trackType)
      {
            case STEPSEQUENCE:
                  if(curSequence < maxSequenceIndex)
                        retrievedNote = sequences[curSequence].getNoteParams(step);
                  else
                        retrievedNote = sequences[maxSequenceIndex].getNoteParams(step);
                  break;
            case SIMPLEBEAT:
                  break;
            default:
                  Serial.println("trackType DEFAULTED");
                  break;
      }
#ifdef DEBUG
      Serial.print("Track::getNoteParams retNote pitch is ");
//    Serial.print((unsigned int) &retNote);
      Serial.print(retNote.pitchVal);
      Serial.print(" swingTicks ");
      Serial.print(retNote.swingTicks);
      Serial.print(" for step ");
      Serial.print(step);
      Serial.print(" of sequence ");
      Serial.print(curSequence);
      Serial.print(" nn track ");
      Serial.println(trackNumber);
#endif

      // this may be completely redundant ?
      memcpy(&retNote, &retrievedNote, sizeof(note));
      return retNote;
}

void Track::activate()
{
      Serial.print("Track ");
      Serial.print(trackNumber);
      Serial.println(" active.");
}

void Track::deactivate()
{
  
}

// Helpers


// Setters
void Track::setName(char *namePar)
{
  
}


bool Track::setCurrentSequenceIndex(byte newIndex)
{
      bool success = false;
      if( newIndex <= maxSequenceIndex)
      {
            currentSequenceIndex = newIndex;
            currentSequence = &sequences[newIndex];
            success = true;
      }
      return success;
}

StepSequence* Track::getCurrentSequenceRef()
{
      if(currentSequence != NULL)
            return currentSequence;
      else{
            Serial.print("getCurrentSequenceRef fail on track ");
            Serial.println(trackNumber);
            return &sequences[0];
      }
}

StepSequence* Track::getSequenceRef(int index)
{
      StepSequence* retVal = NULL;
      if(index <= maxSequenceIndex)
      {
            if(sequences != NULL)
                  retVal = &sequences[index];
            else{
                  Serial.print("getSequenceRef fail on track ");
                  Serial.println(trackNumber);
            }
      } else {
                  Serial.print("getSequenceRef index ");
                  Serial.print(index);
                  Serial.print(" out of range on track ");
                  Serial.println(trackNumber);
      }
      return retVal;
}

StepSequence* Track::getRootSequenceRef(int index)
{
      StepSequence* retVal = NULL;
      if(index <= maxSequenceIndex)
      {
            if(rootSequences != NULL)
                  retVal = &rootSequences[index];
            else{
                  Serial.print("getRootSequenceRef fail on track ");
                  Serial.println(trackNumber);
            }
      } else {
                  Serial.print("getRootSequenceRef index ");
                  Serial.print(index);
                  Serial.print(" out of range on track ");
                  Serial.println(trackNumber);
      }
      return retVal;
}

// Getters
char* Track::getName()
{
  return trackName;
}

byte Track::getNumber()
{
      return trackNumber;
}

byte Track::getCurrentSequenceIndex()
{
      return currentSequenceIndex;
}

byte Track::getMaxSequenceIndex()
{
      return maxSequenceIndex;
}

void Track::setRecallBufferActive(bool trueOrFalse)
{
      b_recallBufferIsActive = trueOrFalse;
}

bool Track::recallBufferIsActive()
{
      return b_recallBufferIsActive;
}

