#ifndef __STEPSEQUENCE
#define __STEPSEQUENCE

#include <Arduino.h>
#include "Enum.h"
#include "Note.h"
#include "Path.h"

class StepSequence
{
    public:
        enum{max_notes = 16};
    
        StepSequence();

        void begin();
        void reset();
        void copySeqTo(StepSequence &destination);
        void copySeqTo(StepSequence *destination);
        byte getNote(int _step);
        float getDuration(int _step); // kg
        byte getProbability(int _step); // kg
        byte getTicks(int _step); // kg
        byte getTransposedNote(int _step);
        byte getTransposition();
        void setTransposition(byte trans);
        bool getMute(int _step);
        bool getHold(int _step);
        byte getAccent(int _step);
        byte getRetrig(int _step);
        byte getVelocity(int _step);
        bool playItOrNot(int _step);
        retrigDivisions getRetrigDivider(int count);
        note getSequenceNoteParams(int _step, Path aPath);
        byte getLength();
        int getMaxLength();
        byte getPath();

        void setNote(int _step, byte note);
        void setLength(byte _length);
        void setDuration(int _step, float duration); // kg...
        void setProbability(int _step, byte probabil); // kg...
        void setTicks(int _step, float repetition); // kg...
        void setMute(int _step, bool muteFlag);
        void setHold(int _step, bool holdFlag);
        void setAccent(int _step, byte accent);
        void setRetrig(int _step, byte retrig);
        void setVelocity(int _step, byte velocity);
        void setPath(byte path);

        // Utility
        void printSequence();

    private:
        unsigned long calcNextNoteDuration(const note aNote);
        byte assembleHolds(const note aNote, Path aPath);
        byte assembleMutes(const note aNote, Path aPath);

//      note m_noteStruct;
        byte m_notes[max_notes];
        float m_duration[max_notes];   // float
        byte m_probability[max_notes];
        byte m_ticks[max_notes];
        bool m_unmuted[max_notes];     // bool true = unmuted
        bool m_hold[max_notes];        // bool
        byte m_accent[max_notes];
        byte m_retrig[max_notes];
        byte m_velocity[max_notes];
        byte m_length;
        byte m_transposition;
        byte m_path;        
};

#endif
