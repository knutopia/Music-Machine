# Music-Machine

Step sequencer / synthesizer with real time access for playability

<img src="images/hardware_prototype_small.jpg" width="300" alt="Hardware Prototype">

Hardware:
* Keypad for pattern editing
* 2nd keypad for quick access to utilities
* Six mode buttons for quick editing
* Dedicated play control buttons
* LCD display with 4 encoders
* Teensy 3.6

Features:
* 16 patterns, up to 16 steps each
* 2 sets of buffers
* Single / multi step edit
* Queue up realtime edits during playback
* Step selection shortcuts
* Joystick for realtime 2d control
* Swing
* Synth engine with parameter editing
* MIDI out
* Song edit (in progress)
* SD storage
               
### To Address
* Chain edit
* Midi
* Joystick

## Buffer Mechanics
Each pattern has a "root" and an "edit" buffer. 
Changing something on the root copies the changed version into the edit buffer.
Changes to the edit buffer accumulate in the edit buffer.
Using the Pattern Mode Button, root and edit buffer can be swapped, edits can be committed to the root, and edits can be saved to the root of another pattern.
Saving to SD card saves all the edit buffers to become roots.


               
## Encoder Functions
4 encoders under the LCD display:

Common functions: Pitch for selected steps, Duration for selected steps, Swing (global), Tempo (global)

Other functions for synth edit

## Play Control Buttons

PLAYBUTTON  Start / stop play
  
REWINDBUTTON  Rewind, (hold:) Save to SD, reset playback counter

SELECTBUTTON  Cycle step selection: select no steps - select all steps - select current selection  |  Also: confirm actions
               

## Main Buttons and Their Modes

PATTERNMODEBUTTON

    pattern_select 
        Pick a pattern using right keypad
        Press the button again to toggle between pattern root and edit buffer.
        (Hold:) commit current pattern edit buffer to current pattern
    
    pattern_save
        Pick a pattern to capture edits from current buffer to
    
    chain_edit
        Chain edit / song mode
        
    synth_edit
        Synth patch choice & editing
        
    
MUTEMODEBUTTON

    step_mute
        Select steps to mute
        
    step_hold
        Select steps to mark as hold

STEPEDITMODEBUTTON

    step_edit
        Select steps to edit pitch, duration
        Selection is sticky
        
    track_select
        Select which track to direct current realtime edits to
    
    track_mute
        Select tracks to mute

    (Hold:) reset Trellis (keypad) display
               
ACCENTEDITMODEBUTTON

    accent_edit
        Select steps to accent
    
LENGTHEDITMODEBUTTON

    length_edit
        Select a step to mark the pattern length

PATHEDITMODEBUTTON

    path_select
        Select a playback path for the pattern

## Left Keypad Keys

### Top Row
SAVETOSDBUTTON Save mem to SD

SYNTHEDITBUTTON Enter synth editing | (hold:) commit current synth edits to current synth patch

QUEUEBUTTON Start to queue realtime edits

QUEUEPRIMEBUTTON Arm queued edits

### 2nd Row
NORMALSPEEDBUTTON, DOUBLESPEEDBUTTON, TRIPLESPEEDBUTTON, QUADSPEEDBUTTON

Change playback speed multiplier

### 3rd Row
LOWPROBBUTTON, MEDPROBBUTTON, HIGHPROPBBUTTON, FULLPROBBUTTON

Change probability of selected steps

### 4th Row
ONETICKBUTTON, TWOTICKBUTTON, THREETICKBUTTON, FOURTICKBUTTON

Change racheting of selected steps


