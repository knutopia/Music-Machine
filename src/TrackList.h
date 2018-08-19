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

    ~LinkedTrackList()
    {
#ifdef DEBUG
        Serial.print("Destructor LinkedTrackList ");
#endif
        trackNode *die = head;

        while(die) 
        {
            head = die->next;
            delete die->trackRef;
            delete die;
            die = head;

#ifdef DEBUG
            Serial.print("die ");
#endif
        }

        head = NULL;
        cur = NULL;
        tail = NULL;

#ifdef DEBUG
        Serial.print("done ");
#endif
    }

    Track* getTrackRef(byte trackNum)
    {
        Track* retVal = NULL;
        trackNode *curBup = cur;

        rewind();
        while(hasValue())
        {
            if(cur->trackNumber == trackNum)
            {
                retVal = cur->trackRef;
                break;
            }
            next();
        }
        cur = curBup;
        return retVal;
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
            delete head;
            head = cur;
        } else 
        { 
            if (cur == tail)
            {
                delete tail;
                tail = prev;
                prev->next = NULL;
            } else 
            {
                prev->next = cur->next;
                delete cur;
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
            else
                Serial.println("NULL TRACKREF");

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

    int count()
    {
        int retVal = 0;
        rewind();

        while( hasValue()){
            retVal++;
            next();
        }        
        return retVal;
    }

private:
    trackNode *head;
    trackNode *cur;
    trackNode *tail;
};

#endif