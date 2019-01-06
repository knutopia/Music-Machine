#include "Path.h"
#include "Enum.h"

//Public constructor and methods
Path::Path()
{
    m_currentPath = 0;
    m_currentStep = 0;
}

//"Getters" and "setters"
byte Path::getAndAdvanceStepPos(byte seqLength)
{
    m_currentStep += 1;
    m_currentStep %= seqLength;
    return m_paths[m_currentPath][m_currentStep];
}

byte Path::getDontAdvanceStepPos(byte seqLength)
{
    m_currentStep %= seqLength;
    return m_paths[m_currentPath][m_currentStep];
}

byte Path::getStepPosForward(byte index, byte seqLength)
{
    byte stepAhead = (m_currentStep + index) % seqLength;
    return m_paths[m_currentPath][stepAhead];
}

byte Path::getCurrentStepCount(){return m_currentStep;}

bool Path::detectPatternRollover(byte seqLength)
{
    bool retVal = false;

    if( m_currentStep + 1 >= seqLength)
        retVal = true;

    return retVal;
}

bool Path::checkForSequenceStart()
{
    bool retVal = false;
    if((m_currentStep) == 0) retVal = true;
    return retVal;        
}

void Path::setPath(int index)
{
    m_currentPath = index % MAXPATHSTEPS;
}

void Path::setNextPath()
{
    m_currentPath = (++m_currentPath)  % MAXPATHSTEPS;
}

void Path::setPrevPath()
{
    m_currentPath = (--m_currentPath)  % MAXPATHSTEPS;
}

const char* Path::getPathName()
{
    return m_pathNames[m_currentPath];
}

int Path::getCurrentPathNumber()
{
    return m_currentPath;
}


//Helper methods
void Path::resetStep() 
{
    m_currentStep = 0;
}

void Path::resetPath()
{
    m_currentPath = 0;
}