#include "SynthEngine.h"
#include "InOutHelper.h"
#include "NoteOffList.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
#include <SD_t3.h>
#include <SerialFlash.h>

//#define MIDION true

// GUItool: begin automatically generated code
AudioEffectWaveshaper    waveshape1;     //xy=412.22220611572266,861.0000276565552
AudioSynthWaveform       OSC1;           //xy=682.6666259765625,662.9999797344208
AudioSynthWaveform       OSC2;           //xy=683.6666259765625,723.9999797344208
AudioSynthKarplusStrong  string2;        //xy=684.6666259765625,788.9999797344208
AudioSynthWaveform       SubOSC;      //xy=685.3333320617676,855.555585861206
AudioSynthSimpleDrum     Track2drum;          //xy=689.1428031921387,606.4284830093384
AudioSynthWaveformSine   lfo1;           //xy=848.6666259765625,682.9999797344208
AudioMixer4              inputMixer;     //xy=850.6666259765625,742.9999797344208
AudioEffectEnvelope      VCAenvelope;    //xy=853.6666259765625,879
AudioEffectEnvelope      VCAenvelope2;      //xy=855,824
AudioFilterStateVariable LFOedFilter;    //xy=1063.6666259765625,755.9999797344208
AudioFilterStateVariable EnvelopedFilter; //xy=1065.6666259765625,822.9999797344208
AudioSynthWaveformSine   lfo2;           //xy=1151.6666259765625,882.9999797344208
AudioMixer4              FilterMixer;    //xy=1237.6666259765625,752.9999797344208
AudioFilterStateVariable turboFilter;    //xy=1304.6666259765625,825.9999797344208
AudioEffectDelay         delay1;         //xy=1348.6666259765625,1028.9999797344208
AudioMixer4              mixer1;         //xy=1489.6666870117188,753.2222518920898
AudioEffectReverb        reverb1;        //xy=1519.2221031188965,1045.3333003520966
AudioFilterStateVariable delayFilter;    //xy=1522.2221031188965,990.3333003520966
AudioMixer4              mixer2;         //xy=1523.2221488952637,834.3333320617676
AudioOutputI2S           i2s2;           //xy=1628.4444580078125,751.3333425521851
//AudioOutputUSB           usb1;           //xy=1641,691
AudioConnection          patchCord1(OSC1, 0, inputMixer, 0);
AudioConnection          patchCord2(OSC2, 0, inputMixer, 1);
AudioConnection          patchCord3(string2, 0, inputMixer, 2);
AudioConnection          patchCord4(SubOSC, 0, inputMixer, 3);
AudioConnection          patchCord5(Track2drum, 0, mixer1, 2);
AudioConnection          patchCord6(lfo1, 0, LFOedFilter, 1);
AudioConnection          patchCord7(inputMixer, VCAenvelope2);
AudioConnection          patchCord8(VCAenvelope, 0, EnvelopedFilter, 0);
AudioConnection          patchCord9(VCAenvelope, 0, EnvelopedFilter, 1);
AudioConnection          patchCord10(VCAenvelope, 0, LFOedFilter, 0);
AudioConnection          patchCord11(VCAenvelope2, VCAenvelope);
AudioConnection          patchCord12(LFOedFilter, 0, FilterMixer, 0);
AudioConnection          patchCord13(EnvelopedFilter, 0, FilterMixer, 1);
AudioConnection          patchCord14(lfo2, 0, delayFilter, 1);
AudioConnection          patchCord15(lfo2, 0, turboFilter, 1);
AudioConnection          patchCord16(FilterMixer, 0, turboFilter, 0);
AudioConnection          patchCord17(turboFilter, 0, mixer1, 0);
AudioConnection          patchCord18(turboFilter, 0, mixer2, 0);
AudioConnection          patchCord19(delay1, 0, mixer1, 1);
AudioConnection          patchCord20(delay1, 0, delayFilter, 0);
AudioConnection          patchCord21(delay1, 1, reverb1, 0);
AudioConnection          patchCord22(mixer1, 0, i2s2, 0);
AudioConnection          patchCord23(mixer1, 0, i2s2, 1);
//AudioConnection          patchCord24(mixer1, 0, usb1, 0);
//AudioConnection          patchCord25(mixer1, 0, usb1, 1);
AudioConnection          patchCord26(reverb1, 0, mixer2, 2);
AudioConnection          patchCord27(delayFilter, 0, mixer2, 1);
AudioConnection          patchCord28(mixer2, delay1);
AudioControlSGTL5000     sgtl5000_1;     //xy=689.2380256652832,551.4285535812378
// GUItool: end automatically generated code


