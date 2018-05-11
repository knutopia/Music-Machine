
// inspired by Arduino for Musicians

#define uint8_t byte

// to make SD card work
#include <Audio.h>
#include <SD_t3.h>

// general
#include <EEPROM.h>
#include <math.h>
#include "Enum.h"
#include "InOutHelper.h"
#include "SynthPatch.h"
#include "SynthEngine.h"
#include "StepSequence.h"
#include "StepSequencer.h"
#include "Path.h"
#include "Timebase.h"


// Constants for storing and retrieving data from EEPROM 
const byte EEPROM_ID = 0x99;
const int ID_ADDR = 0;
boolean storingData = false;

// For SD card
const int chipSelect = BUILTIN_SDCARD;
File myFile;

// kg
const float NORMAL_VEL = 0.7;
                                                 // TO DO: make this the play button LED
boolean b_led1_on_state = false;                 // remembers what we want the state of the LED to be.
boolean b_note_on = false;
volatile unsigned long recentInterruptTime;
volatile unsigned long v_note_off_time = 0;
unsigned long note_off_time = 0;
unsigned long next_note_durationMS = 0;
volatile bool vb_prep_next_step = false;      // grab the next step's note to be ready for play
volatile bool b_timer_on = false;
float next_note_freq;
int next_note;
int next_note_unmuted;
bool next_note_playIt;
int save_sequence_destination = -1;
bool save_to_SD_done = false;

IntervalTimer myTimer;

// uses seqModes enum
int currentMode = pattern_select;

const char *modeNames[] = {"foo1",
                           "Pattern Select", 
                           "Pattern Save", 
                           "Step Mute", 
                           "Step Hold",
                           "Step Edit", 
                           "Accent Edit",
                           "Length Edit", 
                           "Path Select", 
                           "Synth Edit",
                           "Save to SD", 
                           "foo2"};


//Time variables and constants
long g_step_duration = 500000;                //The amount of time between steps
//long micro_compare2;

speedFactor speedMultiplier = NORMAL;
const int buttonDebounceTime  = 800;    //Button debounce time
const int encoderDebounceTime  = 5;     //Encoder debounce time
//Sample rate:
const unsigned long SAMPLE_RATE = 16384;

//Playback variables 
int currentEditStep = 0;                //The current sequence edit position
bool playbackOn = false;                //true when sequencer is running
int playbackStep = 0;                   //The current playback step

//Global Time Management object:
Timebase metro;

//Global Path Manager object:
Path playpath;

//Global StepSequencer object:
StepSequencer sequencer;
// 144288

//Input-Output
InOutHelper inout;

//Global Synth Engine object:
SynthEngine synth;

//Callbacks for inputs
void ChangeModeCb(bool forward)
{
    if (forward) {
       currentMode++;
       if(currentMode == last) currentMode = synth_edit;      // Wrap around
       if(currentMode < synth_edit) currentMode = synth_edit; // Skip lower modes
    } else {
       currentMode--;
       if(currentMode < synth_edit) currentMode = save_to_sd; // Wrap around    
    }
    
    switch (currentMode) {
      case synth_edit:
        synth.prepSynPatchForEdit(); // no synth global in inOutHelper, so doing this here
        break;
      default:
        break;
    }
}


void ChangeRepetitionCb(int noteRepetition,  boolean activeSteps[]) {
  
    int seqLength = sequencer.getLength();
    
    for (int i=0; i < seqLength; i++) {
      if (activeSteps[i]) {
              sequencer.setTicks(i, (byte)noteRepetition); //no truncation yet
      }
    }
}


void ChangePatternLengthCb(int patternLength) {
  
    byte seqMaxLength = sequencer.getMaxLength();
    byte newLength = (byte)patternLength + 1;

    if (newLength <= seqMaxLength) {
      sequencer.setLength((byte)newLength);
    } 
}


