#include "PatternChainHandler.h"
#include "InOutHelper.h"
#include "StepSequencer.h"

extern InOutHelper inout;

byte PatternChainHandler::currentLeadTrack;
intFunc PatternChainHandler::updateSequenceNumberCb;
speedFactorFunc PatternChainHandler::updateSpeedMultiplierCb;

PatternChainHandler::PatternChainHandler()
{
    
};

PatternChainHandler::~PatternChainHandler()
{
    // remove the links
    for(int f = 0; f < MAXCHAINCOUNT; f++)
        for(int l = 0; l < chains[f].numberOfLinks; l++)
            if( chains[f].links[l] != NULL)
                delete chains[f].links[l];
};

void PatternChainHandler::begin(simpleFunc stopCbPointer,
                                intFunc changeSequenceNumberCbPointer,
                                speedFactorFunc changeSpeedMultiplierCbPointer)
{
    stopPlaybackCb = stopCbPointer;
    updateSequenceNumberCb = changeSequenceNumberCbPointer;
    updateSpeedMultiplierCb = changeSpeedMultiplierCbPointer;

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
    currentLinkIndex = 0;
    currentChainPlayCount = 0;
    currentLeadTrack = 1;

    // set up test chain content

    PatternChainLink* newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 0, false);
        newLink->addTrackPatterntoLink(2, 0, false);
        newLink->setLeadTrack(1);
        newLink->setTimesToPlay(2);
    }

    newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 1, false);
        newLink->addTrackPatterntoLink(2, 1, false);
        newLink->setLeadTrack(1);
        newLink->setTimesToPlay(2);
    }

    newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 2, false);
        newLink->addTrackPatterntoLink(2, 2, false);
        newLink->setLeadTrack(1);
        newLink->setTimesToPlay(2);
    }
    setTimesToPlay(1);

    reset();
};

// setters
void PatternChainHandler::setTimesToPlay(byte times)
{
    if (currentChain == NULL)
    {
        currentChainIndex = 0;
        if(chains == NULL)
        {
            inout.ShowErrorOnLCD("PCH:sTTP:chains NULL");
            return;
        }
        currentChain = &chains[currentChainIndex];
    }

    if (currentChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH:sTTP:curCh NULL");
        return;
    }
    
    currentChain->timesToPlay = times;
};

// for editing...
void PatternChainHandler::selectLink(byte linkNum)
{

};

PatternChainLink* PatternChainHandler::appendLink()
{
    if (currentChain == NULL)
    {
        currentChainIndex = 0;
        if(chains == NULL)
        {
            inout.ShowErrorOnLCD("PCH:aL:chains NULL");
            return NULL;
        }
        currentChain = &chains[currentChainIndex];
    }

    if (currentChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH:aL:curCh NULL");
        return NULL;
    }

    if (currentChain->numberOfLinks >= MAXLINKSPERCHAIN)
    {
        inout.ShowErrorOnLCD("PCH:aL:nOL max");
        return NULL;
    }

    PatternChainLink* newLink = new PatternChainLink();

    if (newLink == NULL)
    {
        inout.ShowErrorOnLCD("PCH:aL:newL NULL");
        return NULL;
    }

    if (currentChain->links == NULL)
    {
        inout.ShowErrorOnLCD("PCH:aL:links NULL");
        delete newLink;
        return NULL;
    }
    
    if (currentChain->numberOfLinks > 0)
    {
        PatternChainLink* prevLink = currentChain->links[currentChain->numberOfLinks-1];
        if (prevLink != NULL)
        {
            prevLink->setNextLinkIndex(currentChain->numberOfLinks);
        }
        else inout.ShowErrorOnLCD("setNeLiIx NO");
    }
    currentChain->links[currentChain->numberOfLinks] = newLink;
    currentChain->numberOfLinks++;

    newLink->begin();

    return newLink;
};

bool PatternChainHandler::removeLink()
{
    return true;
};

void PatternChainHandler::clearLink()
{

};

// management during play

