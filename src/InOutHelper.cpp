
#define ENCODER_DO_NOT_USE_INTERRUPTS

#include "Arduino.h"
#include "InOutHelper.h"
#include <Encoder.h>
//#include <Wire.h>
#include <Bounce2.h>
#include "Adafruit_Trellis.h"
#include "pins_arduino.h"     
//#include <LiquidCrystalFast.h>
#include "Adafruit_LiquidCrystal.h"

#include "Path.h"
#include "Timebase.h"
#include "StepSequencer.h"
#include "SynthEngine.h"

// Encoders:
Encoder EncA(0, 1);
Encoder EncB(2, 3);
Encoder EncC(4, 5);
Encoder EncD(6, 8);

//Adafruit_LiquidCrystal lcd(0);        // I2c
Adafruit_LiquidCrystal lcd(34, 35, 33); // SPI

// Mode from main file
extern int currentMode;
extern const char *modeNames[];
extern StepSequencer sequencer;
extern Timebase metro;
extern SynthEngine synth;
extern int save_pattern_destination;
extern bool save_to_SD_done;
extern bool shiftActive;
extern bool g_queueing;
extern bool g_queuePrimed;

//floating
int putInRange(int iVar, int iRange)
{
    while(iVar < 0) iVar += (iRange);
    int retval = iVar % iRange;
    return retval;
}



//Public constructor and methods
InOutHelper::InOutHelper()
{
    // Just a hello
    Serial.begin(115200);
    Serial.println("InOutHelper: 2 trellises, 4 encoders and an lcd display walk into a bar...");
  
    // set up the LCD's number of rows and columns:
    lcd.begin(20, 4);
  
    // Print a message to the LCD.
    lcd.print("Knequencer");
  
    // Joystick
    pinMode(JoystickX_PIN, INPUT_PULLUP);
    pinMode(JoystickY_PIN, INPUT_PULLUP);
  
    // Arcade buttons  
    pinMode(PlayButton_PIN, INPUT_PULLUP);
    pinMode(RewindButton_PIN, INPUT_PULLUP);
    pinMode(SelectButton_PIN, INPUT_PULLUP);
    pinMode(PatternModeButton_PIN, INPUT_PULLUP);
    pinMode(MuteModeButton_PIN, INPUT_PULLUP);
    pinMode(StepEditModeButton_PIN, INPUT_PULLUP);
    pinMode(AccentEditModeButton_PIN, INPUT_PULLUP);
    pinMode(LengthEditModeButton_PIN, INPUT_PULLUP);
    pinMode(PathEditModeButton_PIN, INPUT_PULLUP);
  
    PlayButton.attach(PlayButton_PIN);
    RewindButton.attach(RewindButton_PIN);
    SelectButton.attach(SelectButton_PIN);
    PatternModeButton.attach(PatternModeButton_PIN);
    MuteModeButton.attach(MuteModeButton_PIN);
    StepEditModeButton.attach(StepEditModeButton_PIN);
    AccentEditModeButton.attach(AccentEditModeButton_PIN);
    LengthEditModeButton.attach(LengthEditModeButton_PIN);
    PathEditModeButton.attach(PathEditModeButton_PIN);
  
    PlayButton.interval(BUTTON_DEBOUNCE_TIME);
    RewindButton.interval(BUTTON_DEBOUNCE_TIME);
    SelectButton.interval(BUTTON_DEBOUNCE_TIME);
    PatternModeButton.interval(BUTTON_DEBOUNCE_TIME);
    MuteModeButton.interval(BUTTON_DEBOUNCE_TIME);
    StepEditModeButton.interval(BUTTON_DEBOUNCE_TIME);
    AccentEditModeButton.interval(BUTTON_DEBOUNCE_TIME);
    LengthEditModeButton.interval(BUTTON_DEBOUNCE_TIME);
    PathEditModeButton.interval(BUTTON_DEBOUNCE_TIME);
    
    // Encoder buttons
    pinMode(EncButA_PIN, INPUT_PULLUP);
    pinMode(EncButB_PIN, INPUT_PULLUP);
    pinMode(EncButC_PIN, INPUT_PULLUP);
    pinMode(EncButD_PIN, INPUT_PULLUP);
  
    EncButA.attach(EncButA_PIN);
    EncButB.attach(EncButB_PIN);
    EncButC.attach(EncButC_PIN);
    EncButD.attach(EncButD_PIN);
  
    EncButA.interval(BUTTON_DEBOUNCE_TIME);
    EncButB.interval(BUTTON_DEBOUNCE_TIME);
    EncButC.interval(BUTTON_DEBOUNCE_TIME);
    EncButD.interval(BUTTON_DEBOUNCE_TIME);
}


void InOutHelper::begin(ReactToInputBool updateModeCbPointer,
                        ReactToInputIntArray updateRepetitionCbPointer,
                        ReactToInputInt updatePatternLengthCbPointer,
                        ReactToInputInt updatePatternNumberCbPointer,
                        ReactToInputInt updateSavePatternDestPointer,
                        ReactToInputInt updateTempoPointer,
                        ReactToInputSpeedFactor updateSpeedMultiplierPointer,
                        ReactToInput SaveToSdCbPointer,
                        ReactToInput startStopCbPointer,
                        ReactToInputInt updateSynthCbPointer,
                        ReactToInputInt updateTrackCbPointer,
                        ReactToInputAction recordActionCbPointer) {

  updateModeCb = updateModeCbPointer;
  updateRepetitionCb = updateRepetitionCbPointer;
  updatePatternLengthCb = updatePatternLengthCbPointer;
  updatePatternNumberCb = updatePatternNumberCbPointer;
  updateSavePatternDestCb = updateSavePatternDestPointer;
  updateTempoCb = updateTempoPointer;
  updateSpeedMultiplierCb = updateSpeedMultiplierPointer;
  updateTrackCb = updateTrackCbPointer;
  recordActionCb = recordActionCbPointer;
  
  SaveToSdCb = SaveToSdCbPointer;
  startStopCb = startStopCbPointer;
  updateSynthCb = updateSynthCbPointer;

  // no arcade button presses tracked
  for(byte i = 0; i < HOLDABLEBUTTONCOUNT; i++) {
    HoldableButtonPressedTime[i] = 0;
  }
  
  // no step selected
  for (uint8_t i = 0; i < 16; i++) {
    selectedSteps[i] = false;
  }
  
  // no selection stored for cycling
  for (uint8_t i = 0; i < 16; i++) {
    selectionBuffer[i] = false;
  }

  // no playback steps lit
  for (uint8_t i = 0; i < 16; i++) {
    stepLedOffTimes[i] = NULL;
  }


  // no button press processed
  for (uint8_t i = 0; i < 32; i++) {
    pressProcessed[i] = false;
  }

  // Trellisses
  // light up all the LEDs in order
  // then turn them off
  trellis.begin(0x70, 0x71);

  //supposedly a faster i2c bus
  //TWBR = 12; 

  for (uint8_t i = 0; i < numKeys; i++) {
    trellis.setLED(i);
    trellis.writeDisplay();
    delay(10);
  }
  for (uint8_t i = 0; i < numKeys; i++) {
    trellis.clrLED(i);
    trellis.writeDisplay();
    delay(10);
  }
  delay(20);
  setupNewMode();
  ShowModeOnLCD();
  ShowBPMOnLCD(metro.getEffectiveBPM());
  ShowSwingOnLCD(metro.getSwing());
  ShowPathNumberOnLCD(sequencer.getPath());
  ShowTrackNumberOnLCD(sequencer.getCurrentTrack());
}


void InOutHelper::setupNewMode() {
      
    switch (currentMode) {
      case pattern_select:

/*
        for (uint8_t i = 0; i < 16; i++) {
          Serial.print(stepLedOffTimes[i]);
          Serial.print("  ");
          Serial.println(stepsToCheck[i]);
        }        
*/      
        StepButtonCb = updatePatternNumberCb; // selection handling as a callback instead ?        
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupPatternSelectModeTrellis();
        break;      
      case pattern_save:
        StepButtonCb = updateSavePatternDestCb; // selection handling as a callback instead ?        
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupPatternSaveModeTrellis();
        break;      
      case chain_edit:
        StepButtonCb = NULL;    
        StartStopButtonCb = startStopCb; // use a separate one for chain play ?
        initTrackEncoder = true;
        SetupChainEditModeTrellis();
        break;  

      case step_mute: 
        StepButtonCb = NULL; // selection handling as a callback instead ?
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupMuteOrHoldModeTrellis();
        break;
      case step_hold:
        StepButtonCb = NULL; // selection handling as a callback instead ?
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupMuteOrHoldModeTrellis();
        break;      
      case step_edit: 
        StepButtonCb = NULL; // selection handling as a callback instead ?
        StartStopButtonCb = startStopCb;
        SetupSelectEditTrellis();
        break;
      case track_select:
        StepButtonCb = updateTrackCb;    
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupTrackSelectModeTrellis();
        break;      
      case track_mute:
        StepButtonCb = NULL;
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupTrackMuteModeTrellis();
        break;      
      case accent_edit:
        StepButtonCb = NULL; // selection handling as a callback instead ?
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupAccentModeTrellis();
        break;
      case length_edit: 
        StepButtonCb = updatePatternLengthCb;
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupLengthModeTrellis();
        break;
      case path_select:
        StepButtonCb = NULL;
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupPathSelectModeTrellis();
        break;
      case synth_edit:
        StepButtonCb = updateSynthCb;
        StartStopButtonCb = startStopCb;
        initTrackEncoder = true;
        SetupSynthEditTrellis();
        break;
      case save_to_sd:
        StepButtonCb = NULL;
        StartStopButtonCb = startStopCb;
//      StartStopButtonCb = SaveToSdCb;
        SetupSaveModeTrellis();
        break;
      default:
        Serial.print("Uncovered currentMode in setupNewMode(): ");
        Serial.print(currentMode);
    }

}