extern long g_step_duration;
extern bool playbackOn;
extern InOutHelper inout;
extern NoteOffList playingNotes;


SynthEngine::SynthEngine()
{
  m_b_playing_a_note = false;
  m_Midi_NoteforOff = 255;
  m_current_patch = 1;
  m_queue_new_patch = -1;
  m_b_reset_selector_reference = true;
  m_current_param = 0;
  m_previous_encB_value = 0;
  m_reference_Iparval = 0;
  m_reference_Fparval = 0;
  m_button_press_time = 0;
  m_button_pressed_num = 0;
  m_joy_reso = 0.7;
  m_joy_freq = 0;
  m_joy_subVCO = 0;
    
  filterQEmphasis = normalQ;
  Serial.println("SynthEngine constructor ");
}

void SynthEngine::begin()
{
  //required to setup audio. if it's too low it won't run.
  AudioMemory(255); //255 is max
  
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.autoVolumeControl(2,1,0,-18,0.5,2.0);
//  sgtl5000_1.autoVolumeEnable();

  Serial.println("SynthEngine begin ");
  retrievePatch(m_current_patch);
  activatePatch(m_current_patch);

  OSC1.amplitude(255); //TODO: REMOVE
  OSC2.amplitude(255);
  SubOSC.amplitude(255);
}

//"Getters" and "setters"

bool SynthEngine::playingAnote()
{
    return m_b_playing_a_note;
}

//Helper methods

void SynthEngine::playNote(byte aTrack, note aNote)
// will need to track notes for noteOff
{
    switch(aTrack)
    {
      case 1:
        playSynthNote(aNote);
        break;
      case 2:
        playPercNote(aNote);
        break;
     } 
}


void SynthEngine::playTestClick()
{
    Track2drum.frequency(550);
    Track2drum.length(50);
    Track2drum.pitchMod(.5);
    Track2drum.noteOn();
}


void SynthEngine::playPercNote(note aNote)
{
//  Track2drum.frequency(aNote.pitchFreq / 2);
    Track2drum.frequency(aNote.pitchFreq);
//  Track2drum.length(100);
    Track2drum.length(aNote.durationMS / 1000);
    Track2drum.pitchMod(.5);
    Track2drum.noteOn();

#ifdef MIDION
    if(!aNote.accent)
      usbMIDI.sendNoteOn(aNote.pitchVal, 99, 2);  // 60 = C4
    else
    {
      usbMIDI.sendNoteOn(aNote.pitchVal, 127, 2);
      Serial.println("Midi accent");
    }
#endif

}

void SynthEngine::playSynthNote(note aNote)
{
//    float osc2freq = freq + freq / 1000.0 * (float)random (-50, 50) / 100.0;

    float osc2freq = freqOffset(aNote.pitchVal, getEditPval(VCO2detune));
    float subOscFreq = aNote.pitchFreq / 2;
    float subOscAmp = constrain(m_joy_subVCO * .5 + getEditPval(VCO4mix), 0, 1);

#ifdef DEBUG
    Serial.println("playSynthNote");
    Serial.print("  note: ");
    Serial.print(aNote.pitchVal);
    Serial.print("  dur: ");
    Serial.print(aNote.duration);
    Serial.print("  ms: ");
    Serial.println(aNote.durationMS);
#endif

//  trackJoystick();
    prepSynthAccent(aNote.accent);
    AudioNoInterrupts();
    turboFilter.resonance(filterQEmphasis);
    delayFilter.resonance(delayFilterQEmphasis);
    OSC1.frequency(aNote.pitchFreq);
    OSC2.frequency(osc2freq); //modify freq by VCO2detune
    SubOSC.frequency(subOscFreq);
    inputMixer.gain(3, subOscAmp);  // VCO4mix

    string2.noteOn(aNote.pitchFreq, .7);
//  string2.noteOn(aNote.pitchFreq, aNote.velocity); // needs float, is byte
    VCAenvelope2.noteOn();
    VCAenvelope.noteOn();
    AudioInterrupts();

    m_b_playing_a_note = true;

    // dirty midi send
/*
    if (m_Midi_NoteforOff < 255)
    {
#ifdef MIDION
        usbMIDI.sendNoteOff(m_Midi_NoteforOff, 0, MIDISENDCHANNEL);
#endif
#ifdef DEBUG
      Serial.print(" offing: ");
      Serial.println(m_Midi_NoteforOff);
#endif
    }    
*/

#ifdef MIDION
    if(!aNote.accent)
      usbMIDI.sendNoteOn(aNote.pitchVal, 99, MIDISENDCHANNEL);  // 60 = C4
    else
    {
      usbMIDI.sendNoteOn(aNote.pitchVal, 127, MIDISENDCHANNEL);
      Serial.println("Midi accent");
    }
#endif
    m_Midi_NoteforOff = aNote.pitchVal;
}

