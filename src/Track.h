
#ifndef __TRACK
#define __TRACK

#include "Arduino.h"
#include "Enum.h"
#include "Note.h"
#include "StepPattern.h"
#include "Path.h"

// Callback
typedef note (*NoteGetter) (int step);

// extern long g_step_duration;

class Track
{
    public:
    
      //Public constructor and methods
      Track();
      void begin(byte number);
      void begin(StepPattern patternsPtr[], StepPattern rootPatternsPtr[], byte patternsCount, byte number);
      note getTrackNoteParams(int step, byte curPattern);
      note getTrackNoteParams(int step);
      note getTrackNoteParams();
      void unMute();
      void mute();
      void setName(char *namePar);
      bool setCurrentPatternIndex(byte newIndex);
      void advanceStepPosition();
      void resetStepPosition();
      void prepFirstStep();
      void setPath(byte path);
      void updatePath();

      char* getName();
      byte getNumber();
      byte getCurrentPatternIndex();
      byte getMaxPatternIndex();
      byte getPrevPlaybackStep();
      const char* getPathName();
      int getPathNumber();
      StepPattern* getCurrentPatternRef();
      StepPattern* getPatternRef(int index);
      StepPattern* getRootPatternRef(int index);
      void setRecallBufferActive(bool trueOrFalse);
      bool recallBufferIsActive();
      bool isMuted();
      bool isTrackPatternRollOverStep();
      NoteGetter getNoteCb;

    private:

      //Class data members:
      
      trackTypes trackType;
      StepPattern *patterns;
      StepPattern *rootPatterns;
      StepPattern *currentPattern;
      byte currentPatternIndex;
      byte maxPatternIndex;
      instruments instrument;
      bool b_isMuted = false;
      bool b_recallBufferIsActive = false;
//    byte currentPattern = 0;
      char *trackName;
      byte trackNumber;
      Path trackPath;
      byte playbackStep = 0;
      byte prevPlaybackStep = 0;
};

#endif
