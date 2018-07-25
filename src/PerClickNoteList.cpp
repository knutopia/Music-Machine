#include "PerClickNoteList.h"

PerClickNoteList::PerClickNoteList()
{
    head = NULL;
    cur = NULL;
    tail = NULL;
}

PerClickNoteList::~PerClickNoteList()
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

void PerClickNoteList::append(note *aNote, byte aTrack, unsigned long aDurationMS)
{
    notePerClick *n = new notePerClick();   // create new Node
    n->clickNote = aNote;  // set value
    n->track = aTrack;
    n->durationMS = aDurationMS;

    if(tail != NULL)
        tail->next = n; // point previously last node to new one

    tail = n;           // point tail at new node

    if(cur == NULL)
        cur = n;

    if(head == NULL)
        head = n;
}

// deprecated: no track !
void PerClickNoteList::append(note *aNote, unsigned long aDurationMS)
{
    notePerClick *n = new notePerClick();   // create new Node
    n->clickNote = aNote;  // set value
    n->track = 1;
    n->durationMS = aDurationMS;

    if(tail != NULL)
        tail->next = n; // point previously last node to new one

    tail = n;           // point tail at new node

    if(cur == NULL)
        cur = n;

    if(head == NULL)
        head = n;
}

note* PerClickNoteList::getNote()
{
    note* retVal;

    if( cur != NULL )
        retVal = cur->clickNote;
        // really we should raise exception...
    return retVal; 
}

byte PerClickNoteList::getTrack()
{
    byte retVal;

    if( cur != NULL )
        retVal = cur->track;

    return retVal; 
}

int PerClickNoteList::hasValue()
{
    return ( cur != NULL ? true : false );
}

unsigned long PerClickNoteList::getDurationMS()
{
    unsigned long retVal;

    if( cur != NULL )
        retVal = cur->durationMS;
        // really we should raise exception...
    return retVal; 
}

void PerClickNoteList::rewind()
{
        cur = head;
}

void PerClickNoteList::next()
{
        if( cur != NULL )
                cur = cur->next;
}