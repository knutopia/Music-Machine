#include "PerClickNoteList.h"

//#define DEBUG true

PerClickNoteList::PerClickNoteList()
{
    head = NULL;
    cur = NULL;
    tail = NULL;
    readCur = NULL;

#ifdef DEBUG
    Serial.println("Constructor PerClickNoteList ");
#endif
}

PerClickNoteList::~PerClickNoteList()
{
    notePerClick *die;

#ifdef DEBUG
    Serial.print("Destructor PerClickNoteList ");
#endif

    rewind();
    while( hasValue()){
        
        die = cur;
        next();
        delete die;

#ifdef DEBUG
        Serial.print("die ");
#endif

    }
    head = NULL;
    cur = NULL;
    tail = NULL;

#ifdef DEBUG
    Serial.println(" ");
#endif
}

void PerClickNoteList::checkIntegrity(char caller[])
{
    if(cur != NULL)
    {
        if(cur == cur->next)
        {
            Serial.print("PerClickNoteList next error called from ");
            Serial.println(caller);
        }
    }
}

void PerClickNoteList::append(note aNote, byte aTrack, unsigned long aDurationMS)
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

#ifdef DEBUG
    Serial.println("PerClickNoteList::append");
    Serial.print("  head: ");
    Serial.println((unsigned int)head);
    Serial.print("  cur:  ");
    Serial.println((unsigned int)cur);
    Serial.print("  tail: ");
    Serial.println((unsigned int)tail);
#endif

    checkIntegrity("append");
}

note PerClickNoteList::getNote()
{
    note retVal;

    if( cur != NULL )
        retVal = cur->clickNote;
        // really we should raise exception...
    return retVal; 
}

note PerClickNoteList::readNote()
{
    note retVal;

    if( readCur != NULL )
        retVal = readCur->clickNote;
//      memcpy(&retVal, &readCur->clickNote, sizeof(note));
    return retVal; 
}

byte PerClickNoteList::getTrack()
{
    byte retVal;

    if( cur != NULL )
        retVal = cur->track;

    return retVal; 
}

byte PerClickNoteList::readTrack()
{
    byte retVal;

    if( readCur != NULL )
        retVal = readCur->track;

    return retVal; 
}

bool PerClickNoteList::hasValue()
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

unsigned long PerClickNoteList::readDurationMS()
{
    unsigned long retVal;

    if( readCur != NULL )
        retVal = readCur->durationMS;
        // really we should raise exception...
    return retVal; 
}

void PerClickNoteList::rewind()
{
#ifdef DEBUG
    Serial.println("PerClickNoteList::rewind");
    Serial.print("  head: ");
    Serial.println((unsigned int)head);
    Serial.print("  cur:  ");
    Serial.println((unsigned int)cur);
    Serial.print("  tail: ");
    Serial.println((unsigned int)tail);
#endif

    cur = head;
}

void PerClickNoteList::next()
{
        checkIntegrity("next");
        if( cur != NULL )
                cur = cur->next;
}

bool PerClickNoteList::hasReadValue()
{
    return ( readCur != NULL ? true : false );
}

void PerClickNoteList::readRewind()
{
    readCur = head;
}

void PerClickNoteList::readNext()
{
    checkIntegrity("readNext");
    if( readCur != NULL )
            readCur = readCur->next;
}
