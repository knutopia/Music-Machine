#ifndef __PATTERNCHAINLINK
#define __PATTERNCHAINLINK

#include <Arduino.h>
#include "Enum.h"


struct ChainLink {

    byte patternPerTrack[TRACKCOUNT];
    bool mutePerTrack[TRACKCOUNT];
    bool trackUsedInLink[TRACKCOUNT];
    int timesToPlay = 1;
    byte leadTrack;
    byte nextLinkIndex = 255;
};

class PatternChainLink
{
  public:
    PatternChainLink();

    void begin();
    
    void addTrackPatterntoLink(byte trackNum, byte patNum, bool muteIt); // to populate link

    // setters
    bool setLeadTrack(byte trackNum);
    void setTimesToPlay(byte times); 
    void setNextLinkIndex(byte linkNum);
    byte getNextLinkIndex();
    void resetPlayCount();

    // to use during play
    bool timeForNextLinkIndex();
    void incrementLinkPlayCount();

  private:

    ChainLink link;
    int currentPlayCount;
};

#endif