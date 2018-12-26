#include "StepClickList.h"
#include "PerClickNoteList.h"
#include "InOutHelper.h"
#include "NotePerClick.h"

//#define DEBUG true
#define LISTCOUNT true

extern StepClickList activeStepClicks;
extern InOutHelper inout;
extern volatile notePerClick notesToPlay[];

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
        if(die->notes != NULL)
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

void StepClickList::begin(CbWhenStuck panicCbPointer)
{
    PanicCb = panicCbPointer;
}    

void StepClickList::purge()
{
#ifdef DEBUG
    Serial.print("StepClickList purge");
    print();
#endif

    stepClickNode *die = head;

    int sentry = 0;
    while(die) 
    {
        head = die->next;
        die->next = NULL;
        die->prev = NULL;
        if(die->notes != NULL)
            delete die->notes;
        delete die;
        die = head;

#ifdef DEBUG
        Serial.print("die ");
#endif
        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("SCL purge stuck");
            PanicCb();
            break;
        }
    }

    head = NULL;
    cur = NULL;
    tail = NULL;
    readCur = NULL;

#ifdef DEBUG
    Serial.println("done");
#endif
}

bool StepClickList::checkIntegrity(const char caller[])
{
    bool retVal = true;
    if(cur != NULL)
    {
        if(cur == cur->next)
        {
            inout.ShowErrorOnLCD("SCL next Error", caller);
            Serial.print(" !!! StepClickList next error called from ");
            Serial.println(caller);
            retVal = false;
        }
        if(cur == cur->prev)
        {
            inout.ShowErrorOnLCD("SCL prev Error", caller);
            Serial.print(" !!! StepClickList prev error called from ");
            Serial.println(caller);
            retVal = false;
        }
        if(cur->masterStep > 10000000)
        {
            inout.ShowErrorOnLCD("SCL masterStep Error", caller);
            Serial.print(" !!! StepClickList masterStep error called from ");
            Serial.println(caller);

            Serial.print("  cur: ");
            Serial.print((int)cur);
            Serial.print("  masterStep: ");
            Serial.print(cur->masterStep);

            retVal = false;
        }
        if(cur->masterStep == -1)
        {
            inout.ShowErrorOnLCD("SCL masterStep -1 Er", caller);
            Serial.print(" !!! StepClickList masterStep -1 error called from ");
            Serial.println(caller);

            Serial.print("  cur: ");
            Serial.print((int)cur);
            Serial.print("  masterStep: ");
            Serial.print(cur->masterStep);

            retVal = false;
        }
        if(tail != NULL && head != NULL)
        {
            if(tail->next == head)
            {
                inout.ShowErrorOnLCD("SCL tailN Error", caller);
                Serial.print(" !!! StepClickList tail->next == head error called from ");
                Serial.println(caller);

                retVal = false;
            }
            if(head->prev == tail)
            {
                inout.ShowErrorOnLCD("SCL tailP Error", caller);
                Serial.print(" !!! StepClickList head->prev == tail error called from ");
                Serial.println(caller);

                retVal = false;
            }
        }
    }
    return retVal;
}

void StepClickList::addClickNote(const note aNote, byte aTrack, unsigned long aDuration, int aMasterStep, int aClickStep)
{
    if(!checkIntegrity("addClickNote start"))
        print();

    // 1) traverse the list to find the right master step
    // 2) find the right click
    // 3) if it's not there, add it
    // 4) insert the content
    //
    // Could use de-duping between notes on 
    // same track on same clickstep on monophonic tracks
    bool done = false;
#ifdef DEBUG
    Serial.println("");
    Serial.print("addClickNote: aNote is ");
    Serial.print((unsigned int) &aNote);
    Serial.print(" with masterStep ");
    Serial.println(aMasterStep);
    print();
#endif

    rewind();
    int sentry = 0;
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
        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("AddCN count stuck");
            PanicCb();
            break;
        }
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
            print();
#endif
            append(aMasterStep, aClickStep);
            cur = tail;
            if(cur->notes == NULL)
                inout.ShowErrorOnLCD("addCL4 notes NULL");
            else
                cur->notes->append(aNote, aTrack, aDuration);
        }
    }
    if(!checkIntegrity("addClickNote end"))
        print();
}

void StepClickList::append(int aMasterStep, byte aClickStep)
{
#ifdef DEBUG
    Serial.print("append: ");
    print();
#endif

    stepClickNode *n = new stepClickNode();
//  stepClickNode *n;
    n->masterStep = aMasterStep;
    n->clickStep = aClickStep;
    n->notes = new PerClickNoteList();  
    //  noInterrupts();
        if(tail != NULL)
            tail->next = n; // point previously last node to new one

        n->prev = tail; // point new node at previously last one
        n->next = NULL; // last one, so no next... to avoid it being undefined

        tail = n;           // point tail at new node

        if(cur == NULL)
            cur = n;

        if(head == NULL)
            head = n;

    //  interrupts();
    if(!checkIntegrity("append"))
        print();
}


