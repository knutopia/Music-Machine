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

#ifdef DEBUG
    Serial.print("Destructor PerClickNoteList ");
#endif

    notePerClick *die = head;

    while(die) 
    {
        head = die->next;
        delete die;
        die = head;

#ifdef DEBUG
        Serial.print("die ");
#endif
    }

    head = NULL;
    cur = NULL;
    tail = NULL;
    readCur = NULL;

#ifdef DEBUG
    Serial.println("done");
#endif
}

void PerClickNoteList::purge()
{
#ifdef DEBUG
    Serial.print("PerClickNoteList purge ");
#endif

    notePerClick *die = head;

    while(die) 
    {
        head = die->next;
        delete die;
        die = head;

#ifdef DEBUG
        Serial.print("die ");
#endif

    }

    head = NULL;
    cur = NULL;
    tail = NULL;
    readCur = NULL;

#ifdef DEBUG
    Serial.println("done");
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

void PerClickNoteList::print()
{
    Serial.print("PerClickNoteList print: ");
    while(hasValue()){
        Serial.println();
        note n = getNote();
        Serial.print("  pitch: ");
        Serial.print(n.pitchVal);
        Serial.print("  durMS: ");
        Serial.print(getDurationMS());
        Serial.print("  cur: ");
        Serial.print(getCur());
        Serial.print("  next: ");
        Serial.print(getNext());
        next();
    }    
    Serial.println();
}

void PerClickNoteList::append(note aNote, byte aTrack, unsigned long aDurationMS)
{
    notePerClick *n = new notePerClick();   // create new Node
//  notePerClick *n;
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

int PerClickNoteList::getCur()
{
    return (int)cur;
}

int PerClickNoteList::getNext()
{
    return (int)cur->next;
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

int PerClickNoteList::count()
{
    int count = 0;
    notePerClick *buf = cur;
    rewind();
    while(hasValue())
    {
        count++;
        next();
    }
    cur = buf;
    return count;
}