void PatternChainHandler::startChainPlay()
{

    Serial.println("startChainPlay");

    if(currentLink != NULL)
        currentLink->primeLinktoPlay();
    else {
        inout.ShowErrorOnLCD("PCH: sCP cL NULL");
        return;
    }

    inout.ShowChainLinkOnLCD(currentChainIndex,
                             currentChainPlayCount,
                             currentChain->timesToPlay,
                             currentLinkIndex, 
                             currentLink->getCurrentPlayCount(), 
                             currentLink->getTimesToPlay());
}

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
    //          if not:
    //              increment chain play count
    //              go to first link in chain
    //          if next chain is due:
    //              if next chain available: increment chain, set up first link
    //              if no more chains available: stop play
    

    Serial.print("updateLinkOrChainIfNeeded, currentLinkPlayCount is ");
    Serial.print(currentLink->getCurrentPlayCount());
    Serial.print("/");
    Serial.print(currentLink->getTimesToPlay());
    Serial.print(" in link ");
    Serial.print(currentLinkIndex);
    Serial.print(" with nextLinkIndex ");
    Serial.println(currentLink->getNextLinkIndex());

    if(currentLink == NULL)
    {
        inout.ShowErrorOnLCD("PCH:uLOCIN cL NULL");
        return false;
    }
    
    if(currentChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH:uLOCIN cC NULL");
        return false;
    }
    
    // check link play count to see if next link is due
    if(currentLink->timeForNextLinkIndex())
    {

        Serial.print(" it is time for next link");

    // if it is:
    //      check if more links in chain
        byte nextLinkIndex = currentLink->getNextLinkIndex();
//      byte checkNextLinkIndex = currentLinkIndex+1; // #######################

        Serial.print("  nextLinkIndex ");
        Serial.print(nextLinkIndex);
//      Serial.print(" checkNextLinkIndex ");
//      Serial.print(checkNextLinkIndex);

        if(nextLinkIndex < currentChain->numberOfLinks)
        {

            Serial.print("  another link available in chain");

    //      if yes:
    //          go to next link
            currentLinkIndex = nextLinkIndex;
            currentLink = currentChain->links[currentLinkIndex];
            currentLink->resetPlayCount();
        } else {

            Serial.print("  no more links in chain");

    //      if no:
    //          check chain play count to see if next chain is due
            if(currentChainPlayCount < currentChain->timesToPlay)
            {

                Serial.print("  currentChain still going");

    //          if not:
    //              increment chain play count
    //              go to first link in chain
                currentChainPlayCount++;
                currentLinkIndex = 0;
                currentLink = currentChain->links[0];
                currentLink->resetPlayCount();

            } else {

                Serial.print("  next chain due");

    //          if next chain is due:
                byte theNextChain = currentChain->nextChain;
                if(theNextChain < 255)
                {

                    Serial.print("  Next chain is due");

    //              if next chain available: increment chain, set up first link
                    currentChain = &chains[theNextChain];
                    if(currentChain != NULL)
                    {

                        Serial.print("  Readying next chain");

                        currentChainPlayCount = 1;
                        currentLinkIndex = 0;
                        currentLink = currentChain->links[0];
                        currentLink->resetPlayCount();

                    } else {

                        Serial.print("  currentChain NULL error");

                        inout.ShowErrorOnLCD("PCH:uLOCIN NcC NULL");
                        stopPlaybackCb();
                        return false;
                    }
                } else {

                    Serial.println("");
                    Serial.print("  Stopping playback");

    //              if no more chains available: stop play
                    stopPlaybackCb();
                }
            }
        }

        Serial.println("  updateLinkOrChainIfNeeded done.");


        return true;

    } else {       

        // increment the link play count
        currentLink->incrementLinkPlayCount();

        Serial.println("  updateLinkOrChainIfNeeded done. NOT yet time for next link.");


        return false;
    }
};

void PatternChainHandler::updateChainAndLinkDisplay()
{
    inout.ShowChainLinkOnLCD(currentChainIndex,
                            currentChainPlayCount,
                            currentChain->timesToPlay,
                            currentLinkIndex, 
                            currentLink->getCurrentPlayCount(), 
                            currentLink->getTimesToPlay());
}

void PatternChainHandler::playCurrentChainLink()
{

    Serial.println("playCurrentChainLink");


    if(currentLink != NULL)
        currentLink->primeLinktoPlay();
    else
        inout.ShowErrorOnLCD("PCH:pCCL cL NULL");
}

void PatternChainHandler::reset()
{
    currentChain = &chains[0];

    if(currentChain != NULL)
    {
        currentChainPlayCount = 1;
        currentLink = currentChain->links[0];
        currentLinkIndex = 0;
        currentLink->resetPlayCount();                      
    } else
        inout.ShowErrorOnLCD("PCH:reset cC NULL");
}

bool PatternChainHandler::timeForNextChain()
{
    bool retVal = false;

    if(currentChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH tFNCh curCh NULL");
        return false;
    }

    if(currentChainPlayCount == currentChain->timesToPlay)
        retVal = true;
    else
        if(currentChainPlayCount > currentChain->timesToPlay)
        {
            retVal = true;
            inout.ShowErrorOnLCD("PCH tFNCh curCPC >");
        }

    return retVal;
};