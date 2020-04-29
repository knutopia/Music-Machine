#ifndef __PATTERNCHAINHANDLER
#define __PATTERNCHAINHANDLER

#include <Arduino.h>
#include "Enum.h"
#include "PatternChainLink.h"
#include <ArduinoJson.h>

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
               intFunc changePatternNumberCbPointer,
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

    // Input processing
    void handleSelectButton();
    void handleEncoder(int encoder, int value);
    void handleButton(int butNum);

    // class data
    static byte currentLeadTrack;
    static intFunc updatePatternNumberCb;
    static speedFactorFunc updateSpeedMultiplierCb;

    // data
    Chain chains[MAXCHAINCOUNT];

  private:

    simpleFunc stopPlaybackCb;

    bool timeForNextChain(); // utility

//  Chain chains[MAXCHAINCOUNT];
    Chain* currentChain;
    byte currentChainIndex;
    PatternChainLink* currentLink;
    byte currentLinkIndex;
    int currentChainPlayCount;
};

#endif