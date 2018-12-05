#include "PatternChainLink.h"
#include "InOutHelper.h"

extern InOutHelper inout;

PatternChainLink::PatternChainLink()
{

};

void PatternChainLink::begin()
{
    for(int f = 0; f < TRACKCOUNT; f++)
    {
        link.trackUsedInLink[f] = false;
        link.mutePerTrack[f] = false;
        link.patternPerTrack[f] = 255;
    }
    link.nextLink = 255;
    link.timesToPlay = 0;
};

void PatternChainLink::addTrackPatterntoLink(byte trackNum, byte patNum, bool muteIt)
{
    link.trackUsedInLink[trackNum] = true;
    link.mutePerTrack[trackNum] = muteIt;
    link.patternPerTrack[trackNum] = patNum;
};

// setters
void PatternChainLink::setTimesToPlay(byte times)
{
    link.timesToPlay = times;
};

bool PatternChainLink::setLeadTrack(byte trackNum)
{
    bool success = false;

    if(link.trackUsedInLink[trackNum])
    {
        success = true;
        link.leadTrack = trackNum;
    }
    return success;
};

void PatternChainLink::setNextLink(byte linkNum)
{
    link.nextLink = linkNum;
};

// to use during play
bool PatternChainLink::timeForNextLink()
{
    bool retVal = false;

    if(currentPlayCount == link.timesToPlay)
        retVal = true;

    if(currentPlayCount > link.timesToPlay)
    {
        retVal = true;

        inout.ShowErrorOnLCD("PCL tFNL count err ");
        Serial.print("currentPlayCount: ");
        Serial.print(currentPlayCount);
        Serial.print("  link.timesToPlay: ");
        Serial.println(link.timesToPlay);
    }
    return retVal;
};

void PatternChainLink::incrementLinkPlayCount()
{
    currentPlayCount++;
};