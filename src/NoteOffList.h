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
    byte getTrack();
    byte getMidiNote();
    unsigned long getNoteOffTime();
    int hasValue();
    int count();

private:
    volatile noteOffNode *head;
    volatile noteOffNode *cur;
    volatile noteOffNode *tail;
};

#endif