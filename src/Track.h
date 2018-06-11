
#ifndef __TRACK
#define __TRACK

#include "Arduino.h"
#include "Enum.h"
#include "Note.h"

// Callback
typedef note (*NoteGetter) (int step);

// extern long g_step_duration;

class Track
{
    public:
    
      //Public constructor and methods
      Track();
      void begin(byte number);
      void begin(NoteGetter noteGetterRef, byte number);
      void activate();
      void deactivate();
      void setName(char *namePar);
      void setCurrentPattern(byte newPat);
      char* getName();
      byte getCurrentPattern();
      NoteGetter getNoteCb;

    private:

      //Class data members:
      
      trackTypes trackType;
      instruments instrument;
      bool b_IsActive = false;
      byte currentPattern = 0;
      char *trackName;
      byte trackNumber;
};

#endif