void SynthEngine::endNote(byte aTrack, byte aMidiNote)
{
    switch (aTrack)
    {
      case 1:
        endSynthNote(1.0);
        break;

      case 2:

        break;

      default:
        Serial.print("endNote: track choice defaulted ");
        Serial.println(aTrack);
    }
    endMidiNote(aTrack, aMidiNote);
}

void SynthEngine::endSynthNote(float velocity)
{
    AudioNoInterrupts();
    string2.noteOff(velocity);
    //      OSC1.amplitude(0);
    //      OSC2.amplitude(0);
    VCAenvelope2.noteOff();
    VCAenvelope.noteOff();

    AudioInterrupts();

    m_b_playing_a_note = false;

/*
    // dirty midi send
    if (m_Midi_NoteforOff < 255) {
#ifdef MIDION
        usbMIDI.sendNoteOff(m_Midi_NoteforOff, 0, MIDISENDCHANNEL);
//      usbMIDI.send_now();
#endif
      m_Midi_NoteforOff = 255;
    }
*/
}

void SynthEngine::endMidiNote(byte aMidiChannel, byte aMidiNote)
{
#ifdef MIDION
        usbMIDI.sendNoteOff(aMidiNote, 0, aMidiChannel);
//      usbMIDI.send_now();
#endif
}

void SynthEngine::allNotesOff()
{
    int foo = 0;
    playingNotes.readRewind();
    while(playingNotes.hasReadValue())
    {
        endNote(playingNotes.readTrack(), 
                playingNotes.readMidiNote());
        playingNotes.dropNode();
        playingNotes.readNext();
        foo++;
    }

/*
    endSynthNote(1);
    m_Midi_NoteforOff = 255;
    m_b_playing_a_note = false;
*/
}

void SynthEngine::prepSynthAccent(byte empFlag)
{
  if (empFlag == 0) {
//      filterQEmphasis = normalQ;
    filterQEmphasis = m_joy_reso;
    delayFilterQEmphasis = 3.0;
  }
  else {
//      filterQEmphasis = emphasizedQ;
    filterQEmphasis = constrain(m_joy_reso + 4, 0.7, 5.0);
    delayFilterQEmphasis = 7.0;
  }
}    

void SynthEngine::prepPatchIfNeeded()
{
    if (m_queue_new_patch >= 0 ) {
      m_current_patch = m_queue_new_patch;
      m_queue_new_patch  = -1;
      m_b_reset_selector_reference = true;
      prepSynPatchForEdit();
    } 
}


void SynthEngine::handleButtonRelease(int butNum)
{
  if (m_button_press_time > 0) 
  {
      if (millis() > m_button_press_time + BUTTONHOLDTIME)    // long press
      {
        inout.ShowValueInfoOnLCD("Patch: ", m_current_patch);
        
      } else {                                 // short press
       
        // call up matching patch
         m_queue_new_patch = putInRange(butNum, max_patches, 0);
        if (!playbackOn) prepPatchIfNeeded();
      }
      m_button_press_time = 0;
  }
}

void SynthEngine::handleButton(int butNum)
{
    switch (butNum) {
      case EncoderA:
        if (m_edit_state == PatchChoice) 
          m_edit_state = ParamEdit;
        else {
          m_edit_state = PatchChoice;
          prepSynPatchForEdit();
          m_b_reset_selector_reference = true;
        }
        break;
      case EncoderB:
        break;
      default:  
        m_button_pressed_num = butNum;
        m_button_press_time = millis();
    }
}

int SynthEngine::getButtonPressed()
{
    return m_button_pressed_num;
}

