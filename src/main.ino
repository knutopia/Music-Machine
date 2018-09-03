
// inspired by Arduino for Musicians

#define uint8_t byte
//#define MIDION true
//#define DEBUG true

// to make SD card work
#include <Audio.h>
#include <SD_t3.h>

// general
#include <EEPROM.h>
#include <math.h>
#include "Enum.h"
#include "Note.h"
#include "LinkedNoteList.h"
#include "PerClickNoteList.h"
#include "StepClickList.h"
#include "NoteOffList.h"
#include "NotePerClick.h"
#include "Track.h"
#include "TrackList.h"
#include "InOutHelper.h"
#include "SynthPatch.h"
#include "SynthEngine.h"
#include "StepSequence.h"
#include "StepSequencer.h"
#include "Path.h"
#include "Timebase.h"
//#include "elapsedMicros.h"

int g_activeGlobalStep;


// Constants for storing and retrieving data from EEPROM 
const byte EEPROM_ID = 0x99;
const int ID_ADDR = 0;
boolean storingData = false;

// For SD card
const int chipSelect = BUILTIN_SDCARD;

// kg
const float NORMAL_VEL = 0.7;
                                                 // TO DO: make this the play button LED
//boolean b_note_on = false;
volatile unsigned long v_note_off_time = 0;
unsigned long g_note_off_time = 0;
//volatile bool vb_prep_next_step = false;       // grab the next step's note to be ready for play
volatile bool vb_clickHappened = false;          //follow up closely after each click
volatile unsigned long v_note_trigger_time = 0;

int save_sequence_destination = -1;
bool save_to_SD_done = false;

note nextNote;

#ifdef DEBUG
volatile unsigned long timeTracker;
#endif

// midi
int midiBaseNote = 255;
int midiTranspose = 0;
int g_midiClickCount = 0;

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
                           "Track Select",
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
bool playbackOn = false;                //true when sequencer is running
bool startFromZero = true;              //Has reset been pressed ?

//Global Time Management object:
Timebase metro;

//Global Path Manager object:
//Path playpath;

//Global StepSequencer object:
StepSequencer sequencer;
// 144288

//Input-Output
InOutHelper inout;

//Global Synth Engine object:
SynthEngine synth;

//Magic Crashy Superlists:
LinkedNoteList activeNotes;
StepClickList activeStepClicks;
NoteOffList playingNotes;
PerClickNoteList notesToTrig;

//To replace PerClickNoteList:
volatile notePerClick notesToPlay[TRACKCOUNT];

//To replace timer
elapsedMicros clickTrack;

//To track global modifiers
bool shiftActive = false;

//Helper
void listCounts()
{
    Serial.print("activeNotes: ");
    Serial.print(activeNotes.count());
    Serial.print("  activeStepClicks: ");
    Serial.print(activeStepClicks.count());
    Serial.print("  notesToTrig: ");
    Serial.print(notesToTrig.count());
    Serial.print("  playingNotes: ");
    Serial.println(playingNotes.count());
}

//Callback for sequencer problems
void EmergencyCb()
{
    if(playbackOn == true) stopPlayback();
    synth.allNotesOff();
    sequencer.resetStepPositions();
    startFromZero = true;
    inout.RemoveStepIndicatorOnLCD();
    inout.ShowModeOnLCD();

//  activeNotes.purge();
//  activeStepClicks.purge();
}

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
    inout.ShowSequenceNumberOnLCD(seqNum);
    inout.ShowPathNumberOnLCD(sequencer.getPath());
    sequencer.printSequence();
}


void ChangeTrackCb(int trackNum) {
  
    Serial.print("ChangeTrackCb: ");
    Serial.print(1 + trackNum);
    
    sequencer.setCurrentTrack(1 + (byte)trackNum);

    Serial.print("  result: ");
    Serial.println(sequencer.getCurrentTrack());
}

void ChangeSaveSequenceDestinationCb(int seqNum) {
          
    save_sequence_destination = seqNum;
    inout.ShowValueInfoOnLCD("Save to Seq ", seqNum);
}


void ChangeTempoCb(int newTempo) {
    metro.updateTempo(newTempo);
//  retimeNextDuration();
}


void ChangeSpeedMultiplierCb(speedFactor mult) {
    metro.updateSpeedMultiplier(mult);
//  retimeNextDuration();
}


