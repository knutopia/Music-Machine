#ifndef __PATTERNCHAINLINK
#define __PATTERNCHAINLINK

#include <Arduino.h>
#include "Enum.h"

/*
//UI Stuff
struct LinkIntParam {
  int id;
  int minVal;
  int maxVal;
  int defaultVal;
  int paramVal;
  bool isInt = true;
};

struct LinkByteParam {
  int id;
  byte minVal;
  byte maxVal;
  byte defaultVal;
  byte paramVal;
  bool isInt = false;
};

struct LinkAction {
  int id;
  bool isInt = false;
};
*/

//Structure
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

    // public for serialization
    ChainLink link;

  private:

    int currentPlayCount;
};

#endif