void SynthEngine::handleEncoder(int encoder, int value)
{
    switch (encoder) {
      case EncoderA:
        handleEncoderA(value);
        break;
      case EncoderB:
        handleEncoderB(value);
        m_previous_encB_value = value;
    }
}

void SynthEngine::handleEncoderA(int value)
{
    
    static int reference_patch;
    static int reference_param;
    
    if (m_b_reset_selector_reference) {
      m_b_reset_selector_reference = false;
      reference_patch = m_current_patch;
      reference_param = m_current_param;
    }
    
    switch (m_edit_state) {
      case PatchChoice:
        value = putInRange(value, max_patches, 0);
        m_current_patch = putInRange(reference_patch + value, max_patches, 0);
        
        prepSynPatchForEdit();
        break;
      case ParamEdit:
        value = putInRange(value, SynthPatch::SynParameterCount, 0);
        m_current_param = putInRange(reference_param + value, SynthPatch::SynParameterCount, 0);

        prepSynParEdit(m_edit_patch, m_current_param);
   }
}

void SynthEngine::prepSynPatchForEdit()
{
        retrievePatch(m_current_patch);
        activatePatch(m_current_patch);                       // copy patch into edit patch
        inout.ShowValueInfoOnLCD("Patch: ", m_current_patch);
}

void SynthEngine::prepSynParEdit(SynthPatch patch, int param)
{        
    const char* param_name = patch.paramNames[param];

    if (patch.isInt(param)) 
    {
      int current_parval = patch.getI(param);
      int min_val = patch.getImin(param);
      int max_val = patch.getImax(param);
      int val_delta = max_val - min_val;

      m_reference_Iparval = current_parval - m_previous_encB_value;
      m_reference_Iparval = putInFullRangeI(m_reference_Iparval, val_delta, min_val, 1); //#####

      inout.ShowSynParOnLCD(param_name, (int)patch.get(param));
      
    } else {
      //float current_parval = patch.get(param);
      float min_val = patch.getMin(param);
      float max_val = patch.getMax(param);
      float val_delta = max_val - min_val;
      float fvalue = putInFullRangeF((float)m_previous_encB_value / 10.0, val_delta, min_val, .1); //#####
      inout.resetEncoder(EncoderB);
      m_reference_Fparval = patch.get(param);
//    m_reference_Fparval = putInFullRangeF(m_reference_Fparval, val_delta, min_val, .1); //#####

      inout.ShowSynParOnLCD(param_name, patch.get(param));
    }
}

/*
void SynthEngine::prepSynParEdit(SynthPatch patch, int param)
{        
    const char* param_name = patch.paramNames[param];

    if (patch.isInt(param)) 
    {
      int current_parval = patch.getI(param);
      int min_val = patch.getImin(param);
      int max_val = patch.getImax(param);
      int val_delta = max_val - min_val;

      m_reference_Iparval = current_parval - m_previous_encB_value;
      m_reference_Iparval = putInFullRangeI(m_reference_Iparval, val_delta, min_val, 1); //#####

      inout.ShowSynParOnLCD(param_name, (int)patch.get(param));
      
    } else {
      float current_parval = patch.get(param);
      float min_val = patch.getMin(param);
      float max_val = patch.getMax(param);
      float val_delta = max_val - min_val;
      float fvalue = putInFullRangeF((float)m_previous_encB_value / 10.0, val_delta, min_val, .1); //#####
      m_reference_Fparval = current_parval - fvalue;
      m_reference_Fparval = putInFullRangeF(m_reference_Fparval, val_delta, min_val, .1); //#####

      inout.ShowSynParOnLCD(param_name, patch.get(param));
    }
}
*/