void SaveToSdCb()
{
    bool tracksBackedUp = false;
    bool tracksSaved = false;
    bool patchesBackedUp = false;
    bool patchesSaved = false;

    if (!storingData)
     if (!save_to_SD_done) {
        //Turn off playback
        stopPlayback();
        synth.allNotesOff();

        inout.ShowStoreMessage(START);
//      writeToSDcard();

        tracksBackedUp = backupTrackFile();
        if(tracksBackedUp)
        {
            tracksSaved = true;
            
            for(int f=1; f <= TRACKCOUNT; f++)
                if(!writeTrackToSDcard(f))
                    tracksSaved = false;
        }

        patchesBackedUp = backupPatchFile();
        if(patchesBackedUp)
            patchesSaved = writePatchesToSDcard();

        inout.ShowStoreMessage(STOP);
        save_to_SD_done = true;

        sequencer.resetStepPositions();
        startFromZero = true;
        inout.RemoveStepIndicatorOnLCD();
        inout.ShowModeOnLCD();

        activeNotes.purge();
        activeStepClicks.purge();
      }
}


void StartStopCb()
{
    if(playbackOn == true)
    {
        stopPlayback();
#ifdef MIDION
        usbMIDI.sendRealTime(usbMIDI.Stop);
#endif
        Serial.println("  button off: ");

        synth.reportPerformance();
        inout.ShowValueInfoOnLCD("Mem:", (int)FreeMem() );
        inout.SetLCDinfoTimeout();
    } else {

#ifdef DEBUG
    testLinkedNoteList();
    testLinkedTrackList();
    testPerClickNoteList();
    testStepClickList();
#endif

    playbackOn = true;
    Serial.println("");
    Serial.println("  button on: ----------------------------------------");
    Serial.println("");
    activeStepClicks.print();

    //Midi timer
#ifdef DEBUG
    timeTracker = millis();
#endif

    if(startFromZero)
    {
        startFromZero = false;

        sequencer.prepFirstStep();
        Serial.println("   first step prepped");
        prepNoteGlobals();
        Serial.println("   globals good");
    }

    g_midiClickCount = 0;
    vb_clickHappened = true;
    prepNextClick();

    Serial.println("   next click prepped");
    
    activeStepClicks.print();

    metro.startPlayingRightNow();
    Serial.println("   playing now");
    }  
}


void SynthButtonCb(int butNum)
{
    synth.handleButton(butNum);
}


void OnNoteOn(byte channel, byte note, byte velocity) {
    if (midiBaseNote == 255)
    {
      midiBaseNote = note;
    }
    else
    {
      midiTranspose = note - midiBaseNote;
    }

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ############################# S E T U P #############################
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup()
{
    bool tracksLoaded;
    bool patchesLoaded;

    Serial.print("Start1 Mem: ");
    Serial.println(FreeMem());
    inout.ShowValueInfoOnLCD("Start1:", (int)FreeMem() );
    inout.SetLCDinfoTimeout();


    //Read data from EEPROM
//    readDataFromEEPROM(); 
    #ifdef MIDION
      usbMIDI.setHandleNoteOn(OnNoteOn);
    #endif

    // Joystick reading
    analogReadAveraging(32);
    analogReadResolution(14);
    setupSDcard();
    sequencer.begin(EmergencyCb);
    activeStepClicks.begin(EmergencyCb);

    Serial.println("Reading sequences from SD");
//  readSeqFromSDcard();

    tracksLoaded = readTracksFromSDcard();

    sequencer.copy_edit_buffers_to_roots();
//  sequencer.begin();

    Serial.println("Reading patches from SD");
//  readSndFromSDcard();
    patchesLoaded = readPatchesFromSDcard();
    

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
                SynthButtonCb,
                ChangeTrackCb);
                
    Serial.print("Start2 Mem: ");
    Serial.println(FreeMem());
    inout.ShowValueInfoOnLCD("Start2:", (int)FreeMem() );
    inout.SetLCDinfoTimeout();

    synth.playTestClick();
}

