#define DEBUG true

#include "Track.h"
#include "InOutHelper.h"
//#include "typeinfo.h"

extern InOutHelper inout;
Track::Track()
{

};

void Track::begin(byte number)
{
//    trackType;
//    instrument;
      trackNumber = number;
}

void Track::begin(StepPattern patternsPtr[], StepPattern rootPatternsPtr[], byte patternsCount, byte number)
{
      trackType = STEPPATTERN;
      patterns = patternsPtr;
      rootPatterns = rootPatternsPtr;
      maxPatternIndex = patternsCount - 1;
      b_isMuted = false;
      trackNumber = number;
      currentPatternIndex = 0;
      currentPattern = &patterns[0];
      trackPath.setPath(currentPattern->getPath());
}


// Core Methods

note Track::getTrackNoteParams(int step)
{
      return getTrackNoteParams(step, currentPatternIndex);
}

note Track::getTrackNoteParams(int step, byte curPattern)
{
      note retNote;
      note retrievedNote;

      switch (trackType)
      {
            case STEPPATTERN:
                  if(curPattern < maxPatternIndex)
                        if(&patterns[curPattern] != NULL)
//                      if(typeid(patterns[curPattern]) == typeid(retNote))
                              retrievedNote = patterns[curPattern].getPatternNoteParams(step, trackPath);
                        else
                        {
                              inout.ShowErrorOnLCD("gNP1 s[cS] FAIL");
                              break;
                        }
                  else
                        if(&patterns[maxPatternIndex] != NULL)
                              retrievedNote = patterns[maxPatternIndex].getPatternNoteParams(step, trackPath);
                        else
                        {
                              inout.ShowErrorOnLCD("gNP1 s[mSI] NULL");
                              break;
                        }
                  break;
            case SIMPLEBEAT:
                  break;
            default:
                  inout.ShowErrorOnLCD("gNP1 trackTpe DEFAULT");
//                Serial.println("trackType DEFAULTED");
                  break;
      }
#ifdef DEBUG
      Serial.print("Track::getNoteParams1 retrievedNote pitch is ");
//    Serial.print((unsigned int) &retrievedNote);
      Serial.print(retrievedNote.pitchVal);
      Serial.print(" swingTicks ");
      Serial.print(retrievedNote.swingTicks);
      Serial.print(" for step ");
      Serial.print(step);
      Serial.print(" of pattern ");
      Serial.print(curPattern);
      Serial.print(" nn track ");
      Serial.println(trackNumber);
#endif

      // this may be completely redundant ?
      if(&retrievedNote != NULL)
            if(retrievedNote.notEmpty)
                  memcpy(&retNote, &retrievedNote, sizeof(note));
            else
                  inout.ShowErrorOnLCD("gNP1 Note EMPTY");
      else
            inout.ShowErrorOnLCD("gNP1 NULL");
      return retNote;
}

note Track::getTrackNoteParams()
{
      note retNote;
      note retrievedNote;

      switch (trackType)
      {
            case STEPPATTERN:
                  if(currentPattern != NULL)
                        retrievedNote = currentPattern->getPatternNoteParams(playbackStep, trackPath);
                  else
                        inout.ShowErrorOnLCD("gNP2 cS NULL");
                  break;
            case SIMPLEBEAT:
                  break;
            default:
                  inout.ShowErrorOnLCD("gNP2 trckTpe DEF");
//                Serial.println("trackType DEFAULTED");
                  break;
      }
#ifdef DEBUG
      Serial.print("Track::getNoteParams2 retrievedNote pitch is ");
//    Serial.print((unsigned int) &retrievedNote);
      Serial.print(retrievedNote.pitchVal);
      Serial.print(" notEmpty ");
      Serial.print(retrievedNote.notEmpty);
      Serial.print(" swingTicks ");
      Serial.print(retrievedNote.swingTicks);
      Serial.print(" for step ");
      Serial.print(playbackStep);
      Serial.print(" of pattern ");
      Serial.print(currentPatternIndex);
      Serial.print(" nn track ");
      Serial.println(trackNumber);
#endif

      // this may be completely redundant ?
      if(&retrievedNote != NULL)
            if(retrievedNote.notEmpty)
                  memcpy(&retNote, &retrievedNote, sizeof(note));
            else
                  inout.ShowErrorOnLCD("gNP2 Note EMPTY");
      else
            inout.ShowErrorOnLCD("gNP2 NULL");

      return retNote;
}

void Track::unMute()
{
      b_isMuted = false;

#ifdef DEBUG
      Serial.print("Track ");
      Serial.print(trackNumber);
      Serial.println(" unmuted.");
#endif
}

