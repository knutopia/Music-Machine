
#ifndef __TRACK
#define __TRACK

#include "Arduino.h"
#include "Enum.h"
#include "Note.h"
#include "StepSequence.h"
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
      void begin(StepSequence sequencesPtr[], StepSequence rootSequencesPtr[], byte sequencesCount, byte number);
      note getTrackNoteParams(int step, byte curSequence);
      note getTrackNoteParams(int step);
      note getTrackNoteParams();
      void unMute();
      void mute();
      void setName(char *namePar);
      bool setCurrentSequenceIndex(byte newIndex);
      void advanceStepPosition();
      void resetStepPosition();
      void prepFirstStep();
      void setPath(byte path);
      void updatePath();

      char* getName();
      byte getNumber();
      byte getCurrentSequenceIndex();
      byte getMaxSequenceIndex();
      byte getPrevPlaybackStep();
      char* getPathName();
      StepSequence* getCurrentSequenceRef();
      StepSequence* getSequenceRef(int index);
      StepSequence* getRootSequenceRef(int index);
      void setRecallBufferActive(bool trueOrFalse);
      bool recallBufferIsActive();
      bool isMuted();
      NoteGetter getNoteCb;

    private:

      //Class data members:
      
      trackTypes trackType;
      StepSequence *sequences;
      StepSequence *rootSequences;
      StepSequence *currentSequence;
      byte currentSequenceIndex;
      byte maxSequenceIndex;
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
