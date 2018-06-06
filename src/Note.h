#ifndef __NOTE
#define __NOTE

#include <Arduino.h>
#include "Enum.h"
//#include <cstddef>
//#include <iostream>

using namespace std;

//#define eol "\n"

struct note {
    retrigDivisions retrigClickDivider;
    bool unmuted;
    bool playIt;
    int pitchVal;
    float pitchFreq;

    unsigned long durationMS;
    bool hold;

    float duration;
    
    uint8_t retrigs;
    uint8_t ticks;
    uint8_t accent; //??
    uint8_t velocity;
    uint8_t swingTicks;

//      byte m_probability[max_notes];
//      byte m_retrig[max_notes];

//      byte m_length;
//      byte m_transposition;
//      byte m_path;
};
 
// Thank you Dan Farrell on StackOverflow for linked list example
class LinkedNoteList 
{
    
    struct noteNode {
        int masterStep;
        byte track;
        note trackNote;
        noteNode *next;
    };
 
// public member
public:
    // constructor
    LinkedNoteList()
    {
        head = NULL; // set head to NULL
        cur = NULL;
        tail = NULL;
    }
 
    void dropNotesBeforeStepAndRewind(int aStep)
    {
        while(head != NULL && head->masterStep < aStep)
            dropHeadNote();
        cur = head;
    }

    void dropHeadNote()
    {
        if (head != NULL)
        {
            noteNode *newHead = head->next;
/*            
            head->masterStep = NULL;
            head->track = NULL;
            head->note = NULL;
            head->next = NULL;
*/
            if (cur == head)
                cur = newHead;
            head = newHead;
        }
    }

    // This prepends a new value at the beginning of the list
    void prependNote(int aStep, byte aTrack, note aNote)
    {
        noteNode *n = new noteNode();   // create new Node
        n->masterStep = aStep;  // set value
        n->track = aTrack;
        n->trackNote = aNote;
        n->next = head;         // make the node point to the next node.
                                //  If the list is empty, this is NULL, so the end of the list --> OK
        head = n;               // last but not least, make the head point at the new node.
        if( cur == NULL)
                cur = head;

        if(tail == NULL)
                tail = n;
    }

    // add value at the end -kg
    void appendNote(int aStep, byte aTrack, note aNote)
    {
        noteNode *n = new noteNode();   // create new Node
        n->masterStep = aStep;  // set value
        n->track = aTrack;
        n->trackNote = aNote;

        if(tail != NULL)
            tail->next = n; // point previously last node to new one

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

    int getStep()
    {
            if( cur != NULL )
                    return cur->masterStep;
            return 0; // really we should raise exception
    }

    int getTrack()
    {
            if( cur != NULL )
                    return cur->track;
            return 0; // really we should raise exception
    }

    note getNote()
    {
        note retVal;

        if( cur != NULL )
            retVal = cur->trackNote;
            // really we should raise exception...
        return retVal; 
    }
    
    int hasValue()
    {
            return ( cur != NULL ? true : false );
    }

private:
    noteNode *head;
    noteNode *cur;
    noteNode *tail;
};

#endif