void StepClickList::insertBefore(int aMasterStep, byte aClickStep)
{
#ifdef DEBUG
            Serial.println("insertBefore");
            print();
#endif
    stepClickNode *n = new stepClickNode();
    n->masterStep = aMasterStep;
    n->clickStep = aClickStep;
    n->notes = new PerClickNoteList();

    n->next = cur;

    if (tail == NULL)
    {
//      noInterrupts();
            tail = cur;
//      interrupts();
    }

    if (cur == NULL)
    {
//      noInterrupts();
            cur = head;
//      interrupts();
    }
    
    if (cur == NULL)
    {
//      noInterrupts();
            cur = n;
            head = n;
//      interrupts();
    } else
    {
        if(cur == head)
        {
            n->next = head;
//          noInterrupts();
                head->prev = n;
                head = n;
                cur = n;
//          interrupts();
        } else
        {
            if(cur->prev != NULL)
            {
                stepClickNode *prev = cur->prev;
                prev->next = n;
            } 
//          noInterrupts();
                cur->prev = n;
                cur = n;
//          interrupts();
        }
    }
    if(!checkIntegrity("insertBefore"))
        print();
}


int StepClickList::getMasterStep()
{
    int retVal = -1;

    bool integrity = checkIntegrity("getMasterStep");

    if( cur != NULL )
        retVal = cur->masterStep;
    else
        inout.ShowErrorOnLCD("SCL getMS NULL");
    return retVal; 
}

