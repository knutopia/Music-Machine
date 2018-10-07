#ifndef __PATTERNCHAINLINK
#define __PATTERNCHAINLINK

#include <Arduino.h>
#include "Enum.h"


struct ChainLink {

    byte patternPerTrack[TRACKCOUNT];
    bool mutePerTrack[TRACKCOUNT];
    bool trackUsedInLink[TRACKCOUNT];
    int timesToPlay;
    byte nextLink;
};

class PatternChainLink
{
  public:
    PatternChainLink();

    void begin();
    
    void addTrackPatterntoLink(byte trackNum, byte patNum, bool muteIt); // to populate link

    // setters
    void setTimesToPlay(byte times); 
    void setNextLink(byte linkNum);

    // to use during play
    bool timeForNextLink();
    void incrementLinkPlayCount();

  private:

    ChainLink link;
    int currentPlayCount;
};

#endif