#ifndef __NOTEPERCLICK
#define __NOTEPERCLICK

#include <Arduino.h>
#include "Note.h"

using namespace std;

struct notePerClick {
    note clickNote;
    unsigned long durationMS;
    byte track;
    bool active;
};

#endif
