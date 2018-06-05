
#ifndef __SYNTHENGINE
#define __SYNTHENGINE

#include "Arduino.h"
#include "Enum.h"
#include "SynthPatch.h"
#include "Note.h"


extern long g_step_duration;
extern bool playbackOn;

class SynthEngine
{
    public:
       //Use enumeration to define a class constant
      enum{max_patches = 32, edit_patch = 0};
      SynthPatch m_patches[max_patches];
    
      //Public constructor and methods
      SynthEngine();
      void begin();
      bool playingAnote();
      void playNote(note aNote);
      void endNote(float velocity);
      void allNotesOff();
      void prepAccent(byte empFlag);
      void prepPatchIfNeeded();
//    void trackSafeHoldTime();
      void handleButtonRelease(int butNum);
      void handleButton(int butNum);
      int getButtonPressed();
      void handleEncoder(int encoder, int value);
      int getCurrentPatchNumber();
      void prepSynPatchForEdit();
      void reportPerformance();
      void saveToPatch(int p);
      void trackJoystick();

    private:
      void handleEncoderA(int value);
      void handleEncoderB(int value);
      void prepSynParEdit(SynthPatch patch, int param);
      int putInRange(int iVar, int iRange, int iMin);
      int putInFullRangeI(int iVar, int iRange, int iMin, int stepSize);
      float putInFullRangeF(float fVar, float fRange, float fMin, float stepSize);
      void setCurrentPatch(int index);
      bool playOrNot(int index);
      void resetPatch(int index);
      void selectPreviousPatch();
      void selectNextPatch();
      int getPvalI(int patchIndex, int paramName);
      float getPval(int patchIndex, int paramName);
      int getEditPvalI(int paramName);
      float getEditPval(int paramName);
      void activatePatch(int p);
      void retrievePatch(int p);
      void updateAudioEngine();
      float freqOffset(int note, float offset);

      //Class data members:
      float filterQEmphasis;
      float delayFilterQEmphasis;
      const float normalQ = 0.6;
      const float emphasizedQ = 6.0;
      SynthPatch m_edit_patch;
      int m_edit_state = PatchChoice;
  
      int m_current_patch;
      bool m_b_reset_selector_reference;
      int m_current_param;
      int m_previous_encB_value;
      int m_reference_Iparval;
      float m_reference_Fparval;
  
      unsigned long m_button_press_time;
      int m_button_pressed_num;  
      float m_joy_reso;
      float m_joy_freq;
      float m_joy_subVCO;
      
      int m_queue_new_patch;    
      bool m_b_playing_a_note;

      int m_Midi_NoteforOff;
};

#endif
