#include "AudioBip.h"

// AudioEffect
#include <nn/nn_Assert.h>

#include <cmath>
#include <limits>
#include <algorithm>

#include <nns/nns_Log.h>

#include <nn/mem.h>

#include <nn/audio.h>
#include <nns/audio/audio_HidUtilities.h>
#include <nns/audio/audio_WavFormat.h>
#include <nn/hid/hid_KeyboardKey.h>

#include <nn/settings/settings_DebugPad.h>


namespace AB
{
    ///////////////////////////////////////////////
    // AudioEffect
    ///////////////////////////////////////////////

    char g_HeapBuffer[256 * 1024];
    const char Title[] = "AudioOut";

    void InitializeHidDevices()
    {
        //Map keyboard keys to DebugPad buttons.
        nn::settings::DebugPadKeyboardMap map;
        nn::settings::GetDebugPadKeyboardMap(&map);
        map.buttonA = nn::hid::KeyboardKey::A::Index;
        map.buttonB = nn::hid::KeyboardKey::B::Index;
        map.buttonX = nn::hid::KeyboardKey::X::Index;
        map.buttonY = nn::hid::KeyboardKey::Y::Index;
        map.buttonL = nn::hid::KeyboardKey::L::Index;
        map.buttonR = nn::hid::KeyboardKey::R::Index;
        map.buttonZL = nn::hid::KeyboardKey::U::Index;
        map.buttonZR = nn::hid::KeyboardKey::V::Index;
        map.buttonLeft = nn::hid::KeyboardKey::LeftArrow::Index;
        map.buttonRight = nn::hid::KeyboardKey::RightArrow::Index;
        map.buttonUp = nn::hid::KeyboardKey::UpArrow::Index;
        map.buttonDown = nn::hid::KeyboardKey::DownArrow::Index;
        map.buttonStart = nn::hid::KeyboardKey::Space::Index;
        nn::settings::SetDebugPadKeyboardMap(map);
    }

    //
    // This function returns the state name.
    //
    const char* GetAudioOutStateName(nn::audio::AudioOutState state)
    {
        switch (state)
        {
        case nn::audio::AudioOutState_Started:
            return "Started";
        case nn::audio::AudioOutState_Stopped:
            return "Stopped";
        default:
            NN_UNEXPECTED_DEFAULT;
        }
    }

    //
    // This function returns the sample format name.
    //
    const char* GetSampleFormatName(nn::audio::SampleFormat format)
    {
        switch (format)
        {
        case nn::audio::SampleFormat_Invalid:
            return "Invalid";
        case nn::audio::SampleFormat_PcmInt8:
            return "PcmInt8";
        case nn::audio::SampleFormat_PcmInt16:
            return "PcmInt16";
        case nn::audio::SampleFormat_PcmInt24:
            return "PcmInt24";
        case nn::audio::SampleFormat_PcmInt32:
            return "PcmInt32";
        case nn::audio::SampleFormat_PcmFloat:
            return "PcmFloat";
        default:
            NN_UNEXPECTED_DEFAULT;
        }
    }

    //
    // The square waveform generating function that supports nn::audio::SampleFormat_PcmInt8. Not yet implemented.
    //
    void GenerateSquareWaveInt8(void* buffer, int channelCount, int sampleRate, int sampleCount, int amplitude)
    {
        NN_UNUSED(buffer);
        NN_UNUSED(channelCount);
        NN_UNUSED(sampleRate);
        NN_UNUSED(sampleCount);
        NN_UNUSED(amplitude);
        NN_ABORT("Not implemented yet\n");
    }

