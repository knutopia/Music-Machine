#ifndef __STEPSEQUENCE
#define __STEPSEQUENCE

#include "Enum.h"

class StepSequence
{
    public:
    //use enumeration to define a class constant
    enum{max_notes = 16};
    
    private:
    byte m_notes[max_notes];
    float m_duration[max_notes];   // float
    byte m_probability[max_notes];
    byte m_ticks[max_notes];
    bool m_mute[max_notes];        // bool
    bool m_hold[max_notes];        // bool
    byte m_accent[max_notes];
    byte m_retrig[max_notes];
    byte m_velocity[max_notes];
    byte m_length;
    byte m_transposition;
    byte m_path;

    public:
    StepSequence()
    {
       m_length = max_notes;
       m_transposition = 0;
       m_path = 0;
       reset();
    };
    
    void reset()
    {
         //Initialize sequence flat
        for(int n = 0; n < max_notes; n++)
        {
           m_notes[n] = 32;
           m_duration[n] = .5;
           m_probability[n] = 3;
           m_ticks[n] = 1;
           m_mute[n] = true;
           m_hold[n] = false;
           m_accent[n] = 0;
           m_retrig[n] = 0;
           m_velocity[n] = 128;
        }
    }

    void copySeqTo(StepSequence &destination) 
    {
        for(byte n = 0; n < max_notes; n++)
        {
           destination.setNote(n, m_notes[n]); 
           destination.setDuration(n, m_duration[n]);
           destination.setProbability(n, m_probability[n]);
           destination.setTicks(n, m_ticks[n]);
           destination.setMute(n, m_mute[n]);
           destination.setHold(n, m_hold[n]);
           destination.setAccent(n, m_accent[n]);
           destination.setRetrig(n, m_retrig[n]);
           destination.setVelocity(n, m_velocity[n]);
        }
        destination.setLength(m_length);
        destination.setTransposition(m_transposition);
        destination.setPath(m_path);
    }


    
    byte getNote(int _step)
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_notes[_step];
       } 
       return -1;  //error
    }
    
    float getDuration(int _step) // kg
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_duration[_step];
       } 
       return -1;  //error
    }
    
    byte getProbability(int _step) // kg
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_probability[_step];
       } 
       return -1;  //error
    }
        
    byte getTicks(int _step) // kg
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_ticks[_step];
       } 
       return -1;  //error
    }
    
    byte getTransposedNote(int _step)
    {
       return getNote(_step) + m_transposition; 
    }
    
    byte getTransposition(){return m_transposition;};
    
    void setTransposition(byte trans)
    {
       m_transposition = trans; 
    }
    
    bool getMute(int _step)
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_mute[_step];
       } 
       return false;  //error
    }
    
    bool getHold(int _step)
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_hold[_step];
       } 
       return false;  //error
    }
    
    byte getAccent(int _step)
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_accent[_step];
       } 
       return false;  //error
    }

    
    byte getRetrig(int _step)
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_retrig[_step];
       } 
       return false;  //error
    }

    byte getVelocity(int _step)
    {
       if(_step >=0 && _step < max_notes)
       {
          return m_velocity[_step];
       } 
       return false;  //error
    }

    void setNote(int _step, byte note)
    {
       if(_step >=0 && _step < max_notes)
       {
           m_notes[_step] = note;  
       }
    }
    
    void setLength(byte _length)
    {
       if(_length <= max_notes && _length >0)
       {
          m_length = _length;
       } 
    }

    void setDuration(int _step, float duration) // kg...
    {
       if(_step >=0 && _step < max_notes)
       {
           m_duration[_step] = duration;  
       }      
    }
    
    void setProbability(int _step, byte probabil) // kg...
    {
       if(_step >=0 && _step < max_notes)
       {
           m_probability[_step] = probabil;  
       }      
    }
        
    void setTicks(int _step, float repetition) // kg...
    {
       if(_step >=0 && _step < max_notes)
       {
           m_ticks[_step] = repetition;  
       }      
    }

    void setMute(int _step, bool muteFlag)
    {
       if(_step >=0 && _step < max_notes)
       {
           m_mute[_step] = muteFlag;  
       }
    }

    void setHold(int _step, bool holdFlag)
    {
       if(_step >=0 && _step < max_notes)
       {
           m_hold[_step] = holdFlag;  
       }
    }


    void setAccent(int _step, byte accent)
    {
       if(_step >=0 && _step < max_notes)
       {
           m_accent[_step] = accent;  
       }
    }


    void setRetrig(int _step, byte retrig)
    {
       if(_step >=0 && _step < max_notes)
       {
           m_retrig[_step] = retrig;  
       }
    }


    void setVelocity(int _step, byte velocity)
    {
       if(_step >=0 && _step < max_notes)
       {
           m_velocity[_step] = velocity;  
       }
    }


    void setPath(byte path)
    {
        m_path = path;
    }

    byte getLength(){return m_length;};
    int getMaxLength(){return max_notes;};
    byte getPath(){return m_path;}

    // Utility
    void printSequence()
    {
        Serial.println("Note\tDuratn\tProb\tRepeat\tNotMute\tHold\tAccent\tRetrig\tVelocity");
        for(int n = 0; n < max_notes; n++)
        {
           Serial.print(m_notes[n]); 
           Serial.print("\t"); 
           Serial.print(m_duration[n]);
           Serial.print("\t"); 
           Serial.print(m_probability[n]);
           Serial.print("\t"); 
           Serial.print(m_ticks[n]);
           Serial.print("\t"); 
           Serial.print(m_mute[n]);
           Serial.print("\t"); 
           Serial.print(m_hold[n]);
           Serial.print("\t"); 
           Serial.print(m_accent[n]);
           Serial.print("\t"); 
           Serial.print(m_retrig[n]);
           Serial.print("\t"); 
           Serial.println(m_velocity[n]);
        }
        Serial.println("Length\tTrans\tPath"); 
        Serial.print(m_length); 
        Serial.print("\t"); 
        Serial.print(m_transposition); 
        Serial.print("\t"); 
        Serial.println(m_path);
    }
};

#endif
