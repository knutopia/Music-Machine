#ifndef __PATTERNCHAINHANDLER
#define __PATTERNCHAINHANDLER

#include <Arduino.h>
#include "Enum.h"
#include "PatternChainLink.h"

//typedef for callback
typedef void (*simpleFunc) ();

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
    ~PatternChainHandler();

    void begin(simpleFunc stopCbPointer);
    
    // setters
    void setTimesToPlay(byte times); 

    // for editing...
    void selectLink(byte linkNum);
    PatternChainLink* appendLink();
    bool removeLink();
    void clearLink();

    // management during play
    bool updateLinkOrChainIfNeeded();
    void reset();

    static byte currentLeadTrack;

  private:

    simpleFunc stopPlaybackCb;
    bool timeForNextChain(); // utility

    Chain chains[MAXCHAINCOUNT];
    Chain* currentChain;
    byte currentChainIndex;
    PatternChainLink* currentLink;
    byte currentLinkIndex;
    int currentChainPlayCount;
/*
    QueuedActionRecord acQueue[QUEUEMAXLEN];
    byte queuedActionsCount = 0;
    byte actionRetrievalIndex = 0;
*/
};

#endif