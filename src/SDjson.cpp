#include <Arduino.h>
#include "SDjson.h"
#include "InOutHelper.h"
#include "PatternChainHandler.h"
#include "Enum.h"

extern InOutHelper inout;
extern PatternChainHandler patternChain;

void SDjsonHandler::loadChains()
{
    Serial.println("loadChains");

    File file = SD.open(chainFileName, FILE_READ);
    if (!file) {
        inout.ShowErrorOnLCD("SDjH:lC cFN fail");
        return;
    }

    const int capacity = JSON_ARRAY_SIZE(MAXCHAINCOUNT) 
                       + JSON_OBJECT_SIZE(2) 
                       + MAXCHAINCOUNT * (JSON_OBJECT_SIZE(5)
                                        + JSON_ARRAY_SIZE(MAXLINKSPERCHAIN)
                                        + MAXLINKSPERCHAIN * (JSON_OBJECT_SIZE(6)
                                                            + JSON_ARRAY_SIZE(TRACKCOUNT)
                                                            + TRACKCOUNT * JSON_OBJECT_SIZE(4)));

    StaticJsonBuffer<capacity> jsonBuffer;

    // Parse the root object
    JsonObject &root = jsonBuffer.parseObject(file);

    if (!root.success())
    {
        inout.ShowErrorOnLCD("SDjH:lC root fail");
        return;
    }
    Serial.println("  parseObject success");

//  root["maxChains"] = maxChains;
    int maxChainsInFile = root["maxChains"];

    Serial.print("  maxChainsInFile: ");
    Serial.println(maxChainsInFile);

    JsonArray& chains = root["chains"];
    if (!chains.success())
    {
        inout.ShowErrorOnLCD("SDjH:lC chains fail");
        return;
    }
    Serial.println("  parseArray success");

    int count = chains.size();    
    Serial.print("  chains size: ");
    Serial.println(count);

    int c = 0;
    for (JsonObject& cElem : chains) 
    {
        Serial.print("  chain ");
        Serial.println(c);

        JsonObject& chain = cElem;
        patternChain.chains[c].timesToPlay = chain["timesToPlay"];
        patternChain.chains[c].numberOfLinks = chain["numberOfLinks"];
        patternChain.chains[c].nextChain = chain["nextChain"];
        JsonArray& links = chain["links"];
        if (!links.success())
        {
            inout.ShowErrorOnLCD("SDjH:lC links fail");
            return;
        }
        int l = 0;
        for (JsonObject& lElem : links)
        {
            Serial.print("    link ");
            Serial.println(l);

            JsonObject& link = lElem;
            patternChain.chains[c].links[l]->link.timesToPlay = link["timesToPlay"];
            patternChain.chains[c].links[l]->link.leadTrack = link["leadTrack"];
            patternChain.chains[c].links[l]->link.nextLinkIndex = link["nextLinkIndex"];
            patternChain.chains[c].links[l]->link.lengthOverride = link["lengthOverride"];
            patternChain.chains[c].links[l]->link.pathOverride = link["pathOverride"];

            int speedMult = link["speedMult"];
            switch (speedMult)
            {
              case 0:
                patternChain.chains[c].links[l]->link.speedMult = UNDEFINED;
                break;
              case 1:
                patternChain.chains[c].links[l]->link.speedMult = NORMAL;
                break;
              case 2:
                patternChain.chains[c].links[l]->link.speedMult = DOUBLE;
                break;
              case 3:
                patternChain.chains[c].links[l]->link.speedMult = TRIPLE;
                break;
              case 4:
                patternChain.chains[c].links[l]->link.speedMult = QUAD;
                break;
              default:
                patternChain.chains[c].links[l]->link.speedMult = UNDEFINED;
                break;
            }
            JsonArray& tracks = link["trackUsedInLink"];
            if (!tracks.success())
            {
                inout.ShowErrorOnLCD("SDjH:lC tracks fail");
                return;
            }
            int t = 0;
            for (JsonObject& tElem : tracks)
            {
                JsonObject& track = tElem;
                patternChain.chains[c].links[l]->link.trackUsedInLink[t] = true;
                patternChain.chains[c].links[l]->link.patternPerTrack[t] = track["pattern"];
                patternChain.chains[c].links[l]->link.mutePerTrack[t] =  track["muted"];

                Serial.print("      track ");
                Serial.print(t);
                Serial.print("  pattern ");
                byte pat = track["pattern"];
                Serial.print(pat);
                Serial.print("  mute ");
                bool mute = track["mute"];
                Serial.println(mute);

                t++;
            }
            l++;
        }
        c++;
    }
    
    Serial.println("loadChains done");

    file.close();

    printFile(chainFileName);

}

