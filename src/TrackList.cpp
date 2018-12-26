#include "Enum.h"
#include "Track.h"
#include "InOutHelper.h"
#include "TrackList.h"

extern InOutHelper inout;


LinkedTrackList::LinkedTrackList()
{
    head = NULL; // set head to NULL
    cur = NULL;
    tail = NULL;
}

LinkedTrackList::~LinkedTrackList()
{
#ifdef DEBUG
    Serial.print("Destructor LinkedTrackList ");
#endif
    trackNode *die = head;

    while(die) 
    {
        head = die->next;
        delete die->trackRef;
        delete die;
        die = head;

#ifdef DEBUG
        Serial.print("die ");
#endif
    }

    head = NULL;
    cur = NULL;
    tail = NULL;

#ifdef DEBUG
    Serial.print("done ");
#endif
}

bool LinkedTrackList::checkIntegrity(const char caller[])
{
    bool retVal = true;
    if(cur != NULL)
    {
        if(cur == cur->next)
        {
            inout.ShowErrorOnLCD("LTL next Error", caller);
            Serial.print(" !!! LinkedTrackList next error called from ");
            Serial.println(caller);
            retVal = false;
        }

        if(tail != NULL && head != NULL)
        {
            if(tail->next == head)
            {
                inout.ShowErrorOnLCD("LTL tailN Error", caller);
                Serial.print(" !!! LinkedTrackList tail->next == head error called from ");
                Serial.println(caller);
                retVal = false;
            }
        }
    }
    return retVal;
}

Track* LinkedTrackList::getTrackRef(byte trackNum)
{
    Track* retVal = NULL;
    trackNode *curBup = cur;

    rewind();
    int sentry = 0;
    while(hasValue())
    {
        if(cur->trackNumber == trackNum)
        {
            retVal = cur->trackRef;
            break;
        }
        next();
        if(++sentry == 1000)
        {
            inout.ShowErrorOnLCD("LTL gTR stuck");
            break;
        }
    }
    cur = curBup;
    return retVal;
}

void LinkedTrackList::dropTrack(byte trackNum)
{
    trackNode *prevNode;

    rewind();
    int sentry = 0;
    while( hasValue()){
        if( cur->trackNumber == trackNum)
        {
            dropNode(prevNode);
        }
        prevNode = cur;
        next();
        if(++sentry == 1000)
        {
            inout.ShowErrorOnLCD("LTL dropT stuck");
            break;
        }
    }

    if(!checkIntegrity("dropTrack"))
        print();
}

void LinkedTrackList::dropNode(trackNode *prev)
{
    if (cur == head)
    {
        cur = head->next;
        delete head;
        head = cur;
    } else 
    { 
        if (cur == tail)
        {
            delete tail;
            tail = prev;
            prev->next = NULL;
        } else 
        {
            prev->next = cur->next;
            delete cur;
            cur = prev;
        }
    }
    if(!checkIntegrity("dropNode"))
        print();
}

// add value at the end -kg
void LinkedTrackList::appendTrack(byte aTrackNum, Track *aTrack)
{
    trackNode *n = new trackNode();   // create new Node
    n->trackNumber = aTrackNum;  // set value
    n->trackRef = aTrack;

    if(tail != NULL)
        tail->next = n; // point previously last node to new one

    tail = n;           // point tail at new node

    if(cur == NULL)
        cur = n;

    if(head == NULL)
        head = n;

    if(!checkIntegrity("appendTrack"))
        print();
}

void LinkedTrackList::rewind()
{
        cur = head;
}
void LinkedTrackList::next()
{
        if( cur != NULL )
                cur = cur->next;
}

Track* LinkedTrackList::getTrackRef()
{
        Track *retVal;

        if(!checkIntegrity("getTrackRef"))
            print();

        if( cur != NULL )
                retVal = cur->trackRef;
        else
            inout.ShowErrorOnLCD("LTL gTR NULL");

        return retVal; // really we should raise exception
}

byte LinkedTrackList::getTrackNumber()
{
        if(!checkIntegrity("getTrackNumber"))
            print();

        if( cur != NULL )
                return cur->trackNumber;
        return 0; // really we should raise exception
}

int LinkedTrackList::hasValue()
{
        return ( cur != NULL ? true : false );
}

int LinkedTrackList::count()
{
    int retVal = 0;
    rewind();
    int sentry = 0;
    while( hasValue()){
        retVal++;
        next();
        if(++sentry == 1000)
        {
            inout.ShowErrorOnLCD("TL count stuck");
            break;
        }
    }        
    return retVal;
}


void LinkedTrackList::print()
{
    trackNode *buf = cur;
    Serial.println("LinkedTrackList print: ");
        Serial.print("  cur: ");
        Serial.print((int)cur);
        Serial.print("  head: ");
        Serial.print((int)head);
        Serial.print("  tail: ");
        Serial.print((int)tail);
    rewind();
    int sentry = 0;
    while(hasValue()){
        Serial.println();
        Serial.print("  trackNumber: ");
        Serial.print(cur->trackNumber);
        Serial.print("  trackRef: ");
        Serial.print((int)cur->trackRef);
        Serial.print("  next: ");
        Serial.print((int)cur->next);

        if(!checkIntegrity("print"))
            break;
        next();

        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("LNL print stuck");
            break;
        }
    }    
    Serial.println();
    cur = buf;
}