    //
    // The square waveform generating function that supports nn::audio::SampleFormat_PcmInt16.
    //
    void GenerateSquareWaveInt16(void* buffer, int channelCount, int sampleRate, int sampleCount, int amplitude)
    {
        static int s_TotalSampleCount[6] = { 0 };
        const int frequencies[6] = { 415, 698, 554, 104, 349, 277 };

        int16_t* buf = reinterpret_cast<int16_t*>(buffer);
        for (int ch = 0; ch < channelCount; ch++)
        {
            int waveLength = sampleRate / frequencies[ch]; // Length of the waveform for one period (in sample count units).

            for (int sample = 0; sample < sampleCount; sample++)
            {
                int16_t value = static_cast<int16_t>(s_TotalSampleCount[ch] < (waveLength / 2) ? amplitude : -amplitude);
                buf[sample * channelCount + ch] = value;
                s_TotalSampleCount[ch]++;
                if (s_TotalSampleCount[ch] == waveLength)
                {
                    s_TotalSampleCount[ch] = 0;
                }
            }
        }
    }

    //
    // The square waveform generating function that supports nn::audio::SampleFormat_PcmInt24. Not yet implemented.
    //
    void GenerateSquareWaveInt24(void* buffer, int channelCount, int sampleRate, int sampleCount, int amplitude)
    {
        NN_UNUSED(buffer);
        NN_UNUSED(channelCount);
        NN_UNUSED(sampleRate);
        NN_UNUSED(sampleCount);
        NN_UNUSED(amplitude);
        NN_ABORT("Not implemented yet\n");
    }

    //
    // The square waveform generating function that supports nn::audio::SampleFormat_PcmInt32. Not yet implemented.
    //
    void GenerateSquareWaveInt32(void* buffer, int channelCount, int sampleRate, int sampleCount, int amplitude)
    {
        NN_UNUSED(buffer);
        NN_UNUSED(channelCount);
        NN_UNUSED(sampleRate);
        NN_UNUSED(sampleCount);
        NN_UNUSED(amplitude);
        NN_ABORT("Not implemented yet\n");
    }

    //
    // The square waveform generating function that supports nn::audio::SampleFormat_PcmFloat. Not yet implemented.
    //
    void GenerateSquareWaveFloat(void* buffer, int channelCount, int sampleRate, int sampleCount, int amplitude)
    {
        NN_UNUSED(buffer);
        NN_UNUSED(channelCount);
        NN_UNUSED(sampleRate);
        NN_UNUSED(sampleCount);
        NN_UNUSED(amplitude);
        NN_ABORT("Not implemented yet\n");
    }

    //
    // Returns the square waveform generating function supported by the sample format.
    //
    typedef void (*GenerateSquareWaveFunction)(void* buffer, int channelCount, int sampleRate, int sampleCount, int amplitude);
    GenerateSquareWaveFunction GetGenerateSquareWaveFunction(nn::audio::SampleFormat format)
    {
        switch (format)
        {
        case nn::audio::SampleFormat_PcmInt8:
            return GenerateSquareWaveInt8;
        case nn::audio::SampleFormat_PcmInt16:
            return GenerateSquareWaveInt16;
        case nn::audio::SampleFormat_PcmInt24:
            return GenerateSquareWaveInt24;
        case nn::audio::SampleFormat_PcmInt32:
            return GenerateSquareWaveInt32;
        case nn::audio::SampleFormat_PcmFloat:
            return GenerateSquareWaveFloat;
        default:
            NN_UNEXPECTED_DEFAULT;
        }
    }

    //
    // Function to create a square waveform.
    //
    void GenerateSquareWave(nn::audio::SampleFormat format, void* buffer, int channelCount, int sampleRate, int sampleCount, int amplitude)
    {
        NN_ASSERT_NOT_NULL(buffer);
        GenerateSquareWaveFunction func = GetGenerateSquareWaveFunction(format);
        if (func)
        {
            func(buffer, channelCount, sampleRate, sampleCount, amplitude);
        }
    }

    void* Allocate(size_t size)
    {
        return std::malloc(size);
    }

    void Deallocate(void* p, size_t size)
    {
        NN_UNUSED(size);
        std::free(p);
    }


    // Audio end
}