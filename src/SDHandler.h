#ifndef __SDHANDLER
#define __SDHANDLER

#include <Arduino.h>
#include <Audio.h>
#include <SD_t3.h>


class SDHandler
{
    public:
        SDHandler();

        void setupSDcard();
        bool backupTrackFile();
        bool writeTrackToSDcard(byte trackNum);
        bool readTracksFromSDcard();
        bool backupPatchFile();
        bool writePatchesToSDcard();
        bool readPatchesFromSDcard();

    private:
        const int chipSelect = BUILTIN_SDCARD;

        File myFile;
        File bupFile;

        void parseAndAssignTrackSD(char *buff);
        void parseAndAssignSndSD(char *buff);
        uint32_t FreeMem();
};
#endif