void Track::mute()
{
      b_isMuted = true;

#ifdef DEBUG
      Serial.print("Track ");
      Serial.print(trackNumber);
      Serial.println(" muted.");
#endif
}

// Helpers


// Setters
void Track::setName(char *namePar)
{
  
}


bool Track::setCurrentPatternIndex(byte newIndex)
{
      bool success = false;
      if( newIndex <= maxPatternIndex)
      {
            currentPatternIndex = newIndex;
            currentPattern = &patterns[newIndex];
            if(currentPattern == NULL)
            {
                  inout.ShowErrorOnLCD("setCSI cS NULL");
                  return success;
            }
            trackPath.setPath(currentPattern->getPath());
            success = true;
      }
      return success;
}

void Track::advanceStepPosition()
{
      if(currentPattern != NULL)
      {
            prevPlaybackStep = playbackStep;
            playbackStep = trackPath.getAndAdvanceStepPos(currentPattern->getLength());
      } else
            inout.ShowErrorOnLCD("gNoteP cS NULL");
}

void Track::resetStepPosition()
{
      playbackStep = 0;
      trackPath.resetStep();
}

void Track::prepFirstStep()
{

      if(currentPattern != NULL)
      {
            byte seqLength = currentPattern->getLength(); // truncate step to available pattern length
            if(playbackStep >= seqLength)
            {
                  inout.ShowErrorOnLCD("YOWZA prepFirstStep");
                  Serial.print("step:");
                  Serial.print(playbackStep);
                  Serial.print(" len:");
                  Serial.print(seqLength);
                  Serial.print("on track ");
                  Serial.print(trackNumber);

                  playbackStep = 0;
            }

            if (playbackStep == 0)
                  playbackStep = trackPath.getDontAdvanceStepPos(seqLength);
            else
                  playbackStep = trackPath.getAndAdvanceStepPos(seqLength);
      } else
            inout.ShowErrorOnLCD("prepFS cS NULL");
}


void Track::setPath(byte path)
{
      if(currentPattern != NULL)
      {
            currentPattern->setPath(path);
            trackPath.setPath(path);
      } else 
            inout.ShowErrorOnLCD("setPath cS NULL");
}

void Track::updatePath()
{
      if(currentPattern != NULL)
      {
            trackPath.setPath(currentPattern->getPath());
      } else 
            inout.ShowErrorOnLCD("updatePath cS NULL");
}

StepPattern* Track::getCurrentPatternRef()
{
      if(currentPattern != NULL)
            return currentPattern;
      else 
      {
            Serial.print("getCurrentPatternRef fail on track ");
            Serial.println(trackNumber);
            return &patterns[0];
      }
}

StepPattern* Track::getPatternRef(int index)
{
      StepPattern* retVal = NULL;
      if(index <= maxPatternIndex)
      {
            if(patterns != NULL)
                  retVal = &patterns[index];
            else{
                  Serial.print("getPatternRef fail on track ");
                  Serial.println(trackNumber);
            }
      } else {
                  Serial.print("getPatternRef index ");
                  Serial.print(index);
                  Serial.print(" out of range on track ");
                  Serial.println(trackNumber);
      }
      return retVal;
}

StepPattern* Track::getRootPatternRef(int index)
{
      StepPattern* retVal = NULL;
      if(index <= maxPatternIndex)
      {
            if(rootPatterns != NULL)
                  retVal = &rootPatterns[index];
            else{
                  Serial.print("getRootPatternRef fail on track ");
                  Serial.println(trackNumber);
            }
      } else {
                  Serial.print("getRootPatternRef index ");
                  Serial.print(index);
                  Serial.print(" out of range on track ");
                  Serial.println(trackNumber);
      }
      return retVal;
}

// Getters
char* Track::getName()
{
  return trackName;
}

byte Track::getNumber()
{
      return trackNumber;
}

byte Track::getCurrentPatternIndex()
{
      return currentPatternIndex;
}

byte Track::getPrevPlaybackStep()
{
      return prevPlaybackStep;
}

byte Track::getMaxPatternIndex()
{
      return maxPatternIndex;
}

const char* Track::getPathName()
{
      return trackPath.getPathName();
}

int Track::getPathNumber()
{
      return trackPath.getCurrentPathNumber();
}

void Track::setRecallBufferActive(bool trueOrFalse)
{
      b_recallBufferIsActive = trueOrFalse;
}

bool Track::recallBufferIsActive()
{
      return b_recallBufferIsActive;
}

bool Track::isMuted()
{
      return b_isMuted;
}

bool Track::isTrackPatternRollOverStep()
{
      bool retVal = false;

      if(currentPattern != NULL)
            retVal = trackPath.detectPatternRollover(currentPattern->getLength());
      else
            inout.ShowErrorOnLCD("iTPROS cS NULL");

      return retVal;
}

