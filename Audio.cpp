/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

 /**
 * @examplesource{AudioDevice.cpp,PageSampleAudioAudioDevice}
 *
 * @brief
 *  Handling of Various Settings for Audio Output Devices Sample
 */

 /**
 * @page PageSampleAudioAudioDevice  Handling of Various Settings for Audio Output Devices Sample
 * @tableofcontents
 *
 * @brief
 *  This sample program describes the settings unique to audio output devices and the settings for application-specific downmixing.
 *
 * @section PageSampleAudioAudioDevice_SectionBrief  Overview
 *  The purpose of this sample program is to describe how to use the following features.
 *  #1 Getting the device to which audio is output, the number of output channels, and the time when output switches to another device.
 *  #2 Setting the volume for each audio output device.
 *  #3 Setting the application-specific downmixing feature.
 *
 * @section PageSampleAudioAudioDevice_SectionFileStructure  File Structure
 *  This sample program is located in
 *  <tt>@link Samples/Sources/Applications/AudioDevice Samples/Sources/Applications/AudioDevice @endlink</tt>.
 *
 * @section PageSampleAudioAudioDevice_SectionNecessaryEnvironment  Required Environment
 *  You must be able to use audio output.
 *
 * @section PageSampleAudioAudioDevice_SectionHowToOperate  How to Use
 *  After this sample program runs, BGM starts playing. You can also use the buttons to play sound effects.
 *  <p>
 *  <table>
 *  <tr><th> Input </th><th> Operation </th></tr>
 *  <tr><td> A Button             </td><td> Play a sound effect.                        </td></tr>
 *  <tr><td> [B]             </td><td> Play a sound effect.                        </td></tr>
 *  <tr><td> [X]             </td><td> Play a sound effect.                        </td></tr>
 *  <tr><td> [Y]             </td><td> Play a sound effect.                        </td></tr>
 *  <tr><td> [L]             </td><td> Play a sound effect.                        </td></tr>
 *  <tr><td> [Left/Right]    </td><td> Select the device to set the volume for.       </td></tr>
 *  <tr><td> [Up/Down]       </td><td> Adjust the volume of the selected device.      </td></tr>
 *  <tr><td> [Select/Minus]  </td><td> View the sample description.                   </td></tr>
 *  <tr><td> START or SPACEBAR   </td><td> Exits the program.                      </td></tr>
 *  </table>
 *  </p>
 *
 * @section PageSampleAudioAudioDevice_SectionPrecaution  Important Information
 *  This sample assumes that you already know how to use basic audio renderers.
 *  For a detailed example of using the audio renderer,
 *  see <tt>@link Samples/Sources/Applications/AudioRenderer Samples/Sources/Applications/AudioRenderer @endlink</tt>.
 *
 * @section PageSampleAudioAudioDevice_SectionHowToExecute  Execution Procedure
 *  Build the sample program and then run it.
 *
 * @section PageSampleAudioAudioDevice_SectionDetail  Description
 *  Descriptions of the following features are presented in the sample code.
 *
 *  #1 Getting the device to which audio is output, the number of output channels, and the time when output switches to another device.
 *  Use the <tt>nn::audio::GetActiveAudioDeviceName()</tt> function to get the name of the device to which audio is currently being output.
 *  Use <tt>nn::audio::GetActiveAudioDeviceChannelCountForOutput()</tt> to get the current number of output channels.
 *  The output device and number of output channels changes dynamically depending on the operating state of the platform, but you can get notification when
 *  the device is switched via the system event obtained by the <tt>nn::audio::AcquireAudioDeviceNotificationForOutput()</tt> function.
 *  Comments relating to the relevant code are provided inside the code as comments #1-1 and #1-2.
 *
 *  #2 Setting the volume for each audio output device.
 *  You can set an output volume separately for each audio output device.
 *  Use the <tt>nn::audio::SetAudioDeviceOutputVolume()</tt> function to set the volume.
 *  Comments relating to the relevant code are provided inside the code as comments #2-1 and #2-2.
 *
 *  #3 Setting the application-specific downmixing feature.
 *  There are two downmix processes: system default downmixing, and application-specific downmixing.
 *  The latter process can be used only in the audio renderer, and a description is provided in this sample program.
 *  For a comprehensive description of both downmix processes, see the Downmix Processing section of @confluencelink{166500119, Audio Library.}
 *  To use application-specific downmixing, first set the parameter using <tt>nn::audio::SetDownMixParameter() </tt> and then
 *  enable it using <tt>nn::audio::SetDownMixParameterEnabled()</tt>.
 *  Comments relating to the relevant code are provided inside the code as comments #3-1 and #3-2.
 */