void ChangeSequenceNumberCb(int seqNum) {
  
    sequencer.setCurrentSequence(seqNum);
    playpath.setPath(sequencer.getPath());
    inout.ShowSequenceNumberOnLCD(seqNum);
    inout.ShowPathNumberOnLCD(sequencer.getPath());
    sequencer.printSequence();
}


void ChangeSaveSequenceDestinationCb(int seqNum) {
          
    save_sequence_destination = seqNum;
    inout.ShowValueInfoOnLCD("Save to Seq ", seqNum);
}


void ChangeTempoCb(int newTempo) {
    metro.updateTempo(newTempo);
    retimeNextDuration();
}


void ChangeSpeedMultiplierCb(speedFactor mult) {
  metro.updateSpeedMultiplier(mult);
  retimeNextDuration();
}


void SaveToSdCb()
{
    if (!storingData)
     if (!save_to_SD_done) {
        //Turn off playback
        stopPlayback();
        inout.ShowStoreMessage(START);
        //Store to EEPROM
//      storeDataToEEPROM();
        writeToSDcard();
//      readSeqFromSDcard();
//      readSndFromSDcard();
        inout.ShowStoreMessage(STOP);
        save_to_SD_done = true;
      }
}


void StartStopCb()
{
    if(playbackOn == true)
    {
      stopPlayback();
//    Serial.println("  button off: ");

      synth.reportPerformance();
                 
    }else{
      playbackOn = true;
//    Serial.println("  button on: ");
      
      //Teensy timer
//    myTimer.priority(255);

//    metro.prepPlay();

      //Midi timer
      metro.runMidiTimer();

      //get first note out immediately
      play_first_step();
      vb_prep_next_step = true;
    }  
}


void SynthButtonCb(int butNum)
{
    synth.handleButton(butNum);
}


void setup()
{

    //Read data from EEPROM
//    readDataFromEEPROM(); 

    setupSDcard();
    Serial.println("Reading sequences from SD");
    readSeqFromSDcard();
    
    sequencer.copy_edit_buffers_to_roots();
        
    Serial.println("Reading patches from SD");
    readSndFromSDcard();
    
    synth.begin();

    inout.begin(ChangeModeCb, 
                ChangeRepetitionCb, 
                ChangePatternLengthCb, 
                ChangeSequenceNumberCb,
                ChangeSaveSequenceDestinationCb,
                ChangeTempoCb,
                ChangeSpeedMultiplierCb,
                SaveToSdCb,
                StartStopCb,
                SynthButtonCb);
                
    playpath.setPath(sequencer.getPath());
    next_note = sequencer.getNote(0);
    next_note_freq = midiToFrequency(next_note); // add transposition once available...
    next_note_unmuted = sequencer.getMute(0);
    next_note_playIt = true;
    synth.prepAccent(sequencer.getAccent(0));
}

void loop()
{
    inout.handleStartStopButton();
    inout.handleSelectButton();
    inout.handleModeButtons();
    inout.handleButtonHolds();
    handleRewindButton();
    followNoteOff();
    prep_next_step();

    inout.handleEncoders();
    inout.handleEncoderButtons();
    inout.handleTrellis();
    inout.handleLCDtimeouts();

//    while (usbMIDI.read()) {
      // ignore incoming messages
//    }
//  inout.showLoopTimer();
}


