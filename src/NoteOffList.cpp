#include "NoteOffList.h"
#include "InOutHelper.h"

extern InOutHelper inout;
 
NoteOffList::NoteOffList()
{
    head = NULL; // set head to NULL
    cur = NULL;
    tail = NULL;
}

NoteOffList::~NoteOffList()
{
    Serial.print("Destructor NoteOffList ");

    volatile noteOffNode *die = head;

    while(die) 
    {
        head = die->next;
        
        die->next = NULL;
        die->prev = NULL;
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

bool NoteOffList::checkIntegrity(char caller[])
{
    bool retVal = true;
    if(cur != NULL)
    {
        if(cur == cur->next)
        {
            inout.ShowErrorOnLCD("NOL cI next err");
            Serial.print("NoteOffList next error called from ");
            Serial.println(caller);
            retVal = false;
        }
        if(cur == cur->prev)
        {
            inout.ShowErrorOnLCD("NOL cI prev err");
            Serial.println("NoteOffList prev loop !");
            Serial.print(caller);
            retVal = false;
        }
    }
    if(readCur != NULL)
    {
        if(readCur == readCur->next)
        {
            inout.ShowErrorOnLCD("NOL cI rC>next err");
            Serial.print("NoteOffList readCur next error called from ");
            Serial.println(caller);
            retVal = false;
        }
        if(readCur == readCur->prev)
        {
            inout.ShowErrorOnLCD("NOL cI rC>prev err");
            Serial.println("NoteOffList readCur prev loop !");
            Serial.print(caller);
            retVal = false;
        }
    }
}

void NoteOffList::printList()
{
    Serial.println("");
    Serial.println("NoteOffList: ");

    volatile noteOffNode *n = cur;
    Serial.print(" head: ");
    Serial.print((int)head);
    Serial.print(" cur: ");
    Serial.print((int)cur);
    Serial.print(" tail: ");
    Serial.print((int)tail);
    Serial.print(" readCur: ");
    Serial.println((int)readCur);

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

    if(!checkIntegrity("dropnode"))
        printList();
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
    if(!checkIntegrity("append"))
        printList();
}

void NoteOffList::rewind()
{
    if(!checkIntegrity("rewind"))
        printList();

    cur = head;
}

void NoteOffList::next()
{
    if(!checkIntegrity("next"))
        printList();

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
    if(!checkIntegrity("readRewind"))
        printList();

    readCur = head;
}

void NoteOffList::readNext()
{
    if(!checkIntegrity("readNext"))
        printList();

    if( readCur != NULL )
            readCur = readCur->next;
}

int NoteOffList::count()
{
    if(!checkIntegrity("count"))
        printList();

    int count = 0;
    volatile noteOffNode *buf = cur;
    rewind();
    int sentry = 0;
    while(hasValue())
    {
        count++;
        next();
        if(++sentry == 1000)
        {
            inout.ShowErrorOnLCD("NOL count stuck");
            break;
        }
    }
    cur = buf;
    return count;
}