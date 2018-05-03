#ifndef __SYNTHPATCH
#define __SYNTHPATCH

#include "Arduino.h"

struct synFparam {
  int id;
  float minVal;
  float maxVal;
  float defaultVal;
  float paramVal;
  bool isInt = false;
};


struct synIparam {
  int id;
  int minVal;
  int maxVal;
  int defaultVal;
  int paramVal;
  bool isInt = true;
};


class synPar
{
  public:
    virtual bool set (int newVal) = 0;   // pure function
    virtual bool set (float newVal) = 0;   // pure function
    virtual bool setMin (int newVal) = 0;
    virtual bool setMin (float newVal) = 0;
    virtual bool setMax (int newVal) = 0;
    virtual bool setMax (float newVal) = 0;
    virtual bool setDefault (int newVal) = 0;
    virtual bool setDefault (float newVal) = 0;
    
    virtual int getId () = 0;
    virtual float getDefault () = 0;
    virtual float get () = 0;
    virtual float getMin () = 0;
    virtual float getMax () = 0;
    virtual bool isInt () = 0;
};

class synIpar : public synPar
{
  public:

    synIpar (synIparam setupVals) {
      parVals = setupVals;
    }

    synIpar (int id, int minVal, int maxVal, int defaultVal, int paramVal) {
      parVals.id = id;
      parVals.minVal = minVal;
      parVals.maxVal = maxVal;
      parVals.defaultVal = defaultVal;
      parVals.paramVal = paramVal;
    }

    bool set(int newVal) {
      bool didSetIt = false;
      if (newVal != parVals.paramVal) {
      parVals.paramVal = constrain(newVal, parVals.minVal, parVals.maxVal);
      didSetIt = true;
      }
      return didSetIt;
    }

    bool set(float newVal) { return false; }
    
    bool setMin(int newVal) {
      parVals.minVal = newVal;
      return true;
    }

    bool setMax(float newVal) { return false; }

    bool setMax(int newVal) {
      parVals.maxVal = newVal;
      return true;
    }

    bool setDefault(float newVal) { return false; }

    bool setDefault(int newVal) {
      parVals.defaultVal = newVal;
      return true;
    }

    bool setMin(float newVal) { return false; }

    int getId () { return parVals.id; }
    
    float get() { return (float)parVals.paramVal; }
    
    float getDefault() { return (float)parVals.defaultVal; }

    float getMin() { return (float)parVals.minVal; }

    float getMax() { return (float)parVals.maxVal; }

    bool isInt() { return parVals.isInt; }

  private:
    synIparam parVals;
};

class synFpar : public synPar
{
  public:
    synFpar (synFparam setupVals) {
      parVals = setupVals;
    }

    synFpar (int id, float minVal, float maxVal, float defaultVal, float paramVal) {
      parVals.id = id;
      parVals.minVal = minVal;
      parVals.maxVal = maxVal;
      parVals.defaultVal = defaultVal;
      parVals.paramVal = paramVal;
    }

    bool set(int newVal) { return false; }
    
    bool set(float newVal) {
      bool didSetIt = false;
      if (newVal != parVals.paramVal) {
      parVals.paramVal = constrain(newVal, parVals.minVal, parVals.maxVal);
      didSetIt = true;
      }
      return didSetIt;
    }

    bool setMin(float newVal) {
      parVals.minVal = newVal;
      return true;
    }

    bool setMin(int newVal) { return false; }

    bool setMax(float newVal) {
      parVals.maxVal = newVal;
      return true;
    }

    bool setMax(int newVal) { return false; }

    bool setDefault(float newVal) {
      parVals.defaultVal = newVal;
      return true;
    }

    bool setDefault(int newVal) { return false; }

    int getId () { return parVals.id; }
    
    float get() { return parVals.paramVal; }

    float getDefault() { return parVals.defaultVal; }

    float getMin() { return parVals.minVal; }

    float getMax() { return parVals.maxVal; }

    bool isInt() { return parVals.isInt; }

  private:
    synFparam parVals;
};


class SynthPatch
{
    public:
      //Public constructor and methods
      SynthPatch();
      void reset();
      void copyPatchTo(SynthPatch &destination);
      float get(int parName);
      float getDefault(int parName);
      float getMin(int parName);
      float getMax(int parName);
      int getI(int parName);
      int getIdefault(int parName);
      int getImin(int parName);
      int getImax(int parName);
      int getId(int parName);
      bool isInt(int parName);
      const char* getPname(int parName);
      void set(int parName, float parVal);
      void setI(int parName, int parVal);
      void setMin(int parName, float minVal);
      void setMax(int parName, float maxVal);
      void setDefault(int parName, float defaultVal);
  
      //Class data members:
      enum {SynParameterCount = 39};
      synPar * params [SynParameterCount];
      const char *paramNames[SynParameterCount] = {
                            "VCO1wform ", "VCO1plwdth ",
                            "VCO2wform ", "VCO2plwdth ", "VCO2detune ",
                            "VCO1mix ", "VCO2mix ", "VCO3mix ", "VCO4mix ",
                            "LfodFilMix ", "EnvFilMix ",
                            "FilOutMix ", "DelayOutMix ",
                            "DryDlyInMix ", "FilDlyFbMix ", "RevDlyFbMix ",
                            "EnvFilFq ", "EnvFilRes ", "EnvFilOct ",
                            "LfoedFilFq ", "LfoedFilRes ", "LfoedFilOct ",
                            "TurboFilFq ", "TurboFilRes ", "TurboFilOct ",
                            "DelayFilFq ", "DelayFilRes ", "DelayFilOct ",
                            "FilLFOAmp ", "FilLFOFreq ",
                            "DelayLFOAmp ", "DelayLFOFq ",
                            "DelayTime ", "DelRevTime ",
                            "VCAEnvAtt ", "VCAEnvHold ", "VCAEnvDcy ",
                            "VCAEnvStn ", "VCAEnvRlse "};       
    private:
      char *patchName;
                                                   
};

#endif
