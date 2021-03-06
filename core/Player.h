#pragma once

#include <vector>
#include <map>
#include <thread>

#include <core/IPlayer.h>
#include <scx/LPVBuffer.hpp>
#include <scx/Signal.hpp>
#include <scx/SemVar.hpp>

using namespace std;

namespace mous {

class IPluginAgent;
class IDecoder;
class IRenderer;

class Player: public IPlayer
{
public:
    Player();
    ~Player();

public:
    EmPlayerStatus Status() const;

    void RegisterDecoderPlugin(const IPluginAgent* pAgent);
    void RegisterDecoderPlugin(vector<const IPluginAgent*>& agents);

    void RegisterRendererPlugin(const IPluginAgent* pAgent);

    void UnregisterPlugin(const IPluginAgent* pAgent);
    void UnregisterPlugin(vector<const IPluginAgent*>& agents);
    void UnregisterAll();

    std::vector<std::string> SupportedSuffixes() const;

    int BufferCount() const;
    void SetBufferCount(int count);

    int Volume() const;
    void SetVolume(int level);

    EmErrorCode Open(const std::string& path);
    void Close();
    std::string FileName() const;

    void Play();
    void Play(uint64_t msBegin, uint64_t msEnd);
    void Pause();
    void Resume();
    void SeekTime(uint64_t msPos);
    void SeekPercent(double percent);

    void PauseDecoder();
    void ResumeDecoder();

    int32_t BitRate() const;
    int32_t SamleRate() const;
    uint64_t Duration() const;
    uint64_t RangeBegin() const;
    uint64_t RangeEnd() const;
    uint64_t RangeDuration() const;
    uint64_t OffsetMs() const;
    uint64_t CurrentMs() const;
    EmAudioMode AudioMode() const;

    std::vector<PluginOption> DecoderPluginOption() const;
    PluginOption RendererPluginOption() const;

public:
    scx::Signal<void (void)>* SigFinished();

private:
    void AddDecoderPlugin(const IPluginAgent* pAgent);
    void RemoveDecoderPlugin(const IPluginAgent* pAgent);

    void SetRendererPlugin(const IPluginAgent* pAgent);
    void UnsetRendererPlugin(const IPluginAgent* pAgent);

    void AddEventListener(const IPluginAgent* pAgent);
    void RemoveEventListener(const IPluginAgent* pAgent);

    void PlayRange(uint64_t beg, uint64_t end);
    inline void DoSeekTime(uint64_t msPos);
    inline void DoSeekUnit(uint64_t unit);

    void ThDecoder();
    void ThRenderer();

private:
    struct UnitBuffer
    {
        char* data;
        uint32_t used;
        uint32_t max;

        uint32_t unitCount;

        UnitBuffer(): 
            data(NULL),
            used(0),
            max(0),
            unitCount(0)
        {
        }

        ~UnitBuffer()
        {
            if (data != NULL)
                delete[] data;
            data = NULL;
            used = 0;
            max = 0;
            unitCount = 0;
        }
    };

    struct DecoderPluginNode
    {
        const IPluginAgent* agent;
        IDecoder* decoder;
    };

private:
    EmPlayerStatus m_Status = PlayerStatus::Closed;

    string m_DecodeFile;
    bool m_StopDecoder = false;
    bool m_SuspendDecoder = true;
    bool m_PauseDecoder = false;
    IDecoder* m_Decoder = nullptr;
    std::thread m_ThreadForDecoder;
    scx::SemVar m_SemWakeDecoder;
    scx::SemVar m_SemDecoderBegin;
    scx::SemVar m_SemDecoderEnd;

    bool m_StopRenderer = false;
    bool m_SuspendRenderer = true;
    IRenderer* m_Renderer = nullptr;
    std::thread m_ThreadForRenderer;
    scx::SemVar m_SemWakeRenderer;
    scx::SemVar m_SemRendererBegin;
    scx::SemVar m_SemRendererEnd;

    scx::LPVBuffer<UnitBuffer> m_UnitBuffers;

    uint64_t m_UnitBeg = 0;
    uint64_t m_UnitEnd = 0;

    uint64_t m_DecoderIndex = 0;
    uint64_t m_RendererIndex = 0;

    double m_UnitPerMs = 0;

    const IPluginAgent* m_RendererPlugin = nullptr;

    std::map<std::string, DecoderPluginNode> m_DecoderPluginMap;

    scx::Signal<void (void)> m_SigFinished;
};

}
