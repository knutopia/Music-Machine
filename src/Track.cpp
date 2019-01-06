//#define DEBUG true

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

void Track::begin(StepSequence sequencesPtr[], StepSequence rootSequencesPtr[], byte sequencesCount, byte number)
{
      trackType = STEPSEQUENCE;
      sequences = sequencesPtr;
      rootSequences = rootSequencesPtr;
      maxSequenceIndex = sequencesCount - 1;
      b_isMuted = false;
      trackNumber = number;
      currentSequenceIndex = 0;
      currentSequence = &sequences[0];
      trackPath.setPath(currentSequence->getPath());
}


// Core Methods

note Track::getTrackNoteParams(int step)
{
      return getTrackNoteParams(step, currentSequenceIndex);
}

note Track::getTrackNoteParams(int step, byte curSequence)
{
      note retNote;
      note retrievedNote;

      switch (trackType)
      {
            case STEPSEQUENCE:
                  if(curSequence < maxSequenceIndex)
                        if(&sequences[curSequence] != NULL)
//                      if(typeid(sequences[curSequence]) == typeid(retNote))
                              retrievedNote = sequences[curSequence].getSequenceNoteParams(step, trackPath);
                        else
                        {
                              inout.ShowErrorOnLCD("gNP1 s[cS] FAIL");
                              break;
                        }
                  else
                        if(&sequences[maxSequenceIndex] != NULL)
                              retrievedNote = sequences[maxSequenceIndex].getSequenceNoteParams(step, trackPath);
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
      Serial.print(" of sequence ");
      Serial.print(curSequence);
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
            case STEPSEQUENCE:
                  if(currentSequence != NULL)
                        retrievedNote = currentSequence->getSequenceNoteParams(playbackStep, trackPath);
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
      Serial.print(" of sequence ");
      Serial.print(currentSequenceIndex);
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


bool Track::setCurrentSequenceIndex(byte newIndex)
{
      bool success = false;
      if( newIndex <= maxSequenceIndex)
      {
            currentSequenceIndex = newIndex;
            currentSequence = &sequences[newIndex];
            if(currentSequence == NULL)
            {
                  inout.ShowErrorOnLCD("setCSI cS NULL");
                  return success;
            }
            trackPath.setPath(currentSequence->getPath());
            success = true;
      }
      return success;
}

void Track::advanceStepPosition()
{
      if(currentSequence != NULL)
      {
            prevPlaybackStep = playbackStep;
            playbackStep = trackPath.getAndAdvanceStepPos(currentSequence->getLength());
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

      if(currentSequence != NULL)
      {
            byte seqLength = currentSequence->getLength(); // truncate step to available sequence length
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
      if(currentSequence != NULL)
      {
            currentSequence->setPath(path);
            trackPath.setPath(path);
      } else 
            inout.ShowErrorOnLCD("setPath cS NULL");
}

void Track::updatePath()
{
      if(currentSequence != NULL)
      {
            trackPath.setPath(currentSequence->getPath());
      } else 
            inout.ShowErrorOnLCD("updatePath cS NULL");
}

StepSequence* Track::getCurrentSequenceRef()
{
      if(currentSequence != NULL)
            return currentSequence;
      else 
      {
            Serial.print("getCurrentSequenceRef fail on track ");
            Serial.println(trackNumber);
            return &sequences[0];
      }
}

StepSequence* Track::getSequenceRef(int index)
{
      StepSequence* retVal = NULL;
      if(index <= maxSequenceIndex)
      {
            if(sequences != NULL)
                  retVal = &sequences[index];
            else{
                  Serial.print("getSequenceRef fail on track ");
                  Serial.println(trackNumber);
            }
      } else {
                  Serial.print("getSequenceRef index ");
                  Serial.print(index);
                  Serial.print(" out of range on track ");
                  Serial.println(trackNumber);
      }
      return retVal;
}

StepSequence* Track::getRootSequenceRef(int index)
{
      StepSequence* retVal = NULL;
      if(index <= maxSequenceIndex)
      {
            if(rootSequences != NULL)
                  retVal = &rootSequences[index];
            else{
                  Serial.print("getRootSequenceRef fail on track ");
                  Serial.println(trackNumber);
            }
      } else {
                  Serial.print("getRootSequenceRef index ");
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

byte Track::getCurrentSequenceIndex()
{
      return currentSequenceIndex;
}

byte Track::getPrevPlaybackStep()
{
      return prevPlaybackStep;
}

byte Track::getMaxSequenceIndex()
{
      return maxSequenceIndex;
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

      if(currentSequence != NULL)
            retVal = trackPath.detectPatternRollover(currentSequence->getLength());
      else
            inout.ShowErrorOnLCD("iTPROS cS NULL");

      return retVal;
}

