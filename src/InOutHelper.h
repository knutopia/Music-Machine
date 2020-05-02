/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef InOutHelper_h
#define InOutHelper_h

#include "Arduino.h"
#include <Bounce2.h>
#include "Adafruit_Trellis.h"
#include "pins_arduino.h"
#include "Enum.h"


// Input callbacks
typedef void (*ReactToInput) ();
typedef void (*ReactToInputBool) (bool passVal);
typedef void (*ReactToInputInt) (int passVal);
typedef void (*ReactToInputSpeedFactor) (speedFactor passVal);
typedef void (*ReactToInputIntArray) (int passVal, boolean passArray[]);
typedef void (*ReactToInputIntInt) (int passVal1, int passVal2);
typedef void (*ReactToInputAction) (actionID whatAction, byte param, byte track);

class InOutHelper
{

  public:
    InOutHelper();
    void begin( ReactToInput updateModeCbPointer, 
                ReactToInputIntArray updateRepetitionPointer,
                ReactToInputInt updatePatternLengthPointer,
                ReactToInputInt updatePatternNumberPointer,
                ReactToInputInt updateSavePatternDestPointer,
                ReactToInputInt updateTempoPointer,
                ReactToInputSpeedFactor updateSpeedMultiplierPointer,
                ReactToInput SaveToSdCbPointer,
                ReactToInput startStopCbPointer,
                ReactToInputInt updateSynthCbPointer,
                ReactToInputInt updateChainCbPointer,
                ReactToInputInt updateTrackCbPointer,
                ReactToInputAction recordActionCbPointer);

    void setupNewMode();
    void handleEncoders();
    void handleEncoderButtons();
    void resetEncoder(encoders enc);
    void handleTrellis();
    void handleStartStopButton();
    void handleSelectButton();
    void handleModeButtons();
    void handleButtonHolds();
    void handleButtonHoldTiming(holdableButton buttn, bool pressed);
    bool trackActionHoldTime(unsigned long pressDuration, holdActionMode mode);
    int checkJoystickX();
    int checkJoystickY();
    bool checkRewindButton();
    
    void setRunningStepIndicators(int step, unsigned long led_off_time);
    void RemoveStepIndicatorOnLCD();
    void ShowInfoOnLCD(const char info[]);
    void ShowValueInfoOnLCD(const char label[], int value);
    void ShowValueInfoOnLCD(const char label[], int value1, const char value2[]);
    void ShowValueInfoOnLCD(const char label[], float value);
    void ShowPlaybackStepOnLCD(int step);
    void ShowMemoryOnLCD(int mem);
    void SetLCDinfoTimeout();
    void SetLCDinfoLabelTimeout();
    void ShowParamOnLCD(const char label[], int value);
    void ShowParamOnLCD(const char label[], float value);
    void ShowParamOnLCD(const char label[], const char value[]);
    void ShowParamOnLCD(const char label[], const char value[], int blinkpos);
    void ShowActionOnLCD(const char label[]);
    void ShowErrorOnLCD(const char error[]);
    void ShowErrorOnLCD(const char error[], byte value);
    void ShowErrorOnLCD(const char error[], int value);
    void ShowErrorOnLCD(const char error[], const char context[]);
    void ShowPatternNumberOnLCD(int seqNum);
    void ShowTrackNumberOnLCD(byte trackNum);
    void ShowPathNumberOnLCD(byte pathNum);
    void ShowChainLinkPlayOnLCD(const byte chain, const byte curChainPlay, const byte maxChainPlays, 
                            const byte link, const byte curLinkPlay, const byte maxLinkPlays);
//  void ShowStoreMessage(int state);
    void ShowHoldActionMessage(holdActionProcess state, holdActionMode mode);
//  void ShowSaveSeqMessage(int state, int seq);
    void ShowModeOnLCD();
    void ShowShiftOnLCD();
    void ShowBPMOnLCD(int bpm);
    void handleLCDtimeouts();
    String midiToNoteName(int note);
    char* midiToNoteChar(int note, char* notename);
    void showLoopTimer();
    void blinketh();
    void enoughbl();

    // public because called by queued actions
    void pathModeTrellisButtonPressed(int index);
    void trackMuteTrellisButtonPressed(int index);
    void simpleIndicatorModeTrellisButtonPressed(int index);
    void showQueuedActions(String out);

  private:
    void ResetTrellis();            
    void SetupMuteOrHoldModeTrellis();
    void SetupSelectEditTrellis();
    void SetupAccentModeTrellis();
    void SetupLengthModeTrellis();
    void SetupPatternSelectModeTrellis();
    void SetupPatternSaveModeTrellis();
    void SetupChainEditModeTrellis();
    void SetupPathSelectModeTrellis();
    void SetupSynthEditTrellis();
    void SetupSaveModeTrellis();
    void SetupTrackSelectModeTrellis();
    void SetupTrackMuteModeTrellis();

