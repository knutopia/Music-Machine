#ifndef __STEPCLICKLIST
#define __STEPCLICKLIST

#include <Arduino.h>
#include "Enum.h"
#include "Note.h"
#include "PerClickNoteList.h"

// List per click in a master step, 
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

    void addClickNote(note *aNote, byte aTrack, 
                      unsigned long aDuration, 
                      int aMasterStep, 
                      int aClickStep);
    void append(int aMasterStep, byte aClickStep);
    void insertBefore(int aMasterStep, byte aClickStep);
    int getMasterStep();
    PerClickNoteList* getClickNoteList(byte a_click);
    byte getClickStep();
    PerClickNoteList* getNotes();
    void dropNotesBeforeStepAndRewind(int aStep);
    void dropHead();
    void rewind();
    void next();
    int hasValue();

private:
    stepClickNode *head;
    stepClickNode *cur;
    stepClickNode *tail;

};
#endif