void prep_next_step()
{
    static byte currentStepTick = 1;
    
    if(vb_prep_next_step) {    // this is triggered right after the interrupt
                              // started playing the current note                              
      vb_prep_next_step = false; 
      note_off_time = v_note_off_time;

      // adjust speed if tempo or multiplier have changed
      metro.updateTimingIfNeeded();

      // two state flags referring to the just-started note
      b_led1_on_state = true;
      b_note_on = true;

      // show step indicator for just-started note N-1
      inout.setRunningStepIndicators(playbackStep, note_off_time);      

                              // schedule the next note's start time
                              // as prepped previously
      
      // gather info about the following note N+1
      byte seqLength = sequencer.getLength(); // truncate step to available sequence length
      byte stepPlayTickCount = sequencer.getTicks(playbackStep);
      byte remainingRetrigs = metro.getAndCountdownRetrigs();

      if (remainingRetrigs < 1) { // next note: either a tick or the next step
        if (currentStepTick < stepPlayTickCount) {  // Next tick
            currentStepTick++;
            metro.resetRemainingRetrigs();
            Serial.print(" A ");
        } 
        else {                                      // Next step
            playbackStep = playpath.getAndAdvanceStepPos(seqLength);

            currentStepTick = 1;
            prepNoteGlobals();
            Serial.print(" B ");
        }
      } 
/*    else {                                      // Next note is a retrig, 
                                                    // but is it the LAST ONE AND IS THERE A HOLD ?
                                                    // (...that's different timing)
                                                    
            Serial.print(" C ");
            if (remainingRetrigs == 1) // last retrig
              if (!(currentStepTick < stepPlayTickCount)) // no more ticks
              {
                next_note_durationMS = calcNextNoteDurationAfterRetrigs();
                Serial.print(" D ");
              }
      }
*/
      // make next note an accent ?
      synth.prepAccent(sequencer.getAccent(playbackStep));

      // change synth patch ?
      synth.prepPatchIfNeeded();
    }
}

/*
void prep_next_step()
{
    static byte currentStepTick = 1;
    
    if(vb_prep_next_step) {    // this is triggered right after the interrupt
                              // started playing the current note
                              
      // first things first: interrupt timer off
      if (b_timer_on) // flag used to avoid overriding first step duration after pb start
      {
        myTimer.end();                    // discard timer - it's job is done
        b_timer_on = false;
        note_off_time = v_note_off_time;  // safe to read - note off time for just-started note
      }

      // with timer is off, it's safer around here
      vb_prep_next_step = false; 

      // adjust speed if tempo or multiplier have changed
      metro.updateTimingIfNeeded();

      // two state flags referring to the just-started note
      b_led1_on_state = true;
      b_note_on = true;

      // show step indicator for just-started note N-1
      inout.setRunningStepIndicators(playbackStep, note_off_time);      

                              // schedule the next note's start time
                              // as prepped previously

      // set new timer for triggering the next note N
      long noteStart = metro.getNoteStartTime(playpath.getCurrentStepCount());
      myTimer.begin(timerBusiness, noteStart);

      Serial.print("  noteStart ");
      Serial.println(noteStart);
      
      // gather info about the following note N+1
      byte seqLength = sequencer.getLength(); // truncate step to available sequence length
      byte stepPlayTickCount = sequencer.getTicks(playbackStep);
      byte remainingRetrigs = metro.getAndCountdownRetrigs();

      if (remainingRetrigs < 1) { // next note: either a tick or the next step
        if (currentStepTick < stepPlayTickCount) {  // Next tick
            currentStepTick++;
            metro.resetRemainingRetrigs();
        } 
        else {                                      // Next step
            playbackStep = playpath.getAndAdvanceStepPos(seqLength);

            currentStepTick = 1;
            prepNoteGlobals();
        }
      } else {                                      // Next note is a retrig, 
                                                    // but is it the LAST ONE AND IS THERE A HOLD ?
                                                    // (...that's different timing)
            if (remainingRetrigs == 1) // last retrig
              if (!(currentStepTick < stepPlayTickCount)) // no more ticks
                next_note_durationMS = calcNextNoteDurationAfterRetrigs();
      }

      // make next note an accent ?
      synth.prepAccent(sequencer.getAccent(playbackStep));

      // change synth patch ?
      synth.prepPatchIfNeeded();
    }
}
*/

