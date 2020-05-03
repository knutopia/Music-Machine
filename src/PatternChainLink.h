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
    int lengthOverride = OVERRIDEINACTIVE;
    int pathOverride = OVERRIDEINACTIVE;
};

class PatternChainLink
{
  public:
    PatternChainLink();

    void begin();
    
    void addTrackPatterntoLink(byte trackNum, byte patNum, bool muteIt); // to populate link

    // getters
    byte getCurrentPlayCount();
    int getTimesToPlay();
    byte getLeadTrack();
    bool isTrackUsedInLink(int trackNum);
    speedFactor getSpeedMult();
    int getLengthOverride();
    int getPathOverride();

    // setters
    bool setLengthOverride(int length);
    bool setPathOverride(int path);
    bool setLeadTrack(byte trackNum);
    void setTimesToPlay(int times); 
    void setSpeedMult(speedFactor times);
    void setSpeedMult(int times);
    void setNextLinkIndex(byte linkNum);
    byte getNextLinkIndex();
    void resetPlayCount();

    // to use during play
    bool timeForNextLinkIndex();
    void incrementLinkPlayCount();
    void primeLinktoPlay();

    // debugging
    void printLink();

    // public for serialization
    ChainLink link;

  private:

    int currentPlayCount;
};

#endif