void loop()
{
    static byte bTimeslice = 0;
//  static int loopcount = 0;
    
    if(clickTrack > metro.midiClickInterval)
    {
/*
        if(clickTrack-metro.midiClickInterval > 2000 && bTimeslice == 2)
        {
            Serial.print(clickTrack-metro.midiClickInterval);
            Serial.print(" ");
            Serial.println(bTimeslice);
        }
*/
        if(playbackOn)
        {
            metro.arrayMidiClick();
            prepNextClick();
        }
        if(clickTrack > metro.midiClickInterval)
            clickTrack -= metro.midiClickInterval;
        else
            clickTrack = clickTrack % metro.midiClickInterval;

//      Serial.print(clickTrack);
//      Serial.print(" ");

//      Serial.println(loopcount);
//      loopcount = 0;
    } else {
//      loopcount++;

        followNoteOff();

        if(clickTrack < metro.loopCutoff)
        {
            bTimeslice = ++bTimeslice % 6;
            switch (bTimeslice)
            {
                case 0:
                    inout.handleStartStopButton();
                    inout.handleSelectButton();
                    inout.handleModeButtons();
                    inout.handleButtonHolds();
                    handleRewindButton();
                    break;
                case 1:
                    inout.handleEncoderButtons();
                    break;
                case 2:
                    inout.handleTrellis();
                    break;
                case 3:
                    inout.handleLCDtimeouts();
                    break;
                case 4:
                    synth.trackJoystick();
                    break;
                case 5:
                    inout.handleEncoders();
                    break;
                default:
                    break;
            } 
        } else {
            inout.handleStartStopButton();
            handleRewindButton();
        }
    }

    #ifdef MIDION
      while (usbMIDI.read()) {
        // ignore incoming messages
      }
    #endif
//  inout.showLoopTimer();
}

void prep_next_note_direct()
{
//  inout.ShowMemoryOnLCD((int)FreeMem());
    inout.ShowPlaybackStepOnLCD(g_activeGlobalStep);

//  Serial.print(" pnnd1");

/*
    Serial.print("prep_next_note after ");
    Serial.print(g_activeGlobalStep);
    Serial.print("  mem: ");
    Serial.println(FreeMem());
*/

    // adjust speed if tempo or multiplier have changed
    metro.updateTimingIfNeeded();
    
    // forward we march
    sequencer.AdvanceStepPositions();

//  Serial.print(" pnnd2");

    // queue the next notes, drop previous ones
    prepNoteGlobals();

//  Serial.print(" pnnd3");

#ifdef DEBUG
    Serial.println(" Note prepped.");
#endif

    // change synth patch ?
    synth.prepPatchIfNeeded();

//  Serial.print(" pnnd4");

}

void prepNextClick()
{
    static int currentPlayingStep = 0;
    bool prepNextStep = false;
    bool clickPlayed;
    unsigned long note_trigger_time;
    
//  noInterrupts();
        clickPlayed = vb_clickHappened;
//  interrupts();

    if(clickPlayed)
    {
//      noInterrupts();
            vb_clickHappened = false;
            note_trigger_time = v_note_trigger_time;
            v_note_trigger_time = 0;
//      interrupts();
    
        // track noteOffs
        if(note_trigger_time != 0)
        {
            notePerClick notesPlayed[TRACKCOUNT];

//          noInterrupts();
            for(int i = 0; i < TRACKCOUNT; i++)
            {
                if(notesToPlay[i].active)
                {
                    note trigNote;

                    trigNote.pitchVal = notesToPlay[i].clickNote.pitchVal;
                    trigNote.playIt = notesToPlay[i].clickNote.playIt;
/*                  trigNote.retrigClickDivider = notesToPlay[i].clickNote.retrigClickDivider;
                    trigNote.unmuted = notesToPlay[i].clickNote.unmuted;
                    trigNote.pitchFreq = notesToPlay[i].clickNote.pitchFreq;
                    trigNote.durationMS = notesToPlay[i].clickNote.durationMS;
                    trigNote.hold = notesToPlay[i].clickNote.hold;
                    trigNote.duration = notesToPlay[i].clickNote.duration;
                    trigNote.retrigs = notesToPlay[i].clickNote.retrigs;
                    trigNote.ticks = notesToPlay[i].clickNote.ticks;
                    trigNote.accent = notesToPlay[i].clickNote.accent;
                    trigNote.velocity = notesToPlay[i].clickNote.velocity;
                    trigNote.swingTicks = notesToPlay[i].clickNote.swingTicks;
                    trigNote.holdsAfter = notesToPlay[i].clickNote.holdsAfter;
                    trigNote.mutesAfter = notesToPlay[i].clickNote.mutesAfter;
*/
                    notesPlayed[i].clickNote = trigNote;
                    notesPlayed[i].durationMS = notesToPlay[i].durationMS;
                    notesPlayed[i].track = notesToPlay[i].track;
                    notesPlayed[i].active = notesToPlay[i].active;
                } else {
                    notesPlayed[i].active = false;
                }
            }
//          interrupts();
            for(int i = 0; i < TRACKCOUNT; i++)
            {
                if(notesPlayed[i].active && notesPlayed[i].clickNote.playIt)
                {
/*
                    Serial.print("playingNotes.append track");
                    Serial.print(notesPlayed[i].track);
                    Serial.print(" played note ");
                    Serial.println(notesPlayed[i].clickNote.pitchVal);
*/
                    playingNotes.append(notesPlayed[i].track, 
                                        notesPlayed[i].clickNote.pitchVal, 
                                        notesPlayed[i].durationMS + note_trigger_time);
                }

                //for the step indicators...
                if(notesPlayed[i].track == 1)
                {
                    inout.setRunningStepIndicators(sequencer.getPreviousStep(), 
                                                   notesPlayed[i].durationMS + note_trigger_time);      
                }
            }
        }

        // track step completion
        g_midiClickCount++;

        if (g_midiClickCount >= MIDICLOCKDIVIDER)
        {
            g_midiClickCount = 0;
            currentPlayingStep = g_activeGlobalStep;
            prepNextStep = true;
    #ifdef DEBUG                                    
            Serial.print("&&&&&& prep_next set, g_activeGlobalStep is ");
            Serial.println(g_activeGlobalStep);
    #endif
        }

        // acquire notes for next click
        if(!activeStepClicks.transferClickNoteArray(g_midiClickCount, currentPlayingStep))
        {
//          Serial.print(" No. ");
        }
        activeStepClicks.readRewind();

//      Serial.print(" pnc 7");

        if(prepNextStep)
            prep_next_note_direct();

//      Serial.print(" pnc 8");        
    }
}

