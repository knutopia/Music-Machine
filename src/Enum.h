#ifndef __ENUM
#define __ENUM

//Enumeration and a variable to track the current mode
//Mode enumeration
enum seqModes {first,
               pattern_select,
               pattern_save, 
               chain_edit,
               step_mute,
               step_hold,
               step_edit, 
               accent_edit,
               length_edit, 
               path_select,
               synth_edit,
               save_to_sd, 
               track_select,
               track_mute,
               last};

enum sequenceVariations {ROOT = 0, EDIT = 255};   // unused            

enum trellisButton {SAVETOSDBUTTON, SYNTHEDITBUTTON, QUEUEBUTTON, QUEUEPRIMEBUTTON, 
                     NORMALSPEEDBUTTON, DOUBLESPEEDBUTTON, TRIPLESPEEDBUTTON, QUADSPEEDBUTTON,
                     LOWPROBBUTTON, MEDPROBBUTTON, HIGHPROPBBUTTON, FULLPROBBUTTON,
                     ONETICKBUTTON, TWOTICKBUTTON, THREETICKBUTTON, FOURTICKBUTTON};

enum trellisConst {TRELLAY = 30000, STEPSOFFSET = 16, MOMENTARY = 0, LATCHING = 1, NUMTRELLIS = 2};


enum {HOLDABLEBUTTONCOUNT = 10};

enum holdableButton {PLAYBUTTON = 0, REWINDBUTTON = 1, SELECTBUTTON = 2, 
                    PATTERNMODEBUTTON = 3, MUTEMODEBUTTON = 4, STEPEDITMODEBUTTON = 5, 
                    ACCENTEDITMODEBUTTON = 6, LENGTHEDITMODEBUTTON = 7, PATHEDITMODEBUTTON = 8,
                    SYNTHPATCHBUTTON = 9};               

enum process {START, BUSY, STOP};

enum holdActionProcess {SHOWNOTHING = 1000, ANNOUNCE = 3000, ACTION = 5000, DONE = 7000, INACTIVE = 0};

enum {BUTTONHOLDTIME = 1000};

enum holdActionMode {NONE, SAVEPATCH, SAVESEQ, TRESET, SAVETOSD};

enum speedFactor {NORMAL = 1, DOUBLE = 2, TRIPLE = 3, QUAD = 4};

enum stepProbability {LOWPROB = 0, MEDPROB = 1, HIGHPROB = 2, FULLPROB = 3};

enum {BLIP = 10000};

enum encoders {EncoderA = 1000, EncoderB = 1001, EncoderC = 1002, EncoderD = 1003};

enum {BUTTONPRESSED = true, NOBUTTONPRESS = false};

enum selectionToggle {NOSTEPS, SOMESTEPS, ALLSTEPS, INVERTEDSTEPS};

enum synParNames {VCO1waveform, VCO1pulsewidth, 
                  VCO2waveform, VCO2pulsewidth, VCO2detune, 
                  VCO1mix, VCO2mix, VCO3mix, VCO4mix,
                  LfoedFilterMix, EnvelopedFilterMix, 
                  FilterOutputMix, DelayOutputMix,
                  DryDelayInMix, FilteredDelayFeedbackMix, RevDlyFeedbackMix,
                  EnvFilterFreq, EnvFilterRes, EnvFilterOct, 
                  LfoedFilterFreq, LfoedFilterRes, LfoedFilterOct,
                  TurboFilterFreq, TurboFilterRes, TurboFilterOct, 
                  DelayFilterFreq, DelayFilterRes, DelayFilterOct, 
                  FilterLFOAmp, FilterLFOFreq,
                  DelayLFOAmp, DelayLFOFreq,
                  DelayTime, DelayedReverbTime,
                  VCAEnvAttack, VCAEnvHold, VCAEnvDecay,
                  VCAEnvSustain, VCAEnvRelease}; //add some sort of filter output choice for delay filter
                  
enum synEditStates {PatchChoice, ParamEdit};

enum {lcdTimeoutDuration = 2000};

enum {MIDICLOCKDIVIDER = 24};

enum {BPMCONSTANT = 60000000};

// enum noteTimingType {Beat, Ticking, Retrig, TickingRetrig};

// enum trackTypes {Melody, Drum, Logic};
enum trackTypes {STEPSEQUENCE, SIMPLEBEAT};

enum instruments {Synth, Bassdrum, Snaredrum};

enum retrigDivisions {NORETRIGS = 0, ONERETRIG = 12,TWORETRIGS = 8,THREERETRIGS = 6};

// enum midiChannels {MIDISENDCHANNEL = 1};

enum {FOURTEENBIT = 16384};

enum {JOYSTICKINTERVAL = 50};

enum {TRACKCOUNT = 2};

enum bufferOrRestore {BUFFER = true, RESTORE = false};

enum midiVelocities {NORMALVEL = 1, ACCENTVEL = 127};

enum actionID {NOACTION, PATTERNCHANGE, PATHCHANGE, LENGTHCHANGE, TRACKMUTECHANGE, SPEEDMULTIPLIERCHANGE, SYNCTRACKS};

enum {QUEUEMAXLEN = 6 * TRACKCOUNT};

enum {MAXLINKSPERCHAIN = 4, MAXCHAINCOUNT = 4};

#endif
