#include "PatternChainLink.h"
#include "PatternChainHandler.h"
#include "InOutHelper.h"
#include "StepSequencer.h"

extern InOutHelper inout;
extern StepSequencer sequencer;
extern int currentMode;

PatternChainLink::PatternChainLink()
{

};


void PatternChainLink::begin()
{
    for(int f = 1; f <= TRACKCOUNT; f++)
    {
        link.trackUsedInLink[f] = false;
        link.mutePerTrack[f] = false;
        link.patternPerTrack[f] = 255;
    }
    link.nextLinkIndex = 255;
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

void PatternChainLink::setNextLinkIndex(byte linkNum)
{
    link.nextLinkIndex = linkNum;
};

void PatternChainLink::resetPlayCount()
{
    currentPlayCount = 0;
};

// getters
byte PatternChainLink::getNextLinkIndex()
{
    return link.nextLinkIndex;
};

// to use during play
bool PatternChainLink::timeForNextLinkIndex()
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

void PatternChainLink::playLink()
{
    byte leadTrack = sequencer.getCurrentTrack();

    Serial.print("playLink: ");

    if( link.speedMult != UNDEFINED)
        PatternChainHandler::updateSpeedMultiplierCb(link.speedMult);

    for(int f = 1; f <= TRACKCOUNT; f++)
    {
        // only deal with link tracks, leave others alone (good idea? mute?)
        if( link.trackUsedInLink[f])
        {

            Serial.print("track ");
            Serial.print(f);


            if( link.patternPerTrack[f] == 255) {
                // TODO: This here is an error condition !
            } else {

                if( link.leadTrack)
                {
                    leadTrack = f;
                    // TODO: what else is so special here ??


                    Serial.print(" leadTrack ");

                }
                
                // PATTERNCHANGE


                Serial.print("  seq ");
                Serial.print(link.patternPerTrack[f]);


                if(link.mutePerTrack[f])
                    Serial.println("  muted ");
                else
                    Serial.println("  unmuted ");

                


                sequencer.setCurrentTrack(f);
                PatternChainHandler::updateSequenceNumberCb(link.patternPerTrack[f]);

                if(currentMode == pattern_select)
                    inout.simpleIndicatorModeTrellisButtonPressed
                            (link.patternPerTrack[f] + STEPSOFFSET);

                // if(currentMode == path_select)
                //    inout.pathModeTrellisButtonPressed
                //          ((int)link.pathPerTrack[f] + STEPSOFFSET);

                // if(currentMode == length_edit)
                //    inout.lengthModeTrellisIndicatorUpdate() //...nonexistent...

                // but all the other modes...
                

                // TRACKMUTECHANGE
                sequencer.setTrackMute(f, link.mutePerTrack[f]);

                if(currentMode == track_mute)
                    inout.trackMuteTrellisButtonPressed(f - 1 + STEPSOFFSET);
            }
        } else {

            Serial.print("  track ");
            Serial.print(f);
            Serial.print(" unused  ");

        }
    }


    Serial.println("...");

    sequencer.setCurrentTrack(leadTrack);
}