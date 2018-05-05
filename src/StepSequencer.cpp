//Arduino for Musicians
//StepSequencer: A container class for StepSequence objects

//#include "arduino.h"
#include "StepSequencer.h"

//  public:

    //Public constructor and methods
    StepSequencer::StepSequencer()
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

    bool StepSequencer::playItOrNot(int _step)
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
    
    void StepSequencer::prime_edit_buffers()
    {
        for(int i = 0; i < max_sequences; i++) 
          reset_edit_seq(i);            
    }

    void StepSequencer::reset_edit_seq(int seqnum)
    {
        m_sequence_root[seqnum].copySeqTo(m_sequence[seqnum]);            
    }

    void StepSequencer::save_sequence(int destination)
    {
        m_sequence[m_currentSequence].copySeqTo(m_sequence_root[destination]);
        m_sequence[m_currentSequence].copySeqTo(m_sequence[destination]);
    }
    
    void StepSequencer::save_edit_seq_to_root(int seqnum)
    {
        m_sequence[seqnum].copySeqTo(m_sequence_root[seqnum]);
    }

    void StepSequencer::copy_edit_buffers_to_roots()
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

    void StepSequencer::swap_edit_root_seqs(int seqnum)
    {
        StepSequence swap_buffer;
        
        m_sequence_root[seqnum].copySeqTo(swap_buffer);
        m_sequence[seqnum].copySeqTo(m_sequence_root[seqnum]);
        swap_buffer.copySeqTo(m_sequence[seqnum]);
    }

    bool StepSequencer::toggle_pattern_recall()
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
    byte StepSequencer::getNote(int _step)
    {
        return m_sequence[m_currentSequence].getNote(_step);
    }
    
    byte StepSequencer::getTransposition()
    {
        return m_sequence[m_currentSequence].getTransposition();
    }
    
    byte StepSequencer::getLength()
    {
        return m_sequence[m_currentSequence].getLength();
    }
    
    byte StepSequencer::getMaxLength()
    {
        return m_sequence[m_currentSequence].getMaxLength();
    }

    float StepSequencer::getDuration(int _step) // kg
    {
        return m_sequence[m_currentSequence].getDuration(_step);
    }
    
    byte StepSequencer::getProbability(int _step) // kg
    {
        return m_sequence[m_currentSequence].getProbability(_step);
    }

    byte StepSequencer::getTicks(int _step) // kg
    {
        return m_sequence[m_currentSequence].getTicks(_step);
    }

    bool StepSequencer::getMute(int _step) // kg
    {
        return m_sequence[m_currentSequence].getMute(_step);
    }

 
    bool StepSequencer::getHold(int _step) // kg
    {
        return m_sequence[m_currentSequence].getHold(_step);
    }

 
    byte StepSequencer::getAccent(int _step) // kg
    {
        return m_sequence[m_currentSequence].getAccent(_step);      
    }


    byte StepSequencer::getRetrig(int _step) // kg
    {
        return m_sequence[m_currentSequence].getRetrig(_step);      
    }


    byte StepSequencer::getVelocity(int _step) // kg
    {
        return m_sequence[m_currentSequence].getVelocity(_step);      
    }


    byte StepSequencer::getPath() // kg
    {
        return m_sequence[m_currentSequence].getPath();
    }

    int StepSequencer::getCurrentSequence()
    {
        return m_currentSequence;
    }

    byte StepSequencer::getLowestSelectedNote(boolean selectedNotes[])
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
          
    byte StepSequencer::getHighestSelectedNote(boolean selectedNotes[])
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
  
    byte StepSequencer::getLowestSelectedVelocity(boolean selectedNotes[])
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
          
    byte StepSequencer::getHighestSelectedVelocity(boolean selectedNotes[])
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
  
    float StepSequencer::getShortestSelectedNote(boolean selectedNotes[])
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
          
    float StepSequencer::getLongestSelectedNote(boolean selectedNotes[])
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
    
    void StepSequencer::offsetSelectedNotes(boolean selectedNotes[], byte note_offset, byte rawHeldStep) // kg
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
    

    void StepSequencer::offsetSelectedDurations(boolean selectedNotes[], float duration_offset, byte rawHeldStep) // kg
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


    void StepSequencer::setSelectedRepetitions(boolean selectedNotes[], byte repetition) 
    {
      for (int i=0; i < 16; i++) {
        if (selectedNotes[i]) m_sequence[m_currentSequence].setTicks(i, repetition);
      }
    }


    void StepSequencer::setSelectedRetrigs(boolean selectedNotes[], byte retrigs) 
    {
      for (int i=0; i < 16; i++) {
        if (selectedNotes[i]) m_sequence[m_currentSequence].setRetrig(i, retrigs);
      }
    }


    void StepSequencer::setSelectedProbabilities(boolean selectedNotes[], byte prob)
    {
      for (int i=0; i < 16; i++) {
        if (selectedNotes[i]) m_sequence[m_currentSequence].setProbability(i, (byte)prob);
      }
    }


    void StepSequencer::setNote(int _step, byte note)
    {
       m_sequence[m_currentSequence].setNote(_step, note); 
    }

    void StepSequencer::setMute(int _step, bool muteFlag)
    {
       m_sequence[m_currentSequence].setMute(_step, muteFlag); 
    }

    void StepSequencer::setHold(int _step, bool holdFlag)
    {
       m_sequence[m_currentSequence].setHold(_step, holdFlag); 
    }

    void StepSequencer::offsetNote(int _step, byte note_offset) // kg
    {
       int current_note = m_sequence[m_currentSequence].getNote(_step);
       int new_note = current_note + note_offset;
       m_sequence[m_currentSequence].setNote(_step, new_note);
    }
    
    void StepSequencer::offsetDuration(int _step, float duration_offset) // kg
    {
       float current_duration = m_sequence[m_currentSequence].getDuration(_step);
       float new_duration = current_duration + duration_offset;
       m_sequence[m_currentSequence].setDuration(_step, new_duration);
    }

    void StepSequencer::setDuration(int _step, float duration) // kg
    {
       m_sequence[m_currentSequence].setDuration(_step, duration);      
    }

    void StepSequencer::setProbability(int _step, byte prob) // kg
    {
       m_sequence[m_currentSequence].setProbability(_step, prob);      
    }

    void StepSequencer::setTicks(int _step, byte repetition) {
       m_sequence[m_currentSequence].setTicks(_step, repetition); 
    }
    
    void StepSequencer::setTransposition(byte transposition)
    {
        m_sequence[m_currentSequence].setTransposition(transposition);
    }
    
    void StepSequencer::setLength(byte _length)
    {
        m_sequence[m_currentSequence].setLength(_length);
    }

    void StepSequencer::setAccent(int _step, byte accent)
    {
        m_sequence[m_currentSequence].setAccent(_step, accent);
    }

    void StepSequencer::setRetrig(int _step, byte retrig)
    {
        m_sequence[m_currentSequence].setRetrig(_step, retrig);
    }

    void StepSequencer::setVelocity(int _step, byte velocity)
    {
        m_sequence[m_currentSequence].setVelocity(_step, velocity);
    }

    void StepSequencer::setPath(byte path)
    {
        m_sequence[m_currentSequence].setPath(path);
    }

    void StepSequencer::setCurrentSequence(int index)
    {
        if(index >= 0 && index < max_sequences && index != m_currentSequence) {
            m_currentSequence = index;
            m_recall_buffer_active = false;
        }
    }
    
    //Helper method
    bool StepSequencer::playOrNot(int index)
    {
      //evaluate probability...
      return false;
    }

    
    void StepSequencer::resetSequence(int index)
    {
        if(index >=0 && index < max_sequences)
          m_sequence[index].reset();
    }
    
    void StepSequencer::selectPreviousSequence()
    {
       if(m_currentSequence > 0)
        m_currentSequence--; 
    }
    
    void StepSequencer::selectNextSequence()
    {
       if(m_currentSequence < max_sequences -1)
        m_currentSequence++; 
    }

    void StepSequencer::printSequence()
    {
        Serial.print("Sequence ");
        Serial.print(m_currentSequence);
        Serial.println(": ");
        m_sequence[m_currentSequence].printSequence();
    }

    bool StepSequencer::notesArrayEmpty(boolean notesArray[])
    {
      bool retVal = true;
      for (int i=0; i < 16; i++) if (notesArray[i]) retVal = false;
      return retVal;
    }