void InOutHelper::ResetTrellis() {

    Wire.endTransmission();


// change pin mux to digital I/O
    pinMode(SDA,INPUT);
    pinMode(SCL,OUTPUT);
    digitalWrite(SCL,HIGH);
    
    int count = 0;
    while(digitalRead(SDA) == 0 && count++ < 10)
    {
        digitalWrite(SCL,LOW);
        delayMicroseconds(5);       // 10us period == 100kHz
        digitalWrite(SCL,HIGH);
        delayMicroseconds(5);
    }
        
//  TWCR = 0; // reset TwoWire Control Register to default, inactive state

    trellis.begin(0x70, 0x71);

    for (uint8_t i = 0; i < numKeys; i++) {
      trellis.clrLED(i);
    }
    trellis.writeDisplay();

    ClearBoolSteps(helperSteps, 16);
    for (uint8_t i = 0; i < 16; i++) {
      selectedSteps[i] = false;
      ShowStepStateOnLCD(i, NOBUTTONPRESS);
    }

    // no playback steps lit
    for (uint8_t i = 0; i < 16; i++) {
      stepLedOffTimes[i] = NULL;
    }

    stepSelectionMode = NOSTEPS;
    selectionChanged = true;

    currentMode = step_edit;
    setupNewMode();
    ShowModeOnLCD();         
    ShowBPMOnLCD(metro.getEffectiveBPM());
    ShowSwingOnLCD(metro.getSwing());
    step_indicator_led_active = true; // ???
}


void InOutHelper::SetupMuteOrHoldModeTrellis() {

    stepsToCheck = helperSteps;      
    RetrievePatternStates();
    
    LiteUpTrellisSteps(helperSteps);
}


void InOutHelper::SetupSelectEditTrellis() {
    
    stepsToCheck = selectedSteps;
    LiteUpTrellisSteps(selectedSteps);
}


void InOutHelper::SetupAccentModeTrellis() {

    stepsToCheck = helperSteps;      
    RetrievePatternStates();
    
    LiteUpTrellisSteps(helperSteps);
}


void InOutHelper::SetupLengthModeTrellis() {

    stepsToCheck = helperSteps;
    ClearBoolSteps(helperSteps, 16);
    helperSteps[sequencer.getLength() - 1] = true;
    
    LiteUpTrellisSteps(helperSteps);
}


void InOutHelper::SetupPatternSelectModeTrellis() {
    
    int seqNum = sequencer.getCurrentPattern();
    
    stepsToCheck = helperSteps;
    ClearBoolSteps(helperSteps, 16);
    helperSteps[seqNum] = true;
    
    LiteUpTrellisSteps(helperSteps);
    ShowPatternNumberOnLCD(seqNum);
}


void InOutHelper::SetupPatternSaveModeTrellis() {
    
    int seqNum = sequencer.getCurrentPattern();
    
    stepsToCheck = helperSteps;
    ClearBoolSteps(helperSteps, 16);
    helperSteps[seqNum] = true;
    
    LiteUpTrellisSteps(helperSteps);
    ShowPatternNumberOnLCD(seqNum);
}


void InOutHelper::SetupChainEditModeTrellis() {
    
    // TODO
    
//  int seqNum = sequencer.getCurrentPattern();
    
    stepsToCheck = helperSteps;
    ClearBoolSteps(helperSteps, 16);
//  helperSteps[seqNum] = true;
    
    LiteUpTrellisSteps(helperSteps);
//  ShowPatternNumberOnLCD(seqNum);
}


void InOutHelper::SetupPathSelectModeTrellis() {
    // ########## TODO
    int pathNum = sequencer.getPath();

    stepsToCheck = helperSteps;
    ClearBoolSteps(helperSteps, 16);
    helperSteps[pathNum] = true;
    
    LiteUpTrellisSteps(helperSteps);
}


void InOutHelper::SetupSynthEditTrellis() {
  
    int patchNum = synth.getCurrentPatchNumber();

    stepsToCheck = helperSteps;
    ClearBoolSteps(helperSteps, 16);
    helperSteps[patchNum] = true;

    LiteUpTrellisSteps(helperSteps);
}


void InOutHelper::SetupSaveModeTrellis() {
    
    //TODO: NEEDS A SELECTION ARRAY
    
    stepsToCheck = helperSteps;
    ClearBoolSteps(helperSteps, 16);
    LiteUpTrellisSteps(helperSteps);
}


void InOutHelper::SetupTrackSelectModeTrellis() {
    
    int trackNum = sequencer.getCurrentTrack();
    
    stepsToCheck = helperSteps;
    ClearBoolSteps(helperSteps, 16);
    helperSteps[trackNum - 1] = true;
    
    LiteUpTrellisSteps(helperSteps);
    ShowTrackNumberOnLCD(trackNum);
}

void InOutHelper::SetupTrackMuteModeTrellis() {
    
    stepsToCheck = helperSteps;      
    ClearBoolSteps(helperSteps, 16);
    sequencer.retrieveMutedTracks(helperSteps, 16);
    LiteUpTrellisSteps(helperSteps);
}


//Loop handlers
void InOutHelper::handleEncoders() {
    switch (currentMode) {
      case synth_edit:
          HandleSynthEncoders();
          break;

      case pattern_select:
      case pattern_save:
      case track_select:
      case track_mute:
      case length_edit:
      case path_select:
      case accent_edit:
      case step_mute:
      case step_hold:
          HandleTrackEncoderA();
          HandlePerformanceEncoders();
          break;
      
      default:
          HandleTranspositionEncoderA();
          HandlePerformanceEncoders();
    }

}

void InOutHelper::HandleSynthEncoders() {
    long newPosition;
    static long oldPositionA  = 0;    
    static int valueA = 0;
    static int prevValueA = 0;
    static long oldPositionB  = 0;
    static int valueB = 0;
    static int prevValueB = 0;
    static int accTrackerB = 0;

    // use selectionChanged to track parameter choice
    if (selectionChanged) {
      selectionChanged = false;
//    ClearInfoOnLCD();
      
      EncA.write(0);
      oldPositionA = 0;
      valueA = 0;
      prevValueA = 0;
      
      EncB.write(0);
      oldPositionB = 0;  
      valueB = 0;
      prevValueB = 0;
    }

    newPosition = EncA.read();

    if (newPosition != oldPositionA)
    {
      valueA = newPosition / 4;
      if (valueA != prevValueA)
      {
        synth.handleEncoder(EncoderA, valueA);
        prevValueA = valueA;
      }
//    ShowValueInfoOnLCD("Transpose:", transposition); DO THIS IN SYNTHENGINE / PATCH
      oldPositionA = newPosition;
    }
    
    newPosition = EncB.read();
    
    if (!EncButB.read()) { 
      if (accTrackerB < 5)
        newPosition = oldPositionB + (newPosition - oldPositionB) * 10;
      else
        if (accTrackerB < 10)
          newPosition = oldPositionB + (newPosition - oldPositionB) * 20;
        else      
          newPosition = oldPositionB + (newPosition - oldPositionB) * 100;
          
      EncB.write(newPosition);
      
    } else {
      accTrackerB = 0;
    }

    if (newPosition != oldPositionB) {
      
      accTrackerB++;  
      valueB = newPosition / 4;

      if (valueB != prevValueB)
      {
        synth.handleEncoder(EncoderB, valueB);
        prevValueB = valueB;
      }
      oldPositionB = newPosition;
    }
}

void InOutHelper::HandleTranspositionEncoderA() {
    long newPosition;
//  float newPosFloat;
    static long oldPositionA  = 0;
    static int transposition = 0;  // static or not ?
    static int prevTransposition;
    static byte pitchChangeLowerBoundary;
    static byte pitchChangeUpperBoundary;

    if (selectionChanged) {
//    preserve the flag for the performance encoders right afterwards
//    selectionChanged = false;

      transposition = 0;
      prevTransposition = 0;
      EncA.write(0);
      oldPositionA = 0;
      pitchChangeLowerBoundary = sequencer.getLowestSelectedNote(selectedSteps);
      pitchChangeUpperBoundary = 127 - sequencer.getHighestSelectedNote(selectedSteps);
    }
    
    // Encoder A for track selection
    newPosition = EncA.read();

    if ((newPosition >= oldPositionA + 4) || (newPosition <= oldPositionA - 4)) 
    {

        if(newPosition < 0 - pitchChangeLowerBoundary * 4) {        
          newPosition = pitchChangeLowerBoundary * -4;
          EncA.write(newPosition);
//        encTimes[0] = (unsigned long)encTime;
        } else if(newPosition > pitchChangeUpperBoundary * 4) {
          newPosition = pitchChangeUpperBoundary * 4;
          EncA.write(newPosition);
//        encTimes[0] = (unsigned long)encTime;
        }
        transposition = newPosition / 4;

        if (transposition != prevTransposition)
        {
          byte newNote = sequencer.offsetSelectedNotes(selectedSteps, transposition - prevTransposition, heldTrellisStep);  
          prevTransposition = transposition;
          char noteName[4];
          ShowValueInfoOnLCD("Transpose:", transposition, midiToNoteChar(newNote, noteName));
          SetLCDinfoTimeout();
        }
//      encTimes[1] = (unsigned long)encTime;

//      encTimes[2] = (unsigned long)encTime;
//      encTimes[3] = (unsigned long)encTime;
        
        oldPositionA = newPosition;
/*
        for(int f=0; f<4; f++)
        {
          Serial.print(encTimes[f]);
          Serial.print(" ");
        }
        Serial.println();
*/
    }
}


