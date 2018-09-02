#include "Track.h"
#include "InOutHelper.h"

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
      b_IsActive = false;
      trackNumber = number;
      currentSequenceIndex = 0;
      currentSequence = &sequences[0];
      trackPath.setPath(currentSequence->getPath());
}


// Core Methods

note Track::getNoteParams(int step)
{
      return getNoteParams(step, currentSequenceIndex);
}

note Track::getNoteParams(int step, byte curSequence)
{
      note retNote;
      note retrievedNote;

      switch (trackType)
      {
            case STEPSEQUENCE:
                  if(curSequence < maxSequenceIndex)
                        retrievedNote = sequences[curSequence].getNoteParams(step, trackPath);
                  else
                        retrievedNote = sequences[maxSequenceIndex].getNoteParams(step, trackPath);
                  break;
            case SIMPLEBEAT:
                  break;
            default:
                  inout.ShowErrorOnLCD("trackType DEFAULTED");
//                Serial.println("trackType DEFAULTED");
                  break;
      }
#ifdef DEBUG
      Serial.print("Track::getNoteParams retNote pitch is ");
//    Serial.print((unsigned int) &retNote);
      Serial.print(retNote.pitchVal);
      Serial.print(" swingTicks ");
      Serial.print(retNote.swingTicks);
      Serial.print(" for step ");
      Serial.print(step);
      Serial.print(" of sequence ");
      Serial.print(curSequence);
      Serial.print(" nn track ");
      Serial.println(trackNumber);
#endif

      // this may be completely redundant ?
      if(&retrievedNote != NULL)
            memcpy(&retNote, &retrievedNote, sizeof(note));
      else
            inout.ShowErrorOnLCD("getNoteParams NULL");
      return retNote;
}

note Track::getNoteParams()
{
      note retNote;
      note retrievedNote;

      switch (trackType)
      {
            case STEPSEQUENCE:
                  if(currentSequence != NULL)
                        retrievedNote = currentSequence->getNoteParams(playbackStep, trackPath);
                  else
                        inout.ShowErrorOnLCD("gNoteP cS NULL");
                  break;
            case SIMPLEBEAT:
                  break;
            default:
                  inout.ShowErrorOnLCD("trackType DEFAULTED");
//                Serial.println("trackType DEFAULTED");
                  break;
      }
#ifdef DEBUG
      Serial.print("Track::getNoteParams retNote pitch is ");
//    Serial.print((unsigned int) &retNote);
      Serial.print(retNote.pitchVal);
      Serial.print(" swingTicks ");
      Serial.print(retNote.swingTicks);
      Serial.print(" for step ");
      Serial.print(playbackStep);
      Serial.print(" of sequence ");
      Serial.print(curSequence);
      Serial.print(" nn track ");
      Serial.println(trackNumber);
#endif

      // this may be completely redundant ?
      if(&retrievedNote != NULL)
            memcpy(&retNote, &retrievedNote, sizeof(note));
      else
            inout.ShowErrorOnLCD("getNoteParams NULL");
      return retNote;
}

void Track::activate()
{
      Serial.print("Track ");
      Serial.print(trackNumber);
      Serial.println(" active.");
}

void Track::deactivate()
{
  
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

char* Track::getPathName()
{
      return trackPath.getPathName();
}

void Track::setRecallBufferActive(bool trueOrFalse)
{
      b_recallBufferIsActive = trueOrFalse;
}

bool Track::recallBufferIsActive()
{
      return b_recallBufferIsActive;
}

