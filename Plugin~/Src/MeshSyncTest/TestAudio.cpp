#include "pch.h"
#include "Test.h"
#include "MeshSync/SceneGraph/msAudio.h"
#include "MeshSync/SceneGraph/msScene.h"
#include "Utility/TestUtility.h"

using namespace mu;

static const int Frequency = 48000;
static const int Channels = 1;

template<class T >
static void GenerateAudioSample(T *dst, const int n) {
    for (int i = 0; i < n; ++i) {
        const float s = std::pow(static_cast<float>(n - i) / n, 0.5f);
        dst[i] = std::sin((static_cast<float>(i) * 1.5f * mu::DegToRad)) * s;
    }
}

static ms::AudioPtr CreateAudioAsset(const char *name, ms::AudioFormat fmt, int id) {
    std::shared_ptr<ms::Audio> a = ms::Audio::create();
    a->id = id;
    a->name = name;
    a->format = fmt;
    a->frequency = Frequency;
    a->channels = Channels;

    void* samples = a->allocate(Frequency / 2); // 0.5 sec
    const int numSampleChannels = static_cast<int>(a->getSampleLength() * Channels);
    switch (fmt) {
    case ms::AudioFormat::U8:
        GenerateAudioSample(static_cast<unorm8n*>(samples), numSampleChannels);
        break;
    case ms::AudioFormat::S16:
        GenerateAudioSample(static_cast<snorm16*>(samples), numSampleChannels);
        break;
    case ms::AudioFormat::S24:
        GenerateAudioSample(static_cast<snorm24*>(samples), numSampleChannels);
        break;
    case ms::AudioFormat::S32:
        GenerateAudioSample(static_cast<snorm32*>(samples), numSampleChannels);
        break;
    case ms::AudioFormat::F32:
        GenerateAudioSample(static_cast<float*>(samples), numSampleChannels);
        break;
    default:
        break;
    }

    std::string filename = name;
    filename += ".wav";
    a->exportAsWave(filename.c_str());
    return a;
}

static ms::AudioPtr CreateAudioFileAsset(const char *path, const int id)
{
    std::shared_ptr<ms::Audio> a = ms::Audio::create();
    a->id = id;
    if (a->readFromFile(path))
        return a;
    return nullptr;
}

TestCase(Test_Audio)
{
    int ids = 0;
    std::shared_ptr<ms::Scene> scene = ms::Scene::create();
    scene->assets.push_back(CreateAudioAsset("audio_u8", ms::AudioFormat::U8, ids++));
    scene->assets.push_back(CreateAudioAsset("audio_s16", ms::AudioFormat::S16, ids++));
    scene->assets.push_back(CreateAudioAsset("audio_s24", ms::AudioFormat::S24, ids++));
    scene->assets.push_back(CreateAudioAsset("audio_s32", ms::AudioFormat::S32, ids++));
    scene->assets.push_back(CreateAudioAsset("audio_f32", ms::AudioFormat::F32, ids++));
    if (const ms::AudioPtr afa = CreateAudioFileAsset("explosion1.wav", ids++))
        scene->assets.push_back(afa);
    TestUtility::Send(scene);
}