void InOutHelper::HandleTrackEncoderA() {
    long newPosition;
    static long oldPositionA = 0;
    static int prevTrackNum;
    static byte lowerTrackBoundary = 1;
    static byte upperTrackBoundary = TRACKCOUNT;

    if (initTrackEncoder) {
      initTrackEncoder = false;
      prevTrackNum = sequencer.getCurrentTrack();
      oldPositionA = prevTrackNum * 4;
      EncA.write(oldPositionA);
    }

    // Encoder A for track selection
    newPosition = EncA.read();
    if ((newPosition <= oldPositionA - 4) || (newPosition >= oldPositionA + 4)) 
    {  
      int trackNum = constrain(sequencer.getCurrentTrack() 
                               + (newPosition - oldPositionA) /4, 
                               lowerTrackBoundary, 
                               upperTrackBoundary);

      if (trackNum != prevTrackNum)
      {
          sequencer.setCurrentTrack(trackNum);  

          ShowValueInfoOnLCD("Track:", trackNum);
          SetLCDinfoTimeout();

          if(currentMode == track_select)
          {
              helperSteps[trackNum - 1] = true;
              helperSteps[prevTrackNum - 1] = false;
              trellis.setLED(trackNum - 1 + STEPSOFFSET);
              trellis.clrLED(prevTrackNum - 1 + STEPSOFFSET);
              trellis_led_dirty = true;
          } else // generally refresh the display to match changed track
          {
              ShowTrackNumberOnLCD(trackNum);
              ShowPathNumberOnLCD(sequencer.getPath());
              setupNewMode();
          }
          prevTrackNum = trackNum;
      }
      oldPositionA = newPosition;
    }    
}


void InOutHelper::HandlePerformanceEncoders() {
    long newPosition;
    float newPosFloat;
//  static long oldPositionA  = 0;
    static float oldPositionBfloat  = 0;
    static long oldPositionC  = 0;
    static long oldPositionD  = 0;
    static int beatsPerMinute = metro.getBPM();
    static int prevBeatsPerMinute;
//  static int transposition = 0;  // static or not ?
//  static int prevTransposition;
    static int swingClicks = metro.getSwing();

    static int prevSwingTiming;
    static float durationChange;
    static float prevDurationChange;
//  static byte pitchChangeLowerBoundary;
//  static byte pitchChangeUpperBoundary;
    static float durationChangeLowerBoundary;
    static float durationChangeUpperBoundary;

    if (selectionChanged) {
      selectionChanged = false;
/*
      transposition = 0;
      prevTransposition = 0;
      EncA.write(0);
      oldPositionA = 0;
      pitchChangeLowerBoundary = sequencer.getLowestSelectedNote(selectedSteps);
      pitchChangeUpperBoundary = 127 - sequencer.getHighestSelectedNote(selectedSteps);
*/
      durationChange = 0;
      prevDurationChange = 0;
      EncB.write(0);
      oldPositionBfloat = 0;
      durationChangeLowerBoundary = sequencer.getShortestSelectedNote(selectedSteps);
      durationChangeUpperBoundary = 1.0 - sequencer.getLongestSelectedNote(selectedSteps);
    }

    // Encoder B for Note Duration - a per-sequence-step edit
    newPosFloat = (float)EncB.read() /100.0;
//  if (newPosFloat != oldPositionBfloat) {
    if ((newPosFloat >= oldPositionBfloat + .04) || (newPosFloat <= oldPositionBfloat - .04)) {
      
//    Serial.print("posdelta: ");   
//    Serial.println(newPosFloat - oldPositionBfloat);   

      if(newPosFloat < 0 - durationChangeLowerBoundary * 4) {        
        newPosFloat = durationChangeLowerBoundary * -4;
        EncB.write(newPosFloat * 100);
      } else if(newPosFloat > durationChangeUpperBoundary * 4) {
        newPosFloat = durationChangeUpperBoundary * 4;
        EncB.write(newPosFloat * 100);
      }

      durationChange = newPosFloat / 4;
      if (durationChange != prevDurationChange)
      {
        sequencer.offsetSelectedDurations(selectedSteps, durationChange - prevDurationChange, heldTrellisStep);  
        prevDurationChange = durationChange;
      }

      ShowValueInfoOnLCD("NoteDur:", (int) (durationChange * 100));
      SetLCDinfoTimeout();

      oldPositionBfloat = newPosFloat;
    }
    
    // Encoder C for Swing - a global setting
    newPosition = EncC.read();
    if ((newPosition <= oldPositionC - 4) || (newPosition >= oldPositionC + 4)) {
      
      swingClicks = constrain(swingClicks + (newPosition - oldPositionC) /4, 0, 8);
      if (swingClicks != prevSwingTiming)
      {
        metro.updateSwing(swingClicks);
        prevSwingTiming = swingClicks;
      }
      
      ShowSwingOnLCD(swingClicks);
      oldPositionC = newPosition;
    }

    // Encoder D for BPM - a global setting
    newPosition = EncD.read();
    if ((newPosition <= oldPositionD - 4) || (newPosition >= oldPositionD + 4)) {
  
      beatsPerMinute = constrain(beatsPerMinute + (newPosition - oldPositionD) /4, 1, 300);
      if (beatsPerMinute != prevBeatsPerMinute)
      {
        updateTempoCb(beatsPerMinute);
        prevBeatsPerMinute = beatsPerMinute;
      }
  
//    ShowBPMOnLCD(beatsPerMinute);
      oldPositionD = newPosition;
      }
}


void InOutHelper::handleButtonHoldTiming(holdableButton buttn, bool pressed) {

    if (pressed)
      HoldableButtonPressedTime[buttn] = millis();
    else {
      HoldableButtonPressedTime[buttn] = 0;

      // clean out tracking of button hold... add buttons as needed
      switch (buttn)
      {
        case PATTERNMODEBUTTON:
          if (currentHoldAction == SAVESEQ) {
            currentHoldAction = NONE;
            holdActionState = INACTIVE;
          }
          break;
        case SYNTHPATCHBUTTON:  // yes, hacky
          if (currentHoldAction == SAVEPATCH) {
            currentHoldAction = NONE;
            holdActionState = INACTIVE;
          }          
          break;
        case STEPEDITMODEBUTTON: 
          if (currentHoldAction == TRESET) {
            currentHoldAction = NONE;
            holdActionState = INACTIVE;
          }        
          break;
        case REWINDBUTTON: 
          if (currentHoldAction == SAVETOSD) {
            currentHoldAction = NONE;
            holdActionState = INACTIVE;
          }        
          break;
        case PLAYBUTTON: 
        case SELECTBUTTON: 
        case MUTEMODEBUTTON: 
        case ACCENTEDITMODEBUTTON: 
        case LENGTHEDITMODEBUTTON: 
        case PATHEDITMODEBUTTON:
        default:
          break;
      }
    }
}


unsigned long InOutHelper::GetHoldableButtonPressed(holdableButton buttn) {
    unsigned long retVal = 0;
    
    if (HoldableButtonPressedTime[buttn] > 0)
      retVal = millis() - HoldableButtonPressedTime[buttn];
      
    return retVal;
}


void InOutHelper::handleTrellis() {
    static unsigned long its_trellis_time = micros() + TRELLAY;

    unsigned long now = micros();
    if (now > its_trellis_time) {
      its_trellis_time = now + TRELLAY; // 30ms (20 !) delay is required, dont remove me!
      
      // Running step LED off when time expired
      // unless the LED is manually latched
      if (step_indicator_led_active) {
        UpdateTrellisStepIndicator(now);
      }
      if (trellis.readSwitches()) {
        // If a button was just pressed or released...
        // go through every button
        for (uint8_t i = 0; i < numKeys; i++) {
          // if it was pressed, turn it on
          if ((trellis.justPressed(i)) && (pressProcessed[i] == false)) ProcessTrellisButtonPress(i);
          if (trellis.justReleased(i)) ProcessTrellisButtonRelease(i);
        }
        // tell the trellis to set the LEDs we requested
        trellis_led_dirty = true;
      }
      
      if (trellis_led_dirty)
      {
//    noInterrupts();
          trellis.writeDisplay();
//    interrupts();
        trellis_led_dirty = false;
      }
  }

}


void InOutHelper::ProcessTrellisButtonRelease(uint8_t i)
{
    pressProcessed[i] = false;

    if (i < STEPSOFFSET) {          // left pad LEDs:
      trellis.clrLED(i);            // turn off on release
       
    } else {                           // for step button (right pad LED): update LCD step indicator
      ShowStepStateOnLCD(i % STEPSOFFSET, pressProcessed[i]);

      Serial.print("Trellis released: ");
      Serial.println(i % STEPSOFFSET);

      if (i == heldTrellisStep)
        heldTrellisStep = 255;

      switch (currentMode) {
        case synth_edit:
          ClearInfoOnLCD();
          handleButtonHoldTiming(SYNTHPATCHBUTTON, false);               
          synth.handleButtonRelease(i % STEPSOFFSET);

          // shorthand to light up current patch again
          // since long press may have saved to different one
          SetupSynthEditTrellis();
          
          break;
        case save_to_sd:     
          trellis.clrLED(i);
          break;
      }
    }
}


