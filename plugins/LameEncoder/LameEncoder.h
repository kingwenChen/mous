#ifndef LAMEENCODER_H
#define LAMEENCODER_H

#include <plugin/IEncoder.h>
#include <lame/lame.h>
#include <stdio.h>
#include <string>
using namespace std;
using namespace mous;

class LameEncoder: public IEncoder
{
public:
    LameEncoder();
    virtual ~LameEncoder();

    virtual const char* GetSuffix() const;

    virtual EmErrorCode OpenOutput(const std::string& path);
    virtual void CloseOutput();

    virtual EmErrorCode Encode(char* buf, uint32_t len);
    virtual EmErrorCode FlushRest();

    virtual void SetChannels(int32_t channels);
    virtual void SetSampleRate(int32_t sampleRate);
    virtual void SetBitsPerSample(int32_t bitsPerSample);

    virtual bool GetOptions(std::vector<const BaseOption*>& list) const;

private:
    RangedIntOption m_Quality;
    EnumedIntOption m_BitRate;

    lame_global_flags* m_gfp;
    FILE* m_OutputFile;

    int m_BitsPerSample;

    unsigned char* m_EncodeBuffer;
    int m_EncodeBufferSize;
};

#endif
