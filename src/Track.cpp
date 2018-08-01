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
      currentPattern = 0;  
      getNoteCb = noteGetterRef;
      trackNumber = number;
}

void Track::begin(StepSequence sequencesPtr[], byte number)
{
      trackType = STEPSEQUENCE;
      sequences = sequencesPtr;
      b_IsActive = false;
      currentPattern = 0;  
      trackNumber = number;
}


// Core Methods

note Track::getNoteParams(int step, byte curSequence)
{
      note retNote;

      switch (trackType)
      {
            case STEPSEQUENCE:
                  retNote = sequences[curSequence].getNoteParams(step);
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
      Serial.print(" for step ");
      Serial.print(step);
      Serial.print(" of sequence ");
      Serial.println(curSequence);
#endif

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

void Track::setCurrentPattern(byte newPat)
{
  
}

// Getters
char* Track::getName()
{
  return trackName;
}

byte Track::getCurrentPattern()
{
  
}

