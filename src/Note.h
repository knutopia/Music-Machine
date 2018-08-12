#ifndef __NOTE
#define __NOTE

#include <Arduino.h>
#include "Enum.h"

//#include <cstddef>
//#include <iostream>

using namespace std;

//#define eol "\n"
extern int g_activeGlobalStep;
//extern LinkedNoteList activeNotes;

struct note {
    retrigDivisions retrigClickDivider;
    bool unmuted;
    bool playIt;
    int pitchVal;
    float pitchFreq;

    unsigned long durationMS;
    bool hold;

    float duration;
    
    uint8_t retrigs;
    uint8_t ticks;
    uint8_t accent; //??
    uint8_t velocity;
    uint8_t swingTicks;
    uint8_t holdsAfter;

//      byte m_probability[max_notes];
//      byte m_retrig[max_notes];

//      byte m_length;
//      byte m_transposition;
//      byte m_path;
};

#endif