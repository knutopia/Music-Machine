#ifndef __PATTERNCHAINHANDLER
#define __PATTERNCHAINHANDLER

#include <Arduino.h>
#include "Enum.h"
#include "PatternChainLink.h"

struct Chain {
    int timesToPlay;
    PatternChainLink* links[MAXLINKSPERCHAIN];
    byte numberOfLinks;
    byte nextChain;
};

class PatternChainHandler
{
  public:
    PatternChainHandler();

    void begin();
    
    // for editing...
    void selectLink(byte linkNum);
    void appendLink();
    void removeLink();
    void clearLink();

    // management during play
    bool updateLinkOrChainIfNeeded();

  private:

    bool timeForNextChain(); // utility

    Chain chains[MAXCHAINCOUNT];
    Chain* currentChain;
    byte currentChainIndex;
    PatternChainLink* currentLink;
    int currentChainPlayCount;
/*
    QueuedActionRecord acQueue[QUEUEMAXLEN];
    byte queuedActionsCount = 0;
    byte actionRetrievalIndex = 0;
*/
};

#endif