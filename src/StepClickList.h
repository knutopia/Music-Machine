#ifndef __STEPCLICKLIST
#define __STEPCLICKLIST

#include <Arduino.h>
#include "Enum.h"
#include "Note.h"
#include "PerClickNoteList.h"

typedef void (*CbWhenStuck) ();

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

    void begin(CbWhenStuck panicCbPointer);    
    void purge();
    bool checkIntegrity(const char caller[]);
    void addClickNote(note aNote, byte aTrack, 
                      unsigned long aDuration, 
                      int aMasterStep, 
                      int aClickStep);
    void append(int aMasterStep, byte aClickStep);
    void insertBefore(int aMasterStep, byte aClickStep);
    int getMasterStep();
    PerClickNoteList getClickNoteList(byte a_click, int a_step);
    bool getClickNoteList(PerClickNoteList *target, byte a_click, int a_step);
    bool transferClickNoteList(PerClickNoteList& target, byte a_click, int a_step);
    bool transferClickNoteArray(byte a_click, int a_step);
    byte getClickStep();
    PerClickNoteList* getNotes();
    void dropNotesBeforeStepAndRewind(int aStep);
    void dropReadCur();
    void dropHead();
    void rewind();
    volatile void readRewind();
    void next();
    volatile void readNext();
    int hasValue();
    volatile int hasReadValue();
    int count();
    void print();

private:

    // Callback
    CbWhenStuck PanicCb;

    stepClickNode *head;
    stepClickNode *cur;
    stepClickNode *tail;
    volatile stepClickNode *readCur;
};
#endif