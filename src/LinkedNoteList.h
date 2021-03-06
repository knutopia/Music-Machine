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
    bool checkIntegrity(const char caller[]);
    void dropNotesBeforeStepAndRewind(int aStep);
    void dropHeadNote();
    void prependNote(int aStep, byte aTrack, const note aNote);
    void appendNote(int aStep, byte aTrack, const note aNote);
    void rewind();
    void next();
    int getStep();
    int getTrack();
    note getNote();
    int hasValue();
    int count();
    void print();

private:
    noteNode *head;
    noteNode *cur;
    noteNode *tail;
};
#endif