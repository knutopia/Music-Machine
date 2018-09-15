#include "ActionQueue.h"
#include "InOutHelper.h"

extern InOutHelper inout;

ActionQueue::ActionQueue()
{
}

void ActionQueue::begin()
{
    clearQueue();
}

void ActionQueue::queueAction(actionID anAction, byte aParam, byte aTrack)
{
    bool actionIDfound = false;
    int foundIndex = -1;
    int f;

    for(f = 0; f < QUEUEDACTIONVARIETY; f++)
    {
        if( aQueue[f].action == anAction)
        {
            actionIDfound = true;
            break;
        }
    }

    if(actionIDfound)
    {
        aQueue[f].param = aParam;
        aQueue[f].track = aTrack; 
    } else {
        if(f < QUEUEDACTIONVARIETY - 1)
        {
            queuedActionsCount++;
            aQueue[f+1].action = anAction;
            aQueue[f+1].param = aParam;
            aQueue[f+1].track = aTrack;
        } else {
            inout.ShowErrorOnLCD("queueA: outOfRange");
            Serial.print("f = ");
            Serial.print(f);
            Serial.print("  queuedActionsCount = ");
            Serial.print(queuedActionsCount);
            Serial.print("  QUEUEDACTIONVARIETY = ");
            Serial.print(QUEUEDACTIONVARIETY);
            Serial.print("  action = ");
            Serial.println(anAction);
        }
    }
}

actionID ActionQueue::retrieveActionID()
{
    actionID retVal = NOACTION;

    if(actionRetrievalIndex < QUEUEDACTIONVARIETY)
        retVal = aQueue[actionRetrievalIndex].action;
    else {
            inout.ShowErrorOnLCD("retrAID: outOfRange");
            Serial.print("actionRetrievalIndex = ");
            Serial.print(actionRetrievalIndex);
            Serial.print("  queuedActionsCount = ");
            Serial.print(queuedActionsCount);
            Serial.print("  QUEUEDACTIONVARIETY = ");
            Serial.println(QUEUEDACTIONVARIETY);
    }
    return retVal;
}

byte ActionQueue::retrieveActionParam()
{
    byte retVal = 0;

    if(actionRetrievalIndex < QUEUEDACTIONVARIETY)
        retVal = aQueue[actionRetrievalIndex].param;
    else {
            inout.ShowErrorOnLCD("retrAPm: outOfRange");
            Serial.print("actionRetrievalIndex = ");
            Serial.print(actionRetrievalIndex);
            Serial.print("  queuedActionsCount = ");
            Serial.print(queuedActionsCount);
            Serial.print("  QUEUEDACTIONVARIETY = ");
            Serial.println(QUEUEDACTIONVARIETY);
    }
    return retVal;
}

byte ActionQueue::retrieveActionTrack()
{
    byte retVal = 0;

    if(actionRetrievalIndex < QUEUEDACTIONVARIETY)
        retVal = aQueue[actionRetrievalIndex].track;
    else {
            inout.ShowErrorOnLCD("retrATr: outOfRange");
            Serial.print("actionRetrievalIndex = ");
            Serial.print(actionRetrievalIndex);
            Serial.print("  queuedActionsCount = ");
            Serial.print(queuedActionsCount);
            Serial.print("  QUEUEDACTIONVARIETY = ");
            Serial.println(QUEUEDACTIONVARIETY);
    }
    return retVal;
}

bool ActionQueue::advanceRetrievalIndex()
{
    bool retVal = false;

    if( ++actionRetrievalIndex < QUEUEDACTIONVARIETY)
        retVal = true;

    return retVal;
}

void ActionQueue::beginRetrievingActions()
{
    actionRetrievalIndex = 0;
}

void ActionQueue::clearQueue()
{
    queuedActionsCount = 0;
    actionRetrievalIndex = 0;

    for(int f = 0; f < QUEUEDACTIONVARIETY; f++)
        aQueue[f].action = NOACTION;
}