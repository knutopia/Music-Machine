#ifndef __TRACKLIST
#define __TRACKLIST

#include <Arduino.h>
#include "Enum.h"
#include "Track.h"
#include "InOutHelper.h"

using namespace std;

class LinkedTrackList 
{
    
    struct trackNode {
        byte trackNumber;
        Track *trackRef;
        trackNode *next;
    };
 
// public member
public:
    // constructor
    LinkedTrackList();
    ~LinkedTrackList();
    bool checkIntegrity(char caller[]);
    Track* getTrackRef(byte trackNum);
    void dropTrack(byte trackNum);
    void dropNode(trackNode *prev);
    void appendTrack(byte aTrackNum, Track *aTrack);
    void rewind();
    void next();
    Track* getTrackRef();
    byte getTrackNumber();
    int hasValue();
    int count();
    void print();

private:
    trackNode *head;
    trackNode *cur;
    trackNode *tail;
};

#endif