#ifndef __PATTERNCHAINHANDLER
#define __PATTERNCHAINHANDLER

#include <Arduino.h>
#include "Enum.h"
#include "PatternChainLink.h"

//typedef for callback
typedef void (*simpleFunc) ();
typedef void (*intFunc) (int passVal);
typedef void (*speedFactorFunc) (speedFactor passVal);


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

    void begin(simpleFunc stopCbPointer,
               intFunc changeSequenceNumberCbPointer,
               speedFactorFunc changeSpeedMultiplierCbPointer);
    
    // setters
    bool setCurrentChain(byte index);
    bool setNextChain(byte chainIndex, byte nextIndex);
    void setTimesToPlay(byte times);

    // for editing...
    void selectLink(byte linkNum);
    PatternChainLink* appendLink();
    bool removeLink();
    void clearLink();

    // management during play
    void startChainPlay();
    bool updateLinkOrChainIfNeeded();
    void playCurrentChainLink();
    void updateChainAndLinkDisplay();
    void reset();

    static byte currentLeadTrack;
    static intFunc updateSequenceNumberCb;
    static speedFactorFunc updateSpeedMultiplierCb;

  private:

    simpleFunc stopPlaybackCb;
//  intFunc updateSequenceNumberCb;
//  speedFactorFunc updateSpeedMultiplierCb;

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