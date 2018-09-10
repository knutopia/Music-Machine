#include "SDHandler.h"
#include "StepSequencer.h"
#include "SynthEngine.h"

extern StepSequencer sequencer;
extern SynthEngine synth;

SDHandler::SDHandler()
{
}

void SDHandler::setupSDcard()
{
    Serial.print("Initializing SD card...");
    
    if (!SD.begin(chipSelect)) {
      Serial.println("initialization failed!");
      return;
    }
    Serial.println("initialization done.");
}

bool SDHandler::backupTrackFile()
{
    bool success = false;

    // make a backup
    if (!SD.exists("seqs.txt")) {
      Serial.println("No seqs.txt data file found - no backup created.");      
    } else {
      myFile = SD.open("seqs.txt", FILE_READ);
      if (!myFile) {
        Serial.println("FAILED to open seqs.txt data file - no backup created.");      
      } else {
        if (SD.exists("seqsbup.txt")) {
          Serial.println("Removing existing seqsbup.txt backup file");
          SD.remove("seqsbup.txt");
          if (SD.exists("seqsbup.txt")) {
            Serial.println("FAILED to remove backup file");
          }
        }
        Serial.println("Creating new backup file");
        bupFile = SD.open("seqsbup.txt", FILE_WRITE);
        if(!bupFile) {
          Serial.println("FAILED to create new backup file seqsbup.txt");
          myFile.close();
        } else {
          Serial.println("Copying seqs.txt to new backup seqsbup.txt");
          int foo = 0;
          int n;  
          uint8_t buf[1024];
          while ((n = myFile.read(buf, sizeof(buf))) > 0) {
            bupFile.write(buf, n);
            foo += n;
            Serial.print(".");
          }
          bupFile.close();
          myFile.close();
          success = true;
          Serial.print(foo);
          Serial.println(" chars total");
          Serial.println("seqs.txt copied to new backup seqsbup.txt");

          if (SD.exists("seqs.txt")) {
            Serial.println("Removing existing seqs.txt file");
            SD.remove("seqs.txt");
            if (SD.exists("seqs.txt")) {
              Serial.println("FAILED to remove existing seqs.txt file");
            }
          }
        }
      }
    }
    return success;
}

bool SDHandler::writeTrackToSDcard(byte trackNum)
{

    bool success = false;

    myFile = SD.open("seqs.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (myFile) {

        Serial.println("writeTrackToSDcard: seqs.txt opened successfully");

        byte trackBuf = sequencer.getCurrentTrack();
        int seqBuf = sequencer.getCurrentSequence();
        
        sequencer.setCurrentTrack(trackNum);
        int storedTrackSeqBuf = sequencer.getCurrentSequence();

        myFile.println();
        myFile.println();
        myFile.print("Track = ");
        myFile.println(trackNum);

        Serial.print("Storing");
        for(int c = 0; c< StepSequencer::max_sequences; c++)
        {
            //Select the sequence with index c
            sequencer.swap_edit_root_seqs(c);
            sequencer.setCurrentSequence(c);
            
//          sequencer.printSequence();
            
            myFile.println();
            myFile.println();
            myFile.print("Sequence = ");
            myFile.println(c);
            myFile.print("Length = ");
            myFile.println(sequencer.getLength());
            myFile.print("Transposition = ");
            myFile.println(sequencer.getTransposition());
            myFile.print("Path = ");
            myFile.println(sequencer.getPath());
            myFile.println(" ");

            for(int n = 0; n < sequencer.getMaxLength(); n++)
            {
                myFile.print("NoteIndex = ");
                myFile.println(n);
                myFile.print("Pitch = ");
                myFile.println(sequencer.getNote(n));
                myFile.print("Duration = ");
                myFile.println(sequencer.getDuration(n));
                myFile.print("Probability = ");
                myFile.println(sequencer.getProbability(n));           
                myFile.print("Ticks = ");
                myFile.println(sequencer.getTicks(n));           
                myFile.print("Mute = ");
                myFile.println(sequencer.getMute(n));           
                myFile.print("Hold = ");
                myFile.println(sequencer.getHold(n));           
                myFile.print("Accent = ");
                myFile.println(sequencer.getAccent(n));           
                myFile.print("Retrig = ");
                myFile.println(sequencer.getRetrig(n));           
                myFile.print("Velocity = ");
                myFile.println(sequencer.getVelocity(n));           
                myFile.println(" ");
            }
            sequencer.swap_edit_root_seqs(c);
            myFile.println(" ");
            Serial.print(" seq");
            Serial.print(c);
        } 
        myFile.close();
        success = true;
        Serial.println();
        Serial.println("Sequence saving done.");
        Serial.println();

        sequencer.setCurrentSequence(storedTrackSeqBuf);
        sequencer.setCurrentTrack(trackBuf);
        sequencer.setCurrentSequence(seqBuf);
  
    } else {
      // if the file didn't open, print an error:
      Serial.println("writeTrackToSDcard: error opening seqs.txt");
    }      
    return success;
}

