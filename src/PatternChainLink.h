#ifndef __PATTERNCHAINLINK
#define __PATTERNCHAINLINK

#include <Arduino.h>
#include "Enum.h"

struct ChainLink {

    byte patternPerTrack[TRACKCOUNT];
    bool mutePerTrack[TRACKCOUNT];
    bool trackUsedInLink[TRACKCOUNT];
    int timesToPlay = 1;
    speedFactor speedMult = UNDEFINED;
    byte leadTrack;
    byte nextLinkIndex = 255;
    byte lengthOverride = 255;
    int pathOverride = 255;
};

class PatternChainLink
{
  public:
    PatternChainLink();

    void begin();
    
    void addTrackPatterntoLink(byte trackNum, byte patNum, bool muteIt); // to populate link

    // getters
    byte getCurrentPlayCount();
    byte getTimesToPlay();

    // setters
    bool setLengthOverride(byte length);
    bool setPathOverride(int path);
    bool setLeadTrack(byte trackNum);
    void setTimesToPlay(byte times); 
    void setNextLinkIndex(byte linkNum);
    byte getNextLinkIndex();
    void resetPlayCount();

    // to use during play
    bool timeForNextLinkIndex();
    void incrementLinkPlayCount();
    void primeLinktoPlay();

    // public for serialization
    ChainLink link;

  private:

    int currentPlayCount;
};

#endif