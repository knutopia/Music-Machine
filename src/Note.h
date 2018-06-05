#ifndef __NOTE
#define __NOTE

#include <Arduino.h>
#include "Enum.h"

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

//      byte m_probability[max_notes];
//      byte m_retrig[max_notes];

//      byte m_length;
//      byte m_transposition;
//      byte m_path;
};

#endif