void prepNoteGlobals()
{
    // TRY THIS>>>
//  Serial.println("before dropNotesBeforeStepAndRewind:");
//  activeStepClicks.print();
    activeStepClicks.dropNotesBeforeStepAndRewind(g_activeGlobalStep);

//  Serial.print(" pNG1");

    g_activeGlobalStep++;

    activeNotes.dropNotesBeforeStepAndRewind(g_activeGlobalStep);

//  Serial.print(" pNG2");

    sequencer.updateNoteList();

//  Serial.print(" pNG3");

    activeNotes.rewind();

//  Serial.print(" pNG4");

    sequencer.updateStepClickList();

//  Serial.print(" pNG5");

#ifdef DEBUG
    Serial.print("###### playingNotes count: ");
    Serial.println(playingNotes.count());
#endif
}

void trackListSizes()
{
    Serial.print("size activeNotes:      ");
    Serial.println(sizeof(activeNotes));
    Serial.print("size activeStepClicks: ");
    Serial.println(sizeof(activeStepClicks));
    Serial.print("size sequencer: ");
    Serial.println(sizeof(sequencer));
    Serial.print("size synth: ");
    Serial.println(sizeof(synth));
}

void followNoteOff()
{
    int foo = 0;
    if(&playingNotes != NULL)
    {
        playingNotes.readRewind();

        int sentry = 0;
        while(playingNotes.hasReadValue())
        {        
            if(playingNotes.readNoteOffTime() < micros())
            {

    #ifdef DEBUG
                Serial.print("followNoteOff on track ");
                Serial.print(playingNotes.readTrack());
                Serial.print(" at index ");
                Serial.print(foo);
                Serial.print(" with midiNote ");
                Serial.println(playingNotes.readMidiNote());
    #endif
                synth.endNote(playingNotes.readTrack(), 
                                playingNotes.readMidiNote());

    #ifdef DEBUG
                playingNotes.printList();
                Serial.println();
    #endif            
                playingNotes.dropNode();
            }
            playingNotes.readNext();
            foo++;

            if(++sentry == 100)
            {
                inout.ShowErrorOnLCD("followNO stuck");
                break;
            }
        }
    }
}


