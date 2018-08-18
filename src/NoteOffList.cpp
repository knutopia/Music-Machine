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
        die->prev = NULL;
        die->next = NULL;
        delete die;
        die = NULL;
        
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
        Serial.print(" tim:");
        Serial.print(cur->noteOffTime);
        Serial.print(" tr:");
        Serial.print(cur->trackNumber);
        Serial.print(" mi:");
        Serial.print(cur->midiNote);
        Serial.print(" cur:");
        Serial.print((int)cur);
        Serial.print(" next:");
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
    if( readCur != NULL)
    {
//      noInterrupts();
            cur = readCur;
//      interrupts();
    }

    if (cur == head)
    {
//      Serial.println("dropNode 1");
//      noInterrupts();
            cur = head->next;
            if(cur != NULL)
                cur->prev = NULL;
            else
            {
                tail = NULL;
            }
            head->next = NULL;
            delete head;
            head = cur;
//      interrupts();
    } else 
    { 
        if (cur == tail)
        {
//          Serial.println("dropNode 2");
//          noInterrupts();
                if( tail->prev != NULL)
                {
                    cur = tail->prev;
                    cur->next = NULL;
                }
/*
                volatile noteOffNode *prevNode = tail->prev;
                if( prevNode != NULL)
                {
                    cur = prevNode;
                    cur->next = NULL;
                }
                // alternative to...
                cur = prevNode;
                if(cur != NULL)
                    cur->next = NULL;
*/
                delete tail;
                tail = cur;
//          interrupts();
        } else 
        {
//          Serial.println("dropNode 3");
            volatile noteOffNode *prevNode;
//          noInterrupts();
                prevNode = cur->prev;
                prevNode->next = cur->next;
                cur->next->prev = cur->prev;
                delete cur;
                cur = prevNode;
//          interrupts();
        }
    }
    readCur = cur;
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
                readCur = readCur->next;
}

int NoteOffList::count()
{

    int count = 0;
    volatile noteOffNode *buf = cur;
    rewind();
    while(hasValue())
    {
        count++;
        next();
    }
    cur = buf;
    return count;

/*
    int retVal = 0;
    rewind();

    while( hasValue()){
        retVal++;
        checkIntegrity("count");
        next();
    }        
    return retVal;
*/
}