bool SDHandler::readTracksFromSDcard()
{

    bool success = false;

    char buffer[40]; // May need to be a bit bigger if you have long names
    byte index = 0;
    
    int trackBuf = sequencer.getCurrentTrack();
    sequencer.bufferAllTrackSeqIndices(BUFFER);

    Serial.print("readTracksFromSDcard: track before loading is ");
    Serial.println(trackBuf);
    sequencer.setCurrentTrack(1); //backwards compatibility

    myFile = SD.open("seqs.txt");
    if (!myFile) {
      // if the file didn't open, print an error:
      Serial.println("readTracksFromSDcard: error opening seqs.txt on SD Card");
      inout.ShowErrorOnLCD("SD Card RT file err");
      myFile.close();
    } else {
      Serial.println("seqs.txt:");

      while (myFile.available())
      {
        char c = myFile.read();
        if(c == '\n' || c == '\r') // Test for <cr> and <lf>
        {
           parseAndAssignTrackSD(buffer);
           index = 0;
           buffer[index] = '\0'; // Keep buffer NULL terminated
        }
        else
        {
           buffer[index++] = c;
           buffer[index] = '\0'; // Keep buffer NULL terminated
        }
      }
      myFile.close();
      success = true;
    }
    sequencer.setCurrentTrack(trackBuf);
    sequencer.bufferAllTrackSeqIndices(RESTORE);
    return success;
}

bool SDHandler::backupPatchFile()
{


    bool success = false;

    if (!SD.exists("snds.txt")) {
      Serial.println("No snds.txt data file found - no backup created.");      
    } else {
      myFile = SD.open("snds.txt", FILE_READ);
      if (!myFile) {
        Serial.println("FAILED to open snds.txt data file - no backup created.");      
      } else {
        if (SD.exists("sndsbup.txt")) {
          Serial.println("Removing existing sndsbup.txt backup file");
          SD.remove("sndsbup.txt");
          if (SD.exists("sndsbup.txt")) {
            Serial.println("FAILED to remove sndsbup.txt backup file");
          }
        }
        Serial.println("Creating new backup file");
        bupFile = SD.open("sndsbup.txt", FILE_WRITE);
        if(!bupFile) {
          Serial.println("FAILED to create new backup file sndsbup.txt");
          myFile.close();
        } else {
          Serial.println("Copying snds.txt to new backup sndsbup.txt");
          int foo = 0;
          int n;  
          uint8_t buf[1024];
          while ((n = myFile.read(buf, sizeof(buf))) > 0) {
            bupFile.write(buf, n);
            foo += n;
            Serial.print(".");
          }
          bupFile.close();
          myFile.close();
          success = true;
          Serial.print(foo);
          Serial.println(" chars total");
          Serial.println("snds.txt copied to new backup sndsbup.txt");
          if (SD.exists("snds.txt")) {
            Serial.println("Removing existing snds.txt file");
            SD.remove("snds.txt");
            if (SD.exists("snds.txt")) {
              Serial.println("FAILED to remove existing snds.txt file");
            }
          }
        }
      }
    }
    return success;
}