void play_first_step()
{
    if(playbackStep >= sequencer.getLength())
    {
       inout.ShowErrorOnLCD("YOWZA play_first_step");
       Serial.print("step:");
       Serial.print(playbackStep);
       Serial.print(" len:");
       Serial.print(sequencer.getLength());
       Serial.print(" >> ");

       playbackStep = 0;
    }

    // gather info about the first note
    byte seqLength = sequencer.getLength(); // truncate step to available sequence length

    if (playbackStep == 0) 
      playbackStep = playpath.getDontAdvanceStepPos(seqLength);
    else
      playbackStep = playpath.getAndAdvanceStepPos(seqLength);

    metro.setRetrigCount(sequencer.getRetrig(playbackStep));
    prepNoteGlobals();
    v_note_off_time = micros() + next_note_durationMS;
    note_off_time = v_note_off_time;

    b_led1_on_state = true;
    b_note_on = true;

/*
    Serial.println(next_note);
    Serial.println(next_note_freq);
    Serial.println(next_note_unmuted);
    Serial.println(next_note_playIt);
    Serial.println(next_note_durationMS);
*/
    
    if (next_note_unmuted) synth.playNote(next_note, next_note_freq, NORMAL_VEL);
//  inout.setRunningStepIndicators(playbackStep, note_off_time);
}

void prepNoteGlobals()
{
    metro.setRetrigCount(sequencer.getRetrig(playbackStep));
    next_note = sequencer.getNote(playbackStep);
    next_note_freq = midiToFrequency(next_note); //add transposition etc
    next_note_unmuted = sequencer.getMute(playbackStep);
    next_note_playIt = sequencer.playItOrNot(playbackStep);
    next_note_durationMS = calcNextNoteDuration();              
}

unsigned long calcNextNoteDuration()
{
    unsigned long retVal;

    // cases for note duration:
    // -A if simple note: 
    //   -note duration as stored
    //   -if there is a hold: full step plus hold (use legato note's duration ?)
    // -B if retrig within note:
    //   -retrig fraction
    //   -if there is a hold and retrig is last one: retrig fraction plus hold (use legato note's duration ?)
    // -C if muted: blip (to keep timer going)
    // -FUTURE: stretch retrigs across holds ?
    
        
    if (next_note_unmuted) {
      byte hold_count = assembleHolds();
      retVal = metro.getStepDurationMS(sequencer.getDuration(playbackStep), hold_count);
      
    } else {
      retVal = BLIP;  // huh ? this is odd
      Serial.println("BLIP found"); // ...even a muted note needs a duration.
    }
    return retVal;
}


unsigned long calcNextNoteDurationAfterRetrigs()
{
    unsigned long retVal;

    // -if there is a hold and retrig is last one: retrig fraction plus hold (use legato note's duration ?)
    // -otherwise, just retrig fraction
    // -FUTURE: stretch retrigs across holds ?

    if (next_note_unmuted) {
      byte hold_count = assembleHolds();
      if (hold_count > 0)
        retVal = metro.getStepDurationRetrigHoldMS(sequencer.getDuration(playbackStep), hold_count);
      else
        retVal = metro.getStepDurationMS(sequencer.getDuration(playbackStep), hold_count);
      
    } else {
      retVal = BLIP;  // huh ? this is odd
      Serial.println("BLIP found"); // ...even a muted note needs a duration.
    }
    return retVal;
}


void retimeNextDuration()
{

    Serial.print("rND");

    // no note playing, next note prepped, next note is first or only retrig
    if (!synth.playingAnote()
        && !vb_prep_next_step 
        && metro.getRetrigs() == sequencer.getRetrig(playbackStep)) 
    {
      // reschedule next note duration
      next_note_durationMS = calcNextNoteDuration(); // ### TO BE BRANCHED...
      Serial.println(" yep");
    } else 
      Serial.println(" nope");
//    next_note_durationMS = calcNextNoteDuration();
}


byte assembleHolds()
{
    // use getStepPosAfterNext to look ahead for holds. 
    // count consecutive forward-holds, to pass into getStepDurationMS
    byte holdStepCount = 0;
    byte stepOffset = 1;
    byte seqLength = sequencer.getLength();
    bool holdNext = true;
    
    while (holdNext) {
      holdNext = sequencer.getHold(playpath.getStepPosForward(stepOffset, seqLength));
      if (holdNext) {
        holdStepCount++;
        stepOffset++;
      }
    }
    return holdStepCount;
}