#include <cmath>
#include <new>

#include <nns/nns_Log.h>
#include <nn/nn_Abort.h>
#include <nn/os.h>
#include <nn/mem.h>
#include <nn/fs.h>
#include <nn/nn_TimeSpan.h>

#include <nn/audio.h>
#include <nns/audio/audio_HidUtilities.h>
#include <nns/audio/audio_WavFormat.h>

#include <nn/hid.h>
#include <nn/hid/hid_KeyboardKey.h>
#include <nn/hid/hid_Npad.h>

#include <nn/settings/settings_DebugPad.h>

namespace AUS {
    namespace {

        // Select the rendering engine sample rate.
        //const int RenderRate = 48000;
        const int RenderRate = 32000;
        const int RenderCount = (RenderRate / 200);

        // Select the number of files to play.
        const int BgmCount = 1;
        const int SeCount = 4;

        const char Title[] = "AudioDevice";

        // - Add or remove these files from the files lists.
        const char* g_BgmFileNames[BgmCount] =
        {
            "asset:/AudioDevice/SampleBgm0-2ch.wav",
        };

        const char* g_SeFileNames[SeCount] =
        {
            "asset:/AudioCommon/SampleSe0.adpcm",
            "asset:/AudioCommon/SampleSe1.adpcm",
            "asset:/AudioCommon/SampleSe2.adpcm",
            "asset:/AudioCommon/SampleSe3.adpcm",
        };

        NN_ALIGNAS(4096) char g_WorkBuffer[8 * 1024 * 1024];
        NN_AUDIO_ALIGNAS_MEMORY_POOL_ALIGN char g_WaveBufferPoolMemory[14 * 1024 * 1024];
        NN_AUDIO_ALIGNAS_MEMORY_POOL_ALIGN char g_EffectBufferPoolMemory[12 * 1024 * 1024];

        nn::mem::StandardAllocator g_Allocator;
        nn::mem::StandardAllocator g_WaveBufferAllocator;
        nn::mem::StandardAllocator g_EffectBufferAllocator;

        char* g_MountRomCacheBuffer = NULL;

    }

    void* Allocate(size_t size)
    {
        return g_Allocator.Allocate(size);
    }

    void Deallocate(void* p, size_t size)
    {
        NN_UNUSED(size);
        g_Allocator.Free(p);
    }

    std::size_t GenerateSineWave(void** data, int sampleRate, int frequency, int sampleCount)
    {
        // The entire memory region managed with g_WaveBufferAllocator is added to the memory pool, waveBufferMemoryPool.
        int16_t* p = static_cast<int16_t*>(g_WaveBufferAllocator.Allocate(sampleCount * sizeof(int16_t), nn::audio::BufferAlignSize));
        NN_ABORT_UNLESS_NOT_NULL(p);
        const float Pi = 3.1415926535897932384626433f;
        for (auto i = 0; i < sampleCount; ++i)
        {
            p[i] = static_cast<int16_t>(std::numeric_limits<int16_t>::max() * sinf(2 * Pi * frequency * i / sampleRate));
        }
        *data = p;
        return sampleCount * sizeof(int16_t);
    }

    std::size_t ReadAdpcmFile(nn::audio::AdpcmHeaderInfo* header, void** adpcmData, const char* filename)
    {
        nn::fs::FileHandle handle;
        nn::Result result = nn::fs::OpenFile(&handle, filename, nn::fs::OpenMode_Read);
        NN_ABORT_UNLESS(result.IsSuccess());

        int64_t size;
        uint8_t adpcmheader[nn::audio::AdpcmHeaderSize];

        result = nn::fs::GetFileSize(&size, handle);
        NN_ABORT_UNLESS(result.IsSuccess());

        *adpcmData = g_WaveBufferAllocator.Allocate(static_cast<std::size_t>(size) - sizeof(adpcmheader), nn::audio::BufferAlignSize);
        NN_ABORT_UNLESS_NOT_NULL(*adpcmData);

        result = nn::fs::ReadFile(handle, 0, adpcmheader, sizeof(adpcmheader));
        NN_ABORT_UNLESS(result.IsSuccess());
        result = nn::fs::ReadFile(handle, sizeof(adpcmheader), *adpcmData, static_cast<size_t>(size) - sizeof(adpcmheader));
        NN_ABORT_UNLESS(result.IsSuccess());
        nn::fs::CloseFile(handle);

        nn::audio::ParseAdpcmHeader(header, adpcmheader, sizeof(adpcmheader));

        return static_cast<std::size_t>(size) - sizeof(adpcmheader);
    }

