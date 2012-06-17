#include "WavFile.h"
#include <cstring>

WavFile::WavFile(const char* filename)
    : isGood(true)
    , file(filename, std::ios::in | std::ios::binary)
{
    if (file.is_open())
    {
        size = file.tellg();
        file.seekg(0, std::ios::beg);
        readRiffChunkDescriptor();
        if (isGood)
        {
            readFmtSubChunk();
        }
        if (isGood)
        {
            readDataSubChunk();
        }
        file.close();
    }
    else
    {
        isGood = false;
    }
}

WavFile::~WavFile()
{
    delete[] data;
}

void WavFile::saveAs(const char* filename)
{
    std::ofstream outFile(filename, std::ios::out | std::ios::binary);
    if (outFile.is_open())
    {
        //write "RIFF" chunk descriptor
        UINT32 riffId = BYTE_SWAP4(RIFF);
        outFile.write((char*)&riffId, 4);
        UINT32 riffChunkSize = 36 + dataSize;
        outFile.write((char*)&riffChunkSize, 4);
        UINT32 waveId = BYTE_SWAP4(WAVE);
        outFile.write((char*)&waveId, 4);
        //write "fmt" subchunk
        UINT32 fmtId = BYTE_SWAP4(FMT);
        outFile.write((char*)&fmtId, 4);
        UINT32 fmtSubchunkSize = 16;
        outFile.write((char*)&fmtSubchunkSize, 4);
        outFile.write((char*)&fmt, sizeof(WavFmt));
        //write "data" subchunk
        UINT32 dataId = BYTE_SWAP4(DATA);
        outFile.write((char*)&dataId, 4);
        outFile.write((char*)&dataSize, 4);
        outFile.write((char*)data, dataSize);
        outFile.close();
    }
}

void WavFile::convertToMono()
{
    if (data && fmt.numChannels == 2)
    {
        if (fmt.bitsPerSample == 16)
        {
            size_t data1Size = dataSize / 2;
            std::cout << "data1Size: " << data1Size << '\n';
            char* data1 = new char[data1Size];
            char* p = data;
            char* p1 = data1;
            for (size_t i=0; i<numSamples; ++i)
            {
                INT16 leftSample = *((INT16*)p);
                *((INT16*)p1) = leftSample;
                p += sizeof(INT16);
                p1 += sizeof(INT16);
                INT16 rightSample = *((INT16*)p);
                p += sizeof(INT16);
            }
            delete[] data;
            data = data1;
            dataSize = data1Size;
            fmt.numChannels = 1;
            recalculateByteRateAndBlockAlign();
        }
    }
}

void WavFile::convertSampleRate()
{
    //convert 44.1 kHz 16 bit mono to 8 kHz 16 bit mono (sample ratio: 5.5125:1)
    if (fmt.bitsPerSample == 16 && fmt.sampleRate == 44100 && fmt.numChannels == 1)
    {
        //for now just take every 5th sample. this should result in a 10.25%
        //slowdown if my math is correct (it probably isn't)
        size_t numSamples1 = numSamples / 5;
        size_t dataSize1 = numSamples1 * sizeof(INT16);
        char* data1 = new char[dataSize1];
        char* p = data;
        char* p1 = data1;
        for (size_t i=0; i<numSamples; ++i)
        {
            if (i % 5 == 0)
            {
                INT16 sample = *((INT16*)p);
                *((INT16*)p1) = sample;
                p1 += sizeof(INT16);
            }
            p += sizeof(INT16);
        }
        delete[] data;
        data = data1;
        dataSize = dataSize1;
        fmt.sampleRate = 8000;
        numSamples = numSamples1;
        recalculateByteRateAndBlockAlign();
    }
}

void WavFile::convertBitDepth()
{
    //convert 16 bit mono to 8 bit mono
    if (data && fmt.bitsPerSample == 16)
    {
        size_t dataSize1 = dataSize / 2;
        char* data1 = new char[dataSize1];
        char* p = data;
        char* p1 = data1;
        for (size_t i=0; i<numSamples; ++i)
        {
            INT16 sample = *((INT16*)p);
            UINT8 sample1 = UINT8((sample >> 8) + 128);
            *p1 = sample1;
            p += sizeof(INT16);
            p1 += sizeof(UINT8);
        }
        delete[] data;
        data = data1;
        dataSize = dataSize1;
        fmt.bitsPerSample = 8;
        recalculateByteRateAndBlockAlign();
    }
}

void WavFile::recalculateByteRateAndBlockAlign()
{
    fmt.byteRate = fmt.sampleRate * fmt.numChannels * (fmt.bitsPerSample/8);
    fmt.blockAlign = fmt.numChannels * (fmt.bitsPerSample/8);
}

void WavFile::readChunkHeader(UINT32& chunkId, UINT32& chunkSize)
{
    file.read((char*)&chunkId, sizeof(chunkId));
    chunkId = BYTE_SWAP4(chunkId);
    file.read((char*)&chunkSize, sizeof(chunkSize));
}