void handleRewindButton()
{
    if (inout.checkRewindButton()) 
    {      
        if(playbackOn == true) stopPlayback();
        Serial.print("1 ");
        synth.allNotesOff();
        Serial.print("2 ");
        sequencer.resetStepPositions();
        Serial.print("3 ");
        startFromZero = true;
        Serial.print("4 ");
        inout.RemoveStepIndicatorOnLCD();
        Serial.print("5 ");
        inout.ShowModeOnLCD();
        Serial.print("6 ");

        if(&activeNotes != NULL)
            activeNotes.purge();
        else
            inout.ShowErrorOnLCD("handleRB aN NULL");
        Serial.print("7 ");

        if(&activeStepClicks != NULL)
            activeStepClicks.purge();
        else
            inout.ShowErrorOnLCD("handleRB aSC NULL");
        Serial.print("8 ");
    }
}


void stopPlayback()
{
     metro.stopMidiTimer();
     playbackOn = false;
}

// SD Card business:

bool backupTrackFile()
{
    File myFile;
    File bupFile;
    bool success = false;

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
          success = true;
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
    return success;
}

bool writeTrackToSDcard(byte trackNum)
{
    File myFile;
    bool success = false;

    myFile = SD.open("seqs.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (myFile) {

        Serial.println("writeTrackToSDcard: seqs.txt opened successfully");

        byte trackBuf = sequencer.getCurrentTrack();
        int seqBuf = sequencer.getCurrentSequence();
        
        sequencer.setCurrentTrack(trackNum);

        myFile.println();
        myFile.println();
        myFile.print("Track = ");
        myFile.println(trackNum);

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
        success = true;
        Serial.println("Sequence saving done.");
        Serial.println();
        sequencer.setCurrentTrack(trackBuf);
        sequencer.setCurrentSequence(seqBuf);
  
    } else {
      // if the file didn't open, print an error:
      Serial.println("writeTrackToSDcard: error opening seqs.txt");
    }      
    return success;
}

