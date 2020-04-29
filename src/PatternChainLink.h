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

    // to handle UI
    void prepChainLinkForEdit();

    const char *LinkEditOptionNames[linkEditOptionsCount] = {
                          "Current Chain", "Current Link", 
                          "Ch Times To Play", "Chain Content", "Next Chain",
                          "Override This Link", "Insert After Current",
                          "Append to Chain", "Start New Chain", 
                          "Delete This Link", "Duplicate Link", 
                          "Copy Link", "Paste Link", 
                          "Link Content", "Lead Track",
                          "Times to Play", "Speed Factor",
                          "Length Override", "Path Override",
                          "Previous Link", "Next Link"};       

  private:

    int currentPlayCount;
};

#endif