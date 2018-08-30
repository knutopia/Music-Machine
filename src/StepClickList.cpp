#include "StepClickList.h"
#include "PerClickNoteList.h"
#include "InOutHelper.h"

//#define DEBUG true
#define LISTCOUNT true

extern StepClickList activeStepClicks;
extern InOutHelper inout;

StepClickList::StepClickList()
{
    head = NULL;
    cur = NULL;
    tail = NULL;
    readCur = NULL;

#ifdef DEBUG
    inout.ShowInfoOnLCD("StepClickList alive !");
    inout.SetLCDinfoTimeout();
    Serial.println("StepClickList alive !");
#endif
}

StepClickList::~StepClickList()
{
#ifdef DEBUG
    Serial.print("Destructor StepClickList ");
#endif

    stepClickNode *die = head;

    while(die) 
    {
        head = die->next;
        die->next = NULL;
        die->prev = NULL;
        delete die->notes;
        delete die;
        die = head;

#ifdef DEBUG
        Serial.print("die ");
#endif
    }
/*
    while( hasValue()){
        die = cur;
        next();
        die->next = NULL;
        die->prev = NULL;
        delete die->notes;
        delete die;
        die = NULL;

#ifdef DEBUG
        Serial.print("die ");
#endif
    }
*/
    head = NULL;
    cur = NULL;
    tail = NULL;
    readCur = NULL;

#ifdef DEBUG
    Serial.println("done");
#endif
}

