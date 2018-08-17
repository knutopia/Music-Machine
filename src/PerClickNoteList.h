#ifndef __PERCLICKNOTELIST
#define __PERCLICKNOTELIST

#include <Arduino.h>
#include "Note.h"

// List of all notes to play in that click
// as note + durationMS combo
// used per step by ClickNodeList below
class PerClickNoteList
{
    struct notePerClick {
        note clickNote;
        unsigned long durationMS;
        byte track;
        notePerClick *next;
    };

public:
    PerClickNoteList();
    ~PerClickNoteList();
    
    void purge();
    void checkIntegrity(char caller[]);
    void print();
    void append(note aNote, byte aTrack, unsigned long aDurationMS);
    note getNote();
    int getCur();
    int getNext();
    note readNote();
    byte getTrack();
    byte readTrack();
    bool hasValue();
    unsigned long getDurationMS();
    unsigned long readDurationMS();
    void rewind();
    void next();
    bool hasReadValue();
    void readRewind();
    void readNext();
private:
    notePerClick *head;
    notePerClick *cur;
    notePerClick *tail;
    notePerClick *readCur;
};
#endif