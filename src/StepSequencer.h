//Arduino for Musicians
//StepSequence: A container class for StepPattern objects

#ifndef __STEPSEQUENCER
#define __STEPSEQUENCER

#include <Arduino.h>
#include "StepPattern.h"
#include "Track.h"
#include "TrackList.h"
#include "LinkedNoteList.h"
#include "StepClickList.h"

typedef void (*CbWhenStuck) ();

class StepSequencer
{
    struct ActiveTrack {
        byte trackNum;
        note getNoteCB;
    };

    public:
      //Use enumeration to define a class constant
      enum{max_patterns = 16};
    
      //Public constructor and methods
      StepSequencer();
      void begin(CbWhenStuck panicCbPointer);
//    bool playItOrNot(int _step);
//    void prime_edit_buffers();
//    void reset_edit_seq(int seqnum);
      void save_pattern(int destination);
      void save_all_patterns();
      void save_edit_pattern_to_root(int seqnum);
      void copy_edit_buffers_to_roots();
      void swap_edit_root_seqs(int seqnum);
      bool toggle_pattern_recall();
      
      //"Getters" and "setters"
      byte getNote(int _step);
      byte getTransposition();
      byte getLength();
      byte getMaxLength();
      float getDuration(int _step); // kg
      byte getProbability(int _step); // kg
      byte getTicks(int _step); // kg
      bool getMute(int _step); // kg
      bool getHold(int _step); // kg
      byte getAccent(int _step); // kg
      byte getRetrig(int _step); // kg
      byte getVelocity(int _step); // kg
      note getNoteParams(int _step);
      byte getPath(); // kg
      const char* getPathName();
      bool getTrackMute(byte trackNum);

      int getCurrentPattern();
      int getCurrentPattern(byte trackNum);
      byte getCurrentTrack();
      byte getPreviousStep();
      byte getLowestSelectedNote(boolean selectedNotes[]);
      byte getHighestSelectedNote(boolean selectedNotes[]);
      byte getLowestSelectedVelocity(boolean selectedNotes[]);
      byte getHighestSelectedVelocity(boolean selectedNotes[]);
      float getShortestSelectedNote(boolean selectedNotes[]);            
      float getLongestSelectedNote(boolean selectedNotes[]);
      byte offsetSelectedNotes(boolean selectedNotes[], byte note_offset, byte rawHeldStep); // kg
      void offsetSelectedDurations(boolean selectedNotes[], float duration_offset, byte rawHeldStep); // kg
      void setSelectedRepetitions(boolean selectedNotes[], byte repetition);
      void setSelectedRetrigs(boolean selectedNotes[], byte retrigs);
      void setSelectedProbabilities(boolean selectedNotes[], byte prob);
      void setNote(int _step, byte note);
      void setMute(int _step, bool muteFlag);
      void setHold(int _step, bool holdFlag);
      void offsetNote(int _step, byte note_offset); // kg
      void offsetDuration(int _step, float duration_offset); // kg
      void setDuration(int _step, float duration); // kg
      void setProbability(int _step, byte prob); // kg
      void setTicks(int _step, byte repetition);
      void setTransposition(byte transposition);
      void setLength(byte _length);
      void setAccent(int _step, byte accent);
      void setRetrig(int _step, byte retrig);
      void setVelocity(int _step, byte velocity);
      void setPath(byte path);
      void setCurrentPattern(int index);
      void setCurrentTrack(byte trackNum);
      void updateNoteList();
      void updateStepClickList();
      void AdvanceStepPositions();
      void resetStepPositions();
      void prepFirstStep();


      //Helper method
      bool playOrNot(int index);

      void resetPattern(int index);
      void selectPreviousPattern();
      void selectNextPattern();
      void printPattern();
      bool notesArrayEmpty(boolean notesArray[]);
      void bufferAllTrackPatternIndices(bool bufOrRestore);
      void retrieveMutedTracks(bool arrayPointer[], int arrayLength);
      bool setTrackMute(byte trackNum, bool muteFlag);
      byte toggleCurrentTrackMute();
      bool isPatternRolloverStep();
      bool isPatternRolloverStep(byte trackNum);
      
// static LinkedNoteList activeNotes;
// static StepClickList activeStepClicks;


    private:
    
      bool checkActiveEditTrackRefandCurrentSeqRef();
      
      // Callback
      CbWhenStuck PanicCb;

      // These two moved into Track:
//    int m_currentPattern;     // index of currently active pattern
//    bool m_recall_buffer_active; // is there something in the recall buffer ?

      //This array stores the patterns
      StepPattern m_pattern[max_patterns];                           // current, edit buffer
      StepPattern m_pattern_root[max_patterns];
      StepPattern m_beat_pattern[max_patterns];
      StepPattern m_beat_pattern_root[max_patterns];
      
      LinkedTrackList m_activeTracks;
      Track tmpTrack1;
      Track tmpTrack2;
      Track *activeEditTrack;
      byte trackSeqNumBuf[TRACKCOUNT];

};

#endif