void timerBusiness() 
{
/*
    if(playbackOn == true)
    {
      recentInterruptTime = micros();
      v_note_off_time = recentInterruptTime + next_note_durationMS;
          
      vb_prep_next_step = true;
      b_timer_on = true;
      
      if (next_note_unmuted && next_note_playIt) 
          synth.playNote(next_note, next_note_freq, NORMAL_VEL);
    }
*/
}


void followNoteOff()
{
    if((b_note_on == true) && (note_off_time < micros()))
    {
      b_note_on = false;
      synth.endNote(NORMAL_VEL);
    }
}


void handleRewindButton()
{
    if (inout.checkRewindButton()) 
    {      
      inout.RemoveStepIndicatorOnLCD();
      playpath.resetStep();
      playbackStep = 0;

      if(playbackOn == true) stopPlayback();
    }
}


void stopPlayback()
{
//   myTimer.end();
     playbackOn = false;
     vb_prep_next_step = false;
     metro.stopMidiTimer();
}


float midiToFrequency(int note)
{  
  return (float) 440.0 * (float)(pow(2, (note-57) / 12.0));
}


// SD Card business:

void writeToSDcard()
{   
    File bupFile;

    // make a backup
    if (!SD.exists("seqs.txt")) {
      Serial.println("No seqs.txt data file found - no backup created.");      
    } else {
      myFile = SD.open("seqs.txt", FILE_READ);
      if (!myFile) {
        Serial.println("FAILED to open seqs.txt data file - no backup created.");      
      } else {
        if (SD.exists("seqsbup.txt")) {
          Serial.println("Removing existing seqsbup.txt backup file");
          SD.remove("seqsbup.txt");
          if (SD.exists("seqsbup.txt")) {
            Serial.println("FAILED to remove backup file");
          }
        }
        Serial.println("Creating new backup file");
        bupFile = SD.open("seqsbup.txt", FILE_WRITE);
        if(!bupFile) {
          Serial.println("FAILED to create new backup file seqsbup.txt");
          myFile.close();
        } else {
          Serial.println("Copying seqs.txt to new backup seqsbup.txt");
          int foo = 0;
          int n;  
          uint8_t buf[1024];
          while ((n = myFile.read(buf, sizeof(buf))) > 0) {
            bupFile.write(buf, n);
            foo += n;
            Serial.print(".");
          }
          bupFile.close();
          myFile.close();
          Serial.print(foo);
          Serial.println(" chars total");
          Serial.println("seqs.txt copied to new backup seqsbup.txt");
          if (SD.exists("seqs.txt")) {
            Serial.println("Removing existing seqs.txt file");
            SD.remove("seqs.txt");
            if (SD.exists("seqs.txt")) {
              Serial.println("FAILED to remove existing seqs.txt file");
            }
          }
        }
      }
    }
    
    myFile = SD.open("seqs.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (myFile) {

        int seqBuf = sequencer.getCurrentSequence();
  
        Serial.println("writeToSDcard: seqs.txt opened successfully");

        for(int c = 0; c< StepSequencer::max_sequences; c++)
        {
          //Select the sequence with index c
          sequencer.swap_edit_root_seqs(c);
          sequencer.setCurrentSequence(c);
          
          sequencer.printSequence();
          
          myFile.println();
          myFile.println();
          myFile.print("Sequence = ");
          myFile.println(c);
          myFile.print("Length = ");
          myFile.println(sequencer.getLength());
          myFile.print("Transposition = ");
          myFile.println(sequencer.getTransposition());
          myFile.print("Path = ");
          myFile.println(sequencer.getPath());
          myFile.println(" ");
          for(int n = 0; n < sequencer.getMaxLength(); n++)
          {
             myFile.print("NoteIndex = ");
             myFile.println(n);
             myFile.print("Pitch = ");
             myFile.println(sequencer.getNote(n));
             myFile.print("Duration = ");
             myFile.println(sequencer.getDuration(n));
             myFile.print("Probability = ");
             myFile.println(sequencer.getProbability(n));           
             myFile.print("Ticks = ");
             myFile.println(sequencer.getTicks(n));           
             myFile.print("Mute = ");
             myFile.println(sequencer.getMute(n));           
             myFile.print("Hold = ");
             myFile.println(sequencer.getHold(n));           
             myFile.print("Accent = ");
             myFile.println(sequencer.getAccent(n));           
             myFile.print("Retrig = ");
             myFile.println(sequencer.getRetrig(n));           
             myFile.print("Velocity = ");
             myFile.println(sequencer.getVelocity(n));           
             myFile.println(" ");
          }
          sequencer.swap_edit_root_seqs(c);
          myFile.println(" ");
          Serial.print("Stored seq");
          Serial.println(c);
          Serial.println();
        } 
        myFile.close();
        Serial.println("Sequence saving done.");
        Serial.println();
        sequencer.setCurrentSequence(seqBuf);
  
    } else {
      // if the file didn't open, print an error:
      Serial.println("writeToSDcard: error opening seqs.txt");
    }      

    // sound patches

    // make a backup
    if (!SD.exists("snds.txt")) {
      Serial.println("No snds.txt data file found - no backup created.");      
    } else {
      myFile = SD.open("snds.txt", FILE_READ);
      if (!myFile) {
        Serial.println("FAILED to open snds.txt data file - no backup created.");      
      } else {
        if (SD.exists("sndsbup.txt")) {
          Serial.println("Removing existing sndsbup.txt backup file");
          SD.remove("sndsbup.txt");
          if (SD.exists("sndsbup.txt")) {
            Serial.println("FAILED to remove sndsbup.txt backup file");
          }
        }
        Serial.println("Creating new backup file");
        bupFile = SD.open("sndsbup.txt", FILE_WRITE);
        if(!bupFile) {
          Serial.println("FAILED to create new backup file sndsbup.txt");
          myFile.close();
        } else {
          Serial.println("Copying snds.txt to new backup sndsbup.txt");
          int foo = 0;
          int n;  
          uint8_t buf[1024];
          while ((n = myFile.read(buf, sizeof(buf))) > 0) {
            bupFile.write(buf, n);
            foo += n;
            Serial.print(".");
          }
          bupFile.close();
          myFile.close();
          Serial.print(foo);
          Serial.println(" chars total");
          Serial.println("snds.txt copied to new backup sndsbup.txt");
          if (SD.exists("snds.txt")) {
            Serial.println("Removing existing snds.txt file");
            SD.remove("snds.txt");
            if (SD.exists("snds.txt")) {
              Serial.println("FAILED to remove existing snds.txt file");
            }
          }
        }
      }
    }
    myFile = SD.open("snds.txt", FILE_WRITE);
    
    // if the file opened okay, write to it:
    if (!myFile) {
        // if the file didn't open, print an error:
        Serial.println("writeToSDcard: error opening snds.txt");
    } else {

        Serial.println("writeToSDcard: snds.txt opened successfully");

        Serial.print("max_patches: ");
        Serial.println(SynthEngine::max_patches);

        for(int c = 0; c < SynthEngine::max_patches; c++)
        {
          // Select the patch with index c
          // synth.setCurrentPatch(c);

          Serial.print("Patch: ");
          Serial.println(c);
          
          myFile.println(" ");
          myFile.println(" ");
          myFile.print("Patch = ");
          myFile.println(c);
          myFile.println(" ");

          for(int p = 0; p < SynthPatch::SynParameterCount; p++)
          {
/*
            Serial.print("Param: ");
            Serial.print(p);
            Serial.print("\t");
            Serial.println(synth.m_patches[c].getPname(p)); 
*/            
            myFile.print("Param = ");
            myFile.println(p);
            myFile.println(" ");
  
            bool isInt = synth.m_patches[c].isInt(p);
            if (isInt)
            {
              myFile.print("ParamName = ");
              myFile.println(synth.m_patches[c].getPname(p));
              myFile.print("Id = ");
              myFile.println(synth.m_patches[c].getId(p));
              myFile.print("isInt = ");
              myFile.println(isInt);
              myFile.print("Min = ");
              myFile.println(synth.m_patches[c].getImin(p));
              myFile.print("Max = ");
              myFile.println(synth.m_patches[c].getImax(p));
              myFile.print("Default = ");
              myFile.println(synth.m_patches[c].getIdefault(p));
              myFile.print("Value = ");
              myFile.println(synth.m_patches[c].getI(p));
              myFile.print("End = ");
              myFile.println(synth.m_patches[c].getPname(p));
              
            } else {
              myFile.print("ParamName = ");
              myFile.println(synth.m_patches[c].getPname(p));
              myFile.print("Id = ");
              myFile.println(synth.m_patches[c].getId(p));
              myFile.print("isInt = ");
              myFile.println(isInt);
              myFile.print("Min = ");
              myFile.println(synth.m_patches[c].getMin(p));
              myFile.print("Max = ");
              myFile.println(synth.m_patches[c].getMax(p));
              myFile.print("Default = ");
              myFile.println(synth.m_patches[c].getDefault(p));
              myFile.print("Value = ");
              myFile.println(synth.m_patches[c].get(p));             
              myFile.print("End = ");
              myFile.println(synth.m_patches[c].getPname(p));
            }
            myFile.println(" ");
          }

          myFile.println(" ");
          Serial.print("Stored patch ");
          Serial.println(c);
          Serial.println();
        } 
        myFile.close();
        Serial.println("Sound patches saving done.");
        Serial.println();
    }      

}

