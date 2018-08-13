#include "NoteOffList.h"
 
NoteOffList::NoteOffList()
{
    head = NULL; // set head to NULL
    cur = NULL;
    tail = NULL;
}

NoteOffList::~NoteOffList()
{
    volatile noteOffNode *die;

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

void NoteOffList::checkIntegrity(char caller[])
{
    if(cur != NULL)
    {
        if(cur == cur->next)
        {
            Serial.print("NoteOffList next error called from ");
            Serial.println(caller);
        }
        if(cur == cur->prev)
        {
            Serial.println("NoteOffList prev loop !");
            Serial.print(caller);
        }
    }
}

void NoteOffList::printList()
{
    Serial.println("");
    Serial.println("NoteOffList: ");

    volatile noteOffNode *n = cur;

    rewind();
    while( hasValue())
    {
        Serial.print(cur->noteOffTime);
        Serial.print(" ");
        Serial.print(cur->trackNumber);
        Serial.print(" ");
        Serial.print(cur->midiNote);
        Serial.print(" ");
        Serial.print((int)cur);
        Serial.print(" ");
        Serial.println((int)cur->next);
        if(cur==cur->next)
        {
            Serial.println("NoteOffList LOOP REFERENCE");
            break;
        }
        next();
    }
    Serial.println("");
    cur = n;
}

void NoteOffList::dropNode()
{
    if (cur == head)
    {
//          Serial.println("dropNode 1");
        cur = head->next;
        if(cur != NULL)
            cur->prev = NULL;
        else
        {
//              Serial.println("NoteOffList tail nulled");
            tail = NULL;
        }
        delete head;
        head = cur;
    } else 
    { 
        if (cur == tail)
        {
//              Serial.println("dropNode 2");
            volatile noteOffNode *prevNode = tail->prev;
            cur = prevNode;
            if(cur != NULL)
                cur->next = NULL;
            delete tail;
            tail = cur;
        } else 
        {
//              Serial.println("dropNode 3");
            volatile noteOffNode *prevNode = cur->prev;
            prevNode->next = cur->next;
            cur->next->prev = cur->prev;
            delete cur;
            cur = prevNode;
        }
    }
#ifdef DEBUG
    Serial.println("dropNode done");
    printList();
#endif
    checkIntegrity("dropnode");
}

// add value at the end -kg
void NoteOffList::append(byte aTrackNum, byte aMidiNote, unsigned long anOffTime)
{
    volatile noteOffNode *n = new noteOffNode();
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
#ifdef DEBUG
    Serial.println("Append done");
    printList();
#endif
    checkIntegrity("append");
}

void NoteOffList::rewind()
{
        cur = head;
}

void NoteOffList::next()
{
        checkIntegrity("next");
        if( cur != NULL )
                cur = cur->next;
}

byte NoteOffList::readTrack()
{
    return readCur->trackNumber;
}

byte NoteOffList::readMidiNote()
{
    return readCur->midiNote;
}

unsigned long NoteOffList::readNoteOffTime()
{
    return readCur->noteOffTime;
}

int NoteOffList::hasValue()
{
        return ( cur != NULL ? true : false );
}

int NoteOffList::hasReadValue()
{
        return ( readCur != NULL ? true : false );
}

void NoteOffList::readRewind()
{
        readCur = head;
}

void NoteOffList::readNext()
{
        checkIntegrity("next");
        if( readCur != NULL )
                readCur = cur->next;
}

int NoteOffList::count()
{
    int retVal = 0;
    rewind();

    while( hasValue()){
        retVal++;
        checkIntegrity("count");
        next();
    }        
    return retVal;
}