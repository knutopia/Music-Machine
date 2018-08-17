#ifndef __STEPCLICKLIST
#define __STEPCLICKLIST

#include <Arduino.h>
#include "Enum.h"
#include "Note.h"
#include "PerClickNoteList.h"

// List of clicks in a master step, 
// holding all notes to play in that click
// assembled from the individual tracks in the sequencer
// to service the playback interrupt in Timebase.cpp

class StepClickList
{
    struct stepClickNode {
        int masterStep;
        byte clickStep;
        PerClickNoteList *notes;
        stepClickNode *prev;
        stepClickNode *next;
    };

   public:
    StepClickList();
    ~StepClickList();
    
    void checkIntegrity(char caller[]);
    void addClickNote(note aNote, byte aTrack, 
                      unsigned long aDuration, 
                      int aMasterStep, 
                      int aClickStep);
    void append(int aMasterStep, byte aClickStep);
    void insertBefore(int aMasterStep, byte aClickStep);
    int getMasterStep();
    PerClickNoteList* getClickNoteList(byte a_click, int a_step);
    bool getClickNoteListVal(PerClickNoteList *target, byte a_click, int a_step);
    byte getClickStep();
    PerClickNoteList* getNotes();
    void dropNotesBeforeStepAndRewind(int aStep);
    void dropHead();
    void rewind();
    volatile void readRewind();
    void next();
    volatile void readNext();
    int hasValue();
    volatile int hasReadValue();

private:
    stepClickNode *head;
    stepClickNode *cur;
    stepClickNode *tail;
    volatile stepClickNode *readCur;
};
#endif