void SynthEngine::handleEncoderB(int value)
{        
    switch (m_edit_state) {
      case PatchChoice:
        // nothing for encoder B
        break;
      case ParamEdit:

        if (m_edit_patch.isInt(m_current_param)) 
        {
          int current_parval = m_edit_patch.getI(m_current_param);
          int min_val = m_edit_patch.getImin(m_current_param);
          int max_val = m_edit_patch.getImax(m_current_param);
          int val_delta = max_val - min_val;
          int raw_enc_val = value;
          
          current_parval = putInFullRangeI(m_reference_Iparval + value, val_delta, min_val, 1); //#####
          m_edit_patch.setI(m_current_param, current_parval);
          const char* param_name = m_edit_patch.paramNames[m_current_param];
          inout.ShowSynParOnLCD(param_name, m_edit_patch.getI(m_current_param));

        } else {
          float current_parval = m_edit_patch.get(m_current_param);
          float min_val = m_edit_patch.getMin(m_current_param);
          float max_val = m_edit_patch.getMax(m_current_param);
          float val_delta = max_val - min_val;

          if (max_val - min_val > 1) 
          {
            float fvalue = (float)value / 10;
            current_parval = putInFullRangeF(m_reference_Fparval + fvalue, val_delta, min_val, .1); //#####
          } else {
            float fvalue = (float)value / 100;
            current_parval = putInFullRangeF(m_reference_Fparval + fvalue, val_delta, min_val, .01); //#####
          } 
          m_edit_patch.set(m_current_param, current_parval);
          const char* param_name = m_edit_patch.paramNames[m_current_param];
          inout.ShowSynParOnLCD(param_name, m_edit_patch.get(m_current_param));  
        }
        updateAudioEngine();
      }
}


int SynthEngine::putInRange(int iVar, int iRange, int iMin)
{
    while(iVar < 0) iVar += (iRange);
    int retval = iVar % iRange;
    return retval;
}

int SynthEngine::putInFullRangeI(int iVar, int iRange, int iMin, int stepSize)
{
    int retVal = iVar;
    int trueRange = iRange + stepSize;
    while(retVal < iMin) retVal += trueRange;
    retVal = iMin +((retVal - iMin) % trueRange);
  
    return retVal;
}

float SynthEngine::putInFullRangeF(float fVar, float fRange, float fMin, float stepSize)
{
    int iVar = (int) round((100.0 * fVar));
    int iRange = (int) round((100.0 * fRange));
    int iMin = (int) round((100.0 * fMin));
    int iStepSize = (int) round((100.0 * stepSize));
    int retVal = putInFullRangeI(iVar, iRange, iMin, iStepSize);
    return (float)retVal / 100.0;
}
/*
float SynthEngine::putInFullRangeF(float fVar, float fRange, float fMin, float stepSize)
{
    int iVar = (int) round((10.0 * fVar));
    int iRange = (int) round((10.0 * fRange));
    int iMin = (int) round((10.0 * fMin));
    int iStepSize = (int) round((10.0 * stepSize));
    int retVal = putInFullRangeI(iVar, iRange, iMin, iStepSize);
    return (float)retVal / 10.0;
}
*/
void SynthEngine::setCurrentPatch(int index)
{
    if(index >=0 && index < max_patches)
      m_current_patch = index;
}

//Helper method
bool playOrNot(int index)
{
  //evaluate probability...
  return false;
}


void SynthEngine::resetPatch(int index)
{
    if(index >=0 && index < max_patches)
      m_patches[index].reset();
}

void SynthEngine::selectPreviousPatch()
{
   if(m_current_patch > 0)
    m_current_patch--; 
}

void SynthEngine::selectNextPatch()
{
   if(m_current_patch < max_patches -1)
    m_current_patch++; 
}

int SynthEngine::getPvalI(int patchIndex, int paramName)
{
  return m_patches[patchIndex].getI(paramName);
}

float SynthEngine::getPval(int patchIndex, int paramName)
{
  return m_patches[patchIndex].get(paramName);
}

int SynthEngine::getEditPvalI(int paramName)
{
  return m_edit_patch.getI(paramName);
}

float SynthEngine::getEditPval(int paramName)
{
  return m_edit_patch.get(paramName);
}

void SynthEngine::saveToPatch(int p)
{
  Serial.print("Saving to patch ");
  Serial.print(p);
  
  m_edit_patch.copyPatchTo(m_patches[p]);
  setCurrentPatch(p);
}

void SynthEngine::activatePatch(int p)
{
  Serial.print("activatePatch ");
  Serial.println(p);
  
  m_patches[p].copyPatchTo(m_edit_patch);      
}


int SynthEngine::getCurrentPatchNumber()
{
  int retVal = m_current_patch;
  if(m_queue_new_patch >= 0) 
    retVal = m_queue_new_patch;

  return retVal;
}