bool readTracksFromSDcard()
{
    File myFile;
    bool success = false;

    char buffer[40]; // May need to be a bit bigger if you have long names
    byte index = 0;
    int trackBuf = sequencer.getCurrentTrack();
    int seqBuf = sequencer.getCurrentSequence();

    Serial.print("readTracksFromSDcard: track before loading is ");
    Serial.println(trackBuf);
    sequencer.setCurrentTrack(1); //backwards compatibility

    myFile = SD.open("seqs.txt");
    if (!myFile) {
      // if the file didn't open, print an error:
      Serial.println("readTracksFromSDcard: error opening seqs.txt on SD Card");
      inout.ShowErrorOnLCD("SD Card RT file err");
      myFile.close();
    } else {
      Serial.println("seqs.txt:");

      while (myFile.available())
      {
        char c = myFile.read();
        if(c == '\n' || c == '\r') // Test for <cr> and <lf>
        {
           parseAndAssignTrackSD(buffer);
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
      success = true;
    }
    sequencer.setCurrentTrack(trackBuf);
    sequencer.setCurrentSequence(seqBuf);
    return success;
}

void parseAndAssignTrackSD(char *buff)
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
            if(strcmp(name, "Track") == 0) 
            {
                Serial.print("Track ");
                Serial.println(valu);

                sequencer.setCurrentTrack(atoi(valu));

                Serial.print("  Mem: ");
                Serial.println(FreeMem());
            }
            
            if(strcmp(name, "Sequence") == 0) 
            {
                Serial.print("Sequence ");
                Serial.println(valu);

                sequencer.setCurrentSequence(atoi(valu));

                Serial.print("  Mem: ");
                Serial.println(FreeMem());
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

bool backupPatchFile()
{
    File myFile;
    File bupFile;
    bool success = false;

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
          success = true;
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
    return success;
}

bool writePatchesToSDcard()
{
    File myFile;
    bool success = false;
    myFile = SD.open("snds.txt", FILE_WRITE);
    
    // if the file opened okay, write to it:
    if (!myFile) {
        // if the file didn't open, print an error:
        Serial.println("writePatchesToSDcard: error opening snds.txt");
    } else {

        Serial.println("writePatchesToSDcard: snds.txt opened successfully");

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
        success = true;
        Serial.println("Sound patches saving done.");
        Serial.println();
    }      
    return success;
}

bool readPatchesFromSDcard()
{
    File myFile;
    bool success = false;

    char buffer[40];
    byte index = 0;
       
    myFile = SD.open("snds.txt");
    if (!myFile) {
      // if the file didn't open, print an error:
      Serial.println("readPatchesFromSDcard: error opening snds.txt on SD Card");
      inout.ShowErrorOnLCD("SD Card RP file err");
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
      success = true;
      Serial.println();      
    }
    return success;
}

// Original SD Card business:
void writeToSDcard()
{   
    File myFile;
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
    File myFile;

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

              Serial.print("Sequence ");
              Serial.println(valu);

              sequencer.setCurrentSequence(atoi(valu));

              Serial.print("  Mem: ");
              Serial.println(FreeMem());

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
    File myFile;
       
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

float midiToFrequency(int note)
{  
  return (float) 440.0 * (float)(pow(2, (note-57) / 12.0));
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


void printLinkedNoteList(LinkedNoteList *list)
{
    while( list->hasValue()){
            Serial.print("Note: ");
            Serial.print(list->getNote().pitchFreq);
            Serial.print("  Step: ");
            Serial.println(list->getStep());
            list->next();
    }
    Serial.print("Count: ");
    Serial.println(list->count());
 }

void testLinkedNoteList() 
{
    Serial.println("Testing LinkedNoteList");

    LinkedNoteList list;

    note noteOne;
    note noteTwo;
    note noteThree;

    noteOne.pitchFreq = 110;
    noteTwo.pitchFreq = 220;
    noteThree.pitchFreq = 330;

    list.appendNote(1, 1, noteOne);
    list.appendNote(2, 2, noteTwo);
    list.appendNote(3, 3, noteThree);

    list.rewind(); 
    printLinkedNoteList(&list);
    
    list.dropNotesBeforeStepAndRewind(2);
//  cout << eol << "dropNotesBeforeStepAndRewind 2 " << eol << eol;
    Serial.println("dropNotesBeforeStepAndRewind 2 ");

    list.rewind(); 
    printLinkedNoteList(&list);

    list.prependNote(1, 1, noteOne);
//  cout << eol << "prepending 1 back " << eol << eol;
    Serial.println("prepending 1 back ");

    list.rewind(); 
    printLinkedNoteList(&list);

    list.dropNotesBeforeStepAndRewind(4);
    Serial.println("dropNotesBeforeStepAndRewind 4 ");

    list.rewind(); 
    printLinkedNoteList(&list);

    list.prependNote(1, 1, noteOne);
    Serial.println("prepending 1 back to empty list ");

    list.rewind(); 
    printLinkedNoteList(&list);

    list.dropNotesBeforeStepAndRewind(4);
    Serial.println("dropNotesBeforeStepAndRewind 4 ");

    list.rewind(); 
    printLinkedNoteList(&list);

    list.appendNote(1, 1, noteOne);
    Serial.println("appending 1 back to empty list ");

    list.rewind(); 
    printLinkedNoteList(&list);
}

void testLinkedTrackList() 
{
    Serial.println();
    Serial.println("Testing LinkedTrackList");

    LinkedTrackList list;

    Track trackOne;
    Track trackTwo;
    Track trackThree;
    Track *trackCheck;

    trackOne.begin(1);
    trackTwo.begin(2);
    trackThree.begin(3);

    list.appendTrack(1, &trackOne);
    list.appendTrack(2, &trackTwo);
    list.appendTrack(3, &trackThree);

    list.rewind();
    while( list.hasValue()){
            Serial.print("Track: ");
            Serial.print(list.getTrackNumber());
            Serial.print("  Ping: ");
            trackCheck = list.getTrackRef();
            trackCheck->activate();
            list.next();
    }
    
    list.dropTrack(2);
    Serial.println("dropTrack 2 ");

    list.rewind();
    while( list.hasValue()){
            Serial.print("Track: ");
            Serial.print(list.getTrackNumber());
            Serial.print("  Ping: ");
            trackCheck = list.getTrackRef();
            trackCheck->activate();
            list.next();
    }
    
    list.appendTrack(1, &trackTwo);
    Serial.println("appending 2 back ");

    list.rewind();
    while( list.hasValue()){
            Serial.print("Track: ");
            Serial.print(list.getTrackNumber());
            Serial.print("  Ping: ");
            trackCheck = list.getTrackRef();
            trackCheck->activate();
            list.next();
    }    
}

void printPerClickNoteList(PerClickNoteList *list)
{
    while( list->hasValue()){
        Serial.print("Note: ");
//      note *n = list->getNote();
        note n = list->getNote();
//      if(n != NULL)
//        Serial.print(n.pitchVal);
//      else
//        Serial.print("NULL");
        Serial.print("  pitch: ");
        Serial.print(n.pitchVal);
        Serial.print("  durMS: ");
        Serial.print(list->getDurationMS());
        Serial.print("  cur: ");
        Serial.print(list->getCur());
        Serial.print("  next: ");
        Serial.println(list->getNext());
        list->next();
    }
}

void printStepClickList(StepClickList *list)
{
    while( list->hasValue()){
            Serial.print(" MasterStep: ");
            Serial.print(list->getMasterStep());
            Serial.print("  Click: ");
            Serial.println(list->getClickStep());
            Serial.println(" Notes: ");
            PerClickNoteList *notes = list->getNotes();
            notes->rewind();
            printPerClickNoteList(notes);
            Serial.println();
            list->next();
    }
}

void testPerClickNoteList() 
{
    Serial.println();
    Serial.println("Testing PerClickNoteList");
    
    PerClickNoteList list;

    note noteOne;
    note noteTwo;
    note noteThree;

    noteOne.pitchVal = 1;
    noteTwo.pitchVal = 2;
    noteThree.pitchVal = 3;
    
    list.append(noteOne, 1, 100);
    list.append(noteTwo, 2, 200);
    list.append(noteThree, 3, 300);
    
    printPerClickNoteList(&list);
    list.rewind();
 
    list.append(noteThree, 3, 300);
    Serial.println("Appended 3 again after rewind");

    printPerClickNoteList(&list);
    
    list.append(noteOne, 1, 100);
    list.append(noteTwo, 2, 200);
    list.append(noteThree, 3, 300);
    Serial.println("Rebuilt list.");

    printPerClickNoteList(&list);

    Serial.println("Not calling Delete on list.");
}

void testStepClickList() 
{
    Serial.println();
    Serial.println("Testing StepClickList");
    
    StepClickList list;

    note noteOne;
    note noteTwo;
    note noteThree;

    noteOne.pitchVal = 1;
    noteTwo.pitchVal = 2;
    noteThree.pitchVal = 3;
    
    list.addClickNote(noteOne, 1, 1100, 1, 1);
    list.addClickNote(noteTwo, 2, 2100, 1, 1);
    list.addClickNote(noteThree, 3, 3100, 1, 1);
/*
    list.addClickNote(&noteOne, 1, 1100, 1, 1);
    list.addClickNote(&noteTwo, 2, 2100, 1, 1);
    list.addClickNote(&noteThree, 3, 3100, 1, 1);
*/
    Serial.println("Three notes on masterStep 1 click 1");

    list.rewind();
    printStepClickList(&list);
    list.rewind();
 
    list.addClickNote(noteOne, 1, 1200, 2, 1);
    list.addClickNote(noteTwo, 2, 2200, 2, 1);
    list.addClickNote(noteThree, 3, 3200, 2, 2);
/*
    list.addClickNote(&noteOne, 1, 1200, 2, 1);
    list.addClickNote(&noteTwo, 2, 2200, 2, 1);
    list.addClickNote(&noteThree, 3, 3200, 2, 2);
*/
    Serial.println("Appended 2 notes on masterStep 2 click 1 and 1 note on masterStep 2 click 2");

    list.rewind();
    printStepClickList(&list);
    list.rewind();

    list.addClickNote(noteOne, 1, 1300, 1, 1);
    list.addClickNote(noteTwo, 2, 2300, 2, 1);
    list.addClickNote(noteThree, 3, 3300, 2, 2);

/*
    list.addClickNote(&noteOne, 1, 1300, 1, 1);
    list.addClickNote(&noteTwo, 2, 2300, 2, 1);
    list.addClickNote(&noteThree, 3, 3300, 2, 2);
*/
    Serial.println("Appended another note on each click");
    
    list.rewind();
    printStepClickList(&list);

    list.dropNotesBeforeStepAndRewind(2);

    Serial.println("Dropped notes before masterStep 2");
    
    list.rewind();
    printStepClickList(&list);
/*
    delete &list;
    Serial.println("Deleted list.");

    printStepClickList(&list);
*/
    list.addClickNote(noteOne, 1, 1400, 1, 1);
    list.addClickNote(noteTwo, 2, 2400, 2, 1);
    list.addClickNote(noteThree, 3, 3400, 2, 2);

    Serial.println("Added 3 more.");

    list.rewind();
    printStepClickList(&list);
}

uint32_t FreeMem(){ // for Teensy 3.0
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t) &stackTop;

    // current position of heap.
    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);

    // The difference is (approximately) the free, available ram.
    return stackTop - heapTop;
}