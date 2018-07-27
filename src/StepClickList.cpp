#include "StepClickList.h"
#include "PerClickNoteList.h"
#include "InOutHelper.h"

extern StepClickList activeStepClicks;
extern InOutHelper inout;

StepClickList::StepClickList()
{
    head = NULL;
    cur = NULL;
    tail = NULL;

    inout.ShowInfoOnLCD("StepClickList alive !");
    inout.SetLCDinfoTimeout();
    Serial.println("StepClickList alive !");

}

StepClickList::~StepClickList()
{
    stepClickNode *die;

    Serial.print("Destructor StepClickList ");

    while( hasValue()){
        die = cur;
        next();
        delete die->notes;
        delete die;

        Serial.print("die ");
    }
    Serial.println();
}

void StepClickList::addClickNote(note *aNote, byte aTrack, unsigned long aDuration, int aMasterStep, int aClickStep)
{
    // 1) traverse the list to find the right master step
    // 2) find the right click
    // 3) if it's not there, add it
    // 4) insert the content
    //
    // Could use de-duping between notes on 
    // same track on same clickstep on monophonic tracks
    bool done = false;

    rewind();
    while( hasValue() && !done)
    {
        if (cur->masterStep == aMasterStep) 
        {
            if (cur->clickStep == aClickStep)
            {
                // add new clickNoteNode
                cur->notes->append(aNote, aTrack, aDuration);
                done = true;
            } else
            {
                if (cur->clickStep > aClickStep) 
                {
                    // no matching stepClickNode yet, make it.
                    insertBefore(cur->masterStep, aClickStep);
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
            rewind();
            insertBefore(aMasterStep, aClickStep);
            cur->notes->append(aNote, aTrack, aDuration);
        } else 
        { // add stepClickNode at the end
            append(aMasterStep, aClickStep);
            cur = tail;
            cur->notes->append(aNote, aTrack, aDuration);
        }
    }
}

void StepClickList::append(int aMasterStep, byte aClickStep)
{
    stepClickNode *n = new stepClickNode();
    n->masterStep = aMasterStep;
    n->clickStep = aClickStep;
    n->notes = new PerClickNoteList();
    
    if(tail != NULL)
        tail->next = n; // point previously last node to new one

    tail = n;           // point tail at new node

    if(cur == NULL)
        cur = n;

    if(head == NULL)
        head = n;
}

void StepClickList::insertBefore(int aMasterStep, byte aClickStep)
{
    stepClickNode *n = new stepClickNode();
    n->masterStep = aMasterStep;
    n->clickStep = aClickStep;
    n->notes = new PerClickNoteList();

    n->next = cur;

    if (tail == NULL)
        tail = cur;

    if (cur == NULL)
        cur = head;

    if (cur == NULL)
    {
        cur = n;
        head = n;
    } else
    {
        if(cur == head)
        {
            n->next = head;
            head->prev = n;
            head = n;
            cur = n;
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
}

int StepClickList::getMasterStep()
{
    int retVal;

    if( cur != NULL )
        retVal = cur->masterStep;
        // really we should raise exception...
    return retVal; 
}

PerClickNoteList* StepClickList::getClickNoteList(byte a_click)
{
    PerClickNoteList *retVal;

    while(hasValue())
    {
        if(cur->masterStep == g_activeGlobalStep
            && a_click == cur->clickStep)
        {
            retVal = cur->notes;
            break;
        }
//          StepSequencer::activeStepClicks.next();
        activeStepClicks.next();
    }
    return retVal;
}

byte StepClickList::getClickStep()
{
    byte retVal;

    if( cur != NULL )
        retVal = cur->clickStep;
        // really we should raise exception...
    return retVal; 
}

PerClickNoteList* StepClickList::getNotes()
{
    PerClickNoteList *retVal;

    if( cur != NULL )
        retVal = cur->notes;
        // really we should raise exception...
    return retVal; 
}

void StepClickList::dropNotesBeforeStepAndRewind(int aStep)
{
    Serial.print("dropNotesBeforeStepAndRewind before ");
    Serial.println(aStep);

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
                    Serial.print("head->masterStep == NULL before ");
                    Serial.println(aStep);
                    b = false;
            } else
            {
                if (head->masterStep < aStep)
                {
                    Serial.print("dropping before ");
                    Serial.println(aStep);
                    dropHead();
                } else
                {
                    Serial.print("head->masterStep NOT < aStep: ");
                    Serial.println(aStep);
                    Serial.print("  head->masterStep: ");
                    Serial.println(head->masterStep);
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
}

void StepClickList::dropHead()
{
    if (head != NULL)
    {
        stepClickNode *newHead = head->next;

        if (cur == head)
            cur = newHead;

        delete head->notes;
        delete head;
        head = newHead;
    }
}

void StepClickList::rewind()
{
        cur = head;
}

void StepClickList::next()
{
        if( cur != NULL )
                cur = cur->next;
}

int StepClickList::hasValue()
{
        return ( cur != NULL ? true : false );
}