bool SDHandler::writePatchesToSDcard()
{

    bool success = false;
    myFile = SD.open("snds.txt", FILE_WRITE);
    
    // if the file opened okay, write to it:
    if (!myFile) {
        // if the file didn't open, print an error:
        Serial.println("writePatchesToSDcard: error opening snds.txt");
    } else {

        Serial.println("writePatchesToSDcard: snds.txt opened successfully");

        Serial.print("max_patches: ");
        Serial.println(SynthEngine::max_patches);

        for(int c = 0; c < SynthEngine::max_patches; c++)
        {
          // Select the patch with index c
          // synth.setCurrentPatch(c);

          Serial.print(" Patch: ");
          Serial.print(c);
          
          myFile.println(" ");
          myFile.println(" ");
          myFile.print("Patch = ");
          myFile.println(c);
          myFile.println(" ");

          for(int p = 0; p < SynthPatch::SynParameterCount; p++)
          {
/*
            Serial.print("Param: ");
            Serial.print(p);
            Serial.print("\t");
            Serial.println(synth.m_patches[c].getPname(p)); 
*/            
            myFile.print("Param = ");
            myFile.println(p);
            myFile.println(" ");
  
            bool isInt = synth.m_patches[c].isInt(p);
            if (isInt)
            {
              myFile.print("ParamName = ");
              myFile.println(synth.m_patches[c].getPname(p));
              myFile.print("Id = ");
              myFile.println(synth.m_patches[c].getId(p));
              myFile.print("isInt = ");
              myFile.println(isInt);
              myFile.print("Min = ");
              myFile.println(synth.m_patches[c].getImin(p));
              myFile.print("Max = ");
              myFile.println(synth.m_patches[c].getImax(p));
              myFile.print("Default = ");
              myFile.println(synth.m_patches[c].getIdefault(p));
              myFile.print("Value = ");
              myFile.println(synth.m_patches[c].getI(p));
              myFile.print("End = ");
              myFile.println(synth.m_patches[c].getPname(p));
              
            } else {
              myFile.print("ParamName = ");
              myFile.println(synth.m_patches[c].getPname(p));
              myFile.print("Id = ");
              myFile.println(synth.m_patches[c].getId(p));
              myFile.print("isInt = ");
              myFile.println(isInt);
              myFile.print("Min = ");
              myFile.println(synth.m_patches[c].getMin(p));
              myFile.print("Max = ");
              myFile.println(synth.m_patches[c].getMax(p));
              myFile.print("Default = ");
              myFile.println(synth.m_patches[c].getDefault(p));
              myFile.print("Value = ");
              myFile.println(synth.m_patches[c].get(p));             
              myFile.print("End = ");
              myFile.println(synth.m_patches[c].getPname(p));
            }
            myFile.println(" ");
          }

          myFile.println(" ");
          Serial.print(" stored ");
//        Serial.print(c);
        } 
        myFile.close();
        success = true;
        Serial.println();
        Serial.println("Sound patches saving done.");
        Serial.println();
    }      
    return success;
}

bool SDHandler::readPatchesFromSDcard()
{

    bool success = false;

    char buffer[40];
    byte index = 0;
       
    myFile = SD.open("snds.txt");
    if (!myFile) {
      // if the file didn't open, print an error:
      Serial.println("readPatchesFromSDcard: error opening snds.txt on SD Card");
      inout.ShowErrorOnLCD("SD Card RP file err");
    } else {
      Serial.println("snds.txt:");

      while (myFile.available())
      {
        char c = myFile.read();
        if(c == '\n' || c == '\r') // Test for <cr> and <lf>
        {
           parseAndAssignSndSD(buffer);
           index = 0;
           buffer[index] = '\0'; // Keep buffer NULL terminated
        }
        else
        {
           buffer[index++] = c;
           buffer[index] = '\0'; // Keep buffer NULL terminated
        }
      }
      myFile.close();
      success = true;
      Serial.println();      
    }
    return success;
}

// privates:

void SDHandler::parseAndAssignTrackSD(char *buff)
{
    static int note_index = 0;
    char *name = strtok(buff, " =");
    if(name)
    {
      char *junk = strtok(NULL, " ");
      if(junk)
      {
        char *valu = strtok(NULL, " ");
        if(valu)
        {
/*
            Serial.print(name);
            Serial.print(" ");
            Serial.print(valu);
            Serial.println();
*/
            if(strcmp(name, "Track") == 0) 
            {
                Serial.print("Track ");
                Serial.println(valu);

                sequencer.setCurrentTrack(atoi(valu));

                Serial.print("  Mem: ");
                Serial.println(FreeMem());
            }
            
            if(strcmp(name, "Sequence") == 0) 
            {
                Serial.print("Sequence ");
                Serial.println(valu);

                sequencer.setCurrentSequence(atoi(valu));

                Serial.print("  Mem: ");
                Serial.println(FreeMem());
            }
            
            else if(strcmp(name, "Length") == 0)
                sequencer.setLength(atoi(valu));           

            else if(strcmp(name, "Transposition") == 0)
                sequencer.setTransposition(atoi(valu));           

            else if(strcmp(name, "Path") == 0)
                sequencer.setPath(atoi(valu));           
            
            else if(strcmp(name, "NoteIndex") == 0)
                note_index = atoi(valu);       

            else if(strcmp(name, "Pitch") == 0)
                sequencer.setNote(note_index, atoi(valu));           

            else if(strcmp(name, "Duration") == 0)
                sequencer.setDuration(note_index, atof(valu));           

            else if(strcmp(name, "Probability") == 0)
                sequencer.setProbability(note_index, atoi(valu));  
                        
            else if(strcmp(name, "Ticks") == 0)
                sequencer.setTicks(note_index, atoi(valu));      
                    
            else if(strcmp(name, "Mute") == 0)
                sequencer.setMute(note_index, atoi(valu));        
                    
            else if(strcmp(name, "Hold") == 0)
                sequencer.setHold(note_index, atoi(valu));        
                    
            else if(strcmp(name, "Accent") == 0)
                sequencer.setAccent(note_index, atoi(valu));
            
            else if(strcmp(name, "Retrig") == 0)
                sequencer.setRetrig(note_index, atoi(valu));

            else if(strcmp(name, "Velocity") == 0)
                sequencer.setVelocity(note_index, atoi(valu));
         }
      }
    }
}

void SDHandler::parseAndAssignSndSD(char *buff)
{
    static int patch = 0;
    static int param = 0;
    static bool isInt = false;
    static float minVal = 0;
    static float maxVal = 0;
    static float defaultVal = 0;
    static float value = 0;
    
    // use statics to fill a constructor for synIpar. synFpar
    
    char *name = strtok(buff, " =");
    if(name)
    {
      char *junk = strtok(NULL, " ");
      if(junk)
      {
        char *valu = strtok(NULL, " ");
        if(valu)
        {
/*          
          Serial.print(name);
          Serial.print(" ");
          Serial.print(valu);
          Serial.println();
*/
          if(strcmp(name, "Patch") == 0) {
              patch = atoi(valu);
              Serial.println();
              Serial.print("Patch ");
              Serial.println(patch);
          }
          
          if(strcmp(name, "Param") == 0)   // Use ID ?
              param = atoi(valu);

          if(strcmp(name, "isInt") == 0)
              isInt = (bool) atoi(valu);

          if(strcmp(name, "Min") == 0)
              minVal = atof(valu);

          if(strcmp(name, "Max") == 0)
              maxVal = atof(valu);

          if(strcmp(name, "Default") == 0)
              defaultVal = atof(valu);

          if(strcmp(name, "Value") == 0)
              value = atof(valu);

          if(strcmp(name, "End") == 0) {
/*
            Serial.print(synth.m_patches[patch].getPname(param));
            Serial.print(": ");
            Serial.println(value);
*/            
            if (isInt) {
              synth.m_patches[patch].params[param] = new synIpar((synIparam){param, 
                                                                             (int)minVal, 
                                                                             (int)maxVal, 
                                                                             (int)defaultVal, 
                                                                             (int)value});
            } else {
              synth.m_patches[patch].params[param] = new synFpar((synFparam){param, 
                                                                             minVal, 
                                                                             maxVal, 
                                                                             defaultVal, 
                                                                             value});
            }
          }        
         }
      }
    }
}

uint32_t SDHandler::FreeMem()
{ // for Teensy 3.0
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t) &stackTop;

    // current position of heap.
    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);

    // The difference is (approximately) the free, available ram.
    return stackTop - heapTop;
}