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

// List of all notes to play in that click
// as note + durationMS combo
// used per step by ClickNodeList below
class PerClickNoteList
{
   struct notePerClick {
        note *clickNote;
        unsigned long durationMS;
        notePerClick *next;
    };

public:
    PerClickNoteList()
    {
        head = NULL;
        cur = NULL;
        tail = NULL;
    }

    ~PerClickNoteList()
    {
        notePerClick *die;

        Serial.print("Destructor PerClickNoteList ");

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

    void append(note *aNote, unsigned long aDurationMS)
    {
        notePerClick *n = new notePerClick();   // create new Node
        n->clickNote = aNote;  // set value
        n->durationMS = aDurationMS;

        if(tail != NULL)
            tail->next = n; // point previously last node to new one

        tail = n;           // point tail at new node

        if(cur == NULL)
            cur = n;

        if(head == NULL)
            head = n;
    }

    note* getNote()
    {
        note *retVal;

        if( cur != NULL )
            retVal = cur->clickNote;
            // really we should raise exception...
        return retVal; 
    }
    
    int hasValue()
    {
        return ( cur != NULL ? true : false );
    }

    unsigned long getDurationMS()
    {
        unsigned long retVal;

        if( cur != NULL )
            retVal = cur->durationMS;
            // really we should raise exception...
        return retVal; 
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

private:
    notePerClick *head;
    notePerClick *cur;
    notePerClick *tail;
};


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
    StepClickList()
    {
        head = NULL;
        cur = NULL;
        tail = NULL;
    }

    ~StepClickList()
    {
        stepClickNode *die;

        Serial.print("Destructor StepClickList ");

        while( hasValue()){
            die = cur;
            next();
            delete die->notes;
            delete die;

            Serial.print("die ");
        }
        Serial.println();
    }

    void addClickNote(note *aNote, unsigned long aDuration, int aMasterStep, int aClickStep)
    {
        // 1) traverse the list to find the right master step
        // 2) find the right click
        // 3) if it's not there, add it
        // 4) insert the content
        //
        // Could use de-duping between notes on 
        // same track on same clickstep on monophonic tracks
        bool done = false;

        rewind();
        while( hasValue() && !done)
        {
            if (cur->masterStep == aMasterStep) 
            {
                if (cur->clickStep == aClickStep)
                {
                    // add new clickNoteNode
                    cur->notes->append(aNote, aDuration);
                    done = true;
                } else
                {
                    if (cur->clickStep > aClickStep) 
                    {
                        // no matching stepClickNode yet, make it.
                        insertBefore(cur->masterStep, aClickStep);
                        cur->notes->append(aNote, aDuration);
                        done = true;
                    }
                }
            }
            next();
        }
        if (!done)
        {
            // not done, so 
            if(head != NULL && head->masterStep > aMasterStep)
            { // add stepClickNode at the start
                rewind();
                insertBefore(aMasterStep, aClickStep);
                cur->notes->append(aNote, aDuration);
            } else 
            { // add stepClickNode at the end
                append(aMasterStep, aClickStep);
                cur = tail;
                cur->notes->append(aNote, aDuration);
            }
        }
    }

    void append(int aMasterStep, byte aClickStep)
    {
        stepClickNode *n = new stepClickNode();
        n->masterStep = aMasterStep;
        n->clickStep = aClickStep;
        n->notes = new PerClickNoteList();
        
        if(tail != NULL)
            tail->next = n; // point previously last node to new one

        tail = n;           // point tail at new node

        if(cur == NULL)
            cur = n;

        if(head == NULL)
            head = n;
    }

    void insertBefore(int aMasterStep, byte aClickStep)
    {
        stepClickNode *n = new stepClickNode();
        n->masterStep = aMasterStep;
        n->clickStep = aClickStep;
        n->notes = new PerClickNoteList();

        n->next = cur;

        if (tail == NULL)
            tail = cur;

        if (cur == NULL)
            cur = head;

        if (cur == NULL)
        {
            cur = n;
            head = n;
        } else
        {
            if(cur == head)
            {
                n->next = head;
                head->prev = n;
                head = n;
                cur = n;
            } else
            {
                if(cur->prev != NULL)
                {
                    stepClickNode *prev = cur->prev;
                    prev->next = n;
                } 
                cur->prev = n;
                cur = n;
            }
        }
    }

    int getMasterStep()
    {
        int retVal;

        if( cur != NULL )
            retVal = cur->masterStep;
            // really we should raise exception...
        return retVal; 
    }

    byte getClickStep()
    {
        byte retVal;

        if( cur != NULL )
            retVal = cur->clickStep;
            // really we should raise exception...
        return retVal; 
    }

    PerClickNoteList* getNotes()
    {
        PerClickNoteList *retVal;

        if( cur != NULL )
            retVal = cur->notes;
            // really we should raise exception...
        return retVal; 
    }

    void dropNotesBeforeStepAndRewind(int aStep)
    {
        Serial.print("dropNotesBeforeStepAndRewind before ");
        Serial.println(aStep);

        bool b = true;
        while(b)
        {
            if (head == NULL)
            {
                Serial.print("head == NULL before ");
                Serial.println(aStep);
                b = false;
            } else
            {
                if (head->masterStep == NULL)
                {
                        Serial.print("head->masterStep == NULL before ");
                        Serial.println(aStep);
                        b = false;
                } else
                {
                    if (head->masterStep < aStep)
                    {
                        Serial.print("dropping before ");
                        Serial.println(aStep);
                        dropHead();
                    } else
                    {
                        Serial.print("head->masterStep NOT < aStep");
                        Serial.println(aStep);
                        b = false;
                    }
                }
            }
        }
/*        
        while(head != NULL && head->masterStep < aStep)
        {
            Serial.print("dropping before ");
            Serial.println(aStep);
            dropHead();
        }
*/
        cur = head;
    }

    void dropHead()
    {
        if (head != NULL)
        {
            stepClickNode *newHead = head->next;

            if (cur == head)
                cur = newHead;

            delete head->notes;
            delete head;
            head = newHead;
        }
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
    
    int hasValue()
    {
            return ( cur != NULL ? true : false );
    }

private:
    stepClickNode *head;
    stepClickNode *cur;
    stepClickNode *tail;
};

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
 
// public member
public:
    // constructor
    LinkedNoteList()
    {
        head = NULL;
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

            if (cur == head)
                cur = newHead;

            delete head;
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
        n->next = NULL;

        if(tail != NULL && tail != n)
            tail->next = n; // point previously last node to new one

        tail = n;           // point tail at new node

        if(cur == NULL)
            cur = n;

        if(head == NULL) // REPEATED in print section below...
            head = n;

/*
        Serial.println("Notelist appendNote");

        if(head == NULL)
        {
            Serial.println(" :head was NULL");
            head = n;
        } else
            Serial.println(" :head was Object");

        if (cur == n)
            Serial.println(" :cur == n");
        else
            Serial.println(" :cur != n");
        if (cur == head)
            Serial.println(" :cur == head");
        else
            Serial.println(" :cur != head");
        if (cur == head)
            Serial.println(" :cur == head");
        else
            Serial.println(" :cur != head");
        if (cur == tail)
            Serial.println(" :cur == tail");
        else
            Serial.println(" :cur != tail");
        if (cur == NULL)
            Serial.println(" :cur == NULL");
        else
            Serial.println(" :cur Object");
        if (head == NULL)
            Serial.println(" :head == NULL");
        else
            Serial.println(" :head Object");
        if (n->next == NULL)
            Serial.println(" :n->next == NULL");
        else
            Serial.println(" :n->next Object");
        if (head->next == NULL)
            Serial.println(" :head->next == NULL");
        else
            Serial.println(" :head->next Object");
        if (n == n->next)
            Serial.println(" :n->next circular");
        else
            Serial.println(" :n->next OK");
        if (head == head->next)
            Serial.println(" :head->next circular");
        else
            Serial.println(" :head->next OK");
*/
    }

    void rewind()
    {
            cur = head;
            if(head == NULL)
                Serial.println("NULL head on Notelist rewind");
    }
    void next()
    {
            if( cur != NULL )
            {
                if( cur == cur->next)
                    Serial.println("cur->next circular on Notelist next");
                cur = cur->next;
            }
            else
                Serial.println("NULL cur on Notelist next");
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

        Serial.print("Notelist GetNote ");

        if( cur != NULL )
        {
            retVal = cur->trackNote;

            Serial.println(retVal.pitchVal);

            // really we should raise exception...
        } else
            Serial.println(" NULL cur");

        return retVal; 
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
            Serial.print("Count ");
            Serial.println(retVal);

            retVal++;
            next();
        }        
        return retVal;
    }

private:
    noteNode *head;
    noteNode *cur;
    noteNode *tail;
};

#endif