void InOutHelper::ProcessTrellisButtonPress(uint8_t i)
{
    trellis.setLED(i);
    pressProcessed[i] = true;
    
    switch (i) {
      case SYNTHEDITBUTTON:
        if (currentMode == pattern_select 
            || currentMode == save_to_sd 
            || currentMode == pattern_save) {

          currentMode = synth_edit;
          setupNewMode();
          ShowModeOnLCD();        
          selectionChanged = true;

//        updateModeCb(true);

        } else {
          ShowInfoOnLCD("Retrig 1");
          SetLCDinfoTimeout();
          RetrigButtonPressed(1);          
        }
        break;
      case SAVETOSDBUTTON: //mode button
        if (currentMode == pattern_select 
            || currentMode == synth_edit 
            || currentMode == pattern_save) {

          currentMode = save_to_sd;
          setupNewMode();
          ShowModeOnLCD();                  
          selectionChanged = true;   
     
        } else {
          ShowInfoOnLCD("No Retrig");
          SetLCDinfoTimeout();
          RetrigButtonPressed(0);          
        }
        break;
      case QUEUEBUTTON:
        if (currentMode == pattern_select  ||
            currentMode == length_edit ||
            currentMode == path_select) 
        {
          if(QueueButtonPressed())
            ShowInfoOnLCD("Queue Actions");
          else
            ShowInfoOnLCD("Queue End");
          SetLCDinfoTimeout();
        } else {
          ShowInfoOnLCD("Retrig 2");
          SetLCDinfoTimeout();
          RetrigButtonPressed(2);          
        }
        break;
      case QUEUEPRIMEBUTTON:
        if (currentMode == pattern_select  ||
            currentMode == length_edit ||
            currentMode == path_select) 
        {
          if(PrimeQueueButtonPressed())
            ShowInfoOnLCD("Queue Primed");
          else
            ShowInfoOnLCD("Queue Canceled");
          SetLCDinfoTimeout();
        } else {
          ShowInfoOnLCD("Retrig 3");
          SetLCDinfoTimeout();
          RetrigButtonPressed(3);
        }
        break;
      case NORMALSPEEDBUTTON:
        if(!g_queueing)
        {
            ShowInfoOnLCD("Normal Speed");
            SetLCDinfoTimeout();
        }
        updateSpeedMultiplierCb(NORMAL);
        break;
      case DOUBLESPEEDBUTTON:
        if(!g_queueing)
        {
            ShowInfoOnLCD("Double Speed");
            SetLCDinfoTimeout();
        }
        updateSpeedMultiplierCb(DOUBLE);
        break;
      case TRIPLESPEEDBUTTON:
        if(!g_queueing)
        {
            ShowInfoOnLCD("Triple Speed");
            SetLCDinfoTimeout();
        }
        updateSpeedMultiplierCb(TRIPLE);
        break;
      case QUADSPEEDBUTTON:
        if(!g_queueing)
        {
            ShowInfoOnLCD("Quad Speed");
            SetLCDinfoTimeout();
        }
        updateSpeedMultiplierCb(QUAD);
        break;

      case LOWPROBBUTTON:
        ShowInfoOnLCD("Low Prob *");
        SetLCDinfoTimeout();
        ProbabilityButtonPressed(LOWPROB);
        break;
      case MEDPROBBUTTON:
        ShowInfoOnLCD("Med Prob *");
        SetLCDinfoTimeout();
        ProbabilityButtonPressed(MEDPROB);
        break;
      case HIGHPROPBBUTTON:
        ShowInfoOnLCD("High Prob *");
        SetLCDinfoTimeout();
        ProbabilityButtonPressed(HIGHPROB);
        break;
      case FULLPROBBUTTON:        
        ShowInfoOnLCD("Full Prob *");
        SetLCDinfoTimeout();
        ProbabilityButtonPressed(FULLPROB);
        break;
      
      case ONETICKBUTTON:
        ShowInfoOnLCD("Step *");
        SetLCDinfoTimeout();
        RepeatButtonPressed(1);
        break;
      case TWOTICKBUTTON:
        ShowInfoOnLCD("Step * *");
        SetLCDinfoTimeout();
        RepeatButtonPressed(2);
        break;
      case THREETICKBUTTON: 
        ShowInfoOnLCD("Step * * *");
        SetLCDinfoTimeout();
        RepeatButtonPressed(3);
        break;
      case FOURTICKBUTTON:
        ShowInfoOnLCD("Step * * * *");
        SetLCDinfoTimeout();
        RepeatButtonPressed(4);
        break;
      default:  // latch the steps
    
        if (i >= STEPSOFFSET) {
          
          ShowStepStateOnLCD(i % STEPSOFFSET, pressProcessed[i]);

          if(i < heldTrellisStep)
            heldTrellisStep = i;

          if (StepButtonCb)
              StepButtonCb(i-STEPSOFFSET);

          // switch cases by mode: do length callback in length_edit, momentary light up
          // add in other momentary actions plus callback
          // otherwise latch & update selection - do the latching as a callback too, but locally defined ??
    
          switch (currentMode)
          {
            case step_mute:
              MuteModeTrellisButtonPressed(i);
              break;
            case step_hold:
              HoldModeTrellisButtonPressed(i);
              break;
            case accent_edit:
              AccentModeTrellisButtonPressed(i);
              break;
            case track_select:
              TrackSelectTrellisButtonPressed(i);
              break;
            case track_mute:        // TODO: ADD QUEUEING
              trackMuteTrellisButtonPressed(i);
              break;
            case length_edit:       // TODO: ADD QUEUEING
            case pattern_select:
              save_pattern_destination = -1;
              QueueableSimpleIndicatorModeTrellisButtonPressed(i);
              break;
            case pattern_save:
              simpleIndicatorModeTrellisButtonPressed(i);
              break;
            case path_select:      // TODO: ADD QUEUEING
              pathModeTrellisButtonPressed(i);
              break;
            case synth_edit:
              SynthEditModeTrellisButtonPressed(i);
              break;
            case save_to_sd:
              break;
            default:
              SelectEditTrellisButtonPressed(i);
              showStepInfoOnLCD(i % STEPSOFFSET);
              SetLCDinfoLabelTimeout();
              SetLCDinfoTimeout();
          }
        }
    }
}


void InOutHelper::UpdateTrellisStepIndicator(unsigned long time_now)
{
    step_indicator_led_active = false; // flag off, assuming all active step led's will be turned off

    for (uint8_t i = 0; i < 16; i++) {
      unsigned long stepLedOffTime = stepLedOffTimes[i];
      
      if (stepLedOffTime > 0) {
        if (stepLedOffTime > time_now) {    // this one can wait, so
          step_indicator_led_active = true; // leave the led on, keep flag going
        } else {
          stepLedOffTimes[i] = 0;           // deal with this one - it's time has passed

#ifdef DEBUG
          Serial.print("UpdateTrellisStepIndicator offing ");
          Serial.println(i);
          Serial.print("  Late by ");
          Serial.print((stepLedOffTime-time_now)/1000);
          Serial.print(" millis or ");
          Serial.print(stepLedOffTime-time_now);
          Serial.println(" micros");
#endif

          // manually latched to on ?
          if (stepsToCheck[i] == false)       // should be off after step indication
          {                                   // so turn it off
            trellis.clrLED(i + STEPSOFFSET);
            trellis_led_dirty = true;
          } else {                                       // should be on after step indication
            if (trellis.isLED(i + STEPSOFFSET) == false) // so, if its off,
            {                                            // turn it on
              trellis.setLED(i + STEPSOFFSET);
              trellis_led_dirty = true;        
            }
          }
        }  
      }
    }  
}


void InOutHelper::MuteModeTrellisButtonPressed(int i) 
{
    if (helperSteps[i % STEPSOFFSET]) {
      helperSteps[i % STEPSOFFSET] = false;
      trellis.clrLED(i);
      sequencer.setMute(i % STEPSOFFSET, true);
    } else {
      helperSteps[i % STEPSOFFSET] = true;
      trellis.setLED(i);                  
      sequencer.setMute(i % STEPSOFFSET, false);
    }
}


void InOutHelper::HoldModeTrellisButtonPressed(int i) 
{
    if (helperSteps[i % STEPSOFFSET]) {
      helperSteps[i % STEPSOFFSET] = false;
      trellis.clrLED(i);
      sequencer.setHold(i % STEPSOFFSET, false);
    } else {
      helperSteps[i % STEPSOFFSET] = true;
      trellis.setLED(i);                  
      sequencer.setHold(i % STEPSOFFSET, true);
    }
}


void InOutHelper::AccentModeTrellisButtonPressed(int i) 
{
    if (helperSteps[i % STEPSOFFSET]) {
      helperSteps[i % STEPSOFFSET] = false;
      trellis.clrLED(i);
      sequencer.setAccent(i % STEPSOFFSET, false);
    } else {
      helperSteps[i % STEPSOFFSET] = true;
      trellis.setLED(i);                  
      sequencer.setAccent(i % STEPSOFFSET, true);
    }
}


void InOutHelper::SelectEditTrellisButtonPressed(int i)
{
    if (selectedSteps[i % STEPSOFFSET]) {
      selectedSteps[i % STEPSOFFSET] = false;
      trellis.clrLED(i);
    } else {
      selectedSteps[i % STEPSOFFSET] = true;
      trellis.setLED(i);                  
    }
    stepSelectionMode = SOMESTEPS;
    selectionChanged = true;
}


void InOutHelper::pathModeTrellisButtonPressed(int i) 
{
    if(!g_queueing && currentMode == path_select)
    {
        simpleIndicatorModeTrellisButtonPressed(i);
        
        sequencer.setPath(i % STEPSOFFSET);
        ShowInfoOnLCD(sequencer.getPathName());
        SetLCDinfoTimeout();
        ShowPathNumberOnLCD(i % STEPSOFFSET);
    } else {
        recordActionCb(PATHCHANGE, i % STEPSOFFSET, sequencer.getCurrentTrack());
    }
}


