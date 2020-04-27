#include "PatternChainLink.h"
#include "PatternChainHandler.h"
#include "InOutHelper.h"
#include "StepSequencer.h"

extern InOutHelper inout;
extern StepSequencer sequencer;
extern int currentMode;

// Using trackNum for track numbers, starting at 1
// Using trackIndex for array indices, starting at 0

PatternChainLink::PatternChainLink()
{
    link.nextLinkIndex = 255;
    link.timesToPlay = 0;
};


void PatternChainLink::begin()
{
    for(int trackIndex = 0; trackIndex < TRACKCOUNT; trackIndex++)
    {
        link.trackUsedInLink[trackIndex] = false;
        link.mutePerTrack[trackIndex] = false;
        link.patternPerTrack[trackIndex] = 255;
    }
};

void PatternChainLink::addTrackPatterntoLink(byte trackNum, byte patNum, bool muteIt)
{
    byte trackIndex = trackNum - 1; // tracks start at 1, array starts at 0...

    link.trackUsedInLink[trackIndex] = true;
    link.mutePerTrack[trackIndex] = muteIt;
    link.patternPerTrack[trackIndex] = patNum;
};

// getters

byte PatternChainLink::getCurrentPlayCount()
{
    return currentPlayCount;
}

byte PatternChainLink::getTimesToPlay()
{
    return link.timesToPlay;
}

// setters

bool PatternChainLink::setLengthOverride(byte length)
{
    bool success = false;

    if(length > 0 && length <= MAXNOTES)
    {
        link.lengthOverride = length;
        success = true;
    } else
        inout.ShowErrorOnLCD("PCL:sLO inval len", length);

    return success;
}

bool PatternChainLink::setPathOverride(int path)
{
    bool success = false;

    if(path < 16)
    {
        link.pathOverride = path;
        success = true;
    } else
        inout.ShowErrorOnLCD("PCL:sPO inval path", path);

    return success;    
}

void PatternChainLink::setTimesToPlay(byte times)
{
    link.timesToPlay = times;
};

bool PatternChainLink::setLeadTrack(byte trackNum)
{
    bool success = false;
    byte trackIndex = trackNum - 1; // tracks start at 1, array starts at 0...

    if(link.trackUsedInLink[trackIndex])
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

void PatternChainLink::primeLinktoPlay()
{
    Serial.println("primeLinktoPlay: ");

    if( link.speedMult != UNDEFINED)
        PatternChainHandler::updateSpeedMultiplierCb(link.speedMult);

    for(int trackNum = 1; trackNum <= TRACKCOUNT; trackNum++)
    {
        int trackIndex = trackNum - 1; // tracks start at 1, array starts at 0...
        // only deal with link tracks, leave others alone (good idea? mute?)
        if( link.trackUsedInLink[trackIndex])
        {

            Serial.print("  track ");
            Serial.print(trackNum);


            if( link.patternPerTrack[trackIndex] == 255) {
                // TODO: This here is an error condition !
            } else {
                

                Serial.print("  seq ");
                Serial.print(link.patternPerTrack[trackIndex]);


                if(link.mutePerTrack[trackIndex])
                    Serial.println("  muted ");
                else
                    Serial.println("  unmuted ");

                // PATTERNCHANGE
                sequencer.setCurrentTrack(trackNum);
                PatternChainHandler::updatePatternNumberCb(link.patternPerTrack[trackIndex]);
                
                // TRACKMUTECHANGE
                sequencer.setTrackMute(trackNum, link.mutePerTrack[trackIndex]);

                // OVERRIDES
                if(trackNum == link.leadTrack)
                {
                    if(link.pathOverride < 255)
                    {
                        sequencer.setPath(link.pathOverride % STEPSOFFSET);
                        inout.ShowPathNumberOnLCD(link.pathOverride % STEPSOFFSET);
                    }
                
                    if(link.lengthOverride < 255)
                    {
                        byte seqMaxLength = sequencer.getMaxLength();  // still redundant MAXLENGTH
                            if (link.lengthOverride <= seqMaxLength)
                                sequencer.setLength(link.lengthOverride);
                            else
                                inout.ShowErrorOnLCD("PCL:PLtP lO RNG", link.lengthOverride);
                    }
                }

                // sync trellis, LCD
                switch(currentMode)
                {
                    case pattern_select:
                    case chain_edit:
                        inout.simpleIndicatorModeTrellisButtonPressed
                            (link.patternPerTrack[trackIndex] + STEPSOFFSET);
                        break;

                    case path_select:
                        inout.pathModeTrellisButtonPressed
                            (sequencer.getPath() + STEPSOFFSET);
                        inout.ShowInfoOnLCD(sequencer.getPathName());
                        inout.SetLCDinfoTimeout();
                        break;

                    case length_edit:
                        inout.simpleIndicatorModeTrellisButtonPressed(sequencer.getLength() - 1 + STEPSOFFSET);
                        break;

                    case track_mute:
                        inout.trackMuteTrellisButtonPressed(trackNum - 1 + STEPSOFFSET);
                        break;

                    default:
                        break;
                    // but all the other modes...
                }
            }
        } else {

            Serial.print("  track ");
            Serial.print(trackNum);
            Serial.print(" unused  ");

        }
    }
    sequencer.setCurrentTrack(link.leadTrack);

    currentPlayCount = 1;


    Serial.println(" primeLinktoPlay done");
    Serial.println("");

}