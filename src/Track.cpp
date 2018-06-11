#include "Track.h"
#include "InOutHelper.h"

Track::Track()
{

};

void Track::begin(NoteGetter noteGetterRef, byte number)
{
      b_IsActive = false;
      currentPattern = 0;  
      getNoteCb = noteGetterRef;
      trackNumber = number;
}

void Track::begin(byte number)
{
//    trackType;
//    instrument;
      trackNumber = number;
}
// Core Methods

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
  
}

byte Track::getCurrentPattern()
{
  
}