void SynthEngine::retrievePatch(int p)
{      
    
    OSC1.begin(getPvalI(p, VCO1waveform));              // VCO1waveform
    OSC1.pulseWidth(getPval(p, VCO1pulsewidth));
    OSC2.begin(getPvalI(p, VCO2waveform));              // VCO2waveform
    OSC2.pulseWidth(getPval(p, VCO2pulsewidth));
  
    SubOSC.begin(WAVEFORM_SQUARE);
    
    inputMixer.gain(0, getPval(p, VCO1mix));            // VCO1mix
    inputMixer.gain(1, getPval(p, VCO2mix));            // VCO2mix
    inputMixer.gain(2, getPval(p, VCO3mix));            // VCO3mix
    inputMixer.gain(3, getPval(p, VCO4mix));            // VCO4mix
  
    FilterMixer.gain(0, getPval(p, LfoedFilterMix)); // LfoedFilterMix
    FilterMixer.gain(1, getPval(p, EnvelopedFilterMix)); // EnvelopedFilterMix
  
    EnvelopedFilter.frequency(getPvalI(p, EnvFilterFreq));   // EnvFilterFreq
    EnvelopedFilter.resonance(getPval(p, EnvFilterRes));     // EnvFilterRes
    EnvelopedFilter.octaveControl(getPval(p, EnvFilterOct)); // EnvFilterOct
    LFOedFilter.frequency(getPvalI(p, LfoedFilterFreq));   // LfoedFilterFreq
    LFOedFilter.resonance(getPval(p, LfoedFilterRes));     // LfoedFilterRes
    LFOedFilter.octaveControl(getPval(p, LfoedFilterOct)); // LfoedFilterOct
  
    turboFilter.frequency(getPvalI(p, TurboFilterFreq));       // TurboFilterFreq
    turboFilter.resonance(getPval(p, TurboFilterRes));         // TurboFilterRes
    turboFilter.octaveControl(getPval(p, TurboFilterOct));     // TurboFilterOct
    delayFilter.frequency(getPvalI(p, DelayFilterFreq));        // DelayFilterFreq
    delayFilter.resonance(getPval(p, DelayFilterRes));          // DelayFilterRes
    delayFilter.octaveControl(getPval(p, DelayFilterOct));     // DelayFilterOct
    lfo1.amplitude(getPval(p, FilterLFOAmp));   // FilterLFOAmp
    lfo1.frequency(getPval(p, FilterLFOFreq));  // FilterLFOFreq
    lfo2.amplitude(getPval(p, DelayLFOAmp));   // DelayLFOAmp
    lfo2.frequency(getPval(p, DelayLFOFreq));  // DelayLFOFreq
    mixer1.gain(0, getPval(p, FilterOutputMix));   // FilterOutputMix
    mixer1.gain(1, getPval(p, DelayOutputMix));   // DelayOutputMix
  
    mixer1.gain(2, 1);
    
    mixer2.gain(0, getPval(p, DryDelayInMix));                      // DryDelayInMix
    mixer2.gain(1, getPval(p, FilteredDelayFeedbackMix));           // FilteredDelayFeedbackMix
    mixer2.gain(2, getPval(p, RevDlyFeedbackMix));                  // RevDlyFeedbackMix
    delay1.delay(0, getPval(p, DelayTime) * g_step_duration / 1000);  // DelayTime
    reverb1.reverbTime(getPval(p, DelayedReverbTime));              // DelayedReverbTime      
    VCAenvelope2.attack(getPval(p, VCAEnvAttack));   // VCAEnvAttack
    VCAenvelope2.hold(getPval(p, VCAEnvHold));       // VCAEnvHold
    VCAenvelope2.decay(getPval(p, VCAEnvDecay));     // VCAEnvDecay
    VCAenvelope2.sustain(getPval(p, VCAEnvSustain)); // VCAEnvSustain
    VCAenvelope2.release(getPval(p, VCAEnvRelease)); // VCAEnvRelease
    VCAenvelope.attack(getPval(p, VCAEnvAttack));   // VCAEnvAttack
    VCAenvelope.hold(getPval(p, VCAEnvHold));       // VCAEnvHold
    VCAenvelope.decay(getPval(p, VCAEnvDecay));     // VCAEnvDecay
    VCAenvelope.sustain(getPval(p, VCAEnvSustain)); // VCAEnvSustain
    VCAenvelope.release(getPval(p, VCAEnvRelease)); // VCAEnvRelease
}

