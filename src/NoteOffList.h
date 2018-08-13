#ifndef __NOTEOFFLIST
#define __NOTEOFFLIST

#include <Arduino.h>
#include "Enum.h"

using namespace std;
 
class NoteOffList 
{
    
    struct noteOffNode {
        unsigned long noteOffTime;
        byte trackNumber;
        byte midiNote;
        volatile noteOffNode *prev;
        volatile noteOffNode *next;
    };
 
// public member
public:
    // constructor
    NoteOffList();
    ~NoteOffList();
    void checkIntegrity(char caller[]);
    void printList();
    void dropNode();
    void append(byte aTrackNum, byte aMidiNote, unsigned long anOffTime);
    void rewind();
    void next();
    byte readTrack();
    byte readMidiNote();
    unsigned long readNoteOffTime();
    int hasValue();
    int hasReadValue();
    void readRewind();
    void readNext();
    int count();

private:
    volatile noteOffNode *head;
    volatile noteOffNode *cur;
    volatile noteOffNode *tail;
    volatile noteOffNode *readCur;
};

#endif