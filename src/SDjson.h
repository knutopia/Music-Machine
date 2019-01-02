#ifndef __SDJSON
#define __SDJSON

#include <Arduino.h>
#include <Audio.h>
#include <SD_t3.h>
#include <ArduinoJson.h>

// Configuration that we'll store on disk
struct JsonConfig {
  char hostname[64];
  int port;
};


class SDjsonHandler
{
    public:
        JsonConfig config; // <- global configuration object

        void loadChains();
        void saveChains();
        void loadConfiguration(const char *filename, JsonConfig &config);
        void saveConfiguration(const char *filename, const JsonConfig &config);
        void printFile(const char *filename);
        void jsonSetup();

    private:
        const char *filename = "/config.txt";  // <- SD library uses 8.3 filenames
        const char *chainFileName ="/chains.txt";
};
#endif