void InOutHelper::TrackSelectTrellisButtonPressed(int i) 
{
      byte curTrack = sequencer.getCurrentTrack();
      if(curTrack == 1 + ((byte)i % STEPSOFFSET))
      {
          simpleIndicatorModeTrellisButtonPressed(i);
          
          ShowTrackNumberOnLCD(curTrack);
          ShowPathNumberOnLCD(sequencer.getPath());

          // back to step edit mode
          currentMode = step_edit;
          setupNewMode();
          ShowModeOnLCD();        
          selectionChanged = true;

      } else {
          LiteUpTrellisSteps(helperSteps);
      }
}


void InOutHelper::trackMuteTrellisButtonPressed(int i) 
{
    byte track = 1 + ((byte)i % STEPSOFFSET);

    if(currentMode == track_mute) // actual button press, or act like it
    {
        if (helperSteps[i % STEPSOFFSET]) {
            // successful if tracknumber valid, fewer tracks than helperSteps
            if(!g_queueing)
            {
                bool success = sequencer.setTrackMute(track, false);
                if(success)
                {
                    helperSteps[i % STEPSOFFSET] = false;
                    trellis.clrLED(i);
                    ShowValueInfoOnLCD("Unmuted track", track);
                    SetLCDinfoTimeout();
                }
            } else
                recordActionCb(TRACKMUTECHANGE, (byte)false, track);

        } else {
            if(!g_queueing)
            {
                bool success = sequencer.setTrackMute(track, true);
                if(success)
                {
                    helperSteps[i % STEPSOFFSET] = true;
                    trellis.setLED(i);                  
                    ShowValueInfoOnLCD("Muted track", track);
                    SetLCDinfoTimeout();
                } else
                    trellis.clrLED(i);
            } else
                recordActionCb(TRACKMUTECHANGE, (byte)true, track);
    } 
}








}


void InOutHelper::SynthEditModeTrellisButtonPressed(int i)
{
    simpleIndicatorModeTrellisButtonPressed(i);
    handleButtonHoldTiming(SYNTHPATCHBUTTON, true);
}


void InOutHelper::simpleIndicatorModeTrellisButtonPressed(int i) 
{   
    for (uint8_t foo = 0; foo < 16; foo++)
      if (helperSteps[foo% STEPSOFFSET]) {
        trellis.clrLED(foo + STEPSOFFSET);
        helperSteps[foo] = false;
      }
    helperSteps[i % STEPSOFFSET] = true;
    trellis.setLED(i);
}


void InOutHelper::QueueableSimpleIndicatorModeTrellisButtonPressed(int i) 
{   
    if(!g_queueing)
        simpleIndicatorModeTrellisButtonPressed(i);
}


void InOutHelper::RepeatButtonPressed(byte repetitions)
{
    sequencer.setSelectedRepetitions(selectedSteps, repetitions);
}


void InOutHelper::RetrigButtonPressed(byte retrigs)
{
    sequencer.setSelectedRetrigs(selectedSteps, retrigs);
}


void InOutHelper::ProbabilityButtonPressed(stepProbability prob)
{
    sequencer.setSelectedProbabilities(selectedSteps, (byte)prob);
}

bool InOutHelper::QueueButtonPressed()
{
    g_queueing = !g_queueing;
    g_queuePrimed = false;

    return g_queueing;
}

bool InOutHelper::PrimeQueueButtonPressed()
{
    if(g_queuePrimed)
    {
        g_queuePrimed = false;
        g_queueing = false;
        // TODO: reset the queue here

    } else {
        g_queuePrimed = true;
        g_queueing = false;
    }
    return g_queuePrimed;
}

void InOutHelper::LiteUpTrellisSteps(bool stepsArray[])
{
    for (uint8_t i = 0; i < 16; i++)
      if (stepsArray[i]) trellis.setLED(i + STEPSOFFSET);
      else trellis.clrLED(i + STEPSOFFSET);
      
    trellis_led_dirty = true;
}

void InOutHelper::handleEncoderButtons()
{
    switch (currentMode) {
      case synth_edit:
        if (EncButA.update() && EncButA.fell()) {
          EncA.write(0);
          if (StepButtonCb) StepButtonCb(EncoderA);          
        }
        if (EncButB.update() && EncButB.fell()) { //nothing on B -it's used for accell - so we keep the update call
//        EncB.write(0);
//        if (StepButtonCb) StepButtonCb(EncoderB);          
        }

        if (EncButC.update() && EncButC.fell()) EncC.write(0);  // not using these
        if (EncButD.update() && EncButD.fell()) EncD.write(0);  // two for synth edit
        break;

      case pattern_select:
      case pattern_save:
      case track_select:
      case length_edit:
      case path_select:
      case accent_edit:
      case step_mute:
      case step_hold:

        if (EncButA.update() && EncButA.fell()) 
        {
            byte mutedTrack = sequencer.toggleCurrentTrackMute();
        }
        if (EncButB.update() && EncButB.fell()) EncB.write(0);
        if (EncButC.update() && EncButC.fell()) EncC.write(0);
        if (EncButD.update() && EncButD.fell()) EncD.write(0);
        break;
    
      case track_mute:
        if (EncButA.update() && EncButA.fell()) 
        {
            byte stepRef = sequencer.toggleCurrentTrackMute() - 1;
            helperSteps[stepRef] = !helperSteps[stepRef];
            if(helperSteps[stepRef])
              trellis.setLED(stepRef + STEPSOFFSET);
            else
              trellis.clrLED(stepRef + STEPSOFFSET);

            trellis_led_dirty = true;
        }
        if (EncButB.update() && EncButB.fell()) EncB.write(0);
        if (EncButC.update() && EncButC.fell()) EncC.write(0);
        if (EncButD.update() && EncButD.fell()) EncD.write(0);
        break;
      
      default:
        if (EncButA.update() && EncButA.fell()) EncA.write(0);
        if (EncButB.update() && EncButB.fell()) EncB.write(0);
        if (EncButC.update() && EncButC.fell()) EncC.write(0);
        if (EncButD.update() && EncButD.fell()) EncD.write(0);
    }
}


void InOutHelper::resetEncoder(encoders enc)
{
  switch (enc) {
    case EncoderA:
      EncA.write(0);
      break;
    case EncoderB:
      EncB.write(0);
      break;
    case EncoderC:
      EncC.write(0);
      break;
    case EncoderD:
      EncD.write(0);
      break;
  }
}

void InOutHelper::handleStartStopButton()
{
    if (PlayButton.update() && PlayButton.fell()) {
      StartStopButtonCb();
    }
}

void InOutHelper::handleSelectButton()
{
    if (SelectButton.update() && SelectButton.fell()) {
      switch (currentMode) {
        case pattern_select:
          {
            bool recall = sequencer.toggle_pattern_recall();
            if (recall) {
              ShowValueInfoOnLCD("Restoring root", sequencer.getCurrentPattern());
              SetLCDinfoTimeout();

            } else {
              ShowValueInfoOnLCD("Recalling edit", sequencer.getCurrentPattern());
              SetLCDinfoTimeout();
            }
//          playpath.setPath(sequencer.getPath());
          }
          break;
        case pattern_save:
          {
            if (save_pattern_destination != -1) {
              sequencer.save_pattern(save_pattern_destination);
              ShowValueInfoOnLCD("Saved to seq", save_pattern_destination);
              SetLCDinfoTimeout();
              save_pattern_destination = -1;

              currentMode = pattern_select;
              setupNewMode();
              ShowModeOnLCD();        
              selectionChanged = true;

            } else {
              currentMode = pattern_select;
              setupNewMode();
              ShowModeOnLCD();        
              selectionChanged = true;
            }
          }
          break;
  
        case save_to_sd:
          {
            if (!save_to_SD_done) {
              SaveToSdCb();
            } else {
              currentMode = pattern_select;
              setupNewMode();
              ShowModeOnLCD();        
              selectionChanged = true;
              save_to_SD_done = false;
            }
          }
          break;
        case path_select:
        case length_edit:
          {
            shiftActive = !shiftActive;
            ShowShiftOnLCD();
          }
          break;
        default:
          {
            switch (stepSelectionMode) {
              case NOSTEPS: 
              
                for (uint8_t i = 0; i < 16; i++) {
                  selectedSteps[i] = !selectionBuffer[i];
                  ShowStepStateOnLCD(i, NOBUTTONPRESS);
                }
        
                stepSelectionMode = SOMESTEPS;
                selectionChanged = true;
                break;
              
              case SOMESTEPS:
                
                for (uint8_t i = 0; i < 16; i++) {
                  selectionBuffer[i] = selectedSteps[i];
                  selectedSteps[i] = !selectedSteps[i];
                  ShowStepStateOnLCD(i, NOBUTTONPRESS);
                }
        
                stepSelectionMode = INVERTEDSTEPS;
                selectionChanged = true;
                break;
             
              case INVERTEDSTEPS:
                
                for (uint8_t i = 0; i < 16; i++) {
                  selectionBuffer[i] = selectedSteps[i];
                  selectedSteps[i] = true;
                  ShowStepStateOnLCD(i, NOBUTTONPRESS);
                }
        
                stepSelectionMode = ALLSTEPS;
                selectionChanged = true;
                break;
        
              case ALLSTEPS:
        
                for (uint8_t i = 0; i < 16; i++) {
                  selectedSteps[i] = false;
                  ShowStepStateOnLCD(i, NOBUTTONPRESS);
                }
        
                stepSelectionMode = NOSTEPS;
                selectionChanged = true;
                break;
        
              default:
        
                Serial.print("Invalid stepSelectionMode in handleSelectButton: ");
                Serial.println(stepSelectionMode);
        
            }
            if (currentMode == step_edit) LiteUpTrellisSteps(selectedSteps); 
          }     
      }
    } 
}

