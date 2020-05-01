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

    // Input processing / editing
    void prepPatternChainForEdit();
    void handleSelectButton();
    void handleEncoder(int encoder, int value);
    void handleButton(int butNum);
    void showEditParam();
    int captureEditParamStartVal();
    void editParam(int value);


    // class data
    static byte currentLeadTrack;
    static intFunc updatePatternNumberCb;
    static speedFactorFunc updateSpeedMultiplierCb;

    // data
    Chain chains[MAXCHAINCOUNT];

  private:

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
                          "Chain: ", "Link: ", 
                          "Ch Plays: ", "Chain Content:", 
                          "[Previous Chain]", "[Next Chain]",
                          "[Override Link]", "[Insert After]",
                          "[Append to Chain]", "[Start New Chain]", 
                          "[Delete This Link]", "[Duplicate Link]", 
                          "[Copy Link]", "[Paste Link]", 
                          "Link Content:", "Lead Track:",
                          "Times to Play:", "Speed Factor:",
                          "Length Override:", "Path Override:",
                          "[Previous Link]", "[Next Link]"};       

    bool timeForNextChain(); // utility

//  Chain chains[MAXCHAINCOUNT];
    Chain* currentChain;
    byte currentChainIndex;
    PatternChainLink* currentLink;
    byte currentLinkIndex;
    int currentChainPlayCount;
};

#endif