void readSeqFromSDcard()
{
    char buffer[40]; // May need to be a bit bigger if you have long names
    byte index = 0;
    int seqBuf = sequencer.getCurrentSequence();
   
    myFile = SD.open("seqs.txt");
    if (!myFile) {
      // if the file didn't open, print an error:
      Serial.println("readSeqFromSDcard: error opening seqs.txt on SD Card");
      inout.ShowErrorOnLCD("SD Card file error");
    } else {
      Serial.println("seqs.txt:");

      while (myFile.available())
      {
        char c = myFile.read();
        if(c == '\n' || c == '\r') // Test for <cr> and <lf>
        {
           parseAndAssignSeqSD(buffer);
           index = 0;
           buffer[index] = '\0'; // Keep buffer NULL terminated
        }
        else
        {
           buffer[index++] = c;
           buffer[index] = '\0'; // Keep buffer NULL terminated
        }
      }
    }
    myFile.close();
//  sequencer.copy_edit_buffers_to_roots();
    sequencer.setCurrentSequence(seqBuf);
}

void parseAndAssignSeqSD(char *buff)
{
    static int note_index = 0;
    char *name = strtok(buff, " =");
    if(name)
    {
      char *junk = strtok(NULL, " ");
      if(junk)
      {
        char *valu = strtok(NULL, " ");
        if(valu)
        {
/*
          Serial.print(name);
          Serial.print(" ");
          Serial.print(valu);
          Serial.println();
*/
          if(strcmp(name, "Sequence") == 0) {
              sequencer.setCurrentSequence(atoi(valu));

              Serial.print("Sequence ");
              Serial.println(valu);
          }
          
          else if(strcmp(name, "Length") == 0)
              sequencer.setLength(atoi(valu));           
    
          else if(strcmp(name, "Transposition") == 0)
              sequencer.setTransposition(atoi(valu));           

          else if(strcmp(name, "Path") == 0)
              sequencer.setPath(atoi(valu));           
          
          else if(strcmp(name, "NoteIndex") == 0)
              note_index = atoi(valu);       

          else if(strcmp(name, "Pitch") == 0)
              sequencer.setNote(note_index, atoi(valu));           
 
          else if(strcmp(name, "Duration") == 0)
              sequencer.setDuration(note_index, atof(valu));           

          else if(strcmp(name, "Probability") == 0)
              sequencer.setProbability(note_index, atoi(valu));  
                       
          else if(strcmp(name, "Ticks") == 0)
              sequencer.setTicks(note_index, atoi(valu));      
                   
          else if(strcmp(name, "Mute") == 0)
              sequencer.setMute(note_index, atoi(valu));        
                 
          else if(strcmp(name, "Hold") == 0)
              sequencer.setHold(note_index, atoi(valu));        
                 
          else if(strcmp(name, "Accent") == 0)
              sequencer.setAccent(note_index, atoi(valu));
          
          else if(strcmp(name, "Retrig") == 0)
              sequencer.setRetrig(note_index, atoi(valu));

          else if(strcmp(name, "Velocity") == 0)
              sequencer.setVelocity(note_index, atoi(valu));
         }
      }
    }
}

