# Music-Machine

Step sequencer / synthesizer with real time access for playability

* 16 patterns, up to 16 steps each
* 2 sets of buffers
* Single / multi step edit
* Step selection shortcuts
* Swing
* Synth engine with parameter editing
* Song edit (in progress)
* SD storage

### Modes TO ADDRESS
               save_to_sd,         
               track_select,
               track_mute
               
## Encoder Functions


## Play Control Buttons:

PLAYBUTTON  Start / stop play
  
REWINDBUTTON  Rewind, (hold:)Reset ADDRESS SAVE ALL

SELECTBUTTON  Cycle step selection: select no steps - select all steps - select current selection  |  Confirm actions
               

## Main Buttons and Their Modes:

PATTERNMODEBUTTON

    pattern_select 
        Pick a pattern using right keypad    
    
    pattern_save
        Capture edits from current buffer to current pattern 
    
    chain_edit
        Enter chain edit / song mode
    
MUTEMODEBUTTON

    step_mute
        Select steps to mute
        
    step_hold
        Select steps to mark as hold

STEPEDITMODEBUTTON

    step_edit
        Select steps to edit pitch, duration
        Selection is sticky
               
ACCENTEDITMODEBUTTON

    accent_edit
        Select steps to accent
    
LENGTHEDITMODEBUTTON

    length_edit
        Select a step to mark the pattern length

PATHEDITMODEBUTTON

    path_select
        Select a playback path for the pattern

## Left Keypad keys:

### Top row:
> SAVETOSDBUTTON,   SYNTHEDITBUTTON,  QUEUEBUTTON,          QUEUEPRIMEBUTTON

> Save mem to SD    Enter synth edit  Start to queue edits  Arm queued edits

### 2nd row:
NORMALSPEEDBUTTON, DOUBLESPEEDBUTTON, TRIPLESPEEDBUTTON, QUADSPEEDBUTTON

Change playback speed multiplier

### 3rd row:
LOWPROBBUTTON, MEDPROBBUTTON, HIGHPROPBBUTTON, FULLPROBBUTTON

Change probability of selected steps

### 4th row:
ONETICKBUTTON, TWOTICKBUTTON, THREETICKBUTTON, FOURTICKBUTTON

Change racheting of selected steps