void SDjsonHandler::saveChains()
{
    SD.remove(chainFileName);

    File file = SD.open(chainFileName, FILE_WRITE);
    if (!file) {
        inout.ShowErrorOnLCD("SDjH:sC cFN fail");
        return;
    }

    // Allocate the memory pool on the stack
    // Don't forget to change the capacity to match your JSON document.
    // Use https://arduinojson.org/assistant/ to compute the capacity.

/*
    const int capacity = JSON_ARRAY_SIZE(MAXCHAINCOUNT) 
                       + JSON_OBJECT_SIZE(2) 
                       + MAXCHAINCOUNT*JSON_OBJECT_SIZE(5)
                       + MAXCHAINCOUNT*JSON_ARRAY_SIZE(MAXLINKSPERCHAIN)
                       + MAXCHAINCOUNT*MAXLINKSPERCHAIN*JSON_OBJECT_SIZE(5);
*/
    const int capacity = JSON_ARRAY_SIZE(MAXCHAINCOUNT) 
                       + JSON_OBJECT_SIZE(2) 
                       + MAXCHAINCOUNT * (JSON_OBJECT_SIZE(5)
                                        + JSON_ARRAY_SIZE(MAXLINKSPERCHAIN)
                                        + MAXLINKSPERCHAIN * (JSON_OBJECT_SIZE(6)
                                                            + JSON_ARRAY_SIZE(TRACKCOUNT)
                                                            + TRACKCOUNT * JSON_OBJECT_SIZE(4)));

    Serial.print("capacity = ");
    Serial.println(capacity);


    StaticJsonBuffer<capacity> jsonBuffer;
//  StaticJsonBuffer<1024> jsonBuffer;

    // Parse the root object
    JsonObject &root = jsonBuffer.createObject();

/*  
struct Chain {
    int timesToPlay;
    PatternChainLink* links[MAXLINKSPERCHAIN];
    byte numberOfLinks;
    byte nextChain;
};

struct ChainLink {
    byte patternPerTrack[TRACKCOUNT];
    bool mutePerTrack[TRACKCOUNT];
    bool trackUsedInLink[TRACKCOUNT];
    int timesToPlay = 1;
    speedFactor speedMult = UNDEFINED;
    byte leadTrack;
    byte nextLinkIndex = 255;
    byte lengthOverride = 255;
    byte pathOverride = 255;
}; */

    int maxChains = MAXCHAINCOUNT;
    int maxLinks = MAXLINKSPERCHAIN;

    root["maxChains"] = maxChains;

    JsonArray& chains = root.createNestedArray("chains");

    for(int f = 0; f < maxChains; f++)
    {
        JsonObject& chain = chains.createNestedObject();
        chain["chainNumber"] = f;
        chain["timesToPlay"] = patternChain.chains[f].timesToPlay;
        chain["numberOfLinks"] = patternChain.chains[f].numberOfLinks;
        chain["nextChain"] = patternChain.chains[f].nextChain;
        JsonArray& links = chain.createNestedArray("links");

        for(int l = 0; l < patternChain.chains[f].numberOfLinks; l++)
        {
            JsonObject& link = links.createNestedObject();
            link["linkNumber"] = l;
            link["timesToPlay"] = patternChain.chains[f].links[l]->link.timesToPlay;
            link["speedMult"] = int(patternChain.chains[f].links[l]->link.speedMult);
            link["leadTrack"] = patternChain.chains[f].links[l]->link.leadTrack;
            link["nextLinkIndex"] = patternChain.chains[f].links[l]->link.nextLinkIndex;
            link["lengthOverride"] = patternChain.chains[f].links[l]->link.lengthOverride;
            link["pathOverride"] = patternChain.chains[f].links[l]->link.pathOverride;
            
            JsonArray& tracksUsedInLink = link.createNestedArray("trackUsedInLink");

            for(int t = 0; t < TRACKCOUNT; t++)
            {
                if(patternChain.chains[f].links[l]->link.trackUsedInLink[t])
                {
                    JsonObject& track = tracksUsedInLink.createNestedObject();
                    track["trackIndex"] = t;
                    track["pattern"] = patternChain.chains[f].links[l]->link.patternPerTrack[t];
                    track["muted"] = patternChain.chains[f].links[l]->link.mutePerTrack[t];
                }
            }
        }
    }
        
    // Serialize JSON to file
    if (root.prettyPrintTo(file) == 0) {
        inout.ShowErrorOnLCD("SDjH:sC writefail");
    }

    file.close();

    printFile(chainFileName);
}

void SDjsonHandler::loadConfiguration(const char *filename, JsonConfig &config) {
  // Open file for reading
  File file = SD.open(filename);

  // Allocate the memory pool on the stack.
  // Don't forget to change the capacity to match your JSON document.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<512> jsonBuffer;

  // Parse the root object
  JsonObject &root = jsonBuffer.parseObject(file);

  if (!root.success())
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonObject to the Config
  config.port = root["port"] | 2731;
  strlcpy(config.hostname,                   // <- destination
          root["hostname"] | "example.com",  // <- source
          sizeof(config.hostname));          // <- destination's capacity

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

// Saves the configuration to a file
void SDjsonHandler::saveConfiguration(const char *filename, const JsonConfig &config) {
  // Delete existing file, otherwise the configuration is appended to the file
  SD.remove(filename);

  // Open file for writing
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate the memory pool on the stack
  // Don't forget to change the capacity to match your JSON document.
  // Use https://arduinojson.org/assistant/ to compute the capacity.
  StaticJsonBuffer<256> jsonBuffer;

  // Parse the root object
  JsonObject &root = jsonBuffer.createObject();

  // Set the values
  root["hostname"] = config.hostname;
  root["port"] = config.port;

  // Serialize JSON to file
  if (root.printTo(file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

// Prints the content of a file to the Serial
void SDjsonHandler::printFile(const char *filename) {
  // Open file for reading
  File file = SD.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void SDjsonHandler::jsonSetup() {
/*
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;

  // Initialize SD library
  while (!SD.begin()) {
    Serial.println(F("Failed to initialize SD library"));
    delay(1000);
  }
*/

  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  inout.ShowInfoOnLCD("Loading json");
  loadConfiguration(filename, config);

  // Create configuration file
  Serial.println(F("Saving configuration..."));
  inout.ShowInfoOnLCD("Saving json");
  saveConfiguration(filename, config);

  // Dump config file
  Serial.println(F("Print config file..."));
  inout.ShowInfoOnLCD("Printing json");
  printFile(filename);
}