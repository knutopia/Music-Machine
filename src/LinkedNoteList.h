#ifndef __LINKEDNOTELIST
#define __LINKEDNOTELIST

#include <Arduino.h>
#include "Enum.h"
#include "Note.h"

// (Thank you Dan Farrell on StackOverflow for linked list example)
// Getting active notes from tracks, per track
// ...quickly deprecated
class LinkedNoteList 
{
    struct noteNode {
        int masterStep;
        byte track;
        note trackNote;
        noteNode *next;
    };
 
public:
    LinkedNoteList();
    ~LinkedNoteList();

    void purge();
    void printActiveNote();
    void checkIntegrity(char caller[]);
    void dropNotesBeforeStepAndRewind(int aStep);
    void dropHeadNote();
    void prependNote(int aStep, byte aTrack, note aNote);
    void appendNote(int aStep, byte aTrack, note aNote);
    void rewind();
    void next();
    int getStep();
    int getTrack();
    note getNote();
    int hasValue();
    int count();

private:
    noteNode *head;
    noteNode *cur;
    noteNode *tail;
};
#endif