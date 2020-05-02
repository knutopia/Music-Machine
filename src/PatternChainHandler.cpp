#include "PatternChainHandler.h"
#include "InOutHelper.h"
#include "StepSequencer.h"
#include "SDjson.h"

#define DEBUG true

extern InOutHelper inout;
extern SDjsonHandler jsonHandler;

byte PatternChainHandler::currentLeadTrack;
intFunc PatternChainHandler::updatePatternNumberCb;
speedFactorFunc PatternChainHandler::updateSpeedMultiplierCb;

PatternChainHandler::PatternChainHandler()
{
    currentChain = NULL;
    currentChainIndex = 0;
    currentLink = NULL;
    currentLinkIndex = 0;
    currentChainPlayCount = 0;
    currentLeadTrack = 1;
    m_b_reset_encoder_reference = true;
    m_button_pressed_num = 0;
    m_button_press_time = 0;    
}

PatternChainHandler::~PatternChainHandler()
{
    // remove the links
    for(int f = 0; f < MAXCHAINCOUNT; f++)
        for(int l = 0; l < chains[f].numberOfLinks; l++)
            if( chains[f].links[l] != NULL)
                delete chains[f].links[l];
}

void PatternChainHandler::begin(simpleFunc stopCbPointer,
                                intFunc changePatternNumberCbPointer,
                                speedFactorFunc changeSpeedMultiplierCbPointer)
{
    stopPlaybackCb = stopCbPointer;
    updatePatternNumberCb = changePatternNumberCbPointer;
    updateSpeedMultiplierCb = changeSpeedMultiplierCbPointer;

    for(int f = 0; f < MAXCHAINCOUNT; f++)
    {
        chains[f].nextChain = 255;
        chains[f].numberOfLinks = 0;
        chains[f].timesToPlay = 0;
        // leaving chains[f].links uninitialized
    }
    
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
    setNextChain(0, 1);

    setCurrentChain(1);
    setTimesToPlay(2);
    
    newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 3, false);
        newLink->addTrackPatterntoLink(2, 3, false);
        newLink->setLeadTrack(1);
        newLink->setTimesToPlay(2);
    }

    newLink = appendLink();
    if(newLink != NULL)
    {
        newLink->addTrackPatterntoLink(1, 4, false);
        newLink->addTrackPatterntoLink(2, 4, false);
        newLink->setLeadTrack(1);
        newLink->setTimesToPlay(2);
    }

    reset();

//    jsonHandler.saveChains();

//    jsonSetup();


}

// setters
bool PatternChainHandler::setCurrentChain(byte index)
{
    currentChainIndex = index;
    if(chains == NULL)
    {
        inout.ShowErrorOnLCD("PCH:sCC:chains NULL");
        return false;
    }

    currentChain = &chains[currentChainIndex];

    if (currentChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH:sCC:curCh NULL");
        return false;
    }
    return true;
}

bool PatternChainHandler::setCurrentLink(byte index)
{
    if (currentChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH:sCL:curCh NULL");
        return false;
    }

    if (index > currentChain->numberOfLinks - 1)
        index = 0;

    currentLink = currentChain->links[index];
    if (currentLink == NULL)
    {
        inout.ShowErrorOnLCD("PCH:sCL:curLi NULL");
        return false;
    }    

    currentLinkIndex = index;
    return true;
}

bool PatternChainHandler::setNextChain(byte chainIndex, byte nextIndex)
{
    if(chains == NULL)
    {
        inout.ShowErrorOnLCD("PCH:sNC:chains NULL");
        return false;
    }

    Chain* setChain = &chains[chainIndex];
    if(setChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH:sNC:[sC] NULL");
        return false;
    }

    Chain* nextChain = &chains[nextIndex];
    if(nextChain == NULL)
    {
        inout.ShowErrorOnLCD("PCH:sNC:[nC] NULL");
        return false;
    }

    setChain->nextChain = nextIndex;
    return true;
}

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
}

// for editing...
void PatternChainHandler::selectLink(byte linkNum)
{

}

PatternChainLink* PatternChainHandler::appendLink()
{
    if (currentChain == NULL && (!setCurrentChain(0)))
        return NULL;

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
}

bool PatternChainHandler::removeLink()
{
    return true;
}

void PatternChainHandler::clearLink()
{

}

// management during play

