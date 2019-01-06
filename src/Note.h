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
    bool notEmpty = false;
    
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
    uint8_t mutesAfter;

//      byte m_probability[MAXNOTES];
//      byte m_retrig[MAXNOTES];

//      byte m_length;
//      byte m_transposition;
//      byte m_path;
};

#endif