void readSndFromSDcard()
{
    char buffer[40]; // May need to be a bit bigger if you have long names
    byte index = 0;
   
    myFile = SD.open("snds.txt");
    if (!myFile) {
      // if the file didn't open, print an error:
      Serial.println("readSndFromSDcard: error opening snds.txt on SD Card");
      inout.ShowErrorOnLCD("SD Card file error");
    } else {
      Serial.println("snds.txt:");

      while (myFile.available())
      {
        char c = myFile.read();
        if(c == '\n' || c == '\r') // Test for <cr> and <lf>
        {
           parseAndAssignSndSD(buffer);
           index = 0;
           buffer[index] = '\0'; // Keep buffer NULL terminated
        }
        else
        {
           buffer[index++] = c;
           buffer[index] = '\0'; // Keep buffer NULL terminated
        }
      }
      myFile.close();
      Serial.println();      
    }
}

void parseAndAssignSndSD(char *buff)
{
    static int patch = 0;
    static int param = 0;
    static bool isInt = false;
    static float minVal = 0;
    static float maxVal = 0;
    static float defaultVal = 0;
    static float value = 0;
    
    // use statics to fill a constructor for synIpar. synFpar
    
    char *name = strtok(buff, " =");
    if(name)
    {
      char *junk = strtok(NULL, " ");
      if(junk)
      {
        char *valu = strtok(NULL, " ");
        if(valu)
        {
/*          
          Serial.print(name);
          Serial.print(" ");
          Serial.print(valu);
          Serial.println();
*/
          if(strcmp(name, "Patch") == 0) {
              patch = atoi(valu);
              Serial.println();
              Serial.print("Patch ");
              Serial.println(patch);
          }
          
          if(strcmp(name, "Param") == 0)   // Use ID ?
              param = atoi(valu);

          if(strcmp(name, "isInt") == 0)
              isInt = (bool) atoi(valu);

          if(strcmp(name, "Min") == 0)
              minVal = atof(valu);

          if(strcmp(name, "Max") == 0)
              maxVal = atof(valu);

          if(strcmp(name, "Default") == 0)
              defaultVal = atof(valu);

          if(strcmp(name, "Value") == 0)
              value = atof(valu);

          if(strcmp(name, "End") == 0) {
/*
            Serial.print(synth.m_patches[patch].getPname(param));
            Serial.print(": ");
            Serial.println(value);
*/            
            if (isInt) {
              synth.m_patches[patch].params[param] = new synIpar((synIparam){param, 
                                                                             (int)minVal, 
                                                                             (int)maxVal, 
                                                                             (int)defaultVal, 
                                                                             (int)value});
            } else {
              synth.m_patches[patch].params[param] = new synFpar((synFparam){param, 
                                                                             minVal, 
                                                                             maxVal, 
                                                                             defaultVal, 
                                                                             value});
            }
          }        
         }
      }
    }
}

void setupSDcard()
{
    Serial.print("Initializing SD card...");
    
    if (!SD.begin(chipSelect)) {
      Serial.println("initialization failed!");
      return;
    }
    Serial.println("initialization done.");
}