void SynthEngine::updateAudioEngine()
{      
  int p = m_current_param;
  switch (p) 
  {
    case VCO1waveform:
      OSC1.begin(getEditPvalI(VCO1waveform));              // VCO1waveform
      break;
    case VCO1pulsewidth:
      OSC1.pulseWidth(getEditPval(VCO1pulsewidth));        // VCO1pulsewidth
      break;
    case VCO2waveform:
      OSC2.begin(getEditPvalI(VCO2waveform));              // VCO2waveform
      break;
    case VCO2pulsewidth:
      OSC2.pulseWidth(getEditPval(VCO2pulsewidth));        // VCO2pulsewidth
      break;
    case VCO1mix:
      inputMixer.gain(0, getEditPval(VCO1mix));            // VCO1mix
      break;
    case VCO2mix:
      inputMixer.gain(1, getEditPval(VCO2mix));            // VCO2mix
      break;
    case VCO3mix:
      inputMixer.gain(2, getEditPval(VCO3mix));            // VCO3mix
      break;
    case VCO4mix:
      inputMixer.gain(3, getEditPval(VCO4mix));  // VCO4mix
      break;
    case LfoedFilterMix:
      FilterMixer.gain(0, getEditPval(LfoedFilterMix)); // LfoedFilterMix
      break;
    case EnvelopedFilterMix:
      FilterMixer.gain(1, getEditPval(EnvelopedFilterMix)); // EnvelopedFilterMix
      break;
    case EnvFilterFreq:
      EnvelopedFilter.frequency(getEditPvalI(EnvFilterFreq));   // EnvFilterFreq
      break;
    case EnvFilterRes:
      EnvelopedFilter.resonance(getEditPval(EnvFilterRes));     // EnvFilterRes
      break;
    case EnvFilterOct:
      EnvelopedFilter.octaveControl(getEditPval(EnvFilterOct)); // EnvFilterOct
      break;
    case LfoedFilterFreq:
      LFOedFilter.frequency(getEditPvalI(LfoedFilterFreq));   // LfoedFilterFreq
      break;
    case LfoedFilterRes:
      LFOedFilter.resonance(getEditPval(LfoedFilterRes));     // LfoedFilterRes
      break;
    case LfoedFilterOct:
      LFOedFilter.octaveControl(getEditPval(LfoedFilterOct)); // LfoedFilterOct
      break;
    case TurboFilterFreq:
      turboFilter.frequency(getEditPvalI(TurboFilterFreq));       // TurboFilterFreq
      break;
    case TurboFilterRes:
      turboFilter.resonance(getEditPval(TurboFilterRes));         // TurboFilterRes
      break;
    case TurboFilterOct:
      turboFilter.octaveControl(getEditPval(TurboFilterOct));     // TurboFilterOct
      break;
    case DelayFilterFreq:
      delayFilter.frequency(getEditPvalI(DelayFilterFreq));        // DelayFilterFreq
      break;
    case DelayFilterRes:
      delayFilter.resonance(getEditPval(DelayFilterRes));          // DelayFilterRes
      break;
    case DelayFilterOct:
      delayFilter.octaveControl(getEditPval(DelayFilterOct));     // DelayFilterOct
      break;
    case FilterLFOAmp:
      lfo1.amplitude(getEditPval(FilterLFOAmp));   // FilterLFOAmp
      break;
    case FilterLFOFreq:
      lfo1.frequency(getEditPval(FilterLFOFreq));  // FilterLFOFreq
      break;
    case DelayLFOAmp:
      lfo2.amplitude(getEditPval(DelayLFOAmp));   // DelayLFOAmp
      break;
    case DelayLFOFreq:
      lfo2.frequency(getEditPval(DelayLFOFreq));  // DelayLFOFreq
      break;
    case FilterOutputMix:
      mixer1.gain(0, getEditPval(FilterOutputMix));   // FilterOutputMix
      break;
    case DelayOutputMix:
      mixer1.gain(1, getEditPval(DelayOutputMix));   // DelayOutputMix
      break;
    case DryDelayInMix:
      mixer2.gain(0, getEditPval(DryDelayInMix));   // DryDelayInMix
      break;
    case FilteredDelayFeedbackMix:
      mixer2.gain(1, getEditPval(FilteredDelayFeedbackMix));  // FilteredDelayFeedbackMix
      break;
    case RevDlyFeedbackMix:
      mixer2.gain(2, getEditPval(RevDlyFeedbackMix));  // RevDlyFeedbackMix
      break;
    case DelayTime:
      delay1.delay(0, getEditPval(DelayTime)*(g_step_duration/1000.0)); // DelayTime ??????
      Serial.print("DelayTime ");
      Serial.print(getEditPval(DelayTime));
      Serial.print("  ms:");
      Serial.println(getEditPval(DelayTime)*(g_step_duration/1000.0));
      Serial.print("  g_step_duration:");
      Serial.println(g_step_duration);
      
      break;
    case DelayedReverbTime:
      reverb1.reverbTime(getEditPval(DelayedReverbTime)); //DelayedReverbTime      
      break;
    case VCAEnvAttack:
      VCAenvelope2.attack(getEditPval(VCAEnvAttack));   // VCAEnvAttack
      VCAenvelope.attack(getEditPval(VCAEnvAttack));   // VCAEnvAttack
      break;
    case VCAEnvHold:
      VCAenvelope2.hold(getEditPval(VCAEnvHold));       // VCAEnvHold
      VCAenvelope.hold(getEditPval(VCAEnvHold));       // VCAEnvHold
      break;
    case VCAEnvDecay:
      VCAenvelope2.decay(getEditPval(VCAEnvDecay));     // VCAEnvDecay
      VCAenvelope.decay(getEditPval(VCAEnvDecay));     // VCAEnvDecay
      break;
    case VCAEnvSustain:
      VCAenvelope2.sustain(getEditPval(VCAEnvSustain)); // VCAEnvSustain
      VCAenvelope.sustain(getEditPval(VCAEnvSustain)); // VCAEnvSustain
      break;
    case VCAEnvRelease:
      VCAenvelope2.release(getEditPval(VCAEnvRelease)); // VCAEnvRelease
      VCAenvelope.release(getEditPval(VCAEnvRelease)); // VCAEnvRelease
      break;

      
  }          
}    

