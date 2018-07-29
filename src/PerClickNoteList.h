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
//  void append(note *aNote, byte aTrack, unsigned long aDurationMS);
    void append(note aNote, byte aTrack, unsigned long aDurationMS);
//  note* getNote();
    note getNote();
    byte getTrack();
    int hasValue();
    unsigned long getDurationMS();
    void rewind();
    void next();

private:
    notePerClick *head;
    notePerClick *cur;
    notePerClick *tail;
};
#endif