void PatternChainHandler::startChainPlay()
{



    jsonHandler.loadChains();




    Serial.println("startChainPlay");

    if(currentLink != NULL)
        currentLink->primeLinktoPlay();
    else {
        inout.ShowErrorOnLCD("PCH: sCP cL NULL");
        return;
    }

    inout.ShowChainLinkPlayOnLCD(currentChainIndex,
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
}

void PatternChainHandler::updateChainAndLinkDisplay()
{
    inout.ShowChainLinkPlayOnLCD(currentChainIndex,
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
    currentChainIndex = 0;

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
}

void PatternChainHandler::prepPatternChainForEdit()
{
#ifdef DEBUG
        Serial.println("prepPatternChainForEdit");
#endif

/*
    inout.ShowChainLinkPlayOnLCD(currentChainIndex,
                            currentChainPlayCount,
                            currentChain->timesToPlay,
                            currentLinkIndex, 
                            currentLink->getCurrentPlayCount(), 
                            currentLink->getTimesToPlay());
*/
        currentEditParamIndex = 0;
        m_b_reset_encoder_reference = true;
//      inout.ShowValueInfoOnLCD("Chain:", 1);
        showEditParam();
}

void PatternChainHandler::showEditParam()
{
        int i_param_use = USEINTPARAM;
        int i_param_val = 0;
        char* s_param_val = "                    ";

        const char* param_name = EditOptionNames[currentEditParamIndex];

        switch (currentEditParamIndex)
        {
            case CurrentChain:
            {
                i_param_val = currentChainIndex + 1;
                break;
            }
            case CurrentLink:
            {

                i_param_val = currentLinkIndex + 1;
                break;
            }
            case ChainTimesToPlay:
            {
                i_param_val = chains[currentChainIndex].timesToPlay;
                break;
            }
            case ChainContent:
            {
                i_param_use = USECHARPARAM;
                s_param_val = "(...)";
                break;
            }
            case LinkContent:
            {
                i_param_use = USECHARPARAM;
                s_param_val = "(...)";
                break;
            }
            case LeadTrack:
            {
                i_param_val = currentLink->getLeadTrack();
                break;
            }
            case TimesToPlay:
            {
                i_param_val = currentLink->getTimesToPlay();
                break;
            }
            case SpeedFactor:
            {
                i_param_use = USECHARPARAM;
                int foo = currentLink->getSpeedMult();
                switch (foo)
                {
                    case UNDEFINED:
                    {
                        s_param_val = "Off";
                        break;
                    }
                    case NORMAL:
                    {
                        s_param_val = "Normal";
                        break;
                    }
                    case DOUBLE:
                    {
                        s_param_val = "Double";
                        break;
                    }
                    case TRIPLE:
                    {
                        s_param_val = "Triple";
                        break;
                    }
                    case QUAD:
                    {
                        s_param_val = "Quad";
                        break;
                    }
                }
                break;
            }
            case LengthOverride:
            {
                int foo = currentLink->getLengthOverride();
                if (foo == OVERRIDEINACTIVE)
                {
                    i_param_use = USECHARPARAM;
                    s_param_val = "Off";
                } else {
                    i_param_val = foo;
                }
                break;
            }
            case PathOverride:
            {
                int foo = currentLink->getPathOverride();
                if (foo == OVERRIDEINACTIVE)
                {
                    i_param_use = USECHARPARAM;
                    s_param_val = "Off";
                } else
                    i_param_val = foo + 1;
                break;
            }
            default:
            {
                i_param_use = UNUSED;
                break;                
            }
        }

        if(i_param_use == UNUSED)
            inout.ShowActionOnLCD(param_name);
        else
            if(i_param_use == USECHARPARAM) {
                inout.ShowParamOnLCD(param_name, s_param_val);
#ifdef DEBUG
                Serial.print(param_name);
                Serial.println(s_param_val);
#endif
            } else {
                inout.ShowParamOnLCD(param_name, i_param_val);
#ifdef DEBUG
                Serial.print(param_name);
                Serial.println(i_param_val);
#endif
            }
/*
CurrentChain, CurrentLink, ChainTimesToPlay, ChainContent
 PreviousChain, NextChain, OverrideThisLink, InsertAfterCurrent
 AppendtoChain, StartNewChain, DeleteThisLink, DuplicateLink
 CopyLink, PasteLink
LinkContent, LeadTrack, TimesToPlay, SpeedFactor, LengthOverride, PathOverride
 PreviousLink, NextLink
*/
}

int PatternChainHandler::captureEditParamStartVal()
{
        int retVal = 0;
        
        switch (currentEditParamIndex)
        {
        case CurrentChain:
            retVal = currentChainIndex;
            break;
        
        case CurrentLink:
            retVal = currentLinkIndex;
            break;
        
        case ChainTimesToPlay:
            retVal = chains[currentChainIndex].timesToPlay;
            break;
        
        case LeadTrack:
            retVal = currentLink->getLeadTrack();
            break;
        
        case TimesToPlay:
            retVal = currentLink->getTimesToPlay();
            break;
        
        case SpeedFactor:
            retVal = currentLink->getSpeedMult();
            break;
        
        case LengthOverride:
            retVal = currentLink->getLengthOverride();
            if (retVal == OVERRIDEINACTIVE)
                retVal = 17;
            Serial.print(" getLengthOverride ");
            Serial.print(retVal);
            Serial.print(" ");
            break;

        case PathOverride:
            retVal = currentLink->getPathOverride();
            if (retVal == OVERRIDEINACTIVE)
                retVal = 16;
            Serial.print(" getPathOverride ");
            Serial.print(retVal);
            Serial.print(" ");
            break;
        
        default:
            break;
        }
        return retVal;
}

void PatternChainHandler::editParam(int value)
{
#ifdef DEBUG
        Serial.print("#editParam value ");
        Serial.println(value);
#endif

        switch (currentEditParamIndex)
        {
            case CurrentChain:
            {
                value = putInRange(value, MAXCHAINCOUNT, 0);
                setCurrentChain(value);
                break;
            }
            case CurrentLink:
            {
                value = putInRange(value, currentChain->numberOfLinks, 0);
                setCurrentLink(value);
                break;
            }
            case ChainTimesToPlay:
            {
                value = putInRange(value, 16, 1);
                setTimesToPlay(value);
                break;
            }
            case LeadTrack: // TODO: check included tracks here...
            {
                bool done = false;
                int sentry = 0;
                do {
                    value = putInRange(value, TRACKCOUNT, 1);
                    if (currentLink->isTrackUsedInLink(value))
                        if (currentLink->setLeadTrack(value))
                            done = true;
                        else {
                            inout.ShowErrorOnLCD("PCH:eP LT fail", value);
                            done = true;
                        }
                    else
                    {
                        value++;
                        sentry++;
                        if (sentry > TRACKCOUNT) {
                            inout.ShowErrorOnLCD("PCH:eP LT fail2", value);
                            done = true;
                        }
                    }
                } while (!done);
                break;
            }
            case TimesToPlay:
            {
                value = putInRange(value, 16, 1);
                currentLink->setTimesToPlay(value);
                break;
            }
            case SpeedFactor:
            {
                value = putInRange(value, (int)(speedFactor::MAX )+ 1, 0);
                currentLink->setSpeedMult(value);
                break;
            }
            case LengthOverride:
            {
                value = putInRange(value, 17, 1); //SHIFT DISPLAY PARAM
                if (value < 17)
                    currentLink->setLengthOverride(value);
                else
                    currentLink->setLengthOverride(OVERRIDEINACTIVE);
                
                break;
            }
            case PathOverride:
            {   
                value = putInRange(value, 17, 0);
                if (value <16)
                    currentLink->setPathOverride(value);
                else
                    currentLink->setPathOverride(OVERRIDEINACTIVE);
                    
                break;
            }
            default:
            {
                inout.ShowErrorOnLCD("PCH:eP Default ", value);
                break;                
            }
        }
}

void PatternChainHandler::handleSelectButton()
{
#ifdef DEBUG
        Serial.println("PCH:handleSelectButton");
#endif

}

void PatternChainHandler::handleEncoder(int encoder, int value)
{
#ifdef DEBUG
        Serial.println("PCH:handleEncoder");
#endif

        if(encoder == EncoderA) {
            static int referenceParamIndex;
            
            if (m_b_reset_encoder_reference) {
                m_b_reset_encoder_reference = false;
                referenceParamIndex = currentEditParamIndex;
            }

            switch (m_edit_state) {
                case ParamChoice:
                        // cycle through values
                        currentEditParamIndex = putInRange(value, PatternChainEditOptionsCount, 0);
                        showEditParam();
                        break;
                case ParamEdit:
                        Serial.print("Before edit: ");
                        Serial.print(value);
                        Serial.print(" ");
                        Serial.println(editParamStartVal);

                        editParam(value + editParamStartVal);

                        Serial.print("After edit: ");
                        Serial.print(value);
                        Serial.print(" ");
                        Serial.println(editParamStartVal);
                        
                        showEditParam();
                        break;
                default:
                        break;
            }
        }
}

//handle trellis buttons to choose chain or link
void PatternChainHandler::handleButton(int butNum)
{
    // TODO: also do trellis button shortcuts here
    // TODO: also do trellis button timing & release as save shortcut
    // TODO: also do encoder button
    switch (butNum) {
      case EncoderA:
        if (m_edit_state == ParamChoice) {
          m_edit_state = ParamEdit;
          editParamStartVal = captureEditParamStartVal();
        } else {
          m_edit_state = ParamChoice;
          m_b_reset_encoder_reference = true; // ?????? TODO???
        }
        break;
      default:  
        m_button_pressed_num = butNum;
        m_button_press_time = millis();
        break;
    }
}

//utility for encoder values, copied from SynthEngine, plus inclusion of iMin
int PatternChainHandler::putInRange(int iVar, int iRange, int iMin)
{
    int retval;
    int orig = iVar;

    if (iVar < iMin)
        while(iVar < 0) iVar += (iRange);
    retval = iVar % iRange;
    if (retval < iMin)
        retval += (iRange);

    Serial.print("putInRange ");
    Serial.print(orig);
    Serial.print(" ");
    Serial.println(retval);

    return retval;
}