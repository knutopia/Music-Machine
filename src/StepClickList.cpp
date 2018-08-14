#include "StepClickList.h"
#include "PerClickNoteList.h"
#include "InOutHelper.h"

//#define DEBUG true

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
    stepClickNode *die;

#ifdef DEBUG
    Serial.print("Destructor StepClickList ");
#endif

    while( hasValue()){
        die = cur;
        next();
        delete die->notes;
        delete die;

        Serial.print("die ");
    }
    Serial.println();
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
        if (cur->masterStep == aMasterStep) 
        {
            if (cur->clickStep == aClickStep)
            {
                // add new clickNoteNode
#ifdef DEBUG
                Serial.println("addClickNote: appending to existing node");
#endif
                cur->notes->append(aNote, aTrack, aDuration);
//                cur->notes->rewind();
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
                    cur->notes->append(aNote, aTrack, aDuration);
//                    cur->notes->rewind();
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
            cur->notes->append(aNote, aTrack, aDuration);
        } else 
        { // add stepClickNode at the end
#ifdef DEBUG
            Serial.println("addClickNote: inserting node at the end");
#endif
            append(aMasterStep, aClickStep);
            cur = tail;
            cur->notes->append(aNote, aTrack, aDuration);
        }
    }
    checkIntegrity("addClickNote");
}

void StepClickList::append(int aMasterStep, byte aClickStep)
{
    stepClickNode *n = new stepClickNode();
    n->masterStep = aMasterStep;
    n->clickStep = aClickStep;
    n->notes = new PerClickNoteList();
    
        //noInterrupts();
        if(tail != NULL)
            tail->next = n; // point previously last node to new one
        tail = n;           // point tail at new node

        if(cur == NULL)
            cur = n;

        if(head == NULL)
            head = n;
        //interrupts();
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
            //noInterrupts();
            tail = cur;
            //interrupts();
    }
    if (cur == NULL)
    {
            //noInterrupts();
            cur = head;
            //interrupts();
    }

    if (cur == NULL)
    {
            //noInterrupts();
            cur = n;
            head = n;
            //interrupts();
    } else
    {
        if(cur == head)
        {
            n->next = head;
                //noInterrupts();
                head->prev = n;
                head = n;
                cur = n;
                //interrupts();
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
    checkIntegrity("insertBefore");
}

int StepClickList::getMasterStep()
{
    int retVal;

    checkIntegrity("getMasterStep");

    if( cur != NULL )
        retVal = cur->masterStep;
        // really we should raise exception...
    return retVal; 
}

PerClickNoteList* StepClickList::getClickNoteList(byte a_click, int a_step)
{
    PerClickNoteList *retVal;

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
            retVal = readCur->notes;
            found = true;
            break;
        }
        readNext();
    }
    if(!found) {
#ifdef DEBUG
        Serial.print("nf");
        Serial.print(g_activeGlobalStep);
        Serial.print(",");
        Serial.print(a_click);
        Serial.print(" ");
#endif
        retVal = NULL;
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

byte StepClickList::getClickStep()
{
    byte retVal;

    checkIntegrity("getClickStep");

    if( cur != NULL )
        retVal = cur->clickStep;
        // really we should raise exception...
    return retVal; 
}

PerClickNoteList* StepClickList::getNotes()
{
    PerClickNoteList *retVal;

    checkIntegrity("getNotes");

    if( cur != NULL )
        retVal = cur->notes;
        // really we should raise exception...
    return retVal; 
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
            Serial.print("head == NULL before ");
            Serial.println(aStep);
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

            //noInterrupts();
            if (cur == head)
                cur = newHead;

    //      Serial.print("  @#$ dropHead deleting head->notes");
            delete head->notes;
    //      Serial.print("  @#$ dropHead head->notes deleted");
            delete head;
    //      Serial.print("  @#$ dropHead head deleted");
            head = newHead;
            //interrupts();
    } 
//  else
//      Serial.print("@#$ head == NULL");

    checkIntegrity("dropHead");

//  Serial.print("  @#$ dropHead done");
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