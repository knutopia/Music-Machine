
#ifndef __TRACK
#define __TRACK

#include "Arduino.h"
#include "Enum.h"


// extern long g_step_duration;

class Track
{
    public:
    
      //Public constructor and methods
      Track();
      void begin();
      void activate();
      void deactivate();
      void setName(char *namePar);
      void setCurrentPattern(byte newPat);
      char* getName();
      byte getCurrentPattern();

    private:

      //Class data members:
      
      trackTypes trackType;
      instruments instrument;
      bool b_IsActive = false;
      byte currentPattern = 0;
      char *trackName;
};

#endif