void StepClickList::purge()
{
#ifdef DEBUG
    Serial.print("StepClickList purge");
#endif

    stepClickNode *die = head;

    while(die) 
    {
        head = die->next;
        die->next = NULL;
        die->prev = NULL;
        delete die->notes;
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

void StepClickList::checkIntegrity(char caller[])
{
    if(cur != NULL)
    {
        if(cur == cur->next)
        {
            Serial.print("StepClickList next error called from ");
            Serial.println(caller);
        }
        if(cur == cur->prev)
        {
            Serial.print("StepClickList prev error called from ");
            Serial.println(caller);
        }
    }
}

void StepClickList::addClickNote(note aNote, byte aTrack, unsigned long aDuration, int aMasterStep, int aClickStep)
{
    // 1) traverse the list to find the right master step
    // 2) find the right click
    // 3) if it's not there, add it
    // 4) insert the content
    //
    // Could use de-duping between notes on 
    // same track on same clickstep on monophonic tracks
    bool done = false;
#ifdef DEBUG
    Serial.print("addClickNote: aNote is ");
    Serial.println((unsigned int) &aNote);
#endif

    rewind();
    while( hasValue() && !done)
    {
#ifdef DEBUG        
        Serial.print("cur->masterStep ");
        Serial.print(cur->masterStep);
        Serial.print("  cur->clickStep ");
        Serial.print(cur->clickStep);
        Serial.print("  cur->prev ");
        Serial.println((int)cur->prev);
        Serial.print("aMasterStep ");
        Serial.print(aMasterStep);
        Serial.print("  clickStep ");
        Serial.println(aClickStep);
#endif

        if (cur->masterStep == aMasterStep) 
        {
            if (cur->clickStep == aClickStep)
            {
                // add new clickNoteNode
#ifdef DEBUG
                Serial.println("addClickNote: appending to existing node");
#endif
                if(cur->notes == NULL)
                {
                    inout.ShowErrorOnLCD("addCL notes NULL");
                    break;
                }

                cur->notes->append(aNote, aTrack, aDuration);
                done = true;
            } else
            {
                if (cur->clickStep > aClickStep) 
                {
                    // no matching stepClickNode yet, make it.
#ifdef DEBUG
                    Serial.println("addClickNote: inserting node");
#endif
                    insertBefore(cur->masterStep, aClickStep);
                    if(cur->notes == NULL)
                    {
                        inout.ShowErrorOnLCD("addCL2 notes NULL");
                        break;
                    }
                    cur->notes->append(aNote, aTrack, aDuration);
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
#ifdef DEBUG
            Serial.println("addClickNote: inserting node at the start");
#endif
            rewind();
            insertBefore(aMasterStep, aClickStep);
            if(cur->notes == NULL)
                inout.ShowErrorOnLCD("addCL3 notes NULL");
            else
                cur->notes->append(aNote, aTrack, aDuration);
        } else 
        { // add stepClickNode at the end
#ifdef DEBUG
            Serial.println("addClickNote: inserting node at the end");
#endif
            append(aMasterStep, aClickStep);
            cur = tail;
            if(cur->notes == NULL)
                inout.ShowErrorOnLCD("addCL4 notes NULL");
            else
                cur->notes->append(aNote, aTrack, aDuration);
        }
    }
    checkIntegrity("addClickNote");
}

void StepClickList::append(int aMasterStep, byte aClickStep)
{
    stepClickNode *n = new stepClickNode();
//  stepClickNode *n;
    n->masterStep = aMasterStep;
    n->clickStep = aClickStep;
    n->notes = new PerClickNoteList();  
    
    noInterrupts();
        if(tail != NULL)
            tail->next = n; // point previously last node to new one
        tail = n;           // point tail at new node

        if(cur == NULL)
            cur = n;

        if(head == NULL)
            head = n;
    interrupts();
    checkIntegrity("append");
}


void StepClickList::insertBefore(int aMasterStep, byte aClickStep)
{
    stepClickNode *n = new stepClickNode();
    n->masterStep = aMasterStep;
    n->clickStep = aClickStep;
    n->notes = new PerClickNoteList();

    n->next = cur;

    if (tail == NULL)
    {
        noInterrupts();
            tail = cur;
        interrupts();
    }

    if (cur == NULL)
    {
        noInterrupts();
            cur = head;
        interrupts();
    }
    
    if (cur == NULL)
    {
        noInterrupts();
            cur = n;
            head = n;
        interrupts();
    } else
    {
        if(cur == head)
        {
            n->next = head;
            noInterrupts();
                head->prev = n;
                head = n;
                cur = n;
            interrupts();
        } else
        {
            if(cur->prev != NULL)
            {
                stepClickNode *prev = cur->prev;
                prev->next = n;
            } 
            noInterrupts();
                cur->prev = n;
                cur = n;
            interrupts();
        }
    }
    checkIntegrity("insertBefore");
}


int StepClickList::getMasterStep()
{
    int retVal;

    checkIntegrity("getMasterStep");

    if( cur != NULL )
        retVal = cur->masterStep;
    else
        inout.ShowErrorOnLCD("SCL getMS NULL");
    return retVal; 
}

PerClickNoteList StepClickList::getClickNoteList(byte a_click, int a_step)
{
    PerClickNoteList retVal;

    checkIntegrity("getClickNoteList");

    bool found = false;

    readRewind();
    int sentry = 0;
    while(hasReadValue())
    {
        if(readCur->masterStep == a_step
            && a_click == readCur->clickStep)
        {
#ifdef DEBUG
            Serial.print("Matching ");
            Serial.println(a_step);
#endif
            if(readCur->notes == NULL)
            {
                inout.ShowErrorOnLCD("getCNL notes NULL");
                break;
            }
            readCur->notes->rewind();
            retVal = *readCur->notes;
            found = true;
            break;
        }
        readNext();
        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("getCNList stuck1");
            break;
        }

    }
    if(!found) {
#ifdef DEBUG
        Serial.print("nf");
        Serial.print(g_activeGlobalStep);
        Serial.print(",");
        Serial.print(a_click);
        Serial.print(" ");
#endif
//      retVal = NULL;
    } else {
#ifdef DEBUG
        Serial.print("&&& getClickNoteList: FOUND, ");
        Serial.print(g_activeGlobalStep);
        Serial.print(", ");
        Serial.println(a_click);
#endif
    }
    return retVal;
}

bool StepClickList::getClickNoteList(PerClickNoteList *target, byte a_click, int a_step)
{
    checkIntegrity("getClickNoteList");

    bool found = false;

    readRewind();
    int sentryO = 0;
    while(hasReadValue())
    {
        if(readCur->masterStep == a_step
            && a_click == readCur->clickStep)
        {
#ifdef DEBUG
            Serial.print("Matching ");
            Serial.println(a_step);
#endif
            if(readCur->notes == NULL)
            {
                inout.ShowErrorOnLCD("getCNL notes NULL");
                break;
            }
            readCur->notes->rewind();
            int sentryI = 0;
            while(readCur->notes->hasValue())
            {
//              Serial.print("Looping ");

                target->append(readCur->notes->getNote(),
                              readCur->notes->getTrack(),
                              readCur->notes->getDurationMS());
//              Serial.print(readCur->notes->getNote().pitchVal);
//              Serial.print(" to ");
//              Serial.println(target->getNote().pitchVal);
                readCur->notes->next();
                target->next();

                if(++sentryI == 100)
                {
                    inout.ShowErrorOnLCD("getCNList stuckI");
                    break;
                }
            }

            found = true;
            break;
        }
        readNext();

        if(++sentryO == 100)
        {
            inout.ShowErrorOnLCD("getCNList stuckO");
            break;
        }

    }
    return found;
}

bool StepClickList::transferClickNoteList(PerClickNoteList& target, byte a_click, int a_step)
{
    checkIntegrity("transferClickNoteList");

    bool found = false;

    readRewind();
    int sentryO = 0;
    while(hasReadValue())
    {
        if(readCur->masterStep == a_step
            && a_click == readCur->clickStep)
        {
#ifdef DEBUG
            Serial.print("Matching ");
            Serial.println(a_step);
#endif
            if(readCur->notes == NULL)
            {
                inout.ShowErrorOnLCD("tfrCNL notes NULL");
                break;
            }
            readCur->notes->rewind();
            int sentry = 0;
            while(readCur->notes->hasValue())
            {
//              Serial.print("Looping ");

                target.append(readCur->notes->getNote(),
                              readCur->notes->getTrack(),
                              readCur->notes->getDurationMS());
//              Serial.print(readCur->notes->getNote().pitchVal);
//              Serial.print(" to ");
//              Serial.println(target->getNote().pitchVal);
                readCur->notes->next();
                target.next();

                if(++sentry == 100)
                {
                    inout.ShowErrorOnLCD("traCNLi stuck");
                    break;
                }
            }
            dropReadCur();
            rewind();
            readRewind();

            found = true;
            break;
        }
        readNext();

        if(++sentryO == 100)
        {
            inout.ShowErrorOnLCD("traCNLo stuck");
            break;
        }
    }
    return found;
}

/*

bool StepClickList::transferClickNoteList(PerClickNoteList *target, byte a_click, int a_step)
{
    checkIntegrity("getClickNoteList");

    bool found = false;

    readRewind();
    while(hasReadValue())
    {
        if(readCur->masterStep == a_step
            && a_click == readCur->clickStep)
        {
#ifdef DEBUG
            Serial.print("Matching ");
            Serial.println(a_step);
#endif
            readCur->notes->rewind();
            while(readCur->notes->hasValue())
            {
//              Serial.print("Looping ");

                target->append(readCur->notes->getNote(),
                              readCur->notes->getTrack(),
                              readCur->notes->getDurationMS());
//              Serial.print(readCur->notes->getNote().pitchVal);
//              Serial.print(" to ");
//              Serial.println(target->getNote().pitchVal);
                readCur->notes->next();
                target->next();
            }
            dropReadCur();
            rewind();
            readRewind();

            found = true;
            break;
        }
        readNext();
    }
    return found;
}
*/

byte StepClickList::getClickStep()
{
    byte retVal;

    checkIntegrity("getClickStep");

    if( cur != NULL )
        retVal = cur->clickStep;
    else
        inout.ShowErrorOnLCD("SCL getCS NULL");
    return retVal; 
}

PerClickNoteList* StepClickList::getNotes()
{
    PerClickNoteList *retVal;

    checkIntegrity("getNotes");

    if( cur != NULL )
        retVal = cur->notes;
    else
        inout.ShowErrorOnLCD("SCL getNotes NULL");

    if(cur->notes == NULL)
        inout.ShowErrorOnLCD("SCL getNotesN NULL");

    return retVal; 
}

void StepClickList::dropReadCur()
{
    if( readCur != NULL)
    {
        if( readCur == head)
        {
            head = readCur->next;
        }

        if( readCur == tail)
        {
            tail = readCur->prev;
        }

        delete readCur->notes;
        delete readCur;
        readCur = NULL;
    }
    checkIntegrity("dropReadCur");
}

void StepClickList::dropNotesBeforeStepAndRewind(int aStep)
{
#ifdef DEBUG    
    Serial.print("dropNotesBeforeStepAndRewind before ");
    Serial.println(aStep);
#endif

    bool b = true;
    while(b)
    {
        if (head == NULL)
        {
#ifdef DEBUG
            Serial.print("head == NULL before ");
            Serial.println(aStep);
#endif
            b = false;
        } else
        {
            if (head->masterStep == NULL)
            {
#ifdef DEBUG
                    Serial.print("head->masterStep == NULL before ");
                    Serial.println(aStep);
#endif
                    b = false;
            } else
            {
                if (head->masterStep < aStep)
                {
#ifdef DEBUG
                    Serial.print("dropping before ");
                    Serial.println(aStep);
#endif
                    dropHead();
#ifdef DEBUG
                    Serial.print(", dropped head !");
#endif
                } else
                {
#ifdef DEBUG
                    Serial.print("head->masterStep NOT < aStep: ");
                    Serial.println(aStep);
                    Serial.print("  head->masterStep: ");
                    Serial.println(head->masterStep);
#endif
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

    checkIntegrity("dropNotesBeforeStepAndRewind");
}

void StepClickList::dropHead()
{
    if (head != NULL)
    {
        stepClickNode *newHead = head->next;
        if(head->next == NULL)
            Serial.print("dropHead head->next == NULL");

        noInterrupts();
            if (cur == head)
                cur = newHead;

            Serial.print("  @#$ dropHead deleting head->notes");
            if(head->notes != NULL)
                delete head->notes;
            else
                inout.ShowErrorOnLCD("SCL dh notes NULL");
            Serial.print("  @#$ dropHead head->notes deleted");
            delete head;
            Serial.print("  @#$ dropHead head deleted");
            head = newHead;
        interrupts();
    } 
//  else
//      Serial.print("@#$ head == NULL");

    Serial.print("  @#$before checkIntegrity ");

    checkIntegrity("dropHead");

    Serial.println("  @#$ dropHead done");
}

void StepClickList::rewind()
{
        cur = head;
        
        checkIntegrity("rewind");
}

volatile void StepClickList::readRewind()
{
        readCur = head;
        
        checkIntegrity("rewind");
}

void StepClickList::next()
{
        checkIntegrity("next");
        
        if( cur != NULL )
                cur = cur->next;
}

volatile void StepClickList::readNext()
{
        checkIntegrity("next");
        
        if( readCur != NULL )
                readCur = readCur->next;
}

int StepClickList::hasValue()
{
        return ( cur != NULL ? true : false );
}

volatile int StepClickList::hasReadValue()
{
        return ( readCur != NULL ? true : false );
}

int StepClickList::count()
{
    int count = 0;
    stepClickNode *buf = cur;
    rewind();
    int sentry = 0;
    while(hasValue())
    {
        count++;
        next();
        if(++sentry == 1000)
        {
            inout.ShowErrorOnLCD("SCL count stuck");
            break;
        }
    }
    cur = buf;
    return count;
}