void InOutHelper::handleModeButtons()
{
    static int selectOrSave = pattern_select;
    static int muteOrHold = step_mute;
    static int stepOrTrack = step_edit;
    
    if (PatternModeButton.update() && PatternModeButton.fell()) {
      if (currentMode == pattern_select) {
        currentMode = pattern_save;
        selectOrSave = pattern_select;
      } else
        if (currentMode == pattern_save) {
          currentMode = chain_edit;
          selectOrSave = pattern_select;
          save_pattern_destination = -1;
          ClearInfoOnLCD();
        } else
          if (currentMode == chain_edit) {
            currentMode = pattern_select;
            selectOrSave = pattern_select;
            save_pattern_destination = -1;
            ClearInfoOnLCD();
          } else
            currentMode = selectOrSave;
          
      setupNewMode();
      ShowModeOnLCD();        
      selectionChanged = true;
      handleButtonHoldTiming(PATTERNMODEBUTTON, true);
    } else
      if (PatternModeButton.rose()) {
        handleButtonHoldTiming(PATTERNMODEBUTTON, false);    
      }


    if (StepEditModeButton.update() && StepEditModeButton.fell()) {
      if (currentMode == step_edit) {
        currentMode = track_select;
//      stepOrTrack = track_select;
        stepOrTrack = step_edit;
      } else
        if (currentMode == track_select) {
          currentMode = track_mute;
          stepOrTrack = step_edit;
        } else
          if (currentMode == track_mute) {
            currentMode = step_edit;
            stepOrTrack = step_edit;
          } else
            currentMode = stepOrTrack;

      setupNewMode();
      ShowModeOnLCD();        
      selectionChanged = true;
      handleButtonHoldTiming(STEPEDITMODEBUTTON, true);
    } else
      if (StepEditModeButton.rose()) {
        handleButtonHoldTiming(STEPEDITMODEBUTTON, false);    
      }

/*    
    if (StepEditModeButton.update() && StepEditModeButton.fell()) {
      currentMode = step_edit;
      setupNewMode();
      ShowModeOnLCD();        
      selectionChanged = true;
      handleButtonHoldTiming(STEPEDITMODEBUTTON, true);
    } else
      if (StepEditModeButton.rose()) {
        handleButtonHoldTiming(STEPEDITMODEBUTTON, false);    
      }
*/

    if (MuteModeButton.update() && MuteModeButton.fell()) {
      if (currentMode == step_hold) {
        currentMode = step_mute;
        muteOrHold = step_mute;
      } else
        if (currentMode == step_mute) {
          currentMode = step_hold;
          muteOrHold = step_hold;
        } else
          currentMode = muteOrHold;
      setupNewMode();
      ShowModeOnLCD();        
      selectionChanged = true;
      handleButtonHoldTiming(MUTEMODEBUTTON, true);
    } else
      if (MuteModeButton.rose()) {
        handleButtonHoldTiming(MUTEMODEBUTTON, false);    
      }

    if (AccentEditModeButton.update() && AccentEditModeButton.fell()) {
      currentMode = accent_edit;
      setupNewMode();
      ShowModeOnLCD();        
      selectionChanged = true;
      handleButtonHoldTiming(ACCENTEDITMODEBUTTON, true);
    } else
      if (AccentEditModeButton.rose()) {
        handleButtonHoldTiming(ACCENTEDITMODEBUTTON, false);    
      }
  
    if (LengthEditModeButton.update() && LengthEditModeButton.fell()) {
      currentMode = length_edit;
      setupNewMode();
      ShowModeOnLCD();        
      selectionChanged = true;
      handleButtonHoldTiming(LENGTHEDITMODEBUTTON, true);
    } else
      if (LengthEditModeButton.rose()) {
        handleButtonHoldTiming(LENGTHEDITMODEBUTTON, false);    
      }
  
    if (PathEditModeButton.update() && PathEditModeButton.fell()) {
      currentMode = path_select;
      setupNewMode();
      ShowModeOnLCD();        
      selectionChanged = true;
      handleButtonHoldTiming(PATHEDITMODEBUTTON, true);
    } else
      if (PathEditModeButton.rose()) {
        handleButtonHoldTiming(PATHEDITMODEBUTTON, false);    
      }
}


// mode buttons for now,
// could also absorb the patch button hold-to-save
void InOutHelper::handleButtonHolds()
{
  unsigned long held = GetHoldableButtonPressed(PATTERNMODEBUTTON);

  // holding pattern mode button saves current pattern edit buffer to current pattern
  // TODO: invoke save mode with destination choices
  
  if (held > 0) {
    currentHoldAction = SAVESEQ;
    if (save_pattern_destination == -1) {
      save_pattern_destination = sequencer.getCurrentPattern();
    }      
    if (trackActionHoldTime(held, SAVESEQ)) {
        sequencer.save_pattern(save_pattern_destination);
        ShowPatternNumberOnLCD(save_pattern_destination);
        save_pattern_destination = -1;
    }
  } else {
      
    held = GetHoldableButtonPressed(SYNTHPATCHBUTTON);
    if (held > 0) {
      
      currentHoldAction = SAVEPATCH;
      if (trackActionHoldTime(held, SAVEPATCH))
          synth.saveToPatch(synth.getButtonPressed());
    } else {

      // track step edit button to clean up trellis tracking fail
      held = GetHoldableButtonPressed(STEPEDITMODEBUTTON);
      if (held > 0) {
          currentHoldAction = TRESET;
          if (trackActionHoldTime(held, TRESET))
              ResetTrellis();
      } else {

        held = GetHoldableButtonPressed(REWINDBUTTON);
        if (held > 0) {
            currentHoldAction = SAVETOSD;
            if (trackActionHoldTime(held, SAVETOSD))
            {
                sequencer.save_all_patterns();
                SaveToSdCb();
            }
        }
      }
    }
  }
}


// move to the bottom
bool InOutHelper::trackActionHoldTime(unsigned long pressDuration, holdActionMode mode)
{      
    bool doAction = false;
    if (pressDuration <= SHOWNOTHING)
    {        
        if (holdActionState == INACTIVE) {
          holdActionState = SHOWNOTHING; 
          Serial.println("SHOWNOTHING");
        }   
    }      
    else 
    {
      if (pressDuration <= ANNOUNCE) 
      {      
        if (holdActionState == SHOWNOTHING) {
          holdActionState = ANNOUNCE;      
          ShowHoldActionMessage(ANNOUNCE, mode);
          Serial.println("ANNOUNCE");
        }    
      }
      else 
      {
          if (pressDuration <= ACTION) 
          {            
              if (holdActionState == ANNOUNCE) {
                holdActionState = ACTION;          
                ShowHoldActionMessage(ACTION, mode);
                Serial.println("ACTION");
                doAction = true;
                }
          }
          else 
          {
              if (pressDuration <= DONE)
              {
                  if (holdActionState == ACTION) {
                    holdActionState = DONE;   
                    ShowHoldActionMessage(DONE, mode);
                    Serial.println("DONE");
                  }       
              } else {
                  if (holdActionState == DONE) {
                      holdActionState = INACTIVE;
                      ShowHoldActionMessage(INACTIVE, mode);
                      Serial.println("INACTIVE");
                  }
              }
          }   
        }
    }
    return doAction;
}

int InOutHelper::checkJoystickX()
{
    static int recentX = 0;
    int retval = -1;

//  int readingX = 1024-analogRead(JoystickX_PIN);
    int readingX = FOURTEENBIT-analogRead(JoystickX_PIN);

    if (readingX < recentX - 100 || readingX > recentX + 100 ) {
        recentX = readingX;
        retval = readingX;
    }
    return retval;
}

int InOutHelper::checkJoystickY()
{
    static int recentY = 0;
    int retval = -1;

    int readingY = analogRead(JoystickY_PIN);

    if (readingY < recentY - 100 || readingY > recentY + 100 ) {
        recentY = readingY;
        retval = readingY;
    }
    return retval;
}

// Checkers
bool InOutHelper::checkRewindButton()
{
    bool retVal = false;
    if (RewindButton.update() && RewindButton.fell()) 
    {
        retVal = true;
        handleButtonHoldTiming(REWINDBUTTON, true);
    }
    else 
        if (RewindButton.rose())
            handleButtonHoldTiming(REWINDBUTTON, false);

    return retVal;
}


// Setters
void InOutHelper::setRunningStepIndicators(int step, unsigned long led_off_time)
{
    latestPlaybackStep = step;
    
    if (stepsToCheck[latestPlaybackStep] == false) {//it's not on manually, so turn it on to show the step.        
        trellis.setLED(latestPlaybackStep + STEPSOFFSET);
        ShowStepOnLCD(latestPlaybackStep, false);
    }
    else 
    {    
        trellis.clrLED(latestPlaybackStep + STEPSOFFSET); //it's on by manual press, so turn it off
        ShowStepOnLCD(latestPlaybackStep, true); // LCD with modifier
    }
    
#ifdef DEBUG
    Serial.print("step ");
    Serial.print(step);
    Serial.print("  offtime ");
    Serial.println(led_off_time);
#endif
    
    stepLedOffTimes[latestPlaybackStep] = led_off_time;
    step_indicator_led_active = true;
    trellis_led_dirty = true;
}


void InOutHelper::RemoveStepIndicatorOnLCD()
{
    lcd.setCursor(latestPlaybackStep, 3);
    if(stepsToCheck[latestPlaybackStep]) lcd.print("*");
    else lcd.print(" ");
}