    std::size_t ReadWavFile(nns::audio::WavFormat* format, void** data, const char* filename)
    {
        nn::fs::FileHandle handle;
        nn::Result result = nn::fs::OpenFile(&handle, filename, nn::fs::OpenMode_Read);
        NN_ABORT_UNLESS(result.IsSuccess());

        int64_t size;

        result = nn::fs::GetFileSize(&size, handle);
        NN_ABORT_UNLESS(result.IsSuccess());

        *data = g_WaveBufferAllocator.Allocate(static_cast<std::size_t>(size), nn::audio::BufferAlignSize);
        NN_ABORT_UNLESS_NOT_NULL(*data);

        // The DATA chunk must be read, but this sample assumes that it is found to be 1024 bits or less.
        const std::size_t WavHeaderDataSize = 1024;

        result = nn::fs::ReadFile(handle, 0, *data, WavHeaderDataSize);
        NN_ABORT_UNLESS(result.IsSuccess());

        nns::audio::WavResult wavResult = nns::audio::ParseWavFormat(format, *data, WavHeaderDataSize);
        NN_ABORT_UNLESS_EQUAL(wavResult, nns::audio::WavResult_Success);
        NN_ABORT_UNLESS_EQUAL(format->bitsPerSample, 16);  // This sample assumes 16-bit PCM.

        result = nn::fs::ReadFile(handle, static_cast<std::size_t>(format->dataOffset), *data, static_cast<std::size_t>(format->dataSize));
        NN_ABORT_UNLESS(result.IsSuccess());
        nn::fs::CloseFile(handle);

        return static_cast<std::size_t>(format->dataSize);
    }

    void InitializeFileSystem()
    {
        nn::fs::SetAllocator(Allocate, Deallocate);

        size_t cacheSize = 0;
        NN_ABORT_UNLESS_RESULT_SUCCESS(nn::fs::QueryMountRomCacheSize(&cacheSize));
        g_MountRomCacheBuffer = new(std::nothrow) char[cacheSize];
        NN_ABORT_UNLESS_NOT_NULL(g_MountRomCacheBuffer);

        NN_ABORT_UNLESS_RESULT_SUCCESS(
            nn::fs::MountRom("asset", g_MountRomCacheBuffer, cacheSize)
        );
    }

    void FinalizeFileSystem()
    {
        nn::fs::Unmount("asset");

        delete[] g_MountRomCacheBuffer;
        g_MountRomCacheBuffer = NULL;
    }

