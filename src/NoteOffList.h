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
        noteOffNode *prev;
        noteOffNode *next;
    };
 
// public member
public:
    // constructor
    NoteOffList()
    {
        head = NULL; // set head to NULL
        cur = NULL;
        tail = NULL;
    }

    ~NoteOffList()
    {
        noteOffNode *die;

        Serial.print("Destructor NoteOffList ");
        rewind();
        while( hasValue()){
            
            die = cur;
            next();
            delete die;

            Serial.print("die ");
        }
        head = NULL;
        cur = NULL;
        tail = NULL;

        Serial.println();
    }

    void dropNode()
    {
        if (cur == head)
        {
            Serial.println("dropNode 1");
            cur = head->next;
            if(cur != NULL)
                cur->prev = NULL;
            delete head;
            head = cur;
        } else 
        { 
            if (cur == tail)
            {
                Serial.println("dropNode 2");
                noteOffNode *prevNode = tail->prev;
                cur = prevNode;
                if(cur != NULL)
                    cur->next = NULL;
                delete tail;
                tail = cur;
            } else 
            {
                Serial.println("dropNode 3");
                noteOffNode *prevNode = cur->prev;
                prevNode->next = cur->next;
                cur->next->prev = cur->prev;
                delete cur;
                cur = prevNode;
            }
        }
        Serial.println("dropNode done");
    }

    // add value at the end -kg
    void append(byte aTrackNum, byte aMidiNote, unsigned long anOffTime)
    {
        noteOffNode *n = new noteOffNode();
        n->trackNumber = aTrackNum;
        n->midiNote = aMidiNote;
        n->noteOffTime = anOffTime;

        if(tail != NULL)
        {
            tail->next = n; // point previously last node to new one
            n->prev = tail;
        }

        tail = n;           // point tail at new node

        if(cur == NULL)
            cur = n;

        if(head == NULL)
            head = n;
    }

    void rewind()
    {
            cur = head;
    }

    
    void next()
    {
            if( cur != NULL )
                    cur = cur->next;
    }

    byte getTrack()
    {
        return cur->trackNumber;
    }

    byte getMidiNote()
    {
        return cur->midiNote;
    }

    unsigned long getNoteOffTime()
    {
        return cur->noteOffTime;
    }

    int hasValue()
    {
            return ( cur != NULL ? true : false );
    }

    int count()
    {
        int retVal = 0;
        rewind();

        while( hasValue()){
            retVal++;
            next();
        }        
        return retVal;
    }

private:
    noteOffNode *head;
    noteOffNode *cur;
    noteOffNode *tail;
};

#endif