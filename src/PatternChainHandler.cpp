#include "PatternChainHandler.h"
#include "InOutHelper.h"
#include "StepSequencer.h"

extern InOutHelper inout;
extern StepSequencer sequencer;
extern int currentMode;

byte PatternChainHandler::currentLeadTrack;

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
                                intFunc changePatternLengthCbPointer,
                                speedFactorFunc changeSpeedMultiplierCbPointer)
{
    stopPlaybackCb = stopCbPointer;
    updateSequenceNumberCb = changeSequenceNumberCbPointer;
    updatePatternLengthCb = changePatternLengthCbPointer;
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
        newLink->addTrackPatterntoLink(1, 1, false);
        newLink->addTrackPatterntoLink(2, 1, false);
        newLink->setLeadTrack(1);
        newLink->setTimesToPlay(1);
    }

    newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 2, false);
        newLink->addTrackPatterntoLink(2, 2, false);
        newLink->setLeadTrack(1);
        newLink->setTimesToPlay(1);
    }

    newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 3, false);
        newLink->addTrackPatterntoLink(2, 3, false);
        newLink->setLeadTrack(1);
        newLink->setTimesToPlay(1);
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
    inout.ShowChainLinkOnLCD(currentChainIndex, currentLinkIndex);
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
    
    // increment the link play count
    currentLink->incrementLinkPlayCount();

    // check link play count to see if next link is due
    if(currentLink->timeForNextLinkIndex())
    {

        Serial.print("yo 1");

    // if it is:
    //      check if more links in chain
        byte nextLinkIndex = currentLink->getNextLinkIndex();
//      byte checkNextLinkIndex = currentLinkIndex+1; // #######################

        Serial.print(" nextLinkIndex ");
        Serial.print(nextLinkIndex);
//      Serial.print(" checkNextLinkIndex ");
//      Serial.print(checkNextLinkIndex);

        if(nextLinkIndex < currentChain->numberOfLinks)
        {

            Serial.print("  yo 2");

    //      if yes:
    //          go to next link
            currentLinkIndex = nextLinkIndex;
            currentLink = currentChain->links[currentLinkIndex];
            currentLink->resetPlayCount();
        } else {

            Serial.print("  yo 3");

    //      if no:
    //          check chain play count to see if next chain is due
            if(currentChainPlayCount < currentChain->timesToPlay)
            {

                Serial.print("  yo 4");

    //          if not:
    //              increment chain play count
    //              go to first link in chain
                currentChainPlayCount++;
                currentLinkIndex = 0;
                currentLink = currentChain->links[0];
                currentLink->resetPlayCount();
            } else {

                Serial.print("  yo 5");

    //          if next chain is due:
                byte theNextChain = currentChain->nextChain;
                if(theNextChain < 255)
                {

                    Serial.print("  yo 6");

    //              if next chain available: increment chain, set up first link
                    currentChain = &chains[theNextChain];
                    if(currentChain != NULL)
                    {

                        Serial.print("  yo 7");

                        currentChainPlayCount = 1;
                        currentLinkIndex = 0;
                        currentLink = currentChain->links[0];
                        currentLink->resetPlayCount();                      
                    } else {

                        Serial.print("  yo 8");

                        inout.ShowErrorOnLCD("PCH:uLOCIN NcC NULL");
                        stopPlaybackCb();
                        return false;
                    }
                } else {

                    Serial.print("  yo 9");

    //              if no more chains available: stop play
                    stopPlaybackCb();
                }
            }
        }

        Serial.println(" done.");

        inout.ShowChainLinkOnLCD(currentChainIndex, currentLinkIndex);
        return true;

    } else {       

        Serial.println(" nothing done.");

        return false;
    }
};

void PatternChainHandler::playCurrentChainLink()
{
    Serial.print("rQA");

    byte bufTrack = sequencer.getCurrentTrack();
    String out = "";

    bool done = false;
    while( !done)
    {
//      done = !queuedActions.advanceRetrievalIndex();

        switch (qAction)
        {
            case PATTERNCHANGE:

                Serial.print("PATTERNCHANGE ");
                Serial.println(qAction);

                out.append("Pt");
                out.append(qAction);
                out.append(" ");

                sequencer.setCurrentTrack(qTrack);
                updateSequenceNumberCb(qParam);
                if(currentMode == pattern_select)
                    inout.simpleIndicatorModeTrellisButtonPressed(qParam + STEPSOFFSET);
                break;

            case PATHCHANGE:

                Serial.print("PATHCHANGE ");
                Serial.println(qAction);

                out.append("Ph");
                out.append(qAction);
                out.append(" ");
                
                sequencer.setCurrentTrack(qTrack);
                inout.pathModeTrellisButtonPressed((int)qParam + STEPSOFFSET);
                break;

            case LENGTHCHANGE:

                Serial.print("LENGTHCHANGE ");
                Serial.println(qAction);

                out.append("L");
                out.append(qAction);
                out.append(" ");
                
                sequencer.setCurrentTrack(qTrack);
                updatePatternLengthCb(qParam);
                break;

            case TRACKMUTECHANGE:

                Serial.print("TRACKMUTECHANGE ");
                Serial.println(qAction);

                out.append("Tm");
                out.append(qAction);
                out.append(" ");
                
                sequencer.setCurrentTrack(qTrack);
                sequencer.setTrackMute(qTrack, (bool)qParam);

                if(currentMode == track_mute)
                    inout.trackMuteTrellisButtonPressed(qTrack - 1 + STEPSOFFSET);
                break;

            case SPEEDMULTIPLIERCHANGE:

                Serial.print("SPEEDMULTIPLIERCHANGE ");
                Serial.println(qAction);

                out.append("Sp");
                out.append(qAction);
                out.append(" ");
                
                sequencer.setCurrentTrack(qTrack);
                updateSpeedMultiplierCb((speedFactor)qParam);
                break;

            case SYNCTRACKS:
                // TODO
                break;

            case NOACTION:
                done = true;
                break;

            default:
                inout.ShowErrorOnLCD("runQAcs out of rnge");
                Serial.print("runQueuedActions qAction ");
                Serial.println(qAction);
                done = true;
        }
    }
    sequencer.setCurrentTrack(bufTrack);
    inout.showQueuedActions(out);
    inout.SetLCDinfoTimeout();
    inout.SetLCDinfoLabelTimeout();
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