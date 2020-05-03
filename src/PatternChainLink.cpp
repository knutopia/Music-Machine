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
    link.lengthOverride = OVERRIDEINACTIVE;
    link.pathOverride = OVERRIDEINACTIVE;
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

int PatternChainLink::getTimesToPlay()
{
    return link.timesToPlay;
}

byte PatternChainLink::getLeadTrack()
{
    return link.leadTrack;
}

bool PatternChainLink::isTrackUsedInLink(int trackNum)
{
    if(trackNum <= TRACKCOUNT && link.trackUsedInLink[trackNum])
        return true;
    else
        return false;
}

speedFactor PatternChainLink::getSpeedMult()
{
    return link.speedMult;
}

int PatternChainLink::getLengthOverride()
{
    return link.lengthOverride;
}

int PatternChainLink::getPathOverride()
{
    return link.pathOverride;
}

// setters

bool PatternChainLink::setLengthOverride(int length)
{
    bool success = false;

    // -1 = no override active
    if(length > 0 && length <= MAXNOTES || length == OVERRIDEINACTIVE)
    {
        link.lengthOverride = length;
        success = true;

        Serial.print(" C ");
        Serial.print(length);
        Serial.print(" ");

    } else
        inout.ShowErrorOnLCD("PCL:sLO inval len", length);

    return success;
}

bool PatternChainLink::setPathOverride(int path)
{
    bool success = false;

    // -1 = no override active
    if(path < 16 || path == -1)
    {
        link.pathOverride = path;
        success = true;
    } else
        inout.ShowErrorOnLCD("PCL:sPO inval p", path);

    return success;    
}

void PatternChainLink::setTimesToPlay(int times)
{
    link.timesToPlay = times;
}

void PatternChainLink::setSpeedMult(speedFactor times)
{
    link.speedMult = times;
}

void PatternChainLink::setSpeedMult(int times)
{
    switch (times)
    {
    case UNDEFINED:
        link.speedMult = UNDEFINED;
        break;
    
    case NORMAL:
        link.speedMult = NORMAL;
        break;
    
    case DOUBLE:
        link.speedMult = DOUBLE;
        break;
    
    case TRIPLE:
        link.speedMult = TRIPLE;
        break;
    
    case QUAD:
        link.speedMult = QUAD;
        break;
    
    default:
        inout.ShowErrorOnLCD("PCL:sSM inval arg", times);
        break;
    }
}

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
}

void PatternChainLink::setNextLinkIndex(byte linkNum)
{
    link.nextLinkIndex = linkNum;
}

void PatternChainLink::resetPlayCount()
{
    currentPlayCount = 0;
}

// getters
byte PatternChainLink::getNextLinkIndex()
{
    return link.nextLinkIndex;
}

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
}

void PatternChainLink::incrementLinkPlayCount()
{
    currentPlayCount++;
}

void PatternChainLink::primeLinktoPlay()
{
#ifdef DEBUG
    Serial.println("primeLinktoPlay: ");
#endif

    if( link.speedMult != UNDEFINED)
        PatternChainHandler::updateSpeedMultiplierCb(link.speedMult);

    for(int trackNum = 1; trackNum <= TRACKCOUNT; trackNum++)
    {
        int trackIndex = trackNum - 1; // tracks start at 1, array starts at 0...
        // only deal with link tracks, leave others alone (good idea? mute?)
        if( link.trackUsedInLink[trackIndex])
        {
#ifdef DEBUG
            Serial.print("  track ");
            Serial.print(trackNum);
#endif

            if( link.patternPerTrack[trackIndex] == 255) {
                // TODO: This here is an error condition !
            } else {
                
#ifdef DEBUG
                Serial.print("  seq ");
                Serial.print(link.patternPerTrack[trackIndex]);

                if(link.mutePerTrack[trackIndex])
                    Serial.println("  muted ");
                else
                    Serial.println("  unmuted ");
#endif

                // PATTERNCHANGE
                sequencer.setCurrentTrack(trackNum);
                PatternChainHandler::updatePatternNumberCb(link.patternPerTrack[trackIndex]);
                
                // TRACKMUTECHANGE
                sequencer.setTrackMute(trackNum, link.mutePerTrack[trackIndex]);

                // OVERRIDES
                if(trackNum == link.leadTrack)
                {
                    if(link.pathOverride > -1)
                    {
                        sequencer.setPath(link.pathOverride % STEPSOFFSET);
                        inout.ShowPathNumberOnLCD(link.pathOverride % STEPSOFFSET);
                    }
                
                    if(link.lengthOverride > -1)
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
#ifdef DEBUG
            Serial.print("  track ");
            Serial.print(trackNum);
            Serial.print(" unused  ");
#endif
        }
    }
    sequencer.setCurrentTrack(link.leadTrack);

    currentPlayCount = 1;

#ifdef DEBUG
    Serial.println(" primeLinktoPlay done");
    Serial.println("");
#endif
}

void PatternChainLink::printLink()
{
    Serial.print("leadTrack:");
    Serial.print(link.leadTrack);
    Serial.print(" timesToPlay:");
    Serial.print(link.timesToPlay);
    Serial.print(" speedMult:");
    Serial.print(link.speedMult);

    if (link.lengthOverride != OVERRIDEINACTIVE)
    {
        Serial.print( " lengthOverride:");
        Serial.print(link.lengthOverride);
    }
    if (link.pathOverride != OVERRIDEINACTIVE)
    {
        Serial.print( " pathOverride:");
        Serial.print(link.pathOverride);
    }
    Serial.print(" nextLinkIndex:");
    Serial.println(link.nextLinkIndex);
    
    for (int foo = 0; foo < TRACKCOUNT; foo++)
    {
        if(link.trackUsedInLink[foo])
        {
            Serial.print("  track:");
            Serial.print(foo + 1);
            Serial.print(" pattern:");
            Serial.print(link.patternPerTrack[foo] + 1);
            if (link.mutePerTrack[foo])
                Serial.print(" muted");
            Serial.println();
        }
    }
}