// Sequencer Helpers
void InOutHelper::RetrievePatternStates()
{
    byte seqMaxLength = sequencer.getMaxLength(); //hoping this matches array size
//  byte seqLength = sequencer.getLength();
    int i;

    switch (currentMode) {
      case step_mute:
        for (i = 0; i < seqMaxLength; i++) helperSteps[i] = !sequencer.getMute(i);
        Serial.println("step mute");
       break;
      case step_hold:
        for (i = 0; i < seqMaxLength; i++) helperSteps[i] = sequencer.getHold(i);
        Serial.println("step hold");
       break;
      case accent_edit:
        for (i = 0; i < seqMaxLength; i++) helperSteps[i] = sequencer.getAccent(i);
        Serial.println("accent edit");
      break;
      default:
        ShowErrorOnLCD("YOWZA uncaught mode in RetrievePatternStates");
        Serial.print("mode: ");
        Serial.println(currentMode);
    }
}


void InOutHelper::ClearBoolSteps(bool arrayPointer[], int arrayLength)
{
    
    for (int i = 0; i < arrayLength; i++) arrayPointer[i] = false;
}


// LCD Helpers
void InOutHelper::ShowErrorOnLCD(const char error[])
{
    char buffer [21];

    int foo = snprintf(buffer, 21, "%s                ", error);

    lcd.setCursor(0, 0);
    lcd.print(buffer);

/*
    lcd.setCursor(0, 0);
    lcd.print("                 ");
    lcd.setCursor(0, 0);
    lcd.print(error);
    Serial.println(error);
*/
}


void InOutHelper::ShowErrorOnLCD(const char error[], byte value)
{
    char buffer [21];

    int foo = snprintf(buffer, 21, "%s %d              ", error, value);

    lcd.setCursor(0, 0);
    lcd.print(buffer);
}


void InOutHelper::ShowErrorOnLCD(const char error[], const char context[])
{
    lcd.setCursor(0, 0);
    lcd.print("                 ");
    lcd.setCursor(0, 0);
    lcd.print(error);
    lcd.setCursor(0, 1);
    lcd.print(context);
    Serial.println(error);
    Serial.println(context);
}


void InOutHelper::ShowInfoOnLCD(const char info[])
{
    ClearInfoOnLCD();
    lcd.setCursor(0, 2);
    lcd.print(info);
    ValueOrButtonOnLCDLength = strlen(info);
    Serial.println(info);
}


void InOutHelper::ShowValueInfoOnLCD(const char label[], int value)
{
    char buffer [21];

    int foo = snprintf(buffer, 21, "%s %d", label, value);
    int curLen = strlen(buffer);
    int showLen = curLen > ValueOrButtonOnLCDLength ? curLen : ValueOrButtonOnLCDLength;
    ValueOrButtonOnLCDLength = curLen;

    char buffer2 [showLen + 1];
    foo = snprintf(buffer2, showLen + 1, "%s                   ", buffer);

    lcd.setCursor(0, 2);
    lcd.print(buffer2);

/*
    String sShow  = String(label) + String(value);
    int len = sShow.length();
    int remainderLen = ValueOrButtonOnLCDLength - len;

    lcd.setCursor(0, 2);
    lcd.print(sShow);
    if (remainderLen > 0)
      for(int f = 0; f < remainderLen; f++)
        lcd.print(" ");
    ValueOrButtonOnLCDLength = len;
*/
}


void InOutHelper::ShowValueInfoOnLCD(const char label[], int value1, const char value2[])
{
    char buffer [21];

    int foo = snprintf(buffer, 21, "%s %d %s", label, value1, value2);
    int curLen = strlen(buffer);
    int showLen = curLen > ValueOrButtonOnLCDLength ? curLen : ValueOrButtonOnLCDLength;
    ValueOrButtonOnLCDLength = curLen;

    char buffer2 [showLen + 1];
    foo = snprintf(buffer2, showLen + 1, "%s                   ", buffer);

    lcd.setCursor(0, 2);
    lcd.print(buffer2);

/*
    String sShow  = String(label) + String(value1) + " " + String(value2);
    int len = sShow.length();
    int remainderLen = ValueOrButtonOnLCDLength - len;

    lcd.setCursor(0, 2);
    lcd.print(sShow);
    if (remainderLen > 0)
      for(int f = 0; f < remainderLen; f++)
        lcd.print(" ");
    ValueOrButtonOnLCDLength = len;
*/
}


void InOutHelper::ShowValueInfoOnLCD(const char label[], float value)
{  
    char buffer [21];

    int foo = snprintf(buffer, 21, "%s %f", label, value);
    int curLen = strlen(buffer);
    int showLen = curLen > ValueOrButtonOnLCDLength ? curLen : ValueOrButtonOnLCDLength;
    ValueOrButtonOnLCDLength = curLen;

    char buffer2 [showLen + 1];
    foo = snprintf(buffer2, showLen + 1, "%s                   ", buffer);

    lcd.setCursor(0, 2);
    lcd.print(buffer2);

/*
    String sShow  = String(label) + String(value);
    int len = sShow.length();
    int remainderLen = ValueOrButtonOnLCDLength - len;

    lcd.setCursor(0, 2);
    lcd.print(sShow);
    if (remainderLen > 0)
      for(int f = 0; f < remainderLen; f++)
        lcd.print(" ");
    ValueOrButtonOnLCDLength = len;
*/
}

void InOutHelper::ShowSynParOnLCD(const char label[], int value)
{
    ClearInfoOnLCD();
    lcd.setCursor(0, 2);
    lcd.print(label);
    lcd.setCursor(strlen(label), 2);
    lcd.print(value);
    ValueOrButtonOnLCDLength = constrain(strlen(label) + 6, 0, 20);
}

void InOutHelper::ShowSynParOnLCD(const char label[], float value)
{
    ClearInfoOnLCD();
    lcd.setCursor(0, 2);
    lcd.print(label);
    lcd.setCursor(strlen(label), 1);
    lcd.print(value);
    ValueOrButtonOnLCDLength = constrain(strlen(label) + 6, 0, 20);
}

void InOutHelper::ClearInfoOnLCD()
{
    char buffer [21];
    int foo = snprintf(buffer, ValueOrButtonOnLCDLength + 1, "                    ");

    lcdTimeoutLowerRow = 0;
    lcd.setCursor(0, 2);
    lcd.print(buffer);

//  for (int i=0; i < ValueOrButtonOnLCDLength; i++) lcd.print(" ");    
}

void InOutHelper::ClearLabelRowInfoOnLCD()
{
    char buffer [21];
    int foo = snprintf(buffer, LabelOnLCDLength + 1, "                    ");

    lcdTimeoutUpperRow = 0;
    lcd.setCursor(0, 1);
    lcd.print(buffer);

//  for (int i=0; i < LabelOnLCDLength; i++) lcd.print(" ");    
}

void InOutHelper::ShowStepStateOnLCD(int i, boolean stepButtonPressed)
{
    lcd.setCursor(i, 3);
    if (stepButtonPressed) lcd.print("+");  // + while pressed
    else                                    //   ..and if not:
      if (selectedSteps[i])                 //         if selected:
        if(i == latestPlaybackStep) lcd.print("_"); //   _ if current step
        else lcd.print("*");                        //   * if not
      else                                          // if not selected:
        if(i == latestPlaybackStep) lcd.print("."); //   . if current step
        else {                                      //   and if not:
          byte ticks = sequencer.getTicks(i);
                    
          if(ticks > 1) lcd.print(ticks);           //     show step repetition ticks count
          else lcd.print(" ");                      //     or space if not repeated
        }
}


void InOutHelper::ShowModeOnLCD()
{
    char buffer [21];
    int curNameLen = strlen(modeNames[currentMode]);
    int diplayLen = ModeNameOnLCDLength > curNameLen ? ModeNameOnLCDLength : curNameLen;
    ModeNameOnLCDLength = curNameLen;

    int foo = snprintf(buffer, diplayLen + 1, "%s                   ", modeNames[currentMode]);

    lcd.setCursor(0, 1);
    lcd.print(buffer);
/*    
    lcd.setCursor(0, 1);
    lcd.print("                 ");
    lcd.setCursor(0, 1);
    lcd.print(modeNames[currentMode]);
*/
}

void InOutHelper::ShowShiftOnLCD()
{
    lcd.setCursor(17, 3);

    if(shiftActive)
        lcd.print("^");
    else    
        lcd.print(" ");
}


void InOutHelper::ShowBPMOnLCD(int bpm)
{
    lcd.setCursor(17, 0);
    lcd.print("   ");

    if(bpm >= 100) lcd.setCursor(17, 0);
    else if(bpm >= 10) lcd.setCursor(18, 0);
    else lcd.setCursor(19, 0);

    lcd.print(bpm);
}


void InOutHelper::ShowSwingOnLCD(int swing)
{
    lcd.setCursor(17, 1);
    lcd.print("   ");

    if(swing < 10) lcd.setCursor(19, 1);
    else if(swing < 100) lcd.setCursor(18, 1);
    else lcd.setCursor(17, 1);
    
    lcd.print(swing);
}

// could be rationalized better with ShowStepStateOnLCD
void InOutHelper::ShowStepOnLCD(int step, bool isActive)
{

    ShowStepStateOnLCD(prevPlaybackStep, NOBUTTONPRESS);
    lcd.setCursor(step, 3);
    if (isActive) lcd.print("_");        
    else lcd.print(".");    

    prevPlaybackStep = step;
}


void InOutHelper::ShowPlaybackStepOnLCD(int step)
{
    char buffer [21];

    int foo = snprintf(buffer, 21, "%d", step);
    int len = strlen(buffer);

    lcd.setCursor(16 - len, 0);
    lcd.print(buffer);
/*    
    if (step<100)
    {
        lcd.setCursor(14, 0);
        lcd.print("  ");
        lcd.setCursor(14, 0);
    } else
    if (step<1000)
    {
        lcd.setCursor(13, 0);
        lcd.print("   ");
        lcd.setCursor(13, 0);
    } else
    if (step<10000)
    {
        lcd.setCursor(12, 0);
        lcd.print("    ");
        lcd.setCursor(12, 0);
    } else
    if (step<100000)
    {
        lcd.setCursor(11, 0);
        lcd.print("     ");
        lcd.setCursor(11, 0);
    } else
    {
        lcd.setCursor(10, 0);
        lcd.print("      ");
        lcd.setCursor(10, 0);
    }

    lcd.print(step);
*/
}