    void ProcessTrellisButtonRelease(uint8_t index);
    void ProcessTrellisButtonPress(uint8_t index);
    void UpdateTrellisStepIndicator(unsigned long time_now);
    void HandleSynthEncoders();
    void HandleChainEncoders();
    void HandleTranspositionEncoderA();
    void HandleTrackEncoderA();
    void HandlePerformanceEncoders();
    unsigned long GetHoldableButtonPressed(holdableButton buttn);
    
    void SelectEditTrellisButtonPressed(int index);
    void TrackSelectTrellisButtonPressed(int index);
    void MuteModeTrellisButtonPressed(int index);
    void HoldModeTrellisButtonPressed(int index);
    void AccentModeTrellisButtonPressed(int index);
    void SynthEditModeTrellisButtonPressed(int index);
    void QueueableSimpleIndicatorModeTrellisButtonPressed(int index);
    void RepeatButtonPressed(byte repetitions);
    void RetrigButtonPressed(byte retrigs);
    void ProbabilityButtonPressed(stepProbability prob);
    bool QueueButtonPressed();
    bool PrimeQueueButtonPressed();
    
    void LiteUpTrellisSteps(bool helperSteps[]);
    void RetrievePatternStates();
    void ClearBoolSteps(bool arrayPointer[], int arrayLength);
    
    void ClearInfoOnLCD();
    void ClearLabelRowInfoOnLCD();
    void ShowStepStateOnLCD(int index, boolean stepButtonPressed);
    void ShowSwingOnLCD(int swing);
    void ShowStepOnLCD(int step, bool isActive);
    void showStepInfoOnLCD(int step);

    ReactToInput updateModeCb;
    ReactToInputIntArray updateRepetitionCb;
    ReactToInputInt updatePatternLengthCb;
    ReactToInputInt updatePatternNumberCb;
    ReactToInputInt updateSavePatternDestCb;
    ReactToInputInt updateTempoCb;
    ReactToInputSpeedFactor updateSpeedMultiplierCb;
    ReactToInput SaveToSdCb;
    ReactToInput startStopCb;
    ReactToInputInt StepButtonCb;
    ReactToInput StartStopButtonCb;
    ReactToInputInt updateSynthCb;
    ReactToInputInt updateChainCb;
    ReactToInputInt updateTrackCb;
    ReactToInputAction recordActionCb;
    
    const int numKeys = (NUMTRELLIS * 16);
    const int INTPIN = A2;   

    Adafruit_Trellis matrix0 = Adafruit_Trellis();
    Adafruit_Trellis matrix1 = Adafruit_Trellis();
    Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0, &matrix1);

    // Joystick
    const int JoystickX_PIN = A21;
    const int JoystickY_PIN = A22;

    // Arcade Buttons
    unsigned long HoldableButtonPressedTime[HOLDABLEBUTTONCOUNT];
    const int RewindButton_PIN = 39;
    const int PlayButton_PIN = 38;
    const int SelectButton_PIN = 37;
    const int PatternModeButton_PIN = 36;
    const int MuteModeButton_PIN = 24;
    const int StepEditModeButton_PIN = 26;
    const int AccentEditModeButton_PIN = 25;
    const int LengthEditModeButton_PIN = 27;
    const int PathEditModeButton_PIN = 28;

    // Buttons in Encoders
    const int EncButA_PIN = 29;
    const int EncButB_PIN = 30;
    const int EncButC_PIN = 31;
    const int EncButD_PIN = 32;
    
    const int BUTTON_DEBOUNCE_TIME = 5;

    Bounce PlayButton = Bounce();
    Bounce RewindButton = Bounce();
    Bounce SelectButton = Bounce();
    Bounce PatternModeButton = Bounce();
    Bounce MuteModeButton = Bounce();
    Bounce StepEditModeButton = Bounce();
    Bounce AccentEditModeButton = Bounce();
    Bounce LengthEditModeButton = Bounce();
    Bounce PathEditModeButton = Bounce();
    Bounce EncButA = Bounce();
    Bounce EncButB = Bounce();
    Bounce EncButC = Bounce();
    Bounce EncButD = Bounce();

    bool step_indicator_led_active = false;
    bool trellis_led_dirty = false;
    int latestPlaybackStep;
    int prevPlaybackStep;
    bool selectedSteps[16];
    bool selectionBuffer[16];
    bool helperSteps[16];
    unsigned long stepLedOffTimes[16];
    bool pressProcessed[32];
    bool *stepsToCheck;
    bool selectionChanged = false;
    selectionToggle stepSelectionMode = NOSTEPS;
    int ValueOrButtonOnLCDLength = 0;
    int LabelOnLCDLength = 0;
    int ModeNameOnLCDLength = 0;
    holdActionMode currentHoldAction = NONE;
    holdActionProcess holdActionState = INACTIVE;
    unsigned long lcdTimeoutLowerRow;
    unsigned long lcdTimeoutUpperRow;
    byte heldTrellisStep = 255;
    bool initTrackEncoder = true;
};

#endif
