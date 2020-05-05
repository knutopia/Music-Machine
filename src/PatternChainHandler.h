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
    bool setCurrentLink(byte index);
    bool setNextChain(byte chainIndex, byte nextIndex);
    void setTimesToPlay(byte times);

    // getters
    byte getNumberOfLinks();

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

    // input processing / editing
    void prepPatternChainForEdit();
    void handleSelectButton();
    void handleEncoder(int encoder, int value);
    void handleButton(int butNum);
    void saveToLinkInCurrentChain();

    // class data
    static byte currentLeadTrack;
    static intFunc updatePatternNumberCb;
    static speedFactorFunc updateSpeedMultiplierCb;

    // data
    Chain chains[MAXCHAINCOUNT];

  private:

    // input processing / editing
    void showEditParam();
    int captureEditParamStartVal();
    void editParam(int value);

    bool setActionTarget(byte linkIndex);
    bool actionTargetValid();
    void resetActionTarget();
    bool savePatternsToLink(byte targetChainIndex, byte targetlinkIndex);
    void printChains();

    simpleFunc stopPlaybackCb;
    int putInRange(int iVar, int iRange, int iMin);

    int currentEditParamIndex;
    int editParamStartVal;
    byte b_currentEditParStart;
    bool m_b_reset_encoder_reference;
    int m_edit_state = ParamChoice;
    int m_button_pressed_num;
    unsigned long m_button_press_time;

    const char *EditOptionNames[PatternChainEditOptionsCount] = {
                          "Chain:", "(Chain ", 
                          "Ch Plays: ", "Chain Content:", 
                          "[Previous Chain]", "[Next Chain]",
                          "[SaveToLink:", "[Insert After]",
                          "[Append to Chain]", "[Start New Chain]", 
                          "[Delete Link]", "[Duplicate Link]", 
                          "[Copy Link]", "[Paste Link]", 
                          "Link Content:", "Lead Track:",
                          "Link Plays:", "Speed*:",
                          "Length Ovrd:", "Path Ovrd:",
                          "[Previous Link]", "[Next Link]"};       

    bool timeForNextChain(); // utility

//  Chain chains[MAXCHAINCOUNT];
    Chain* currentChain;
    byte currentChainIndex;
    PatternChainLink* currentLink;
    byte currentLinkIndex;
    int currentChainPlayCount;
    byte actionTargetChainIndex;
    byte actionTargetLinkIndex;
    bool b_actionTargetActive;
};

#endif