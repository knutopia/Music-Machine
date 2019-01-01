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

    for(f = 0; f < queuedActionsCount; f++)
    {
        if( acQueue[f].action == anAction && acQueue[f].track == aTrack)
        {
            actionIDfound = true;
            break;
        }
    }

    if(actionIDfound)
    {
        acQueue[f].param = aParam;
        acQueue[f].track = aTrack; 
    } else {
        if(f < QUEUEMAXLEN - 1)
        {
            queuedActionsCount++;
            acQueue[f].action = anAction;
            acQueue[f].param = aParam;
            acQueue[f].track = aTrack;

        } else {
            inout.ShowErrorOnLCD("queueA: outOfRange");
            Serial.print("f = ");
            Serial.print(f);
            Serial.print("  queuedActionsCount = ");
            Serial.print(queuedActionsCount);
            Serial.print("  QUEUEMAXLEN = ");
            Serial.print(QUEUEMAXLEN);
            Serial.print("  action = ");
            Serial.println(anAction);
        }
    }

    switch (anAction)
    {
        case PATTERNCHANGE:
            inout.ShowValueInfoOnLCD("Queued pattern", aParam);
            break;

        case PATHCHANGE:
            inout.ShowValueInfoOnLCD("Queued path", aParam);
            break;

        case LENGTHCHANGE:
            inout.ShowValueInfoOnLCD("Queued length", aParam);
            break;

        case TRACKMUTECHANGE:
            inout.ShowInfoOnLCD("Queued Tmute toggle");
            break;

        case SPEEDMULTIPLIERCHANGE:
            inout.ShowInfoOnLCD("Queued speed");
            break;

        case SYNCTRACKS:
            // TODO
            break;

        case NOACTION:
            break;

        default:
            inout.ShowErrorOnLCD("quAcs out of rnge");
            Serial.print("queueAction anAction ");
            Serial.println(anAction);
    }
}

actionID ActionQueue::retrieveActionID()
{
    actionID retVal = NOACTION;

    if(actionRetrievalIndex < QUEUEMAXLEN)
    {
        retVal = acQueue[actionRetrievalIndex].action;

        Serial.print("actionRetrievalIndex = ");
        Serial.print(actionRetrievalIndex);
        Serial.print("  action = ");
        Serial.print(retVal);

    }
    else {
            inout.ShowErrorOnLCD("retrAID: outOfRange");
            Serial.print("actionRetrievalIndex = ");
            Serial.print(actionRetrievalIndex);
            Serial.print("  queuedActionsCount = ");
            Serial.print(queuedActionsCount);
            Serial.print("  QUEUEMAXLEN = ");
            Serial.println(QUEUEMAXLEN);
    }
    return retVal;
}

byte ActionQueue::retrieveActionParam()
{
    byte retVal = 0;

    if(actionRetrievalIndex < QUEUEMAXLEN)
        retVal = acQueue[actionRetrievalIndex].param;
    else {
            inout.ShowErrorOnLCD("retrAPm: outOfRange");
            Serial.print("actionRetrievalIndex = ");
            Serial.print(actionRetrievalIndex);
            Serial.print("  queuedActionsCount = ");
            Serial.print(queuedActionsCount);
            Serial.print("  QUEUEMAXLEN = ");
            Serial.println(QUEUEMAXLEN);
    }
    return retVal;
}

byte ActionQueue::retrieveActionTrack()
{
    byte retVal = 0;

    if(actionRetrievalIndex < QUEUEMAXLEN)
        retVal = acQueue[actionRetrievalIndex].track;
    else {
            inout.ShowErrorOnLCD("retrATr: outOfRange");
            Serial.print("actionRetrievalIndex = ");
            Serial.print(actionRetrievalIndex);
            Serial.print("  queuedActionsCount = ");
            Serial.print(queuedActionsCount);
            Serial.print("  QUEUEMAXLEN = ");
            Serial.println(QUEUEMAXLEN);
    }
    return retVal;
}

bool ActionQueue::advanceRetrievalIndex()
{
    bool retVal = false;

    if( ++actionRetrievalIndex < QUEUEMAXLEN)
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

    for(int f = 0; f < QUEUEMAXLEN; f++)
        acQueue[f].action = NOACTION;
}