void WavFile::readRiffChunkDescriptor()
{
    std::cout << "\"RIFF\" chunk descriptor:\n";

    UINT32 chunkId, chunkSize;
    readChunkHeader(chunkId, chunkSize);

    std::cout << "ChunkID: 0x" << std::hex << chunkId;
    if (chunkId == RIFF)
    {
        std::cout << " (RIFF)\n";
    }
    else
    {
        std::cout << " (Unknown)\n";
        isGood = false;
        return;
    }
    std::cout << "ChunkSize: " << std::dec << chunkSize << '\n';

    UINT32 format;
    file.read((char*)&format, sizeof(format));
    format = BYTE_SWAP4(format);
    std::cout << "Format: 0x" << std::hex << format;
    if (format == WAVE)
    {
        std::cout << " (WAVE)\n";
    }
    else
    {
        std::cout << "(Unknown)\n";
        isGood = false;
        return;
    }
}

void WavFile::readFmtSubChunk()
{
    std::cout << "Reading \"fmt\" Sub-Chunk:\n";
    UINT32 chunkId, chunkSize;
    readChunkHeader(chunkId, chunkSize);
    std::cout << "ChunkId: 0x" << std::hex << chunkId;
    if (chunkId == FMT)
    {
        std::cout << " (FMT)\n";
    }
    else
    {
        std::cout << " (Unknown)\n";
        isGood = false;
        return;
    }
    std::cout << "ChunkSize: " << std::dec << chunkSize << '\n';
    file.read((char*)&fmt, sizeof(fmt));
    std::cout << "AudioFormat: " << fmt.audioFormat << '\n';
    std::cout << "NumChannels: " << fmt.numChannels << '\n';
    std::cout << "SampleRate: " << fmt.sampleRate << '\n';
    std::cout << "ByteRate: " << fmt.byteRate << '\n';
    std::cout << "BlockAlign: " << fmt.blockAlign << '\n';
    std::cout << "BitsPerSample: " << fmt.bitsPerSample << '\n';
}

void WavFile::readDataSubChunk()
{
    std::cout << "Reading \"data\" Sub-Chunk:\n";
    UINT32 chunkId, chunkSize;
    readChunkHeader(chunkId, chunkSize);
    std::cout << "ChunkId: 0x" << std::hex << chunkId;
    if (chunkId == DATA)
    {
        std::cout << " (DATA)\n";
    }
    else
    {
        std::cout << " (Unknown)\n";
        isGood = false;
        return;
    }
    std::cout << "ChunkSize: " << std::dec << chunkSize << '\n';
    data = new char[chunkSize];
    memset(data, 0x3e, chunkSize);
    try
    {
        file.read(data, chunkSize);
    } catch(std::ios_base::failure)
    {
        std::cout << "Error reading data\n";
        isGood = false;
        return;
    }

    dataSize = chunkSize;
    numSamples = dataSize / (fmt.numChannels * (fmt.bitsPerSample/8));
}

void WavFile::printSamples()
{
    std::cout << "NumSamples: " << numSamples << '\n';
    char* p = data;
    for (size_t i=0; i<numSamples; ++i)
    {
        if (fmt.numChannels == 2 && fmt.bitsPerSample == 16)
        {
            INT16 leftChannelSample, rightChannelSample;
            leftChannelSample = *((INT16*)p);
            p += sizeof(INT16);
            rightChannelSample = *((INT16*)p);
            p += sizeof(INT16);
            std::cout << leftChannelSample << ',' << rightChannelSample << ' ';
        }
    }
}

bool WavFile::good()
{
    return isGood;
}

UINT16 WavFile::getAudioFormat()
{
    return fmt.audioFormat;
}

UINT16 WavFile::getNumChannels()
{
    return fmt.numChannels;
}

UINT32 WavFile::getSampleRate()
{
    return fmt.sampleRate;
}

UINT32 WavFile::getByteRate()
{
    return fmt.byteRate;
}

UINT16 WavFile::getBlockAlign()
{
    return fmt.blockAlign;
}

UINT16 WavFile::getBitsPerSample()
{
    return fmt.bitsPerSample;
}

size_t WavFile::getNumSamples()
{
    return numSamples;
}

void WavFile::getSample(unsigned int channel, unsigned int sampleNum, char* sample)
{
    int bytesPerSample = fmt.bitsPerSample >> 3;
    unsigned int offset = sampleNum * fmt.numChannels * bytesPerSample;
    offset += channel * bytesPerSample;
    for (int i=0; i<bytesPerSample; ++i)
    {
        sample[i] = data[offset + i];
    }
}

/*
void WavFile::getSample(unsigned int channel, unsigned int sampleNum, UINT16& sample)
{
    int bytesPerSample = fmt.bitsPerSample >> 2;
    unsigned int offset = sampleNum * fmt.numChannels * bytesPerSample;
    offset += channel * bytesPerSample;
    char *p = data + offset;
    sample = *((UINT16*)p);
}
*/
