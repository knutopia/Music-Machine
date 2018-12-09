#include "PatternChainHandler.h"
#include "InOutHelper.h"

extern InOutHelper inout;

byte PatternChainHandler::currentLeadTrack;

PatternChainHandler::PatternChainHandler()
{
    
};

void PatternChainHandler::begin()
{
    for(int f = 0; f < MAXCHAINCOUNT; f++)
    {
        chains[f].nextChain = 255;
        chains[f].numberOfLinks = 0;
        chains[f].timesToPlay = 0;
        // leaving chains[f].links uninitialized
    }
    currentChain = NULL;
    currentChainIndex = 0;
    currentLink = NULL;
    currentChainPlayCount = 0;
};

// for editing...
void PatternChainHandler::selectLink(byte linkNum)
{

};

void PatternChainHandler::appendLink()
{

};

void PatternChainHandler::removeLink()
{

};

void PatternChainHandler::clearLink()
{

};

// management during play

bool PatternChainHandler::updateLinkOrChainIfNeeded()
{
    // run once per pattern loop:
    // increment the link play count
    // check link play count to see if next link is due
    // if it is:
    //      check if more links in chain
    //      if yes:
    //          go to next link
    //      if no:
    //          check chain play count to see if next chain is due
    //          if next chain is due:
    //              if next chain available: increment chain, set up first link
    //              if no more chains available: stop play
    //          if not:
    //              increment chain play count
    //              go to first link in chain

    if(currentLink != NULL)
    {
        currentLink->incrementLinkPlayCount();
        if(currentLink->timeForNextLink())
        {
            
        }
    }
};

bool PatternChainHandler::timeForNextChain()
{
    bool retVal = false;

    if(currentChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH tFNCh curCh NULL");
        retVal = false;
    } else {
        if(currentChainPlayCount == currentChain->timesToPlay)
            retVal = true;
        else
            if(currentChainPlayCount == currentChain->timesToPlay)
            {
                retVal = true;

                inout.ShowErrorOnLCD("PCH tFNCh curCPC err");
                Serial.print("currentChainPlayCount: ");
                Serial.print(currentChainPlayCount);
                Serial.print("  currentChain->timesToPlay: ");
                Serial.println(currentChain->timesToPlay);
            }
    }
    return retVal;
};