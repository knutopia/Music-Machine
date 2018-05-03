//Arduino for Musicians
//StepSequencer: A container class for StepSequence objects

#ifndef __STEPSEQUENCER
#define __STEPSEQUENCER

#include "StepSequence.h"

class StepSequencer
{
    public:
     //Use enumeration to define a class constant
    enum{max_sequences = 16, max_variations = 15};
    
    private:
    //Class data members:
    int m_currentSequence;     // index of currently active sequence
    bool m_recall_buffer_active; // is there something in the recall buffer ?
    //This array stores the sequences
    StepSequence m_sequence[max_sequences];                           // current, edit buffer
    StepSequence m_sequence_root[max_sequences];
//  byte m_sequence_variation[max_sequences];
//  StepSequence m_sequence_array[max_sequences][max_variations];   // root & variations
    
    public:
    //Public constructor and methods
    StepSequencer()
    {
        m_currentSequence = 0;
        m_recall_buffer_active = false;
//      for (int i = 0; i < max_sequences; i++)
//        m_sequence_variation[i] = ROOT;

        Serial.print("Size of m_sequence is ");
        Serial.print(sizeof(m_sequence));
        Serial.print(" for ");
        Serial.print(max_sequences);
        Serial.println(" sequences ");
    }

    bool playItOrNot(int _step)
    {
      // take step probability into account 
      // and check if the step isn't holding from the previous step
      
      bool retVal = false;
      if (!m_sequence[m_currentSequence].getHold(_step))
      {
        int stepProbability = (int)m_sequence[m_currentSequence].getProbability(_step);
        switch (stepProbability) {
          case ZEROPROB:
            retVal = false;
            break;        
          case LOWPROB:
            if (random(100) < 33) retVal = true;
            else retVal = false;
            break;        
          case HIGHPROB:
            if (random(100) < 66) retVal = true;
            else retVal = false;
            break;        
          case FULLPROB:
            retVal = true;
            break;
        }
      }
      return retVal;
    }
    
    void prime_edit_buffers()
    {
        for(int i = 0; i < max_sequences; i++) 
          reset_edit_seq(i);            
    }

    void reset_edit_seq(int seqnum)
    {
        m_sequence_root[seqnum].copySeqTo(m_sequence[seqnum]);            
    }

    void save_sequence(int destination)
    {
        m_sequence[m_currentSequence].copySeqTo(m_sequence_root[destination]);
        m_sequence[m_currentSequence].copySeqTo(m_sequence[destination]);
    }
    
    void save_edit_seq_to_root(int seqnum)
    {
        m_sequence[seqnum].copySeqTo(m_sequence_root[seqnum]);
    }

    void copy_edit_buffers_to_roots()
    {
        for(int i = 0; i < max_sequences; i++) {
          save_edit_seq_to_root(i);
/*
          Serial.println("EDIT:");
          m_sequence[i].printSequence();
          Serial.println("ROOT:");
          m_sequence_root[i].printSequence();
*/          
        }
    }

    void swap_edit_root_seqs(int seqnum)
    {
        StepSequence swap_buffer;
        
        m_sequence_root[seqnum].copySeqTo(swap_buffer);
        m_sequence[seqnum].copySeqTo(m_sequence_root[seqnum]);
        swap_buffer.copySeqTo(m_sequence[seqnum]);
    }

    bool toggle_pattern_recall()
    {
        static StepSequence recall_buffer;
        
        if (!m_recall_buffer_active) {
          m_sequence[m_currentSequence].copySeqTo(recall_buffer);
          m_sequence_root[m_currentSequence].copySeqTo(m_sequence[m_currentSequence]);
          m_recall_buffer_active = true;
          Serial.print(" Restoring root ");          
          Serial.println(m_currentSequence);          
        } else {
          recall_buffer.copySeqTo(m_sequence[m_currentSequence]);
          m_recall_buffer_active = false;
          Serial.print(" Recalling edit ");          
          Serial.println(m_currentSequence);       
        }
        return m_recall_buffer_active;
    }

    //"Getters" and "setters"
    byte getNote(int _step)
    {
        return m_sequence[m_currentSequence].getNote(_step);
    }
    
    byte getTransposition()
    {
        return m_sequence[m_currentSequence].getTransposition();
    }
    
    byte getLength()
    {
        return m_sequence[m_currentSequence].getLength();
    }
    
