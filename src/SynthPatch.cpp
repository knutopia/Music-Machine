#include "SynthPatch.h"
#include "InOutHelper.h"

extern long g_step_duration;
extern InOutHelper inout;


SynthPatch::SynthPatch()
{
  reset();
};

void SynthPatch::reset()
{
  params[0] = new synIpar((synIparam){VCO1waveform  , 0, 7, 1, 1});
  params[1] = new synFpar((synFparam){VCO1pulsewidth, 0, 1, .5, .5});

  params[2] = new synIpar((synIparam){VCO2waveform   , 0, 7,  2,  2});
  params[3] = new synFpar((synFparam){VCO2pulsewidth , 0, 1, .5, .5});
  params[4] = new synFpar((synFparam){VCO2detune     , 0, 14, 0, .0});

  params[5] = new synFpar((synFparam){VCO1mix           , 0, 1, .5, .5});
  params[6] = new synFpar((synFparam){VCO2mix           , 0, 1, .5, .5});
  params[7] = new synFpar((synFparam){VCO3mix           , 0, 1,  0,  0});
  params[8] = new synFpar((synFparam){VCO4mix , 0, 1,  0,  0});

  params[9] = new synFpar((synFparam){LfoedFilterMix     , 0, 1, .5, .5});
  params[10] = new synFpar((synFparam){EnvelopedFilterMix , 0, 1,  0,  0});

  params[11] = new synFpar((synFparam){FilterOutputMix , 0, 1, .5, .5});
  params[12] = new synFpar((synFparam){DelayOutputMix  , 0, 1,  0,  0});

  params[13] = new synFpar((synFparam){DryDelayInMix            , 0, 1, .5,  .5});
  params[14] = new synFpar((synFparam){FilteredDelayFeedbackMix , 0, 1, .25, .25});
  params[15] = new synFpar((synFparam){RevDlyFeedbackMix , 0, 1, .25, .25});

  params[16] = new synIpar((synIparam){EnvFilterFreq, 0, 10000, 1000, 1000});
  params[17] = new synFpar((synFparam){EnvFilterRes , 0.7, 5,  1,  1});
  params[18] = new synFpar((synFparam){EnvFilterOct , 0,   7, .2, .2});

  params[19] = new synIpar((synIparam){LfoedFilterFreq, 0, 10000, 1000, 1000});
  params[20] = new synFpar((synFparam){LfoedFilterRes , 0.7, 5,  1,  1});
  params[21] = new synFpar((synFparam){LfoedFilterOct , 0,   7, .2, .2});

  params[22] = new synIpar((synIparam){TurboFilterFreq, 0, 10000, 1200, 1200});
  params[23] = new synFpar((synFparam){TurboFilterRes , 0.7, 5, .6, .6});
  params[24] = new synFpar((synFparam){TurboFilterOct , 0,   7, .5, .5});

  params[25] = new synIpar((synIparam){DelayFilterFreq, 0, 10000, 800, 800});
  params[26] = new synFpar((synFparam){DelayFilterRes , 0.7, 5,  3,  3});
  params[27] = new synFpar((synFparam){DelayFilterOct , 0,   7, .5, .5});

  params[28] = new synFpar((synFparam){FilterLFOAmp  , 0, 1,      .5,  .5});
  params[29] = new synFpar((synFparam){FilterLFOFreq , 0, 22000, 200, 200});

  params[30] = new synFpar((synFparam){DelayLFOAmp  , 0,     1, .5,  .5});
  params[31] = new synFpar((synFparam){DelayLFOFreq , 0, 22000, .25, .25});

  params[32] = new synFpar((synFparam){DelayTime , 0, 1, .1, .1}); //g_step_duration / 4000.0
  params[33] = new synFpar((synFparam){DelayedReverbTime , 0, 1, .1, .1});      
  params[34] = new synFpar((synFparam){VCAEnvAttack , 1, 500, 10.5 , 10.5});
  params[35] = new synFpar((synFparam){VCAEnvHold , 1, 100, 2.5, 2.5});
  params[36] = new synFpar((synFparam){VCAEnvDecay , 1, 100, 35, 35});
  params[37] = new synFpar((synFparam){VCAEnvSustain , 0, 1, .5, .5});
  params[38] = new synFpar((synFparam){VCAEnvRelease , 1, 500, 100, 100});
}

void SynthPatch::copyPatchTo(SynthPatch &destination) 
{
  for (int pindex = 0; pindex < SynParameterCount; pindex++)
  {
    destination.params[pindex] = params[pindex];
  }
#ifdef DEBUG
  Serial.print(" copyPatchTo ");
#endif
}

float SynthPatch::get(int parName)
{
    return params[parName]->get();
}

float SynthPatch::getDefault(int parName)
{
    return params[parName]->getDefault();
}

float SynthPatch::getMin(int parName)
{
    return params[parName]->getMin();
}

float SynthPatch::getMax(int parName)
{
    return params[parName]->getMax();
}

int SynthPatch::getI(int parName)
{
    return (int)params[parName]->get();
}

int SynthPatch::getIdefault(int parName)
{
    return (int)params[parName]->getDefault();
}

int SynthPatch::getImin(int parName)
{
    return (int)params[parName]->getMin();
}

int SynthPatch::getImax(int parName)
{
    return (int)params[parName]->getMax();
}

int SynthPatch::getId(int parName)
{
    return (int)params[parName]->getId();
}

bool SynthPatch::isInt(int parName)
{
    return params[parName]->isInt();
}

const char* SynthPatch::getPname(int parName)
{
    return paramNames[parName];
}

void SynthPatch::set(int parName, float parVal)
{
    if (!isInt(parName))
      bool foo = params[parName]->set(parVal);
    else
      bool foo = params[parName]->set((int)parVal);
}

void SynthPatch::setI(int parName, int parVal)
{
    bool foo = params[parName]->set(parVal);
}    

void SynthPatch::setMin(int parName, float minVal)
{
    if (isInt(parName))
      bool foo = params[parName]->setMin((int)minVal);
    else
      bool foo = params[parName]->setMin(minVal);

}
/*    
void SynthPatch::setImin(int parName, int minVal)
{
    bool foo = params[parName]->setMin(minVal);
}
*/
void SynthPatch::setMax(int parName, float maxVal)
{
    if (isInt(parName))
      bool foo = params[parName]->setMax((int)maxVal);
    else
      bool foo = params[parName]->setMax(maxVal);
}

/*    
void SynthPatch::setImax(int parName, int maxVal)
{
    bool foo = params[parName]->setMax(minVal);
}
*/

void SynthPatch::setDefault(int parName, float defaultVal)
{
    if (isInt(parName))
      bool foo = params[parName]->setDefault((int)defaultVal);
    else
      bool foo = params[parName]->setDefault(defaultVal);
}