PerClickNoteList StepClickList::getClickNoteList(byte a_click, int a_step)
{
    PerClickNoteList retVal;

    bool integrity = checkIntegrity("getClickNoteList");

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
    bool integrity = checkIntegrity("getClickNoteList");

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
    bool integrity = checkIntegrity("transferClickNoteList");

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

bool StepClickList::transferClickNoteArray(byte a_click, int a_step)
{
    bool integrity = checkIntegrity("transferClickNoteArray");

    // clear out notes to play first...
    for(int i = 0; i < TRACKCOUNT; i++)
        notesToPlay[i].active = false;

    bool found = false;

    readRewind();
    int sentryO = 0;

    // ...then look for step & click match...
    while(hasReadValue())
    {
        if(readCur->masterStep == a_step
            && a_click == readCur->clickStep)
        {
#ifdef DEBUG
            Serial.print("Matching ");
            Serial.println(a_step);
#endif
            // ...and refill if there are notes queued
            if(readCur->notes == NULL)
            {
                inout.ShowErrorOnLCD("tfrCNA notes NULL");
                break;
            }

            readCur->notes->rewind();
            for(int i = 0; i < TRACKCOUNT; i++)
            {
                if(readCur->notes->hasValue())
                {
//                  noInterrupts();
                        notesToPlay[i].clickNote.retrigClickDivider = readCur->notes->getNote().retrigClickDivider;
                        notesToPlay[i].clickNote.unmuted = readCur->notes->getNote().unmuted;
                        notesToPlay[i].clickNote.playIt = readCur->notes->getNote().playIt;
                        notesToPlay[i].clickNote.pitchVal = readCur->notes->getNote().pitchVal;
                        notesToPlay[i].clickNote.pitchFreq = readCur->notes->getNote().pitchFreq;
                        notesToPlay[i].clickNote.durationMS = readCur->notes->getNote().durationMS;
                        notesToPlay[i].clickNote.hold = readCur->notes->getNote().hold;
                        notesToPlay[i].clickNote.duration = readCur->notes->getNote().duration;
                        notesToPlay[i].clickNote.retrigs = readCur->notes->getNote().retrigs;
                        notesToPlay[i].clickNote.ticks = readCur->notes->getNote().ticks;
                        notesToPlay[i].clickNote.accent = readCur->notes->getNote().accent;
                        notesToPlay[i].clickNote.velocity = readCur->notes->getNote().velocity;
                        notesToPlay[i].clickNote.swingTicks = readCur->notes->getNote().swingTicks;
                        notesToPlay[i].clickNote.holdsAfter = readCur->notes->getNote().holdsAfter;
                        notesToPlay[i].clickNote.mutesAfter = readCur->notes->getNote().mutesAfter;

                        notesToPlay[i].durationMS = readCur->notes->getDurationMS();
                        notesToPlay[i].track = readCur->notes->getTrack();
                        notesToPlay[i].active = true;
//                  interrupts();
                    readCur->notes->next();
                } else {
//                  noInterrupts();
                        notesToPlay[i].active = false;
//                  interrupts();
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
            inout.ShowErrorOnLCD("traCNAo stuck");
            print();
            found = false;
            PanicCb();
            break;
        }
    }
    integrity = checkIntegrity("transferClickNoteArray end");
    return found;
}

byte StepClickList::getClickStep()
{
    byte retVal;

    bool integrity = checkIntegrity("getClickStep");

    if( cur != NULL )
        retVal = cur->clickStep;
    else
        inout.ShowErrorOnLCD("SCL getCS NULL");
    return retVal; 
}

PerClickNoteList* StepClickList::getNotes()
{
    PerClickNoteList *retVal;

    bool integrity = checkIntegrity("getNotes");

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
#ifdef DEBUG    
    Serial.println("dropReadCur ");
    print();
#endif

    if( readCur != NULL)
    {
        if( readCur == head)
        {
            head = readCur->next;
            if(head != NULL)
                head->prev = NULL;
        }
        if( readCur == tail)
        {
            tail = readCur->prev;
            if(tail != NULL)
                tail->next = NULL;
        }
        if(readCur->prev != NULL)
            readCur->prev->next = readCur->next;
        if(readCur->next != NULL)
            readCur->next->prev = readCur->prev;

        if(cur == readCur)
            cur = head;

        delete readCur->notes;
        delete readCur;
        readCur = NULL;
    }
#ifdef DEBUG    
    Serial.print("after dropReadCur: ");
    print();
#endif
    if(!checkIntegrity("dropReadCur"))
        print();
}

void StepClickList::dropNotesBeforeStepAndRewind(int aStep)
{
#ifdef DEBUG    
    Serial.print("dropNotesBeforeStepAndRewind before ");
    Serial.println(aStep);
    print();
#endif

    bool b = true;
    int sentry = 0;
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
        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("DNBSAR count stuck");
            break;
        }
    }
    cur = head;

    if(!checkIntegrity("dropNotesBeforeStepAndRewind"))
        print();
}

void StepClickList::dropHead()
{
#ifdef DEBUG    
    Serial.println("dropHead");
#endif

    if (head != NULL)
    {
        stepClickNode *newHead = head->next;

        if (head == tail)
            tail = NULL;

        if(newHead != NULL)
            newHead->prev = NULL;

        if (cur == head)
            cur = newHead;

//          Serial.print("  @#$ dropHead deleting head->notes");
        if(head->notes != NULL)
        {
//          noInterrupts();
                delete head->notes;
//          interrupts();
        }
        else
            inout.ShowErrorOnLCD("SCL dh notes NULL");
//          Serial.print("  @#$ dropHead head->notes deleted");

        delete head;
//          Serial.print("  @#$ dropHead head deleted");
        head = newHead;
    } 
    if(!checkIntegrity("dropHead"))
        print();
}

void StepClickList::rewind()
{
    cur = head;
    
//  bool integrity = checkIntegrity("rewind");
}

volatile void StepClickList::readRewind()
{
    readCur = head;
    
//  if(!checkIntegrity("readRewind"))
//      print();
}

void StepClickList::next()
{
//  bool integrity = checkIntegrity("next");

    if( cur != NULL )
        cur = cur->next;
}

volatile void StepClickList::readNext()
{
    bool integrity = checkIntegrity("readNext");

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
        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("SCL count stuck");
            break;
        }
    }
    cur = buf;
    return count;
}

void StepClickList::print()
{
    stepClickNode *buf = cur;
    Serial.println("StepClickList print: ");
        Serial.print("  cur: ");
        Serial.print((int)cur);
        Serial.print("  head: ");
        Serial.print((int)head);
        Serial.print("  tail: ");
        Serial.print((int)tail);
        Serial.print("  readCur: ");
        Serial.print((int)readCur);
    rewind();
    int sentry = 0;
    while(hasValue()){
        Serial.println();
        Serial.print("  masterStep: ");
        Serial.print(cur->masterStep);
        Serial.print("  clickStep: ");
        Serial.print(cur->clickStep);
        Serial.print("  notes: ");
        Serial.print((int)cur->notes);
        Serial.print("  curnode: ");
        Serial.print((int)cur);
        Serial.print("  prev: ");
        Serial.print((int)cur->prev);
        Serial.print("  next: ");
        Serial.print((int)cur->next);

        if(!checkIntegrity("print"))
            break;
        next();

        if(++sentry == 100)
        {
            inout.ShowErrorOnLCD("SCL print stuck");
            break;
        }
    }    
    Serial.println();
    cur = buf;
}
