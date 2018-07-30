#include "LinkedNoteList.h"    

//#define DEBUG true

LinkedNoteList::LinkedNoteList()
{
    head = NULL;
    cur = NULL;
    tail = NULL;

    Serial.println("LinkedNoteList alive !");
}

LinkedNoteList::~LinkedNoteList()
{
    noteNode *die;

    Serial.print("Destructor LinkedNoteList ");
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

    Serial.println("LinkedNoteList alive !");
}

void LinkedNoteList::dropNotesBeforeStepAndRewind(int aStep)
{
    while(head != NULL && head->masterStep < aStep)
        dropHeadNote();
    cur = head;
}

void LinkedNoteList::dropHeadNote()
{
    if (head != NULL)
    {
        noteNode *newHead = head->next;

        if (cur == head)
            cur = newHead;
        
        delete head;
        head = newHead;
    }
}

// This prepends a new value at the beginning of the list
void LinkedNoteList::prependNote(int aStep, byte aTrack, note aNote)
{
    noteNode *n = new noteNode();   // create new Node
    n->masterStep = aStep;  // set value
    n->track = aTrack;
    n->trackNote = aNote;
    n->next = head;         // make the node point to the next node.
                            //  If the list is empty, this is NULL, so the end of the list --> OK
    head = n;               // last but not least, make the head point at the new node.
    if( cur == NULL)
            cur = head;

    if(tail == NULL)
            tail = n;
}

// add value at the end -kg
void LinkedNoteList::appendNote(int aStep, byte aTrack, note aNote)
{
#ifdef DEBUG
    Serial.print("appendNote aNote is ");
    Serial.println((unsigned int) &aNote);
#endif

    noteNode *n = new noteNode();   // create new Node
    n->masterStep = aStep;  // set value
    n->track = aTrack;
    n->trackNote = aNote;
    n->next = NULL;

    if(tail != NULL && tail != n)
        tail->next = n; // point previously last node to new one

    tail = n;           // point tail at new node

    if(cur == NULL)
        cur = n;

    if(head == NULL) // REPEATED in print section below...
        head = n;

/*
    Serial.println("Notelist appendNote");

    if(head == NULL)
    {
        Serial.println(" :head was NULL");
        head = n;
    } else
        Serial.println(" :head was Object");

    if (cur == n)
        Serial.println(" :cur == n");
    else
        Serial.println(" :cur != n");
    if (cur == head)
        Serial.println(" :cur == head");
    else
        Serial.println(" :cur != head");
    if (cur == head)
        Serial.println(" :cur == head");
    else
        Serial.println(" :cur != head");
    if (cur == tail)
        Serial.println(" :cur == tail");
    else
        Serial.println(" :cur != tail");
    if (cur == NULL)
        Serial.println(" :cur == NULL");
    else
        Serial.println(" :cur Object");
    if (head == NULL)
        Serial.println(" :head == NULL");
    else
        Serial.println(" :head Object");
    if (n->next == NULL)
        Serial.println(" :n->next == NULL");
    else
        Serial.println(" :n->next Object");
    if (head->next == NULL)
        Serial.println(" :head->next == NULL");
    else
        Serial.println(" :head->next Object");
    if (n == n->next)
        Serial.println(" :n->next circular");
    else
        Serial.println(" :n->next OK");
    if (head == head->next)
        Serial.println(" :head->next circular");
    else
        Serial.println(" :head->next OK");
*/
}

void LinkedNoteList::rewind()
{
        cur = head;
        if(head == NULL)
            Serial.println("NULL head on Notelist rewind");
}
void LinkedNoteList::next()
{
        if( cur != NULL )
        {
            if( cur == cur->next)
                Serial.println("cur->next circular on Notelist next");
            cur = cur->next;
        }
        else
            Serial.println("NULL cur on Notelist next");
}

int LinkedNoteList::getStep()
{
        if( cur != NULL )
                return cur->masterStep;
        return 0; // really we should raise exception
}

int LinkedNoteList::getTrack()
{
        if( cur != NULL )
                return cur->track;
        return 0; // really we should raise exception
}

note LinkedNoteList::getNote()
{
    note retVal;

    Serial.print("Notelist getNote ");

    if( cur != NULL )
    {
        retVal = cur->trackNote;

        Serial.print(retVal.pitchVal);
        Serial.print("  track ");
        Serial.println(cur->track);

        // really we should raise exception...
    } else
        Serial.println(" NULL cur");

    return retVal; 
}

int LinkedNoteList::hasValue()
{
        return ( cur != NULL ? true : false );
}

int LinkedNoteList::count()
{
    int retVal = 0;
    rewind();
    while( hasValue()){
//        Serial.print("Count ");
//        Serial.println(retVal);

        retVal++;
        next();
    }        
    return retVal;
}

