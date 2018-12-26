#include "PatternChainHandler.h"
#include "InOutHelper.h"

extern InOutHelper inout;

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

void PatternChainHandler::begin(simpleFunc stopCbPointer)
{
    stopPlaybackCb = stopCbPointer;

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

    // set up test chain content

    PatternChainLink* newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 1, false);
        newLink->addTrackPatterntoLink(2, 1, false);
    }

    newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 2, false);
        newLink->addTrackPatterntoLink(2, 2, false);
    }

    newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 3, false);
        newLink->addTrackPatterntoLink(2, 3, false);
    }
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
        PatternChainLink* prevLink = currentChain->links[currentChain->numberOfLinks];
        if (prevLink != NULL)
            prevLink->setNextLinkIndex(currentChain->numberOfLinks + 1);
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
    // if it is:
    //      check if more links in chain
        byte nextLinkIndex = currentLink->getNextLinkIndex();
        if(nextLinkIndex < 255)
        {
    //      if yes:
    //          go to next link
            currentLink = currentChain->links[nextLinkIndex];
            currentLink->resetPlayCount();
        } else {
    //      if no:
    //          check chain play count to see if next chain is due
            if(currentChainPlayCount < currentChain->timesToPlay)
            {
    //          if not:
    //              increment chain play count
    //              go to first link in chain
                currentChainPlayCount++;
                currentLink = currentChain->links[0];
                currentLink->resetPlayCount();
            } else {
    //          if next chain is due:
                byte theNextChain = currentChain->nextChain;
                if(theNextChain < 255)
                {
    //              if next chain available: increment chain, set up first link
                    currentChain = &chains[theNextChain];
                    if(currentChain != NULL)
                    {
                        currentChainPlayCount = 1;
                        currentLink = currentChain->links[0];
                        currentLink->resetPlayCount();                      
                    } else {
                        inout.ShowErrorOnLCD("PCH:uLOCIN NcC NULL");
                        stopPlaybackCb();
                        return false;
                    }
                } else {
    //              if no more chains available: stop play
                    stopPlaybackCb();
                }
            }
        }
    }
};

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