    void InitializeHidDevices()
    {
        nn::hid::InitializeDebugPad();
        nn::hid::InitializeNpad();
        const nn::hid::NpadIdType npadIds[2] = { nn::hid::NpadId::No1, nn::hid::NpadId::Handheld };
        nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleHandheld::Mask);
        nn::hid::SetSupportedNpadIdType(npadIds, sizeof(npadIds) / sizeof(npadIds[0]));

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
        map.buttonSelect = nn::hid::KeyboardKey::Minus::Index;
        nn::settings::SetDebugPadKeyboardMap(map);
    }


    nn::hid::NpadButtonSet GetButtonDown()
    {
        nn::hid::NpadButtonSet npadButtonDown = {};

        // Get the Npad input.
        if (nn::hid::GetNpadStyleSet(nn::hid::NpadId::No1).Test<nn::hid::NpadStyleFullKey>())
        {
            static nn::hid::NpadFullKeyState npadFullKeyState = {};
            nn::hid::NpadFullKeyState state;
            nn::hid::GetNpadState(&state, nn::hid::NpadId::No1);
            npadButtonDown |= state.buttons & ~npadFullKeyState.buttons;
            npadFullKeyState = state;
        }
        if (nn::hid::GetNpadStyleSet(nn::hid::NpadId::Handheld).Test<nn::hid::NpadStyleHandheld>())
        {
            static nn::hid::NpadHandheldState npadHandheldState = {};
            nn::hid::NpadHandheldState state;
            nn::hid::GetNpadState(&state, nn::hid::NpadId::Handheld);
            npadButtonDown |= state.buttons & ~npadHandheldState.buttons;
            npadHandheldState = state;
        }

        // Get the input from the DebugPad.
        {
            static nn::hid::DebugPadState debugPadState = {};
            nn::hid::DebugPadButtonSet debugPadButtonDown = {};
            nn::hid::DebugPadState state;
            nn::hid::GetDebugPadState(&state);
            debugPadButtonDown |= state.buttons & ~debugPadState.buttons;
            debugPadState = state;
            nns::audio::ConvertDebugPadButtonsToNpadButtons(&npadButtonDown, debugPadButtonDown);
        }

        return npadButtonDown;
    }

    void PrintUsage()
    {
        NNS_LOG("--------------------------------------------------------\n");
        NNS_LOG("%s Sample\n", Title);
        NNS_LOG("--------------------------------------------------------\n");
        NNS_LOG("[A]             StartSound                   (SampleSe0)\n");
        NNS_LOG("[B]             StartSound                   (SampleSe1)\n");
        NNS_LOG("[X]             StartSound                   (SampleSe2)\n");
        NNS_LOG("[Y]             StartSound                   (SampleSe3)\n");
        NNS_LOG("[L]             Toggle Application Specific DownMix     \n");
        NNS_LOG("[Left][Right]   Select Device                           \n");
        NNS_LOG("[Up][Down]      Control Selected Device Volume          \n");
        NNS_LOG("[Select/-]      Print Usage                             \n");
        NNS_LOG("[Start/Space]   Shut down sample program                \n");
        NNS_LOG("--------------------------------------------------------\n");
    }

    void SetDefaultParameter(nn::audio::AudioRendererParameter& parameter)
    {
        nn::audio::InitializeAudioRendererParameter(&parameter);
        parameter.sampleRate = RenderRate;
        parameter.sampleCount = RenderCount;
        parameter.mixBufferCount = 6 + 2; // FinalMix(6) + SubMix(2)
        parameter.voiceCount = 24;
        parameter.subMixCount = 2;
        parameter.sinkCount = 1;
        parameter.effectCount = 3;
        parameter.performanceFrameCount = 0;
    }

    void SetupBgmVoices(
        nn::audio::VoiceType* pOutVoices,
        nn::audio::WaveBuffer* pOutWaveBuffers,
        void** ppDataBuffer,
        nn::audio::AudioRendererConfig& config)
    {
        for (int i = 0; i < BgmCount; ++i)
        {
            nns::audio::WavFormat format;
            std::size_t dataSize = ReadWavFile(&format, &ppDataBuffer[i], g_BgmFileNames[i]);
            nn::audio::AcquireVoiceSlot(&config, &pOutVoices[i], format.sampleRate, format.channelCount, nn::audio::SampleFormat_PcmInt16, nn::audio::VoiceType::PriorityHighest, nullptr, 0);
            nn::audio::SetVoicePlayState(&pOutVoices[i], nn::audio::VoiceType::PlayState_Play);

            pOutWaveBuffers[i].buffer = ppDataBuffer[i];
            pOutWaveBuffers[i].size = dataSize;
            pOutWaveBuffers[i].startSampleOffset = 0;
            pOutWaveBuffers[i].endSampleOffset = static_cast<int32_t>(dataSize / sizeof(int16_t)) / format.channelCount;
            pOutWaveBuffers[i].loop = true;
            pOutWaveBuffers[i].isEndOfStream = false;
            pOutWaveBuffers[i].pContext = nullptr;
            pOutWaveBuffers[i].contextSize = 0;
            nn::audio::AppendWaveBuffer(&pOutVoices[i], &pOutWaveBuffers[i]);
        }
    }

    void SetupSeVoices(
        nn::audio::VoiceType* pOutVoices,
        nn::audio::AdpcmHeaderInfo** pOutHeaders,
        nn::audio::WaveBuffer* pOutWaveBuffers,
        void** pOutDataBuffer,
        nn::audio::AudioRendererConfig config)
    {
        for (int i = 0; i < SeCount; ++i)
        {
            pOutHeaders[i] = reinterpret_cast<nn::audio::AdpcmHeaderInfo*>(g_WaveBufferAllocator.Allocate(sizeof(nn::audio::AdpcmHeaderInfo), NN_ALIGNOF(nn::audio::AdpcmHeaderInfo)));
            std::size_t dataSeSize = ReadAdpcmFile(pOutHeaders[i], &pOutDataBuffer[i], g_SeFileNames[i]);
            nn::audio::AcquireVoiceSlot(&config, &pOutVoices[i], pOutHeaders[i]->sampleRate, 1, nn::audio::SampleFormat_Adpcm, nn::audio::VoiceType::PriorityHighest, &pOutHeaders[i]->parameter, sizeof(nn::audio::AdpcmParameter));
            nn::audio::SetVoicePlayState(&pOutVoices[i], nn::audio::VoiceType::PlayState_Play);

            pOutWaveBuffers[i].buffer = pOutDataBuffer[i];
            pOutWaveBuffers[i].size = dataSeSize;
            pOutWaveBuffers[i].startSampleOffset = 0;
            pOutWaveBuffers[i].endSampleOffset = pOutHeaders[i]->sampleCount;
            pOutWaveBuffers[i].loop = false;
            pOutWaveBuffers[i].isEndOfStream = false;
            pOutWaveBuffers[i].pContext = &pOutHeaders[i]->loopContext;
            pOutWaveBuffers[i].contextSize = sizeof(nn::audio::AdpcmContext);
            nn::audio::AppendWaveBuffer(&pOutVoices[i], &pOutWaveBuffers[i]);
        }
    }

    int FindDevice(nn::audio::AudioDeviceName* deviceNames, int deviceNameCount, const char* targetName)
    {
        int index = -1;
        for (auto i = 0; i < deviceNameCount; ++i)
        {
            if (strncmp(deviceNames[i].name, targetName, nn::audio::AudioDeviceName::NameLength) == 0)
            {
                index = i;
                break;
            }
        }
        NN_ABORT_UNLESS(index > 0);
        return index;
    }


    void UpdateDeviceIndexPamrameter(int* outDeviceIndex, nn::hid::NpadButtonSet& buttonDown, int listedDeviceCount)
    {
        int index = *outDeviceIndex;

        if (buttonDown.Test< ::nn::hid::NpadButton::Left >())
        {
            --index;
            if (index < 0)
            {
                index = listedDeviceCount - 1;
            }
        }
        else if (buttonDown.Test< ::nn::hid::NpadButton::Right >())
        {
            ++index;
            if (index >= listedDeviceCount)
            {
                index = 0;
            }
        }

        *outDeviceIndex = index;
    }

    void UpdateDeviceVolumeParameter(float* outVolume, nn::hid::NpadButtonSet& buttonDown)
    {
        float vol = *outVolume;

        if (buttonDown.Test< ::nn::hid::NpadButton::Up>())
        {
            vol += 0.1f;
            if (vol >= 1.0f)
            {
                vol = 1.0f;
            }
        }
        else if (buttonDown.Test< ::nn::hid::NpadButton::Down>())
        {
            vol -= 0.1f;
            if (vol <= 0.0f)
            {
                vol = 0.0f;
            }
        }

        *outVolume = vol;
    }


    extern "C" void nnMain()
    {
        g_Allocator.Initialize(g_WorkBuffer, sizeof(g_WorkBuffer));
        g_WaveBufferAllocator.Initialize(g_WaveBufferPoolMemory, sizeof(g_WaveBufferPoolMemory));
        g_EffectBufferAllocator.Initialize(g_EffectBufferPoolMemory, sizeof(g_EffectBufferPoolMemory));

        InitializeFileSystem();
        InitializeHidDevices();

        //////////////////////////////////////////////////////////////////////////
        // Configures the basic settings for the audio renderer. For a description and detailed usage example
        // of the audio renderer, see
        // @link Samples/Sources/Applications/AudioRenderer Samples/Sources/Applications/AudioRenderer @endlink.
        //////////////////////////////////////////////////////////////////////////
        nn::audio::AudioRendererParameter parameter;
        SetDefaultParameter(parameter);
        NN_ABORT_UNLESS(
            nn::audio::IsValidAudioRendererParameter(parameter),
            "Invalid AudioRendererParameter specified."
        );

        const int channelCount = 6;
        int8_t mainBus[channelCount];
        mainBus[nn::audio::ChannelMapping_FrontLeft] = 0;
        mainBus[nn::audio::ChannelMapping_FrontRight] = 1;
        mainBus[nn::audio::ChannelMapping_FrontCenter] = 2;
        mainBus[nn::audio::ChannelMapping_LowFrequency] = 3;
        mainBus[nn::audio::ChannelMapping_RearLeft] = 4;
        mainBus[nn::audio::ChannelMapping_RearRight] = 5;

        size_t workBufferSize = nn::audio::GetAudioRendererWorkBufferSize(parameter);
        void* workBuffer = g_Allocator.Allocate(workBufferSize, nn::os::MemoryPageSize);
        NN_ABORT_UNLESS_NOT_NULL(workBuffer);

        nn::audio::AudioRendererHandle handle;
        nn::os::SystemEvent systemEvent;
        NN_ABORT_UNLESS(
            nn::audio::OpenAudioRenderer(&handle, &systemEvent, parameter, workBuffer, workBufferSize).IsSuccess(),
            "Failed to open AudioRenderer"
        );

        size_t configBufferSize = nn::audio::GetAudioRendererConfigWorkBufferSize(parameter);
        void* configBuffer = g_Allocator.Allocate(configBufferSize, nn::os::MemoryPageSize);
        NN_ABORT_UNLESS_NOT_NULL(configBuffer);
        nn::audio::AudioRendererConfig config;
        nn::audio::InitializeAudioRendererConfig(&config, parameter, configBuffer, configBufferSize);

        nn::audio::FinalMixType finalMix;
        nn::audio::AcquireFinalMix(&config, &finalMix, 6);
        nn::audio::DeviceSinkType deviceSink;
        NN_ABORT_UNLESS(nn::audio::AddDeviceSink(&config, &deviceSink, &finalMix, mainBus, 6, "MainAudioOut").IsSuccess());
        NN_ABORT_UNLESS(nn::audio::RequestUpdateAudioRenderer(handle, &config).IsSuccess());
        NN_ABORT_UNLESS(nn::audio::StartAudioRenderer(handle).IsSuccess());

        nn::audio::MemoryPoolType waveBufferMemoryPool;
        NN_ABORT_UNLESS(nn::audio::AcquireMemoryPool(&config, &waveBufferMemoryPool, g_WaveBufferPoolMemory, sizeof(g_WaveBufferPoolMemory)));
        NN_ABORT_UNLESS(nn::audio::RequestAttachMemoryPool(&waveBufferMemoryPool));

        nn::audio::VoiceType voiceBgm[BgmCount];
        nn::audio::WaveBuffer waveBufferBgm[BgmCount];
        void* dataBgm[BgmCount];
        SetupBgmVoices(voiceBgm, waveBufferBgm, dataBgm, config);

        // Specifies finalMix as the output destination for BGM, and Left and Right as the output channels.
        for (auto i = 0; i < BgmCount; ++i)
        {
            nn::audio::SetVoiceDestination(&config, &voiceBgm[i], &finalMix);
            nn::audio::SetVoiceMixVolume(&voiceBgm[i], &finalMix, 0.5f, 0, mainBus[nn::audio::ChannelMapping_FrontLeft]);
            nn::audio::SetVoiceMixVolume(&voiceBgm[i], &finalMix, 0.5f, 1, mainBus[nn::audio::ChannelMapping_FrontRight]);
        }

        nn::audio::VoiceType voiceSe[SeCount];
        nn::audio::WaveBuffer waveBufferSe[SeCount];
        nn::audio::AdpcmHeaderInfo* header[SeCount];
        void* dataSe[SeCount];
        SetupSeVoices(voiceSe, header, waveBufferSe, dataSe, config);

        // Specifies finalMix as the output destination for the sound effects, and distributes the separate sound effects to the output channels Center, Lfe, RearLeft, and RearRight.
        for (auto i = 0; i < SeCount; ++i)
        {
            nn::audio::SetVoiceDestination(&config, &voiceSe[i], &finalMix);
        }
        nn::audio::SetVoiceMixVolume(&voiceSe[0], &finalMix, 0.707f, 0, mainBus[nn::audio::ChannelMapping_FrontCenter]);
        nn::audio::SetVoiceMixVolume(&voiceSe[1], &finalMix, 0.707f, 0, mainBus[nn::audio::ChannelMapping_LowFrequency]);
        nn::audio::SetVoiceMixVolume(&voiceSe[2], &finalMix, 0.707f, 0, mainBus[nn::audio::ChannelMapping_RearLeft]);
        nn::audio::SetVoiceMixVolume(&voiceSe[3], &finalMix, 0.707f, 0, mainBus[nn::audio::ChannelMapping_RearRight]);


        //////////////////////////////////////////////////////////////////////////
        // #1-1: Getting the device to which audio is output and getting when output switches to another device.
        //
        // As described in the Setting the Output Devices section of the @confluencelink{166500119, Audio Library}documentation, the device to which
        // audio output is sent on the NX platform changes dynamically among the various real audio devices according to the operating state.
        // The following code registers a system event as a way to get the timing of changes in output devices.
        // You can use the nn::audio::GetActiveAudioDeviceName() function to get the name of the current output device at any time, unrelated to the notification of this event.
        //////////////////////////////////////////////////////////////////////////
        nn::os::SystemEvent deviceEvent;
        nn::audio::AcquireAudioDeviceNotificationForOutput(&deviceEvent);


        //////////////////////////////////////////////////////////////////////////
        // #2-1: Setting the volume for each audio output device.
        //
        // The nn::audio::ListAudioDeviceName() function gets a list of audio output devices.
        // The following four kinds of devices are obtained.
        // - "AudioTvOutput"
        // - "AudioStereoJackOutput"
        // - "AudioBuiltInSpeakerOutput"
        // - "AudioUsbDeviceOutput"
        // @confluencelink{166500119, For descriptions of these devices, see Setting the Output Devices in the audio library documentation.}
        //////////////////////////////////////////////////////////////////////////
        const int deviceNameCount = 8;
        nn::audio::AudioDeviceName deviceNames[deviceNameCount];
        int listedDeviceCount = nn::audio::ListAudioDeviceName(deviceNames, deviceNameCount);
        NNS_LOG(";; AudioDeviceList\n");
        for (auto i = 0; i < listedDeviceCount; ++i)
        {
            NNS_LOG("[%d] %s\n", i, deviceNames[i].name);
        }
        int deviceIndex = 0;
        float deviceVolume = 0.0f;


        //////////////////////////////////////////////////////////////////////////
        // #3-1: Setting the application-specific downmixing feature.
        //
        // The following code updates the parameter to use for downmixing and disables application-specific downmixing.
        // The parameter for downmixing can be set and the feature for application-specific downmixing can be enabled or disabled at any time,
        // but note that the functions for these tasks are not thread-safe.
        // For more information about downmixing, see the Downmix Processing section in the documentation for the @confluencelink{166500119, Audio Library.}
        //
        // As an extreme example of this sample program, the downmix parameters are set to cut all channels other than FrontLeft and FrontRight.
        // A bit up in the code, voiceBgm was assigned to FrontLeft and FrontRight, and voiceSe was assigned to the other channels,
        // so when application-specific downmixing is enabled the SE sounds will all be cut.
        //////////////////////////////////////////////////////////////////////////
        nn::audio::DeviceSinkType::DownMixParameter downMixParam;
        downMixParam.coeff[0] = 1.f;
        downMixParam.coeff[1] = .0f;
        downMixParam.coeff[2] = .0f;
        downMixParam.coeff[3] = .0f;
        nn::audio::SetDownMixParameter(&deviceSink, &downMixParam);
        nn::audio::SetDownMixParameterEnabled(&deviceSink, false);


        PrintUsage();
        for (;;)
        {
            systemEvent.Wait();
            auto buttonDown = GetButtonDown();
            const auto PlusKeyMask =
                nn::hid::NpadButton::Up::Mask |
                nn::hid::NpadButton::Down::Mask |
                nn::hid::NpadButton::Left::Mask |
                nn::hid::NpadButton::Right::Mask;


            //////////////////////////////////////////////////////////////////////////
            // #1-2: Getting the device to which audio is output, the number of output channels, and the time when output switches to another device.
            // Use the system event obtained with the nn::audio::AcquireAudioDeviceNotificationForOutput()
            // function to verify whether events related to the audio output destination device have occurred.
            // On the NX platform, it is possible to detect when the following events occur.
            // - Changing the audio output destination
            // - Changing the number of output channels
            //
            // This event is initialized to nn::os::EventClearMode_AutoClear.
            //////////////////////////////////////////////////////////////////////////
            if (deviceEvent.TryWait())
            {
                nn::audio::AudioDeviceName deviceName;
                nn::audio::GetActiveAudioDeviceName(&deviceName);
                const auto ChannelCount = nn::audio::GetActiveAudioDeviceChannelCountForOutput();
                NNS_LOG("Active DeviceName:%s Channel Count:%d\n", deviceName.name, ChannelCount);
            }


            //////////////////////////////////////////////////////////////////////////
            // #2-2: Setting the volume for each audio output device.
            //
            // This code configures the volume settings.
            // The volume can be set independently for each output device.
            // Here, the volume for the selected device is manipulated in the range from 0.0f to 1.0f in response to operations using the + Button.
            // The volume can be set as high as 128.0f with the nn::audio::SetAudioDeviceOutputVolume() function,
            // but note that clipping can result if the volume is set too high.
            //////////////////////////////////////////////////////////////////////////
            if ((buttonDown & PlusKeyMask).IsAnyOn())
            {
                UpdateDeviceIndexPamrameter(&deviceIndex, buttonDown, listedDeviceCount);
                // Gets the current value set for the volume.
                deviceVolume = nn::audio::GetAudioDeviceOutputVolume(&deviceNames[deviceIndex]);
                UpdateDeviceVolumeParameter(&deviceVolume, buttonDown);
                NNS_LOG("OutputVolumeControl: %s -> %f\n", deviceNames[deviceIndex].name, deviceVolume);
                nn::audio::SetAudioDeviceOutputVolume(&deviceNames[deviceIndex], deviceVolume);
            }


            //////////////////////////////////////////////////////////////////////////
            // #3-2: Setting the application-specific downmixing feature.
            //
            // This code enables application-specific downmixing.
            // The feature can be enabled and disabled using the nn::audio::SetDownMixParameterEnabled() function.
            // In this sample code, the nn::audio::DeviceSinkType::DownMixParameter settings are configured
            // such that all channels other than FrontLeft and FrontRight are cut,
            // so that no sounds other than BGM are heard when application-specific downmixing is enabled.
            //////////////////////////////////////////////////////////////////////////
            if (buttonDown.Test< ::nn::hid::NpadButton::L >())
            {
                static bool enabled = true;
                nn::audio::SetDownMixParameterEnabled(&deviceSink, enabled);
                enabled = !enabled;
            }


            for (int i = 0; i < SeCount; ++i)
            {
                if (buttonDown.Test(i))
                {
                    if (nn::audio::GetReleasedWaveBuffer(&voiceSe[i]))
                    {
                        nn::audio::AppendWaveBuffer(&voiceSe[i], &waveBufferSe[i]);
                    }
                }
            }

            if (buttonDown.Test< ::nn::hid::NpadButton::Plus >())
            {
                break;
            }

            if (buttonDown.Test< ::nn::hid::NpadButton::Minus >())
            {
                PrintUsage();
            }

            NN_ABORT_UNLESS(nn::audio::RequestUpdateAudioRenderer(handle, &config).IsSuccess());
        }

        // End rendering.
        nn::audio::StopAudioRenderer(handle);
        nn::audio::CloseAudioRenderer(handle);
        nn::os::DestroySystemEvent(systemEvent.GetBase());

        // Free memory.
        for (int i = 0; i < BgmCount; ++i)
        {
            if (dataBgm[i])
            {
                g_WaveBufferAllocator.Free(dataBgm[i]);
                dataBgm[i] = nullptr;
            }
        }
        for (int i = 0; i < SeCount; ++i)
        {
            if (dataSe[i])
            {
                g_WaveBufferAllocator.Free(dataSe[i]);
                dataSe[i] = nullptr;
                g_WaveBufferAllocator.Free(header[i]);
                header[i] = nullptr;
            }
        }
        if (configBuffer)
        {
            g_Allocator.Free(configBuffer);
            configBuffer = nullptr;
        }
        if (workBuffer)
        {
            g_Allocator.Free(workBuffer);
            workBuffer = nullptr;
        }

        FinalizeFileSystem();
    }  // NOLINT(readability/fn_size)
}
