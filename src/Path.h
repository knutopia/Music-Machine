#ifndef __PATH
#define __PATH

#include <Arduino.h>
#include "Enum.h"

class Path
{
  public:
    enum{max_path_steps = 16};
    
    Path();

    //"Getters" and "setters"
    byte getAndAdvanceStepPos(byte seqLength);
    byte getDontAdvanceStepPos(byte seqLength);
    byte getStepPosForward(byte index, byte seqLength);
    byte getCurrentStepCount();
    bool checkForSequenceStart();
    void setPath(int index);
    void setNextPath();
    void setPrevPath();
    char* getPathName();
    
    //Helper methods
    void resetStep();
    void resetPath();

  private:

    //Class data members:
    int m_currentPath = 0;
    int m_currentStep = 0;

    char m_pathNames[16][20] = {"Simple path", 
                        "Reverse Path", 
                        "Back & Forth", 
                        "Back & Forth Rev",
                        "Up & Down", 
                        "Up & Down Rev", 
                        "CW Inward Spiral",
                        "CCW Outward", 
                        "CCW Inward", 
                        "CW Outward",
                        "Double Step", 
                        "Double Step Rev", 
                        "Tight Diagonal",
                        "Tight Dia Rev", 
                        "Double Diagonal", 
                        "Double Diag Rev"}; 
                          
    //  0,  1,  2,  3,  
    //
    //  4,  5,  6,  7,  
    //
    //  8,  9, 10, 11, 
    //
    // 12, 13, 14, 15
    
    const byte m_paths[16][16] = {
      { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15}, //simple
      {15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0}, //reverse
      { 0,  1,  2,  3,  7,  6,  5,  4,  8,  9, 10, 11, 15, 14, 13, 12}, //back and forth
      {12, 13, 14, 15, 11, 10,  9,  8,  4,  5,  6,  7,  3,  2,  1,  0}, //back and forth reverse
      { 0,  4,  8, 12, 13,  9,  5,  1,  2,  6, 10, 14, 15, 11,  7,  3}, //up and down
      { 3,  7, 11, 15, 14, 10,  6,  2,  1,  5,  9, 13, 12,  8,  4,  0}, //up and down reverse
      { 0,  1,  2,  3,  7, 11, 15, 14, 13, 12,  8,  4,  5,  6, 10,  9}, //clockwise inward spiral
      { 9, 10,  6,  5,  4,  8, 12, 13, 14, 15, 11,  7,  3,  2,  1,  0}, //counterclockwise outward
      { 0,  4,  8, 12, 13, 14, 15, 11,  7,  3,  2,  1,  5,  9, 10,  6}, //counterclockwise inward
      { 6, 10,  9,  5,  1,  2,  3,  7, 11, 15, 14, 13, 12,  8,  4,  0}, //clockwise outward
      { 0,  2,  4,  6,  8, 10, 12, 14,  1,  3,  5,  7,  9, 11, 13, 15}, //straight double step
      {15, 13, 11,  9,  7,  5,  3,  1, 14, 12, 10,  8,  6,  4,  2,  0}, //double step reverse
      { 0,  1,  4,  2,  5,  8,  3,  6,  9, 12,  7, 10, 13, 11, 14, 15}, //tight diagonal
      {15, 14, 11, 13, 10,  7, 12,  9,  6,  3,  8,  5,  2,  4,  1,  0}, //tight diagonal reverse
      { 0,  5, 10, 15,  8, 13,  3,  1,  6, 11,  4,  9, 14, 12,  2,  7}, //double diagonal
      { 7,  2, 12, 14,  9,  4, 11,  6,  1,  3, 13,  8, 15, 10,  5,  0}}; //double diagonal reverse

};

#endif
