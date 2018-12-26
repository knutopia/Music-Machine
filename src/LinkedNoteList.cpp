#include "LinkedNoteList.h"    
#include "InOutHelper.h"

//#define DEBUG true

extern InOutHelper inout;

LinkedNoteList::LinkedNoteList()
{
    head = NULL;
    cur = NULL;
    tail = NULL;

    Serial.println("LinkedNoteList alive !");
}

LinkedNoteList::~LinkedNoteList()
{
#ifdef DEBUG
    Serial.print("Destructor LinkedNoteList ");
#endif

    noteNode *die = head;

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

#ifdef DEBUG
    Serial.println("done");
#endif
}

void LinkedNoteList::purge()
{
#ifdef DEBUG
    Serial.print("LinkedNoteList purge ");
#endif

    noteNode *die = head;

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

#ifdef DEBUG
    Serial.println("done");
#endif
}

void LinkedNoteList::printActiveNote()
{
    Serial.println("NoteNode: ");
    Serial.print("  MasterStep: ");
    Serial.print(cur->masterStep);
    Serial.print(" on track: ");
    Serial.println(cur->track);
    Serial.print("  Note: ");
    Serial.print(cur->trackNote.pitchVal);
    Serial.print("  dur: ");
    Serial.print(cur->trackNote.duration);
    Serial.print("  retrigs: ");
    Serial.print(cur->trackNote.retrigs);
    Serial.print("  retrigClickDivider: ");
    Serial.print(cur->trackNote.retrigClickDivider);
    Serial.print("  swingTicks: ");
    Serial.println(cur->trackNote.swingTicks);
}

bool LinkedNoteList::checkIntegrity(const char caller[])
{
    bool retVal = true;
    if(cur != NULL)
    {
        if(cur == cur->next)
        {
            inout.ShowErrorOnLCD("LNL next Error", caller);
            Serial.print("LinkedNoteList next error called from ");
            Serial.println(caller);
            retVal = false;
        }
        if(tail != NULL && head != NULL)
        {
            if(tail->next == head)
            {
                inout.ShowErrorOnLCD("LNL tailN Error", caller);
                Serial.print(" !!! LinkedNoteList tail->next == head error called from ");
                Serial.println(caller);
                retVal = false;
            }
        }
        if(cur->masterStep > 10000000)
        {
            inout.ShowErrorOnLCD("LNL masterStep Error", caller);
            Serial.print(" !!! LinkedNoteList masterStep error called from ");
            Serial.println(caller);
            Serial.print("  cur: ");
            Serial.print((int)cur);
            Serial.print("  masterStep: ");
            Serial.println(cur->masterStep);
            retVal = false;
        }    
        if(cur->masterStep == -1)
        {
            inout.ShowErrorOnLCD("LNL masterStep -1 Er", caller);
            Serial.print(" !!! LinkedNoteList masterStep -1 error called from ");
            Serial.println(caller);

            Serial.print("  cur: ");
            Serial.print((int)cur);
            Serial.print("  masterStep: ");
            Serial.println(cur->masterStep);

            retVal = false;
        }    
    }
    return retVal;
}

void LinkedNoteList::dropNotesBeforeStepAndRewind(int aStep)
{
    while(head != NULL && head->masterStep < aStep)
    {
/*        
        Serial.print(head->masterStep);
        Serial.print(" at ");
        Serial.println(aStep);
//      Serial.print("  ");
        print();
*/

        dropHeadNote();
        rewind();
    }
    cur = head;
    if(!checkIntegrity("dropNotesBeforeStepAndRewind"))
        print();
}

void LinkedNoteList::dropHeadNote()
{
    if (head != NULL)
    {
        if (cur == head)
            cur = NULL;

        if(head->next == NULL)
        {
            if(tail == head)
                tail = NULL;

            delete head;
            head = NULL;
//          Serial.println(" dropHeadNote A ");

        } else 
        {
            noteNode *die = head;
            head = head->next;
            
            die->next = NULL;
            delete die;
            die = NULL;

//          Serial.println(" dropHeadNote B ");

        }
    }
    if(!checkIntegrity("dropHeadNote"))
        print();
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

    if(!checkIntegrity("prependNote"))
        print();
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
    memcpy(&n->trackNote, &aNote, sizeof(note));
    n->next = NULL;

    if(tail != NULL && tail != n)
        tail->next = n; // point previously last node to new one

    tail = n;           // point tail at new node

    if(cur == NULL)
        cur = n;

    if(head == NULL) // REPEATED in print section below...
        head = n;

    if(!checkIntegrity("appendNote"))
        print();
}

void LinkedNoteList::rewind()
{
        cur = head;
//      if(head == NULL)
//          Serial.println("NULL head on Notelist rewind");
}
void LinkedNoteList::next()
{
    if( cur != NULL )
    {
        if( cur == cur->next)
            inout.ShowErrorOnLCD("c->n circular on NL");
        cur = cur->next;
    }
    else
        Serial.println("NULL cur on Notelist next");
        
    if(!checkIntegrity("next"))
        print();
}

int LinkedNoteList::getStep()
{
        if( cur != NULL )
            return cur->masterStep;
        else
            inout.ShowErrorOnLCD("LNL getStep NULL");

        return 0; // really we should raise exception
}

int LinkedNoteList::getTrack()
{
        if( cur != NULL )
                return cur->track;
        else
            inout.ShowErrorOnLCD("LNL getTrack NULL");

        return 0; // really we should raise exception
}

note LinkedNoteList::getNote()
{
    note retVal;

#ifdef DEBUG
    Serial.print("Notelist getNote ");
#endif

    if( cur != NULL )
    {
        retVal = cur->trackNote;

        if (&(cur->trackNote) == NULL)
            inout.ShowErrorOnLCD("LNL getNote tN NULL");

#ifdef DEBUG
        Serial.print(retVal.pitchVal);
        Serial.print("  track ");
        Serial.println(cur->track);
//      Serial.print("  dur ");
//      Serial.println(cur->trackNote.duration);
#endif
    } else
        inout.ShowErrorOnLCD("LNL getNote cur NULL");

    return retVal; 
}

int LinkedNoteList::hasValue()
{
        return ( cur != NULL ? true : false );
}

int LinkedNoteList::count()
{
    int count = 0;
    noteNode *buf = cur;
    rewind();
    int sentry = 0;
    while(hasValue())
    {
        count++;
        next();
        if(++sentry == 1000)
        {
            inout.ShowErrorOnLCD("LNL count stuck");
            break;
        }
    }
    cur = buf;
    return count;
}

void LinkedNoteList::print()
{
    noteNode *buf = cur;
    Serial.println("LinkedNoteList print: ");
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
        Serial.print("  masterStep: ");
        Serial.print(cur->masterStep);
        Serial.print("  trackNote: ");
        Serial.print((int)&(cur->trackNote));
        Serial.print("  track: ");
        Serial.print((int)cur->track);
        Serial.print("  curnode: ");
        Serial.print((int)cur);
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
