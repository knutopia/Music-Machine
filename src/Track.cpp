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

void Track::begin(NoteGetter noteGetterRef, byte number)
{
      b_IsActive = false;
//    currentPattern = 0;  
      getNoteCb = noteGetterRef;
      trackNumber = number;
}

void Track::begin(StepSequence sequencesPtr[], byte sequencesCount, byte number)
{
      trackType = STEPSEQUENCE;
      sequences = sequencesPtr;
      maxSequenceIndex = sequencesCount - 1;
      b_IsActive = false;
//    currentPattern = 0;  
      trackNumber = number;
}


// Core Methods

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
      if( newIndex < maxSequenceIndex)
      {
            currentSequenceIndex = newIndex;
            currentSequence = &sequences[newIndex];
            success = true;
      }
      return success;
}

StepSequence* Track::getCurrentSequenceRef()
{
      return currentSequence;
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

