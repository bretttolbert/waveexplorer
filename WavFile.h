#ifndef WAVFILE_H
#define WAVFILE_H

#include <iostream>
#include <fstream>
#include <cstdlib>

#define BYTE_SWAP4(x) \
  (((x & 0xFF000000) >> 24) | \
   ((x & 0x00FF0000) >> 8) | \
   ((x & 0x0000FF00) << 8)  | \
   ((x & 0x000000FF) << 24))

typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned char UINT8;

const UINT32 RIFF = 0x52494646; // the letters "RIFF" in big-endian form
const UINT32 WAVE = 0x57415645; // the letters "WAVE" in big-endian form
const UINT32 FMT = 0x666d7420; // the letters "fmt " in big-endian form
const UINT32 DATA = 0x64617461; // the letters "data" in big-endian form

struct WavFmt
{
    UINT16 audioFormat;
    UINT16 numChannels;
    UINT32 sampleRate;
    UINT32 byteRate;
    UINT16 blockAlign;
    UINT16 bitsPerSample;
};

class WavFile
{
public:
    WavFile(const char* filename);
    ~WavFile();
    bool good();
    UINT16 getAudioFormat();
    UINT16 getNumChannels();
    UINT32 getSampleRate();
    UINT32 getByteRate();
    UINT16 getBlockAlign();
    UINT16 getBitsPerSample();
    size_t getNumSamples();
    /**
     * @param channel 0=left 1=right
     * @param sampleNum 0 <= sampleNum < numSamples
     * @param sample buffer to get the sample, cast appropriate data type
     * to char* e.g. UINT16 if bitsPerSample is 16:
     *      UINT16 sample;
     *      getSample(0, 0, (char*)&sample);
     */
    void getSample(unsigned int channel, unsigned int sampleNum, char* sample);
    //void getSample(unsigned int channel, unsigned int sampleNum, UINT16& sample);
    void saveAs(const char* filename);
    void convertToMono();
    void convertSampleRate();
    void convertBitDepth();
//protected:
    void readChunkHeader(UINT32& chunkId, UINT32& chunkSize);
    void readRiffChunkDescriptor();
    void readFmtSubChunk();
    void readDataSubChunk();
    void printSamples(); //tbr
    void recalculateByteRateAndBlockAlign();
    bool isGood; //WAV file is "good" until a decode error occurs
    size_t size; //total size of WAV file including all headers
    std::fstream file;
    WavFmt fmt;
    char* data; //nothing but samples (payload of data sub-chunk)
    size_t dataSize;
    size_t numSamples;
};

#endif // WAVFILE_H