    byte getMaxLength()
    {
        return m_sequence[m_currentSequence].getMaxLength();
    }

    float getDuration(int _step) // kg
    {
        return m_sequence[m_currentSequence].getDuration(_step);
    }
    
    byte getProbability(int _step) // kg
    {
        return m_sequence[m_currentSequence].getProbability(_step);
    }

    byte getTicks(int _step) // kg
    {
        return m_sequence[m_currentSequence].getTicks(_step);
    }

    bool getMute(int _step) // kg
    {
        return m_sequence[m_currentSequence].getMute(_step);
    }

 
    bool getHold(int _step) // kg
    {
        return m_sequence[m_currentSequence].getHold(_step);
    }

 
    byte getAccent(int _step) // kg
    {
        return m_sequence[m_currentSequence].getAccent(_step);      
    }


    byte getRetrig(int _step) // kg
    {
        return m_sequence[m_currentSequence].getRetrig(_step);      
    }


    byte getVelocity(int _step) // kg
    {
        return m_sequence[m_currentSequence].getVelocity(_step);      
    }


    byte getPath() // kg
    {
        return m_sequence[m_currentSequence].getPath();
    }

    int getCurrentSequence()
    {
        return m_currentSequence;
    }

    byte getLowestSelectedNote(boolean selectedNotes[])
    {
      byte lowestNote = 127;
      byte currentNote;

      if (notesArrayEmpty(selectedNotes)) {
        return 0;
      } else {
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) {
              currentNote = m_sequence[m_currentSequence].getNote(i);
              lowestNote = (lowestNote > currentNote) ? currentNote : lowestNote;
          }
        }
        return lowestNote;      
    }
    }
          
    byte getHighestSelectedNote(boolean selectedNotes[])
    {
      byte highestNote = 0;
      byte currentNote;
      
      if (notesArrayEmpty(selectedNotes)) {
        return 0;
      } else {
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) {
              currentNote = m_sequence[m_currentSequence].getNote(i);
              highestNote = (highestNote < currentNote) ? currentNote : highestNote; 
          }
        }
        return highestNote; 
       }     
    }
  
    byte getLowestSelectedVelocity(boolean selectedNotes[])
    {
      byte lowestVel = 255;
      byte currentVel;

      if (notesArrayEmpty(selectedNotes)) {
        return 0;
      } else {
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) {
              currentVel = m_sequence[m_currentSequence].getVelocity(i);
              lowestVel = (lowestVel > currentVel) ? currentVel : lowestVel;
          }
        }
        return lowestVel;      
    }
    }
          
    byte getHighestSelectedVelocity(boolean selectedNotes[])
    {
      byte highestVel = 0;
      byte currentVel;
      
      if (notesArrayEmpty(selectedNotes)) {
        return 0;
      } else {
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) {
              currentVel = m_sequence[m_currentSequence].getVelocity(i);
              highestVel = (highestVel < currentVel) ? currentVel : highestVel; 
          }
        }
        return highestVel; 
       }     
    }
  
    float getShortestSelectedNote(boolean selectedNotes[])
    {
      float shortestNote = 1.0;
      float currentNote;

      if (notesArrayEmpty(selectedNotes)) {
        return 0;
      } else {
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) {
              currentNote = m_sequence[m_currentSequence].getDuration(i);
              shortestNote = (shortestNote > currentNote) ? currentNote : shortestNote;
          }
        }
        return shortestNote;      
      }
    }
          
    float getLongestSelectedNote(boolean selectedNotes[])
    {
      float longestNote = 0.0;
      float currentNote;
      
      if (notesArrayEmpty(selectedNotes)) {
        return 0;
      } else {
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) {
              currentNote = m_sequence[m_currentSequence].getDuration(i);
              longestNote = (longestNote < currentNote) ? currentNote : longestNote;
          }
        }
        return longestNote;    
      }  
    }    
    
    void offsetSelectedNotes(boolean selectedNotes[], byte note_offset, byte rawHeldStep) // kg
    {
      byte heldStep = rawHeldStep % STEPSOFFSET;
      
      if (rawHeldStep != 255 && selectedNotes[heldStep]) {

        offsetNote(heldStep, note_offset);
        byte heldStepNote = getNote(heldStep);
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) setNote(i, heldStepNote);
        }

      } else {
        
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) offsetNote(i, note_offset);
        }
      }
    }
    

    void offsetSelectedDurations(boolean selectedNotes[], float duration_offset, byte rawHeldStep) // kg
    {
      byte heldStep = rawHeldStep % STEPSOFFSET;
      
      if (rawHeldStep != 255 && selectedNotes[heldStep]) {
        
        offsetDuration(heldStep, (float)duration_offset);
        float heldStepDur = getDuration(heldStep);

        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) setDuration(i, heldStepDur);
        }

      } else {
        for (int i=0; i < 16; i++) {
          if (selectedNotes[i]) offsetDuration(i, (float)duration_offset);
        }
      }
    }


    void setSelectedRepetitions(boolean selectedNotes[], byte repetition) 
    {
      for (int i=0; i < 16; i++) {
        if (selectedNotes[i]) m_sequence[m_currentSequence].setTicks(i, repetition);
      }
    }


    void setSelectedRetrigs(boolean selectedNotes[], byte retrigs) 
    {
      for (int i=0; i < 16; i++) {
        if (selectedNotes[i]) m_sequence[m_currentSequence].setRetrig(i, retrigs);
      }
    }


    void setSelectedProbabilities(boolean selectedNotes[], byte prob)
    {
      for (int i=0; i < 16; i++) {
        if (selectedNotes[i]) m_sequence[m_currentSequence].setProbability(i, (byte)prob);
      }
    }


    void setNote(int _step, byte note)
    {
       m_sequence[m_currentSequence].setNote(_step, note); 
    }

    void setMute(int _step, bool muteFlag)
    {
       m_sequence[m_currentSequence].setMute(_step, muteFlag); 
    }

    void setHold(int _step, bool holdFlag)
    {
       m_sequence[m_currentSequence].setHold(_step, holdFlag); 
    }

    void offsetNote(int _step, byte note_offset) // kg
    {
       int current_note = m_sequence[m_currentSequence].getNote(_step);
       int new_note = current_note + note_offset;
       m_sequence[m_currentSequence].setNote(_step, new_note);
    }
    
    void offsetDuration(int _step, float duration_offset) // kg
    {
       float current_duration = m_sequence[m_currentSequence].getDuration(_step);
       float new_duration = current_duration + duration_offset;
       m_sequence[m_currentSequence].setDuration(_step, new_duration);
    }

    void setDuration(int _step, float duration) // kg
    {
       m_sequence[m_currentSequence].setDuration(_step, duration);      
    }

    void setProbability(int _step, byte prob) // kg
    {
       m_sequence[m_currentSequence].setProbability(_step, prob);      
    }

    void setTicks(int _step, byte repetition) {
       m_sequence[m_currentSequence].setTicks(_step, repetition); 
    }
    
    void setTransposition(byte transposition)
    {
        m_sequence[m_currentSequence].setTransposition(transposition);
    }
    
    void setLength(byte _length)
    {
        m_sequence[m_currentSequence].setLength(_length);
    }

    void setAccent(int _step, byte accent)
    {
        m_sequence[m_currentSequence].setAccent(_step, accent);
    }

    void setRetrig(int _step, byte retrig)
    {
        m_sequence[m_currentSequence].setRetrig(_step, retrig);
    }

    void setVelocity(int _step, byte velocity)
    {
        m_sequence[m_currentSequence].setVelocity(_step, velocity);
    }

    void setPath(byte path)
    {
        m_sequence[m_currentSequence].setPath(path);
    }

    void setCurrentSequence(int index)
    {
        if(index >= 0 && index < max_sequences && index != m_currentSequence) {
            m_currentSequence = index;
            m_recall_buffer_active = false;
        }
    }
    
    //Helper method
    bool playOrNot(int index)
    {
      //evaluate probability...
      return false;
    }

    
    void resetSequence(int index)
    {
        if(index >=0 && index < max_sequences)
          m_sequence[index].reset();
    }
    
    void selectPreviousSequence()
    {
       if(m_currentSequence > 0)
        m_currentSequence--; 
    }
    
    void selectNextSequence()
    {
       if(m_currentSequence < max_sequences -1)
        m_currentSequence++; 
    }

    void printSequence()
    {
        Serial.print("Sequence ");
        Serial.print(m_currentSequence);
        Serial.println(": ");
        m_sequence[m_currentSequence].printSequence();
    }

    bool notesArrayEmpty(boolean notesArray[])
    {
      bool retVal = true;
      for (int i=0; i < 16; i++) if (notesArray[i]) retVal = false;
      return retVal;
    }
};
#endif