void SynthEngine::reportPerformance()
{
    Serial.print("Audioprocessor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print(" Memory: ");
    Serial.println(AudioMemoryUsageMax());
    AudioProcessorUsageMaxReset();
    AudioMemoryUsageMaxReset();  
}

void SynthEngine::trackJoystick()
{
    static elapsedMillis timeTracker = 0;
    static int prevX = 0;
    static int prevY = 0;
    
    if(timeTracker > JOYSTICKINTERVAL)
    {
        timeTracker = timeTracker - JOYSTICKINTERVAL;

        int joyX = inout.checkJoystickX();    
        int joyY = inout.checkJoystickY();

        if (joyX > -1) {
            float freq = getEditPval(TurboFilterFreq) * (float)joyX / (float)FOURTEENBIT;
            turboFilter.frequency(freq);   

#ifdef MIDION
            if( joyX != prevX)
            {
                usbMIDI.sendControlChange(6, ((unsigned int)joyX >> 7) & 127, 1);
                usbMIDI.sendControlChange(38, (unsigned int)joyX & 127, 1);
                
//              inout.ShowValueInfoOnLCD("delta ", joyX-prevX);

                prevX = joyX;
//              inout.ShowValueInfoOnLCD("joyX ", joyX);
            }
#endif
        }
        if (joyY > -1) {
    //          turboFilter.resonance((float)joyY / 1024.0 * 4.3 + 0.7);
    //      m_joy_reso = constrain((float)joyY / 1024.0 * 4.3 + 0.7, .7, 5);
    //      m_joy_subVCO = constrain((float)joyY / 1024.0, 0, 1);
            m_joy_reso = constrain((float)joyY / (float)FOURTEENBIT * 4.3 + 0.7, .7, 5);
            m_joy_subVCO = constrain((float)joyY / (float)FOURTEENBIT, 0, 1);

#ifdef MIDION
            if( joyY != prevY)
            {
                usbMIDI.sendControlChange(7, ((unsigned int)joyY >> 7) & 127, 1);
                usbMIDI.sendControlChange(39, (unsigned int)joyY & 127, 1);
                prevY = joyY;
            }
#endif  
        }
    }
}    


float SynthEngine::freqOffset(int note, float offset)
{
  int tgtNote = note + (int)offset;
  return (float) 440.0 * (float)(pow(2, (tgtNote-57) / 12.0));      
}