void InOutHelper::ShowMemoryOnLCD(int mem)
{
    lcd.setCursor(0, 0);
    lcd.print("               ");  
    lcd.setCursor(0, 0);
    lcd.print("m:");  
    lcd.print(mem);  
}


void InOutHelper::ShowPatternNumberOnLCD(int seqNum)
{
    lcd.setCursor(18, 2);
    lcd.print("  ");  

    if(seqNum < 9) lcd.setCursor(19, 2);
    else lcd.setCursor(18, 2);

    lcd.print(seqNum);  
}


void InOutHelper::ShowTrackNumberOnLCD(byte trackNum)
{
    lcd.setCursor(15, 2);
    lcd.print("  ");  

    if(trackNum < 10) lcd.setCursor(16, 2);
    else lcd.setCursor(15, 2);

    lcd.print(trackNum);  
}

void InOutHelper::ShowPathNumberOnLCD(byte pathNum)
{
    lcd.setCursor(18, 3);
    lcd.print("  ");  

    if(pathNum < 9) lcd.setCursor(19, 3);
    else lcd.setCursor(18, 3);

    lcd.print(pathNum + 1);  
}


void InOutHelper::ShowChainLinkOnLCD(const byte chain, const byte curChainPlay, const byte maxChainPlays, 
                                     const byte link, const byte curLinkPlay, const byte maxLinkPlays)
{
  char buffer [20];
  int foo = snprintf(buffer, 20, "C:%d,%d/%d L:%d,%d/%d ", 
                                 chain+1, curChainPlay, maxChainPlays,
                                 link+1, curLinkPlay, maxLinkPlays);

  lcd.setCursor(0, 2);
  lcd.print(buffer);

  ValueOrButtonOnLCDLength = strlen(buffer);

  Serial.println();
  Serial.print("ShowChainLinkOnLCD: ");
  Serial.println(buffer);
}


void InOutHelper::ShowHoldActionMessage(holdActionProcess state, holdActionMode mode)
{
    int destination;
    
    switch (mode)
    {
      case SAVEPATCH:
        destination = synth.getButtonPressed();
        switch (state)
        {
          case SHOWNOTHING:
            break;
          case ANNOUNCE:
            ShowValueInfoOnLCD("Hold to save to", destination);
            break;
          case ACTION:
            ShowValueInfoOnLCD("Saving to patch", destination);
            break;
          case DONE:
            ShowValueInfoOnLCD("Saved to patch", destination);
            break;
          case INACTIVE:
          default:
            ClearInfoOnLCD();
            break;
        }        
        break;
        
      case SAVESEQ:
        destination = save_pattern_destination;
        switch (state)
        {
          case SHOWNOTHING:
            break;
          case ANNOUNCE:
            ShowValueInfoOnLCD("Hold to save to", destination);
            SetLCDinfoTimeout();
            break;
          case ACTION:
            ShowValueInfoOnLCD("Saving pattern", destination);
            SetLCDinfoTimeout();
            break;
          case DONE:
            ShowInfoOnLCD("Saved pattern.");
            SetLCDinfoTimeout();
            break;
          case INACTIVE:
          default:
            ClearInfoOnLCD();
            break;
        }        
        break;
        
      case TRESET:
        switch (state)
        {
          case SHOWNOTHING:
            break;
          case ANNOUNCE:
            ShowInfoOnLCD("Hold: reset keys");
            SetLCDinfoTimeout();
            break;
          case ACTION:
            ShowInfoOnLCD("Resetting keys ");
            SetLCDinfoTimeout();
            break;
          case DONE:
            ShowInfoOnLCD("Keys reset.");
            SetLCDinfoTimeout();
            break;
          case INACTIVE:
          default:
            ClearInfoOnLCD();
            break;
        }        
        break;
                
      case SAVETOSD:
        switch (state)
        {
          case SHOWNOTHING:
            break;
          case ANNOUNCE:
            ShowInfoOnLCD("Hold: Save all to SD");
            SetLCDinfoTimeout();
            break;
          case ACTION:
            ShowInfoOnLCD("Saving to SD ");
            SetLCDinfoTimeout();
            break;
          case DONE:
            ShowInfoOnLCD("Saved.");
            SetLCDinfoTimeout();
            break;
          case INACTIVE:
          default:
            ClearInfoOnLCD();
            break;
        }        
        break;
        
      default:
        ShowErrorOnLCD("OOPSIE SHA MSG");
        Serial.print("OOPSIE: invalid mode in InOutHelper::ShowHoldActionMessage");
    }

}

/*
void InOutHelper::ShowStoreMessage(int state)
{
    static int roundRobin = 0;
    
    switch (state)
    {
      case START:

//      lcd.setCursor(0, 0);
//      lcd.print("                   ");
//      lcd.setCursor(0, 0);
//      lcd.print("Saving to SD Card");
//      roundRobin = 0;

        ShowInfoOnLCD("Saving to SD Card");
        SetLCDinfoTimeout();
        break;

      case BUSY:
        // show "..."
        // also blink mode button or something
        break;

      case STOP:

//      lcd.setCursor(0, 0);
//      lcd.print("                   ");
//      lcd.setCursor(0, 0);
//      lcd.print("Saving done.");

        ShowInfoOnLCD("Saving done.");
        SetLCDinfoTimeout();

        break;
    }
}
*/

void InOutHelper::SetLCDinfoTimeout()
{
    lcdTimeoutLowerRow = millis() + lcdTimeoutDuration;
}

void InOutHelper::SetLCDinfoLabelTimeout()
{
    lcdTimeoutUpperRow = millis() + lcdTimeoutDuration;
}

void InOutHelper::handleLCDtimeouts()
{
    if (lcdTimeoutLowerRow > 0 && millis() > lcdTimeoutLowerRow) {
      ClearInfoOnLCD();
      lcdTimeoutLowerRow = 0;
    }

    if (lcdTimeoutUpperRow > 0 && millis() > lcdTimeoutUpperRow) {
      ClearLabelRowInfoOnLCD();
      lcdTimeoutUpperRow = 0;
    }
      
}

void InOutHelper::showStepInfoOnLCD(int step) 
{
    byte prob = 255;
    byte ticks = 255;
    byte retrig = 255;
    byte pitch = 255;
    float dur = 1.0;

    byte prob_count = 0;
    byte ticks_count = 0;
    byte retrig_count = 0;
    byte pitch_count = 0;
    byte dur_count = 0;
     
    if (!sequencer.notesArrayEmpty(selectedSteps)) {
      for (int i=0; i < 16; i++) {
        if (selectedSteps[i]) {
            note curNote = sequencer.getNoteParams(i);
            
            byte foo = curNote.pitchVal;
            if (foo != pitch) pitch_count++;
            pitch = (foo > pitch) ? pitch : foo;
            
            float foof = curNote.duration;
            if (foof != dur) dur_count++;
            dur = (foof > dur) ? dur : foof;
            
            foo = sequencer.getProbability(i);
            if (foo != prob) prob_count++;
            prob = (foo > prob) ? prob : foo;

            foo = curNote.ticks;
            if (foo != ticks) ticks_count++;
            ticks = (foo > ticks) ? ticks : foo;

            foo = curNote.retrigs;
            if (foo != retrig) retrig_count++;
            retrig = (foo > retrig) ? retrig : foo;
        }
      }

      lcd.setCursor(0, 1);
      lcd.print("Pr Rp Rt Not Dur  ");

      String out = String(prob);
      if (prob_count >1) out.append("+");
      while(out.length() < 3) out.append(" ");

      out.append(String(ticks));
      if (ticks_count >1) out.append("+");
      while(out.length() < 6) out.append(" ");
  
      out.append(String(retrig));
      if (retrig_count >1) out.append("+");
      while(out.length() < 9) out.append(" ");
  
      out.append(midiToNoteName(pitch));
      if (pitch_count >1) out.append("+");
      while(out.length() < 13) out.append(" ");
  
      out.append(String(dur));
      if (dur_count >1) out.append("+");
      while(out.length() < 18) out.append(" ");
  
      lcd.setCursor(0, 2);
      lcd.print(out);

      ValueOrButtonOnLCDLength = 18;
      LabelOnLCDLength = 16;
    }
}

void InOutHelper::showQueuedActions(String out)
{
    lcd.setCursor(0, 1);
    lcd.print("Queued Actions Run:");
    lcd.setCursor(0, 2);
    lcd.print(out);

//  ValueOrButtonOnLCDLength = out.length();
    ValueOrButtonOnLCDLength = 19;
    LabelOnLCDLength = 19;
}

String InOutHelper::midiToNoteName(int note)
{
  String notes[12] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};
  byte noteIndex = note % 12;
  byte octave = note / 12;
  String noteName = notes[noteIndex];
  noteName.append(octave);

  return noteName;
}


char* InOutHelper::midiToNoteChar(int note, char* noteName)
{
    const char *notes[12] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};
    byte noteIndex = note % 12;
    byte octave = note / 12;

    int foo = snprintf(noteName, 4, "%s%d", notes[noteIndex], octave);

    return noteName;
}

void InOutHelper::showLoopTimer()
{
    static long loopTimer;
    static long prevTimer;

    loopTimer = micros();
    lcd.setCursor(0, 1);
    lcd.print("       ");
    lcd.setCursor(0, 1);
    lcd.print(loopTimer-prevTimer);
    prevTimer = loopTimer;
}
  

