#ifndef __TRACKLIST
#define __TRACKLIST

#include <Arduino.h>
#include "Enum.h"
#include "Track.h"

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
    LinkedTrackList()
    {
        head = NULL; // set head to NULL
        cur = NULL;
        tail = NULL;
    }
 
    void dropTrack(byte trackNum)
    {
        rewind();
        trackNode *prevNode;

        while( hasValue()){
            if( cur->trackNumber == trackNum)
            {
                dropNode(prevNode);
            }
            prevNode = cur;
            next();
        }
    }

    void dropNode(trackNode *prev)
    {
        if (cur == head)
        {
            cur = head->next;
            head = cur;
        } else 
        { 
            if (cur == tail)
            {
                tail = prev;
                prev->next = NULL;
            } else 
            {
                prev->next = cur->next;
                cur = prev;
            }
        }
    }
    // add value at the end -kg
    void appendTrack(byte aTrackNum, Track *aTrack)
    {
        trackNode *n = new trackNode();   // create new Node
        n->trackNumber = aTrackNum;  // set value
        n->trackRef = aTrack;

        if(tail != NULL)
            tail->next = n; // point previously last node to new one

        tail = n;           // point tail at new node

        if(cur == NULL)
            cur = n;

        if(head == NULL)
            head = n;
    }

    void rewind()
    {
            cur = head;
    }
    void next()
    {
            if( cur != NULL )
                    cur = cur->next;
    }

    Track* getTrackRef()
    {
            Track *retVal;
            if( cur != NULL )
                    retVal = cur->trackRef;
            return retVal; // really we should raise exception
    }

    byte getTrackNumber()
    {
            if( cur != NULL )
                    return cur->trackNumber;
            return 0; // really we should raise exception
    }
    
    int hasValue()
    {
            return ( cur != NULL ? true : false );
    }

private:
    trackNode *head;
    trackNode *cur;
    trackNode *tail;
};

#endif