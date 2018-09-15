#ifndef __QUEUEDACTION
#define __QUEUEDACTION

#include <Arduino.h>
#include "Enum.h"


struct QueuedActionRecord {
    actionID action = NOACTION;
    byte param = 0;
    byte track = 0;
};

class ActionQueue
{
  public:
    ActionQueue();

    void begin();
    void queueAction(actionID anAction, byte aParam, byte aTrack);
    actionID retrieveActionID();
    byte retrieveActionParam();
    byte retrieveActionTrack();
    bool advanceRetrievalIndex();
    void beginRetrievingActions();
    void clearQueue();

  private:
    QueuedActionRecord aQueue[QUEUEDACTIONVARIETY];
    byte queuedActionsCount = 0;
    byte actionRetrievalIndex = 0;
};

#endif