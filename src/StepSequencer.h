//Arduino for Musicians
//StepSequencer: A container class for StepSequence objects

#ifndef __STEPSEQUENCER
#define __STEPSEQUENCER

#include <Arduino.h>
#include "StepSequence.h"

class StepSequencer
{
    public:
      //Use enumeration to define a class constant
      enum{max_sequences = 16, max_variations = 15};
    
      //Public constructor and methods
      StepSequencer();
      bool playItOrNot(int _step);
      void prime_edit_buffers();
      void reset_edit_seq(int seqnum);
      void save_sequence(int destination);
      void save_edit_seq_to_root(int seqnum);
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
      byte getPath(); // kg
      int getCurrentSequence();
      byte getLowestSelectedNote(boolean selectedNotes[]);
      byte getHighestSelectedNote(boolean selectedNotes[]);
      byte getLowestSelectedVelocity(boolean selectedNotes[]);
      byte getHighestSelectedVelocity(boolean selectedNotes[]);
      float getShortestSelectedNote(boolean selectedNotes[]);            
      float getLongestSelectedNote(boolean selectedNotes[]);
      void offsetSelectedNotes(boolean selectedNotes[], byte note_offset, byte rawHeldStep); // kg
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
      void setCurrentSequence(int index);

      //Helper method
      bool playOrNot(int index);

      void resetSequence(int index);
      void selectPreviousSequence();
      void selectNextSequence();
      void printSequence();
      bool notesArrayEmpty(boolean notesArray[]);

    private:
      //Class data members:
      int m_currentSequence;     // index of currently active sequence
      bool m_recall_buffer_active; // is there something in the recall buffer ?
      //This array stores the sequences
      StepSequence m_sequence[max_sequences];                           // current, edit buffer
      StepSequence m_sequence_root[max_sequences];
  //  byte m_sequence_variation[max_sequences];
  //  StepSequence m_sequence_array[max_sequences][max_variations];   // root & variations
          
};
#endif
