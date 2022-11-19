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
 * @examplesource{GfxPrimitiveRenderer.cpp,PageSampleGfxPrimitiveRenderer}
 *
 * @brief
 *  Sample program showing how to render primitives.
 */

 /**
 * @page PageSampleGfxPrimitiveRenderer  Rendering Primitives Sample
 * @tableofcontents
 *
 * @brief
 *  This documentation describes the GfxPrimitiveRenderer sample program.
 *
 * @section PageSampleGfxPrimitiveRenderer_SectionBrief  Overview
 *  This sample renders simple primitives using <tt>nns::gfx::PrimitiveRenderer</tt>.
 *
 * @section PageSampleGfxPrimitiveRenderer_SectionFileStructure  File Structure
 *  This sample program is located in
 *  @link Samples/Sources/Applications/GfxPrimitiveRenderer Samples/Sources/Applications/GfxPrimitiveRenderer @endlink .
 *
 * @section PageSampleGfxPrimitiveRenderer_SectionNecessaryEnvironment  Required Environment
 *  You must be able to use the screen display.
 *
 * @section PageSampleGfxPrimitiveRenderer_SectionHowToOperate  How to Use
 *  Nothing specific.
 *
 * @section PageSampleGfxPrimitiveRenderer_SectionPrecaution  Important Information
 *  Nothing specific.
 *
 * @section PageSampleGfxPrimitiveRenderer_SectionHowToExecute  Execution Procedure
 *  Build the sample program and then run it.
 *
 * @section PageSampleGfxPrimitiveRenderer_SectionDetail  Description
 *
 * @subsection PageSampleGfxPrimitiveRenderer_SectionSampleProgram  Sample Program
 *  This sample program uses the following source code.
 *
 *  GfxPrimitiveRenderer/GfxPrimitiveRenderer.cpp
 * @includelineno GfxPrimitiveRenderer/GfxPrimitiveRenderer.cpp
 *
 * @subsection PageSampleGfxPrimitiveRenderer_SectionSampleDetail  Sample Program Description
 *  This sample program has the following flow.
 *
 *  - Initialize the device.
 *  - Initialize a memory pool.
 *  - Initialize the swap chain.
 *  - Initialize the queue.
 *  - Initialize the command buffer.
 *  - Initialize the viewport.
 *  - Initialize the rasterizer state.
 *  - Initialize the color buffer.
 *  - Initialize the depth stencil buffer.
 *  - Initialize the buffer for <tt>UserShader</tt>.
 *  - Initialize the sampler.
 *  - Initialize the sampler descriptor pool.
 *  - Initialize the texture descriptor pool.
 *  - Initialize the texture resources.
 *  - Initialize <tt>nns::gfx::PrimitiveRenderer</tt>.
 *  - Initialize <tt>UserShader</tt>.
 *  - Begin the loop.
 *  - Select a shader variation.
 *  - Create the command list.
 *  - Execute the command list.
 *  - Display the results on the screen.
 *  - Return to the start of the loop.
 *  - Free all objects used.
 *
 */

// This comment should go away

#include <cstdlib>
#include <cstring>

#include <nn/nn_Assert.h>

#include <nn/fs.h>
#include <nn/init.h>
#include <nn/gfx.h>
#include <nn/vi.h>
#include <nn/gfx/util/gfx_DebugFontTextWriter.h>
#include <nn/nn_Log.h>
#include <nn/util/util_Color.h>

#include <nn/hws/hws_Message.h>

#if NN_GFX_IS_TARGET_NVN
#include <nvn/nvn.h>
#include <nvn/nvn_FuncPtrInline.h>
#endif

#if defined( NN_BUILD_TARGET_PLATFORM_OS_NN ) && defined( NN_BUILD_APISET_NX )
#include <nv/nv_MemoryManagement.h>
#include <nvnTool/nvnTool_GlslcInterface.h>
#endif

#define NN_PERF_PROFILE_ENABLED
#include <nn/perf.h>

#include <nns/gfx/gfx_PrimitiveRenderer.h>
#include <nns/gfx/gfx_PrimitiveRendererMeterDrawer.h>
#include <nns/gfx/gfx_PrimitiveRendererMeshRes.h>

#include <nn/nn_TimeSpan.h>
#include <nn/os.h>


#include <nn/nn_Macro.h>
#include <nn/os/os_Thread.h>
#include <nn/os/os_SystemEvent.h>
#include <nn/hid.h>
#include <nn/hid/hid_Npad.h>
#include <nn/hid/hid_NpadJoy.h>

#include <vector>

// AudioEffect
#include <cmath>
#include <limits>
#include <algorithm>

#include <nns/nns_Log.h>

#include <nn/mem.h>

#include <nn/audio.h>
#include <nns/audio/audio_HidUtilities.h>
#include <nns/audio/audio_WavFormat.h>

#include <nn/settings/settings_DebugPad.h>

// Vibe
#include "NpadController.h"

#include "Circle.h"

namespace {
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

    bool    waits = true;

    nn::hid::NpadIdType g_NpadIds[] = { nn::hid::NpadId::No1,
                                       nn::hid::NpadId::No2,
                                       nn::hid::NpadId::No3,
                                       nn::hid::NpadId::No4,
                                       nn::hid::NpadId::Handheld,
    };
    const int NpadIdCountMax = sizeof(g_NpadIds) / sizeof(nn::hid::NpadIdType);
    nn::hid::NpadHandheldState oldNpadHandheldState;
    nn::hid::NpadHandheldState currentNpadHandheldState;
    nn::hid::NpadFullKeyState oldNpadFullKeyState[NpadIdCountMax];
    nn::hid::NpadFullKeyState currentNpadFullKeyState[NpadIdCountMax];
    nn::hid::NpadJoyDualState oldNpadJoyDualState[NpadIdCountMax * 2];
    nn::hid::NpadJoyDualState currentNpadJoyDualState[NpadIdCountMax * 2];

    void PrintNpadButtonState(const nn::hid::NpadButtonSet& state)
    {
        char buttons[37];
        buttons[0] = (state.Test<nn::hid::NpadJoyButton::A>()) ? 'A' : '_';
        buttons[1] = (state.Test<nn::hid::NpadJoyButton::B>()) ? 'B' : '_';
        buttons[2] = (state.Test<nn::hid::NpadJoyButton::X>()) ? 'X' : '_';
        buttons[3] = (state.Test<nn::hid::NpadJoyButton::Y>()) ? 'Y' : '_';
        buttons[4] = (state.Test<nn::hid::NpadJoyButton::StickL>()) ? 'L' : '_';
        buttons[5] = (state.Test<nn::hid::NpadJoyButton::StickL>()) ? 'S' : '_';
        buttons[6] = (state.Test<nn::hid::NpadJoyButton::StickR>()) ? 'R' : '_';
        buttons[7] = (state.Test<nn::hid::NpadJoyButton::StickR>()) ? 'S' : '_';
        buttons[8] = (state.Test<nn::hid::NpadJoyButton::L>()) ? 'L' : '_';
        buttons[9] = (state.Test<nn::hid::NpadJoyButton::R>()) ? 'R' : '_';
        buttons[10] = (state.Test<nn::hid::NpadJoyButton::ZL>()) ? 'Z' : '_';
        buttons[11] = (state.Test<nn::hid::NpadJoyButton::ZL>()) ? 'L' : '_';
        buttons[12] = (state.Test<nn::hid::NpadJoyButton::ZR>()) ? 'Z' : '_';
        buttons[13] = (state.Test<nn::hid::NpadJoyButton::ZR>()) ? 'R' : '_';
        buttons[14] = (state.Test<nn::hid::NpadJoyButton::Plus>()) ? '+' : '_';
        buttons[15] = (state.Test<nn::hid::NpadJoyButton::Minus>()) ? '-' : '_';
        buttons[16] = (state.Test<nn::hid::NpadJoyButton::Left>()) ? '<' : '_';
        buttons[17] = (state.Test<nn::hid::NpadJoyButton::Up>()) ? '^' : '_';
        buttons[18] = (state.Test<nn::hid::NpadJoyButton::Right>()) ? '>' : '_';
        buttons[19] = (state.Test<nn::hid::NpadJoyButton::Down>()) ? 'v' : '_';
        buttons[20] = (state.Test<nn::hid::NpadJoyButton::LeftSL>()) ? 'S' : '_';
        buttons[21] = (state.Test<nn::hid::NpadJoyButton::LeftSL>()) ? 'L' : '_';
        buttons[22] = (state.Test<nn::hid::NpadJoyButton::LeftSR>()) ? 'S' : '_';
        buttons[23] = (state.Test<nn::hid::NpadJoyButton::LeftSR>()) ? 'R' : '_';
        buttons[24] = (state.Test<nn::hid::NpadJoyButton::RightSL>()) ? 'S' : '_';
        buttons[25] = (state.Test<nn::hid::NpadJoyButton::RightSL>()) ? 'L' : '_';
        buttons[26] = (state.Test<nn::hid::NpadJoyButton::RightSR>()) ? 'S' : '_';
        buttons[27] = (state.Test<nn::hid::NpadJoyButton::RightSR>()) ? 'R' : '_';
        buttons[28] = (state.Test<nn::hid::NpadJoyButton::StickLRight>()) ? '>' : '_';
        buttons[29] = (state.Test<nn::hid::NpadJoyButton::StickLUp>()) ? '^' : '_';
        buttons[30] = (state.Test<nn::hid::NpadJoyButton::StickLLeft>()) ? '<' : '_';
        buttons[31] = (state.Test<nn::hid::NpadJoyButton::StickLDown>()) ? 'v' : '_';
        buttons[32] = (state.Test<nn::hid::NpadJoyButton::StickRRight>()) ? '>' : '_';
        buttons[33] = (state.Test<nn::hid::NpadJoyButton::StickRUp>()) ? '^' : '_';
        buttons[34] = (state.Test<nn::hid::NpadJoyButton::StickRLeft>()) ? '<' : '_';
        buttons[35] = (state.Test<nn::hid::NpadJoyButton::StickRDown>()) ? 'v' : '_';
        buttons[36] = '\0';

        NN_LOG("%s", buttons);
    }

    void PrintNpadStickState(const nn::hid::AnalogStickState& state)
    {
        NN_LOG("(%6d, %6d)", state.x,
            state.y);
    }

    template <typename TState>
    void PrintNpadState(const TState& state)
    {
        NN_LOG("[%6lld] ", state.samplingNumber);
        PrintNpadButtonState(state.buttons);
        NN_LOG(" L");
        PrintNpadStickState(state.analogStickL);
        NN_LOG(" R");
        PrintNpadStickState(state.analogStickR);
        NN_LOG("\n");
    }

    void Init()
    {
        nn::hid::InitializeNpad();

        // Set the style of operation to use.
        nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleJoyDual::Mask | nn::hid::NpadStyleHandheld::Mask);

        // Configure the Npad to use.
        nn::hid::SetSupportedNpadIdType(g_NpadIds, NpadIdCountMax);


        nn::hid::InitializeDebugPad();
        nn::hid::InitializeKeyboard();
        nn::hid::InitializeMouse();
    }

    void Update()
    {
        for (int i = 0; i < NpadIdCountMax; i++)
        {
            // Get the currently enabled style of operation (NpadStyleSet).
            nn::hid::NpadStyleSet style = nn::hid::GetNpadStyleSet(g_NpadIds[i]);

            // If full key operation is enabled.
            if (style.Test<nn::hid::NpadStyleFullKey>() == true)
            {
                oldNpadFullKeyState[i] = currentNpadFullKeyState[i];

                // Get the latest state of the Npad.
                nn::hid::GetNpadState(&(currentNpadFullKeyState[i]), g_NpadIds[i]);

                // A button was pressed.
                if ((currentNpadFullKeyState[i].buttons ^ oldNpadFullKeyState[i].buttons & currentNpadFullKeyState[i].buttons).IsAnyOn())
                {
                    // Display the Npad input state.
                    NN_LOG("NpadFullKey (%d) ", i);
                    PrintNpadState(currentNpadFullKeyState[i]);
                }

                // Quit when both the + Button and - Button are pressed at the same time.
                if (currentNpadFullKeyState[i].buttons.Test<nn::hid::NpadButton::Plus>() && currentNpadFullKeyState[i].buttons.Test<nn::hid::NpadButton::Minus>())
                {
                    waits = false;
                }
            }
            // If Joy-Con operation is enabled.
            if (style.Test<nn::hid::NpadStyleJoyDual>() == true)
            {
                oldNpadJoyDualState[i] = currentNpadJoyDualState[i];

                // Get the latest state of the Npad.
                nn::hid::GetNpadState(&(currentNpadJoyDualState[i]), g_NpadIds[i]);

                // A button was pressed.
                if ((currentNpadJoyDualState[i].buttons ^ oldNpadJoyDualState[i].buttons & currentNpadJoyDualState[i].buttons).IsAnyOn())
                {
                    // Display the Npad input state.
                    NN_LOG("NpadJoyDual (%d) ", i);
                    PrintNpadState(currentNpadJoyDualState[i]);
                }

                // Quit when both the + Button and - Button are pressed at the same time.
                if (currentNpadJoyDualState[i].buttons.Test<nn::hid::NpadButton::Plus>() && currentNpadJoyDualState[i].buttons.Test<nn::hid::NpadButton::Minus>())
                {
                    waits = false;
                }
            }
            // If handheld controller operation is enabled.
            if (style.Test<nn::hid::NpadStyleHandheld>() == true)
            {
                oldNpadHandheldState = currentNpadHandheldState;

                // Get the latest state of the Npad.
                nn::hid::GetNpadState(&(currentNpadHandheldState), g_NpadIds[i]);

                // A button was pressed.
                if ((currentNpadHandheldState.buttons ^ oldNpadHandheldState.buttons & currentNpadHandheldState.buttons).IsAnyOn())
                {
                    // Display the Npad input state.
                    NN_LOG("NpadHandheld ");
                    PrintNpadState(currentNpadHandheldState);
                }

                // Quit when both the + Button and - Button are pressed at the same time.
                if (currentNpadHandheldState.buttons.Test<nn::hid::NpadButton::Plus>() && currentNpadHandheldState.buttons.Test<nn::hid::NpadButton::Minus>())
                {
                    waits = false;
                }
            }
        }
    }
    int g_BufferDescriptorBaseIndex = 0;
    int g_TextureDescriptorBaseIndex = 0;
    int g_SamplerDescriptorBaseIndex = 0;

    int g_RenderWidth = 0;
    int g_RenderHeight = 0;

    nns::gfx::PrimitiveRenderer::MeterDrawer g_MeterDrawer;

    //-----------------------------------------------------------------------------
    // Memory
    static size_t g_MemoryHeapSize = 32 * 1024 * 1024;

    nn::util::BytePtr g_pMemoryHeap(NULL);
    nn::util::BytePtr g_pMemory(NULL);


    static size_t g_VisiblePoolMemorySize = 16 * 1024 * 1024;
    static size_t g_InvisiblePoolMemorySize = 64 * 1024 * 1024;

    void* g_pVisiblePoolMemory = NULL;
    void* g_pInvisiblePoolMemory = NULL;
    void* g_pMemoryPoolStart = NULL;
    ptrdiff_t g_MemoryPoolOffset = 0;
    void* g_pInvisibleMemoryPoolStart = NULL;
    ptrdiff_t g_InvisibleMemoryPoolOffset = 0;

    void* g_MountRomCacheBuffer = NULL;


    void* ReadResource(const char* filename)
    {
        nn::Result result;
        nn::fs::FileHandle hFile;

        int64_t fileSize = 0;
        result = nn::fs::OpenFile(&hFile, filename, nn::fs::OpenMode_Read);
        NN_ASSERT(result.IsSuccess());

        result = nn::fs::GetFileSize(&fileSize, hFile);
        NN_ASSERT(result.IsSuccess());

        nn::util::BinaryFileHeader fileHeader;
        size_t readSize;
        result = nn::fs::ReadFile(&readSize, hFile, 0, &fileHeader, sizeof(nn::util::BinaryFileHeader));
        NN_ASSERT(result.IsSuccess());
        NN_ASSERT(readSize == sizeof(nn::util::BinaryFileHeader));
        size_t alignment = fileHeader.GetAlignment();

        g_pMemory.AlignUp(alignment);
        void* pBuffer = g_pMemory.Get();
        result = nn::fs::ReadFile(&readSize, hFile, 0, pBuffer, static_cast<size_t>(fileSize));
        NN_ASSERT(result.IsSuccess());
        NN_ASSERT(readSize == static_cast<size_t>(fileSize));
        g_pMemory.Advance(static_cast<ptrdiff_t>(fileSize));

        nn::fs::CloseFile(hFile);

        return pBuffer;
    }


    //  nn::vi object.
    nn::vi::Display* g_pDisplay;
    nn::vi::Layer* g_pLayer;

    // Initialize the device.
    nn::gfx::Device g_Device;
    void InitializeDevice()
    {
        nn::gfx::Device::InfoType info;
        info.SetDefault();
        info.SetApiVersion(nn::gfx::ApiMajorVersion, nn::gfx::ApiMinorVersion);
#if defined( NN_SDK_BUILD_DEBUG )
        info.SetDebugMode(nn::gfx::DebugMode_Full);
#endif
        g_Device.Initialize(info);
    }

    //  Layer initialization.
    void InitializeLayer()
    {
        nn::vi::Initialize();

        nn::Result result = nn::vi::OpenDefaultDisplay(&g_pDisplay);
        NN_ABORT_UNLESS_RESULT_SUCCESS(result);
        NN_UNUSED(result);

        g_RenderWidth = 1280;
        g_RenderHeight = 720;

        result = nn::vi::CreateLayer(&g_pLayer, g_pDisplay);
        NN_ABORT_UNLESS_RESULT_SUCCESS(result);
    }

    //  Layer finalization.
    void FinalizeLayer()
    {
        nn::vi::DestroyLayer(g_pLayer);
        nn::vi::CloseDisplay(g_pDisplay);
        nn::vi::Finalize();
    }

    nn::gfx::MemoryPool g_MemoryPool;
    void InitializeMemoryPool()
    {
        nn::gfx::MemoryPool::InfoType info;
        info.SetDefault();
        info.SetMemoryPoolProperty(nn::gfx::MemoryPoolProperty_CpuUncached
            | nn::gfx::MemoryPoolProperty_GpuCached);

        g_VisiblePoolMemorySize = nn::util::align_up(g_VisiblePoolMemorySize,
            nn::gfx::MemoryPool::GetPoolMemorySizeGranularity(&g_Device, info));

        // Determine the alignment to allocate space on an alignment boundary.
        size_t alignment = nn::gfx::MemoryPool::GetPoolMemoryAlignment(&g_Device, info);
        g_pVisiblePoolMemory = malloc(g_VisiblePoolMemorySize + alignment);

        g_pMemoryPoolStart = nn::util::BytePtr(g_pVisiblePoolMemory).AlignUp(alignment).Get();
        info.SetPoolMemory(g_pMemoryPoolStart, nn::util::align_down(g_VisiblePoolMemorySize,
            nn::gfx::MemoryPool::GetPoolMemorySizeGranularity(&g_Device, info)));
        g_MemoryPool.Initialize(&g_Device, info);

        g_MemoryPoolOffset = 0;
    }

    nn::gfx::MemoryPool g_InvisibleMemoryPool;
    void InitializeInvisibleMemoryPool()
    {
        nn::gfx::MemoryPool::InfoType info;
        info.SetDefault();
        info.SetMemoryPoolProperty(nn::gfx::MemoryPoolProperty_CpuInvisible |
            nn::gfx::MemoryPoolProperty_GpuCached |
            nn::gfx::MemoryPoolProperty_Compressible);

        g_InvisiblePoolMemorySize = nn::util::align_up(g_InvisiblePoolMemorySize,
            nn::gfx::MemoryPool::GetPoolMemorySizeGranularity(&g_Device, info));

        // Determine the alignment to allocate space on an alignment boundary.
        size_t alignment = nn::gfx::MemoryPool::GetPoolMemoryAlignment(&g_Device, info);
        g_pInvisiblePoolMemory = malloc(g_InvisiblePoolMemorySize + alignment);

        g_pInvisibleMemoryPoolStart = nn::util::BytePtr(g_pInvisiblePoolMemory).AlignUp(alignment).Get();
        info.SetPoolMemory(g_pInvisibleMemoryPoolStart, nn::util::align_down(g_InvisiblePoolMemorySize,
            nn::gfx::MemoryPool::GetPoolMemorySizeGranularity(&g_Device, info)));
        g_InvisibleMemoryPool.Initialize(&g_Device, info);

        g_InvisibleMemoryPoolOffset = 0;
    }


    // Initialize the swap chain.
    const int g_SwapChainBufferCount = 2;
    nn::gfx::SwapChain g_SwapChain;
    void InitializeSwapChain()
    {
        nn::gfx::SwapChain::InfoType info;

        info.SetDefault();
        info.SetLayer(g_pLayer);
        info.SetWidth(g_RenderWidth);
        info.SetHeight(g_RenderHeight);
        info.SetFormat(nn::gfx::ImageFormat_R8_G8_B8_A8_UnormSrgb);
        info.SetBufferCount(g_SwapChainBufferCount);
        if (NN_STATIC_CONDITION(nn::gfx::SwapChain::IsMemoryPoolRequired))
        {
            size_t size = g_SwapChain.CalculateScanBufferSize(&g_Device, info);
            g_InvisibleMemoryPoolOffset = nn::util::align_up(g_InvisibleMemoryPoolOffset,
                nn::gfx::SwapChain::GetScanBufferAlignment(&g_Device, info));
            g_SwapChain.Initialize(&g_Device, info, &g_InvisibleMemoryPool, g_InvisibleMemoryPoolOffset, size);
            g_InvisibleMemoryPoolOffset += size;
        }
        else
        {
            g_SwapChain.Initialize(&g_Device, info, NULL, 0, 0);
        }
    }

    // Initialize the queue.
    nn::gfx::Queue g_Queue;
    void InitializeQueue()
    {
        nn::gfx::Queue::InfoType info;
        info.SetDefault();
        info.SetCapability(nn::gfx::QueueCapability_Graphics);
        g_Queue.Initialize(&g_Device, info);
    }

    // Initialize the command buffer.
    nn::gfx::CommandBuffer g_CommandBuffer[g_SwapChainBufferCount];
    void InitializeCommandBuffer()
    {
        nn::gfx::CommandBuffer::InfoType info;
        info.SetDefault();
        info.SetQueueCapability(nn::gfx::QueueCapability_Graphics);
        info.SetCommandBufferType(nn::gfx::CommandBufferType_Direct);

        for (int bufferIndex = 0; bufferIndex < g_SwapChainBufferCount; ++bufferIndex)
        {
            g_CommandBuffer[bufferIndex].Initialize(&g_Device, info);
        }
    }

    // Related to the framework.
    nn::gfx::Fence g_DisplayFence[g_SwapChainBufferCount];
    nn::gfx::Fence g_GpuFence[g_SwapChainBufferCount];
    nn::gfx::Semaphore g_DisplaySemaphore[g_SwapChainBufferCount];
    nn::gfx::ColorTargetView* g_pScanBufferViews[g_SwapChainBufferCount];
    nn::gfx::Texture* g_pScanBufferTextures[g_SwapChainBufferCount];
    int g_NextScanBufferIndex = 0;
    void InitializeFramework()
    {
        nn::gfx::Fence::InfoType fenceInfo;
        fenceInfo.SetDefault();
        for (int bufferIndex = 0; bufferIndex < g_SwapChainBufferCount; ++bufferIndex)
        {
            g_DisplayFence[bufferIndex].Initialize(&g_Device, fenceInfo);
            g_GpuFence[bufferIndex].Initialize(&g_Device, fenceInfo);
        }

        nn::gfx::Semaphore::InfoType semaphoreInfo;
        semaphoreInfo.SetDefault();
        for (int bufferIndex = 0; bufferIndex < g_SwapChainBufferCount; ++bufferIndex)
        {
            g_DisplaySemaphore[bufferIndex].Initialize(&g_Device, semaphoreInfo);
        }
        g_SwapChain.GetScanBufferViews(g_pScanBufferViews, g_SwapChainBufferCount);
        g_SwapChain.GetScanBuffers(g_pScanBufferTextures, g_SwapChainBufferCount);
    }

    void FinalizeFramework()
    {
        for (int bufferIndex = 0; bufferIndex < g_SwapChainBufferCount; ++bufferIndex)
        {
            g_DisplayFence[bufferIndex].Finalize(&g_Device);
            g_GpuFence[bufferIndex].Finalize(&g_Device);
            g_DisplaySemaphore[bufferIndex].Finalize(&g_Device);
        }
    }

    // Initialize the viewport scissor.
    nn::gfx::ViewportScissorState g_ViewportScissor;
    void InitializeViewport()
    {
        nn::gfx::ViewportScissorState::InfoType info;
        info.SetDefault();
        info.SetScissorEnabled(true);
        nn::gfx::ViewportStateInfo viewportInfo;
        {
            viewportInfo.SetDefault();
            viewportInfo.SetWidth(static_cast<float>(g_RenderWidth));
            viewportInfo.SetHeight(static_cast<float>(g_RenderHeight));
        }
        nn::gfx::ScissorStateInfo scissorInfo;
        {
            scissorInfo.SetDefault();
            scissorInfo.SetWidth(g_RenderWidth);
            scissorInfo.SetHeight(g_RenderHeight);
        }
        info.SetViewportStateInfoArray(&viewportInfo, 1);
        info.SetScissorStateInfoArray(&scissorInfo, 1);
        g_ViewportScissor.Initialize(&g_Device, info);
    }

    // Initialize the rasterizer state.
    nn::gfx::RasterizerState g_RasterizerState;
    static void InitializeRasterizerState()
    {
        nn::gfx::RasterizerState::InfoType info;
        info.SetDefault();
        info.SetCullMode(nn::gfx::CullMode_None);
        info.SetPrimitiveTopologyType(nn::gfx::PrimitiveTopologyType_Triangle);
        info.SetScissorEnabled(true);
        info.SetDepthClipEnabled(false);
        g_RasterizerState.Initialize(&g_Device, info);
    }

    // Initialize the color buffer.
    nn::gfx::Texture g_ColorBuffer;
    nn::gfx::ColorTargetView g_ColorTargetView;
    nn::gfx::TextureView g_ColorTextureView;
    void InitializeColorBuffer()
    {
        nn::gfx::Texture::InfoType info;
        info.SetDefault();
        info.SetWidth(g_RenderWidth);
        info.SetHeight(g_RenderHeight);
        info.SetGpuAccessFlags(nn::gfx::GpuAccess_ColorBuffer | nn::gfx::GpuAccess_Texture);
        info.SetImageStorageDimension(nn::gfx::ImageStorageDimension_2d);
        info.SetImageFormat(nn::gfx::ImageFormat_R16_G16_B16_A16_Float);
        info.SetMipCount(1);

        g_InvisibleMemoryPoolOffset = nn::util::align_up(g_InvisibleMemoryPoolOffset, nn::gfx::Texture::CalculateMipDataAlignment(&g_Device, info));
        size_t size = nn::gfx::Texture::CalculateMipDataSize(&g_Device, info);
        g_ColorBuffer.Initialize(&g_Device, info, &g_InvisibleMemoryPool, g_InvisibleMemoryPoolOffset, size);
        g_InvisibleMemoryPoolOffset += size;

        // Initialize a color target view.
        nn::gfx::ColorTargetView::InfoType targetInfo;
        targetInfo.SetDefault();
        targetInfo.SetImageDimension(nn::gfx::ImageDimension_2d);
        targetInfo.SetImageFormat(nn::gfx::ImageFormat_R16_G16_B16_A16_Float);
        targetInfo.SetTexturePtr(&g_ColorBuffer);
        g_ColorTargetView.Initialize(&g_Device, targetInfo);

        // Initialize the color buffer texture view.
        nn::gfx::TextureView::InfoType viewInfo;
        viewInfo.SetDefault();
        viewInfo.SetImageDimension(nn::gfx::ImageDimension_2d);
        viewInfo.SetImageFormat(nn::gfx::ImageFormat_R16_G16_B16_A16_Float);
        viewInfo.SetTexturePtr(&g_ColorBuffer);
        g_ColorTextureView.Initialize(&g_Device, viewInfo);
    }

    nn::gfx::Texture g_BlurColorBuffer;
    nn::gfx::ColorTargetView g_BlurColorTargetView;
    nn::gfx::TextureView g_BlurColorTextureView;
    nn::gfx::DescriptorSlot g_BlurColorTextureDescSlot;
    void InitializeBlurBuffer()
    {
        nn::gfx::Texture::InfoType info;
        info.SetDefault();
        info.SetWidth(g_RenderWidth);
        info.SetHeight(g_RenderHeight);
        info.SetGpuAccessFlags(nn::gfx::GpuAccess_ColorBuffer | nn::gfx::GpuAccess_Texture);
        info.SetImageStorageDimension(nn::gfx::ImageStorageDimension_2d);
        info.SetImageFormat(nn::gfx::ImageFormat_R16_G16_B16_A16_Float);
        info.SetMipCount(1);

        g_InvisibleMemoryPoolOffset = nn::util::align_up(g_InvisibleMemoryPoolOffset, nn::gfx::Texture::CalculateMipDataAlignment(&g_Device, info));
        size_t size = nn::gfx::Texture::CalculateMipDataSize(&g_Device, info);
        g_BlurColorBuffer.Initialize(&g_Device, info, &g_InvisibleMemoryPool, g_InvisibleMemoryPoolOffset, size);
        g_InvisibleMemoryPoolOffset += size;

        // Initialize a color target view.
        nn::gfx::ColorTargetView::InfoType targetInfo;
        targetInfo.SetDefault();
        targetInfo.SetImageDimension(nn::gfx::ImageDimension_2d);
        targetInfo.SetImageFormat(nn::gfx::ImageFormat_R16_G16_B16_A16_Float);
        targetInfo.SetTexturePtr(&g_BlurColorBuffer);
        g_BlurColorTargetView.Initialize(&g_Device, targetInfo);

        // Initialize the color buffer texture view.
        nn::gfx::TextureView::InfoType viewInfo;
        viewInfo.SetDefault();
        viewInfo.SetImageDimension(nn::gfx::ImageDimension_2d);
        viewInfo.SetImageFormat(nn::gfx::ImageFormat_R16_G16_B16_A16_Float);
        viewInfo.SetTexturePtr(&g_BlurColorBuffer);
        g_BlurColorTextureView.Initialize(&g_Device, viewInfo);
    }

    // Initialize the depth stencil buffer.
    nn::gfx::Texture g_DepthStencilBuffer;
    nn::gfx::DepthStencilView g_DepthStencilView;
    void InitializeDepthStencilBuffer()
    {
        // Initialize the depth stencil buffer texture.
        nn::gfx::Texture::InfoType info;
        info.SetDefault();
        info.SetWidth(g_RenderWidth);
        info.SetHeight(g_RenderHeight);
        info.SetGpuAccessFlags(nn::gfx::GpuAccess_DepthStencil);
        info.SetImageStorageDimension(nn::gfx::ImageStorageDimension_2d);
        info.SetImageFormat(nn::gfx::ImageFormat_D32_Float);
        info.SetMipCount(1);
        g_InvisibleMemoryPoolOffset = nn::util::align_up(g_InvisibleMemoryPoolOffset, nn::gfx::Texture::CalculateMipDataAlignment(&g_Device, info));
        size_t size = nn::gfx::Texture::CalculateMipDataSize(&g_Device, info);
        g_DepthStencilBuffer.Initialize(&g_Device, info, &g_InvisibleMemoryPool, g_InvisibleMemoryPoolOffset, size);
        g_InvisibleMemoryPoolOffset += size;

        // Initialize the depth stencil view.
        nn::gfx::DepthStencilView::InfoType viewInfo;
        viewInfo.SetDefault();
        viewInfo.SetImageDimension(nn::gfx::ImageDimension_2d);
        viewInfo.SetTexturePtr(&g_DepthStencilBuffer);
        g_DepthStencilView.Initialize(&g_Device, viewInfo);
    }

    // Initialize the sampler.
    nn::gfx::Sampler g_Sampler;
    void InitializeSampler()
    {
        nn::gfx::Sampler::InfoType info;
        info.SetDefault();
        info.SetFilterMode(nn::gfx::FilterMode_MinLinear_MagLinear_MipPoint);
        info.SetAddressU(nn::gfx::TextureAddressMode_Mirror);
        info.SetAddressV(nn::gfx::TextureAddressMode_Mirror);
        info.SetAddressW(nn::gfx::TextureAddressMode_Mirror);
        g_Sampler.Initialize(&g_Device, info);
    }

    nn::gfx::DescriptorPool g_BufferDescriptorPool;
    void InitializeBufferDescriptorPool()
    {
        nn::gfx::DescriptorPool::InfoType info;
        info.SetDefault();
        info.SetDescriptorPoolType(nn::gfx::DescriptorPoolType_BufferView);
        info.SetSlotCount(g_BufferDescriptorBaseIndex + 1);
        size_t size = nn::gfx::DescriptorPool::CalculateDescriptorPoolSize(&g_Device, info);
        g_MemoryPoolOffset = nn::util::align_up(g_MemoryPoolOffset,
            nn::gfx::DescriptorPool::GetDescriptorPoolAlignment(&g_Device, info));
        g_BufferDescriptorPool.Initialize(&g_Device, info, &g_MemoryPool, g_MemoryPoolOffset, size);
        g_MemoryPoolOffset += size;
    }

    nn::gfx::DescriptorPool g_SamplerDescriptorPool;
    void InitializeSamplerDescriptorPool()
    {
        nn::gfx::DescriptorPool::InfoType info;
        info.SetDefault();
        info.SetDescriptorPoolType(nn::gfx::DescriptorPoolType_Sampler);
        info.SetSlotCount(g_SamplerDescriptorBaseIndex + 1);
        size_t size = nn::gfx::DescriptorPool::CalculateDescriptorPoolSize(&g_Device, info);
        g_MemoryPoolOffset = nn::util::align_up(g_MemoryPoolOffset,
            nn::gfx::DescriptorPool::GetDescriptorPoolAlignment(&g_Device, info));
        g_SamplerDescriptorPool.Initialize(&g_Device, info,
            &g_MemoryPool, g_MemoryPoolOffset, size);
        g_MemoryPoolOffset += size;
    }

    nn::gfx::DescriptorPool g_TextureDescriptorPool;
    void InitializeTextureDescriptorPool()
    {
        nn::gfx::DescriptorPool::InfoType info;
        info.SetDefault();
        info.SetDescriptorPoolType(nn::gfx::DescriptorPoolType_TextureView);
        info.SetSlotCount(g_TextureDescriptorBaseIndex + 4);
        size_t size = nn::gfx::DescriptorPool::CalculateDescriptorPoolSize(&g_Device, info);
        g_MemoryPoolOffset = nn::util::align_up(g_MemoryPoolOffset,
            nn::gfx::DescriptorPool::GetDescriptorPoolAlignment(&g_Device, info));
        g_TextureDescriptorPool.Initialize(&g_Device, info, &g_MemoryPool, g_MemoryPoolOffset, size);
        g_MemoryPoolOffset += size;
    }

    nn::gfx::ResTextureFile* g_pResTextureFile;
    int g_IndexBg = -1;
    int g_IndexText = -1;
    void InitializeResTextureFile()
    {
        void* pTextureResource = ReadResource("Contents:/Check.bntx");
        g_pResTextureFile = nn::gfx::ResTextureFile::ResCast(pTextureResource);
        g_pResTextureFile->Initialize(&g_Device);
        for (int idxTexture = 0, textureCount = g_pResTextureFile->GetTextureDic()->GetCount();
            idxTexture < textureCount; ++idxTexture)
        {
            g_pResTextureFile->GetResTexture(idxTexture)->Initialize(&g_Device);

        }
        g_IndexBg = g_pResTextureFile->GetTextureDic()->FindIndex("Check_BG_01");
        NN_ASSERT(g_IndexBg != -1);
        g_IndexText = g_pResTextureFile->GetTextureDic()->FindIndex("Check_Text_01");
        NN_ASSERT(g_IndexText != -1);
    }

    void FinalizeResTextureFile()
    {
        for (int idxTexture = 0, textureCount = g_pResTextureFile->GetTextureDic()->GetCount();
            idxTexture < textureCount; ++idxTexture)
        {
            g_pResTextureFile->GetResTexture(idxTexture)->Finalize(&g_Device);
        }
        g_pResTextureFile->Finalize(&g_Device);
    }

#if defined( NN_BUILD_TARGET_PLATFORM_OS_NN ) && defined( NN_BUILD_APISET_NX )
    void* Allocate(size_t size, size_t alignment, void*)
    {
        return aligned_alloc(alignment, nn::util::align_up(size, alignment));
    }
    void Free(void* addr, void*)
    {
        free(addr);
    }
    void* Reallocate(void* addr, size_t newSize, void*)
    {
        return realloc(addr, newSize);
    }
#endif

    void InitializeResources()
    {
#if defined( NN_BUILD_TARGET_PLATFORM_OS_NN ) && defined( NN_BUILD_APISET_NX )
        // Initialize memory resources for the graphics system.
        {
            const size_t GraphicsSystemMemorySize = 8 * 1024 * 1024;
            nv::SetGraphicsAllocator(Allocate, Free, Reallocate, NULL);
            nv::InitializeGraphics(malloc(GraphicsSystemMemorySize), GraphicsSystemMemorySize);
        }
        // Set the memory allocator for the graphics developer tools and the debug layer.
        nv::SetGraphicsDevtoolsAllocator(Allocate, Free, Reallocate, NULL);
        // Set the memory allocator for GLSLC.
        glslcSetAllocator(Allocate, Free, Reallocate, NULL);
#endif

        // Memory
        g_pMemoryHeap.Reset(malloc(g_MemoryHeapSize));
        g_pMemory = g_pMemoryHeap;

        // Initialize the layer.
        InitializeLayer();

        // Initialize the library.
        nn::gfx::Initialize();

        InitializeDevice();

#if NN_GFX_IS_TARGET_NVN
        nn::gfx::Device::DataType& deviceData = nn::gfx::AccessorToData(g_Device);
        nvnDeviceGetInteger(deviceData.pNvnDevice,
            NVN_DEVICE_INFO_RESERVED_TEXTURE_DESCRIPTORS, &g_TextureDescriptorBaseIndex);
        nvnDeviceGetInteger(deviceData.pNvnDevice,
            NVN_DEVICE_INFO_RESERVED_SAMPLER_DESCRIPTORS, &g_SamplerDescriptorBaseIndex);
#endif

        InitializeMemoryPool();
        InitializeInvisibleMemoryPool();

        InitializeSwapChain();
        InitializeQueue();
        InitializeFramework();

        InitializeCommandBuffer();
        InitializeViewport();

        InitializeRasterizerState();

        InitializeColorBuffer();
        InitializeDepthStencilBuffer();
        InitializeBlurBuffer();

        InitializeSampler();

        InitializeBufferDescriptorPool();
        InitializeSamplerDescriptorPool();
        InitializeTextureDescriptorPool();

        InitializeResTextureFile();

        NN_ASSERT(static_cast<size_t>(g_pMemoryHeap.Distance(g_pMemory.Get())) < g_MemoryHeapSize);
        NN_ASSERT(g_MemoryPoolOffset < static_cast<ptrdiff_t>(g_VisiblePoolMemorySize));
        NN_ASSERT(g_InvisibleMemoryPoolOffset < static_cast<ptrdiff_t>(g_InvisiblePoolMemorySize));
    }

    void FinalizeResources()
    {
        // Free each object.
        FinalizeResTextureFile();

        FinalizeFramework();

        g_Sampler.Finalize(&g_Device);
        g_BlurColorTextureView.Finalize(&g_Device);
        g_BlurColorTargetView.Finalize(&g_Device);
        g_BlurColorBuffer.Finalize(&g_Device);
        g_DepthStencilView.Finalize(&g_Device);
        g_DepthStencilBuffer.Finalize(&g_Device);
        g_ColorTextureView.Finalize(&g_Device);
        g_ColorTargetView.Finalize(&g_Device);
        g_ColorBuffer.Finalize(&g_Device);
        g_RasterizerState.Finalize(&g_Device);
        g_BufferDescriptorPool.Finalize(&g_Device);
        g_TextureDescriptorPool.Finalize(&g_Device);
        g_SamplerDescriptorPool.Finalize(&g_Device);

        g_ViewportScissor.Finalize(&g_Device);
        for (int bufferIndex = 0; bufferIndex < g_SwapChainBufferCount; ++bufferIndex)
        {
            g_CommandBuffer[bufferIndex].Finalize(&g_Device);
        }
        g_SwapChain.Finalize(&g_Device);
        g_Queue.Finalize(&g_Device);
        g_InvisibleMemoryPool.Finalize(&g_Device);
        g_MemoryPool.Finalize(&g_Device);
        g_Device.Finalize();

        // Finalize the library.
        nn::gfx::Finalize();

        // Finalize the layer.
        FinalizeLayer();

        free(g_pMemoryHeap.Get());
        free(g_pVisiblePoolMemory);
        free(g_pInvisiblePoolMemory);
    }

    // OVERLAPPED
    void InitializeFs()
    {
        nn::Result result;

        size_t cacheSize = 0;
        result = nn::fs::QueryMountRomCacheSize(&cacheSize);
        NN_ABORT_UNLESS(result.IsSuccess());

        g_MountRomCacheBuffer = malloc(cacheSize);
        NN_ABORT_UNLESS_NOT_NULL(g_MountRomCacheBuffer);

        result = nn::fs::MountRom("Contents", g_MountRomCacheBuffer, cacheSize);
        NN_ABORT_UNLESS(result.IsSuccess());
    }

    void FinalizeFs()
    {
        nn::fs::Unmount("Contents");

        free(g_MountRomCacheBuffer);
        g_MountRomCacheBuffer = NULL;
    }

}

// Constant buffer structure used by custom shaders.
struct PixelSize
{
    nn::util::Float2 u_pixel_size;
};

void InitializeShader(
    nn::gfx::ResShaderFile** ppOutResShaderFile, nn::gfx::Shader** ppOutShader,
    nn::gfx::Device* pDevice,
    const char* pShaderFileResourcePath)
{
    void* pShaderResource = ReadResource(pShaderFileResourcePath);

    NN_ASSERT(reinterpret_cast<const nn::util::BinaryFileHeader*>(pShaderResource)->IsSignatureValid(nn::gfx::ResShaderFile::Signature));
    size_t binaryFileAlignment = reinterpret_cast<const nn::util::BinaryFileHeader*>(pShaderResource)->GetAlignment();
    NN_ASSERT(nn::util::is_aligned(reinterpret_cast<uintptr_t>(pShaderResource), binaryFileAlignment));
    NN_UNUSED(binaryFileAlignment);

    nn::gfx::ResShaderFile* pResShaderFile = nn::gfx::ResShaderFile::ResCast(pShaderResource);
    nn::gfx::ResShaderContainer* pContainer = pResShaderFile->GetShaderContainer();
    NN_ASSERT_NOT_NULL(pContainer);
    pContainer->Initialize(pDevice);

    NN_ASSERT_EQUAL(pContainer->GetShaderVariationCount(), 1);

    nn::gfx::ResShaderProgram* pResShaderProgram = pContainer->GetResShaderProgram(0);
    NN_ASSERT_NOT_NULL(pResShaderProgram);
    nn::gfx::ShaderInitializeResult shaderResult = pResShaderProgram->Initialize(pDevice);
    NN_ASSERT_EQUAL(shaderResult, nn::gfx::ShaderInitializeResult_Success);
    NN_UNUSED(shaderResult);

    *ppOutResShaderFile = pResShaderFile;
    *ppOutShader = pResShaderProgram->GetShader();
}

void FinalizeShader(nn::gfx::ResShaderFile* pResShaderFile, nn::gfx::Device* pDevice)
{
    nn::gfx::ResShaderContainer* pContainer = pResShaderFile->GetShaderContainer();
    NN_ASSERT_NOT_NULL(pContainer);

    nn::gfx::ResShaderProgram* pResShaderProgram = pContainer->GetResShaderProgram(0);
    NN_ASSERT_NOT_NULL(pResShaderProgram);
    pResShaderProgram->Finalize(pDevice);

    pContainer->Finalize(pDevice);
}

//---------------------------------------------------------------
// Initialize the user shader.
//---------------------------------------------------------------
nn::gfx::ResShaderFile* g_pMosaicResShaderFile = nullptr;
nn::gfx::Shader* g_pMosaicShader = nullptr;

int g_SlotPixelSize = 0;
void InitializeMosaicUserShader()
{
    InitializeShader(
        &g_pMosaicResShaderFile, &g_pMosaicShader,
        &g_Device, "Contents:/MosaicPixelShader.bnsh");

    g_SlotPixelSize = g_pMosaicShader->GetInterfaceSlot(nn::gfx::ShaderStage_Pixel, nn::gfx::ShaderInterfaceType_ConstantBuffer, "TextureParam");
}

nn::gfx::ResShaderFile* g_pShadingWithTexResShaderFile = nullptr;
nn::gfx::Shader* g_pShadingWithTexShader = nullptr;
void InitializeShadingWithTexUserShader()
{
    InitializeShader(
        &g_pShadingWithTexResShaderFile, &g_pShadingWithTexShader,
        &g_Device, "Contents:/ShadingTexPixelShader.bnsh");
}

nn::gfx::ResShaderFile* g_pShadingWithoutTexResShaderFile = nullptr;
nn::gfx::Shader* g_pShadingWithoutTexShader = nullptr;
void InitializeShadingWithoutTexUserShader()
{
    InitializeShader(
        &g_pShadingWithoutTexResShaderFile, &g_pShadingWithoutTexShader,
        &g_Device, "Contents:/ShadingPixelShader.bnsh");

}

//---------------------------------------------------------------
// Initialize the primitive renderer.
//---------------------------------------------------------------
nn::mem::StandardAllocator  g_PrimitiveRendererAllocator;
nns::gfx::PrimitiveRenderer::Renderer* g_pPrimitiveRenderer;
void* g_pPrimitiveRendererAllocatorMemory = nullptr;

//--------------------------------------------------------------------------------------
//  Memory Allocation Function
//--------------------------------------------------------------------------------------
void* AllocateFunction(size_t size, size_t alignment, void* pUserData)
{
    NN_SDK_ASSERT_NOT_NULL(pUserData);
    nn::mem::StandardAllocator* pAllocator = static_cast<nn::mem::StandardAllocator*>(pUserData);
    return pAllocator->Allocate(size, alignment);
}

//--------------------------------------------------------------------------------------
//  Memory Deallocation Function
//--------------------------------------------------------------------------------------
void DeallocateFunction(void* ptr, void* pUserData)
{
    NN_SDK_ASSERT_NOT_NULL(pUserData);
    nn::mem::StandardAllocator* pAllocator = static_cast<nn::mem::StandardAllocator*>(pUserData);
    pAllocator->Free(ptr);
}

void InitializePrimitiveRenderer()
{
    const size_t workMemorySize = 1024 * 1024 * 64;
    g_pPrimitiveRendererAllocatorMemory = malloc(workMemorySize);
    g_PrimitiveRendererAllocator.Initialize(g_pPrimitiveRendererAllocatorMemory, workMemorySize);

    nns::gfx::PrimitiveRenderer::RendererInfo info;
    info.SetDefault();
    info.SetAllocator(AllocateFunction, &g_PrimitiveRendererAllocator);
    info.SetAdditionalBufferSize(1024 * 4);

    // To use a double buffer, specify 2 for SetMultiBufferQuantity.
    // After the buffer has been doubled inside the primitive renderer, use g_pPrimitiveRenderer->Update(); to
    // select which buffer to use, going from 0 to 1 to 0 to 1.
    info.SetMultiBufferQuantity(2);

    // A PrimitiveRenderer instance.
    g_pPrimitiveRenderer = nns::gfx::PrimitiveRenderer::CreateRenderer(&g_Device, info);
    g_pPrimitiveRenderer->SetScreenWidth(g_RenderWidth);
    g_pPrimitiveRenderer->SetScreenHeight(g_RenderHeight);

    // Initialize the user shader.
    InitializeMosaicUserShader();
    InitializeShadingWithTexUserShader();
    InitializeShadingWithoutTexUserShader();
}


//---------------------------------------------------------------
// Free the Primitive Renderer
//---------------------------------------------------------------
void FinalizePrimitiveRenderer()
{
    // Free the user shader.
    FinalizeShader(g_pMosaicResShaderFile, &g_Device);
    g_pMosaicResShaderFile = nullptr;
    g_pMosaicShader = nullptr;

    FinalizeShader(g_pShadingWithoutTexResShaderFile, &g_Device);
    g_pShadingWithoutTexResShaderFile = nullptr;
    g_pShadingWithoutTexShader = nullptr;

    FinalizeShader(g_pShadingWithTexResShaderFile, &g_Device);
    g_pShadingWithTexResShaderFile = nullptr;
    g_pShadingWithTexShader = nullptr;

    nns::gfx::PrimitiveRenderer::DestroyRenderer(g_pPrimitiveRenderer, &g_Device, DeallocateFunction, &g_PrimitiveRendererAllocator);
    g_PrimitiveRendererAllocator.Finalize();
    free(g_pPrimitiveRendererAllocatorMemory);
}

//---------------------------------------------------------------
// Initialize the debug font.
//---------------------------------------------------------------
nn::gfx::util::DebugFontTextWriter g_Writer;
nn::util::BytePtr g_DebugFontHeap(NULL);
void InitializeDebugFont()
{
    const int charCountMax = 1024;
    nn::gfx::util::DebugFontTextWriterInfo info;
    info.SetDefault();
    info.SetCharCountMax(charCountMax);
    info.SetUserMemoryPoolEnabled(false);
    info.SetBufferCount(2);

    size_t debugFontHeapSize = nn::gfx::util::DebugFontTextWriter::GetRequiredMemorySize(
        &g_Device,
        info
    );
    g_DebugFontHeap = nn::util::BytePtr(new uint8_t[debugFontHeapSize]);

    g_Writer.Initialize(
        &g_Device,
        info,
        g_DebugFontHeap.Get(),
        debugFontHeapSize,
        NULL,
        0,
        0
    );

    g_Writer.SetDisplayWidth(g_RenderWidth);
    g_Writer.SetDisplayHeight(g_RenderHeight);
    g_Writer.SetTextureDescriptor(&g_TextureDescriptorPool, g_TextureDescriptorBaseIndex + 3);
    g_Writer.SetSamplerDescriptor(&g_SamplerDescriptorPool, g_SamplerDescriptorBaseIndex);
    g_Writer.SetTextColor(nn::util::Color4u8::Black());
}

//---------------------------------------------------------------
// Free the Debug Font
//---------------------------------------------------------------
void FinalizeDebugFont()
{
    g_Writer.Finalize();
    delete[] reinterpret_cast<uint8_t*>(g_DebugFontHeap.Get());
    g_DebugFontHeap.Reset(nullptr);
}

//---------------------------------------------------------------
// Initialize the Load Meter
//---------------------------------------------------------------
#if defined( NN_BUILD_CONFIG_OS_WIN ) || defined( NN_BUILD_CONFIG_OS_LINUX ) || defined( NN_BUILD_CONFIG_OS_MACOS )
// The setting configured by nn::os::SetThreadCoreMask may not work in the Windows version,
// so always make the core number return 0.
int GetFixedCoreNumber()
{
    return 0;
}
#endif

void* meterMemory;
void InitializeLoadMeter()
{
    nn::perf::LoadMeterCenterInfo info;
    info.SetCoreCount(1);
    info.SetCpuBufferCount(2);
    info.SetGpuBufferCount(3);
    info.SetCpuSectionCountMax(64);
    info.SetGpuSectionCountMax(64);

    size_t memorySize = NN_PERF_GET_BUFFER_SIZE(info);
    size_t memoryAlignment = NN_PERF_GET_BUFFER_ALIGNMENT();
    meterMemory = AllocateFunction(memorySize, memoryAlignment, &g_PrimitiveRendererAllocator);
    size_t memoryPoolSize = NN_PERF_GET_MEMORY_POOL_SIZE(&g_Device, info);
    g_MemoryPoolOffset = nn::util::align_up(g_MemoryPoolOffset, NN_PERF_GET_MEMORY_POOL_ALIGNMENT(&g_Device, info));
    NN_PERF_INITIALIZE_METER(&g_Device, info,
        meterMemory, memorySize,
        &g_MemoryPool, g_MemoryPoolOffset, memoryPoolSize);
    g_MemoryPoolOffset += memoryPoolSize;
#if defined( NN_BUILD_CONFIG_OS_WIN ) || defined( NN_BUILD_CONFIG_OS_LINUX ) || defined( NN_BUILD_CONFIG_OS_MACOS )
    NN_PERF_SET_GET_CORE_NUMBER_FUNCTION(GetFixedCoreNumber);
#endif
}

//---------------------------------------------------------------
// Free the load meter.
//---------------------------------------------------------------
void FinalizeLoadMeter()
{
    NN_PERF_FINALIZE_METER(&g_Device);
    DeallocateFunction(meterMemory, &g_PrimitiveRendererAllocator);
}

//---------------------------------------------------------------
// Generate the Proprietary Vertex Buffer
//---------------------------------------------------------------
int CreateUserMesh(nns::gfx::PrimitiveRenderer::PrimitiveMesh* pMeshBuffer, bool bUseVertexColor, bool bUseVertexUv, bool bUseNormal)
{
    const float width = 0.5f;
    const float height = 0.5f;
    const float depth = 0.5f;

    // Set the index buffer.
    static const uint32_t FaceTable[6][4] =
    {
        { 0, 1, 2, 3 }, // Front
        { 3, 2, 6, 7 }, // Right
        { 7, 6, 5, 4 }, // Back
        { 4, 5, 1, 0 }, // Left
        { 1, 5, 6, 2 }, // Top
        { 4, 0, 3, 7 }, // Bottom
    };

    // Set the vertex buffer.
    const nn::util::Float3 VertexPos[8] =
    {
        NN_UTIL_FLOAT_3_INITIALIZER(width / 2.f,  height, -depth / 2.f),
        NN_UTIL_FLOAT_3_INITIALIZER(width, -height,       -depth),
        NN_UTIL_FLOAT_3_INITIALIZER(-width, -height,       -depth),
        NN_UTIL_FLOAT_3_INITIALIZER(-width / 2.f,  height, -depth / 2.f),
        NN_UTIL_FLOAT_3_INITIALIZER(width / 2.f,  height,  depth / 2.f),
        NN_UTIL_FLOAT_3_INITIALIZER(width, -height,        depth),
        NN_UTIL_FLOAT_3_INITIALIZER(-width, -height,        depth),
        NN_UTIL_FLOAT_3_INITIALIZER(-width / 2.f,  height,  depth / 2.f),
    };

    const nn::util::Float3 VertexNormal[6] =
    {
        NN_UTIL_FLOAT_3_INITIALIZER(0.0f,  0.0f, -1.0f),
        NN_UTIL_FLOAT_3_INITIALIZER(-1.0f,  0.0f,  0.0f),
        NN_UTIL_FLOAT_3_INITIALIZER(0.0f,  0.0f,  1.0f),
        NN_UTIL_FLOAT_3_INITIALIZER(1.0f,  0.0f,  0.0f),
        NN_UTIL_FLOAT_3_INITIALIZER(0.0f, -1.0f,  0.0f),
        NN_UTIL_FLOAT_3_INITIALIZER(0.0f,  1.0f,  0.0f)
    };


    static const nn::util::Float2 VertexUv[4] =
    {
        NN_UTIL_FLOAT_2_INITIALIZER(0.0f,  0.0f),
        NN_UTIL_FLOAT_2_INITIALIZER(0.0f,  1.0f),
        NN_UTIL_FLOAT_2_INITIALIZER(1.0f,  1.0f),
        NN_UTIL_FLOAT_2_INITIALIZER(1.0f,  0.0f),
    };

    static const nn::util::Float4 VertexColor[8] =
    {
        NN_UTIL_FLOAT_4_INITIALIZER(1.0f, 0.2f, 0.2f, 1.0f),
        NN_UTIL_FLOAT_4_INITIALIZER(0.2f, 1.0f, 0.2f, 1.0f),
        NN_UTIL_FLOAT_4_INITIALIZER(0.2f, 0.2f, 1.0f, 1.0f),
        NN_UTIL_FLOAT_4_INITIALIZER(1.0f, 0.2f, 0.2f, 1.0f),
        NN_UTIL_FLOAT_4_INITIALIZER(0.2f, 1.0f, 0.2f, 1.0f),
        NN_UTIL_FLOAT_4_INITIALIZER(0.2f, 0.2f, 1.0f, 1.0f),
        NN_UTIL_FLOAT_4_INITIALIZER(1.0f, 0.2f, 0.2f, 1.0f),
        NN_UTIL_FLOAT_4_INITIALIZER(0.2f, 1.0f, 0.2f, 1.0f),
    };

    const float xScale = 0.8f;
    const float yScale = 0.8f;
    const float zScale = 0.8f;
    uint32_t idxFace = 0;
    const int numIndices = 36;
    const int numVertices = 24;

    // Determine the vertex format.
    uint32_t format = nns::gfx::PrimitiveRenderer::VertexFormat_Pos;
    if (bUseVertexColor == true)
    {
        format |= nns::gfx::PrimitiveRenderer::VertexFormat_Color;
    }
    if (bUseVertexUv == true)
    {
        format |= nns::gfx::PrimitiveRenderer::VertexFormat_Uv;
    }

    if (bUseNormal == true)
    {
        format |= nns::gfx::PrimitiveRenderer::VertexFormat_Normal;
    }

    // Initialize the mesh.
    if (pMeshBuffer->Initialize(g_pPrimitiveRenderer->GetGpuBuffer(), numVertices, numIndices, static_cast<nns::gfx::PrimitiveRenderer::VertexFormat>(format)) == false)
    {
        return false;
    }


    // Set the index buffer.
    {
        size_t idx = 0;
        uint32_t* pIndexData = pMeshBuffer->GetIndexBufferCpuAddress();
        for (idxFace = 0; idxFace < 6; idxFace++)
        {
            pIndexData[idx++] = idxFace * 4 + 0;
            pIndexData[idx++] = idxFace * 4 + 1;
            pIndexData[idx++] = idxFace * 4 + 2;

            pIndexData[idx++] = idxFace * 4 + 0;
            pIndexData[idx++] = idxFace * 4 + 2;
            pIndexData[idx++] = idxFace * 4 + 3;
        }
    }

    size_t idxVertex = 0;
    idxFace = 0;

    {
        nn::util::Float3* pPos = static_cast<nn::util::Float3*>(pMeshBuffer->GetVertexBufferCpuAddress(nns::gfx::PrimitiveRenderer::VertexAttribute_Pos));

        // Vertex coordinates.
        int idxCorner = 0;
        for (idxFace = 0; idxFace < 6; idxFace++)
        {
            for (idxCorner = 0; idxCorner < 4; idxCorner++)
            {
                const size_t index = FaceTable[idxFace][idxCorner];

                // Position.
                pPos[idxVertex].x = VertexPos[index].x * xScale;
                pPos[idxVertex].y = VertexPos[index].y * yScale;
                pPos[idxVertex].z = VertexPos[index].z * zScale;
                idxVertex++;
            }
        }

        // Normal coordinates.
        if (bUseNormal)
        {
            nn::util::Float3* pNorm = static_cast<nn::util::Float3*>(pMeshBuffer->GetVertexBufferCpuAddress(nns::gfx::PrimitiveRenderer::VertexAttribute_Normal));

            idxCorner = 0;
            idxVertex = 0;
            for (idxFace = 0; idxFace < 6; idxFace++)
            {
                for (idxCorner = 0; idxCorner < 4; idxCorner++)
                {
                    // Position.
                    pNorm[idxVertex].x = VertexNormal[idxFace].x * xScale;
                    pNorm[idxVertex].y = VertexNormal[idxFace].y * yScale;
                    pNorm[idxVertex].z = VertexNormal[idxFace].z * zScale;
                    idxVertex++;
                }
            }
        }

        // When using vertex color.
        if (bUseVertexColor == true || bUseVertexUv == true)
        {
            nn::util::Float2* pUv = static_cast<nn::util::Float2*>(pMeshBuffer->GetVertexBufferCpuAddress(nns::gfx::PrimitiveRenderer::VertexAttribute_Uv));
            nn::util::Float4* pColor = static_cast<nn::util::Float4*>(pMeshBuffer->GetVertexBufferCpuAddress(nns::gfx::PrimitiveRenderer::VertexAttribute_Color));
            idxCorner = 0;
            idxVertex = 0;
            for (idxFace = 0; idxFace < 6; idxFace++)
            {
                for (idxCorner = 0; idxCorner < 4; idxCorner++)
                {
                    const size_t index = FaceTable[idxFace][idxCorner];

                    // Color.
                    if (pColor)
                    {
                        pColor[idxVertex].x = VertexColor[index].x;
                        pColor[idxVertex].y = VertexColor[index].y;
                        pColor[idxVertex].z = VertexColor[index].z;
                        pColor[idxVertex].w = VertexColor[index].w;
                    }

                    // UV
                    if (pUv)
                    {
                        pUv[idxVertex].x = VertexUv[idxCorner].x;
                        pUv[idxVertex].y = VertexUv[idxCorner].y;
                    }
                    idxVertex++;
                }
            }
        }
    }

    return numIndices;
} //NOLINT(impl/function_size)

//---------------------------------------------------------------
// Generate the commands.
//---------------------------------------------------------------
bool MakeCommand(int frame, int bufferIndex)
{
    NN_UNUSED(frame);

    NN_ASSERT(bufferIndex < NN_ARRAY_SIZE(g_CommandBuffer));
    nn::gfx::CommandBuffer* pCommandBuffer = &g_CommandBuffer[bufferIndex];
    pCommandBuffer->Reset();

    const int CommandMemorySize = 1024 * 1024 * 2;
#if defined( NN_SDK_BUILD_DEBUG ) && NN_GFX_IS_TARGET_NVN
    const int ControlMemorySize = 16 * 1024;
#else
    const int ControlMemorySize = 256;
#endif

    g_MemoryPoolOffset = nn::util::align_up(g_MemoryPoolOffset, nn::gfx::CommandBuffer::GetCommandMemoryAlignment(&g_Device));
    // nn::gfx::MemoryPoolProperty_CpuUncached for the one-time command buffer. | Using the nn::gfx::MemoryPoolProperty_GpuUncached memory pool is more appropriate.
    NN_ASSERT((g_MemoryPoolOffset + (NN_ARRAY_SIZE(g_CommandBuffer) * CommandMemorySize)) <= g_VisiblePoolMemorySize);
    ptrdiff_t commandMemoryOffset = g_MemoryPoolOffset + (bufferIndex * CommandMemorySize);
    pCommandBuffer->AddCommandMemory(&g_MemoryPool, commandMemoryOffset, CommandMemorySize);

    g_pMemory.AlignUp(256);
    NN_ASSERT(static_cast<size_t>(g_pMemoryHeap.Distance(g_pMemory.Get()) + ControlMemorySize) <= g_MemoryHeapSize);
    void* pControlMemoryBuffer = g_pMemory.Get();
    pCommandBuffer->AddControlMemory(pControlMemoryBuffer, ControlMemorySize);

    pCommandBuffer->Begin();
    NN_PERF_SET_COLOR_GPU(nn::util::Color4u8::Red());
    NN_PERF_BEGIN_MEASURE_GPU(pCommandBuffer);
    {
        nn::gfx::ColorTargetView* pTarget = &g_ColorTargetView;

        pCommandBuffer->InvalidateMemory(nn::gfx::GpuAccess_Descriptor | nn::gfx::GpuAccess_ShaderCode);

        pCommandBuffer->SetDescriptorPool(&g_BufferDescriptorPool);
        pCommandBuffer->SetDescriptorPool(&g_TextureDescriptorPool);
        pCommandBuffer->SetDescriptorPool(&g_SamplerDescriptorPool);

        pCommandBuffer->ClearColor(pTarget, 0.0f, 0.0f, 0.0f, 1.0f, NULL);
        pCommandBuffer->ClearDepthStencil(&g_DepthStencilView, 1.0f, 0,
            nn::gfx::DepthStencilClearMode_DepthStencil, nullptr);

        pCommandBuffer->SetRenderTargets(1, &pTarget, &g_DepthStencilView);
        pCommandBuffer->SetViewportScissorState(&g_ViewportScissor);
        pCommandBuffer->SetRasterizerState(&g_RasterizerState);

        pCommandBuffer->InvalidateMemory(nn::gfx::GpuAccess_Texture | nn::gfx::GpuAccess_IndexBuffer
            | nn::gfx::GpuAccess_ConstantBuffer | nn::gfx::GpuAccess_VertexBuffer);

        nn::gfx::DescriptorSlot constantBufferDescriptor;
        nn::gfx::DescriptorSlot textureDescriptor;
        nn::gfx::DescriptorSlot samplerDescriptor;
        g_BufferDescriptorPool.GetDescriptorSlot(&constantBufferDescriptor, g_BufferDescriptorBaseIndex);
        g_TextureDescriptorPool.GetDescriptorSlot(&textureDescriptor, g_TextureDescriptorBaseIndex);
        g_SamplerDescriptorPool.GetDescriptorSlot(&samplerDescriptor, g_SamplerDescriptorBaseIndex);


        // Update the primitive renderer.
        g_pPrimitiveRenderer->Update(bufferIndex);

        // Set the model, view, and projection matrices to the default.
        g_pPrimitiveRenderer->SetDefaultParameters();

        nn::util::Matrix4x3fType viewMatrix;
        nn::util::Matrix4x4fType projectionMatrix;
        nn::util::Matrix4x3f modelMatrix;
        nn::util::MatrixIdentity(&modelMatrix);
        nn::util::Vector3f translate;
        nn::util::VectorSet(&translate, 0.f, 0.f, 1.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);

        nn::util::Uint8x4 white = { { 255, 255, 255, 255 } };
        nn::util::Uint8x4 red = { { 255, 0, 0, 255 } };
        nn::util::Uint8x4 green = { { 0, 255, 0, 255 } };
        nn::util::Uint8x4 blue = { { 0, 0, 255, 255 } };


        // Blend.
        pCommandBuffer->SetBlendState(g_pPrimitiveRenderer->GetBlendState(nns::gfx::PrimitiveRenderer::BlendType::BlendType_Normal));

        // Depth enable.
        g_pPrimitiveRenderer->SetDepthStencilState(pCommandBuffer, nns::gfx::PrimitiveRenderer::DepthStencilType::DepthStencilType_DepthNoWriteTest);


        // Apply texture to the entire screen.
        nn::util::MatrixIdentity(&viewMatrix);
        nn::util::MatrixIdentity(&projectionMatrix);
        g_pPrimitiveRenderer->SetViewMatrix(&viewMatrix);
        g_pPrimitiveRenderer->SetProjectionMatrix(&projectionMatrix);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawScreenQuad(pCommandBuffer, textureDescriptor, samplerDescriptor);

        // Configure the view and projection.
        float radius = 20.f;
        float x = radius * sin(frame / 500.f);
        float z = radius * cos(frame / 500.f);

        nn::util::Vector3fType camPos = { x, 10.f, z };
        nn::util::Vector3fType camTarget = { 0.f, 0.f, 0.f };
        nn::util::Vector3fType camUp = { 0.f, 1.f, 0.f };
        nn::util::MatrixLookAtRightHanded(&viewMatrix, camPos, camTarget, camUp);
        g_pPrimitiveRenderer->SetViewMatrix(&viewMatrix);

        // Initialize the projection.
        const float fovy = nn::util::FloatPi / 3.0f;
        const float aspect = static_cast<float>(g_RenderWidth) / static_cast<float>(g_RenderHeight);
        nn::util::MatrixPerspectiveFieldOfViewRightHanded(&projectionMatrix, fovy, aspect, 0.1f, 1000.f);
        g_pPrimitiveRenderer->SetProjectionMatrix(&projectionMatrix);

        // Depth enable.
        g_pPrimitiveRenderer->SetDepthStencilState(pCommandBuffer, nns::gfx::PrimitiveRenderer::DepthStencilType::DepthStencilType_DepthWriteTest);

        // Draw the axes.
        float interval = -10.f;
        nn::util::Vector3fType begin;
        nn::util::Vector3fType end;
        g_pPrimitiveRenderer->SetLineWidth(1.f);
        for (int i = 0; i < 21; i++)
        {

            nn::util::VectorSet(&begin, -10.f, 0.f, interval);
            nn::util::VectorSet(&end, 10.f, 0.f, interval);
            g_pPrimitiveRenderer->SetColor(white);
            g_pPrimitiveRenderer->DrawLine(pCommandBuffer, begin, end);
            nn::util::VectorSet(&begin, interval, 0.f, -10.f);
            nn::util::VectorSet(&end, interval, 0.f, 10.f);
            g_pPrimitiveRenderer->DrawLine(pCommandBuffer, begin, end);
            interval += 1.0f;
        }

        g_pPrimitiveRenderer->SetLineWidth(5.f);
        nn::util::Vector3fType zeroVector;
        nn::util::VectorSet(&zeroVector, 0.f, 0.f, 0.f);
        nn::util::Vector3fType axisPos;
        nn::util::VectorSet(&axisPos, 5.f, 0.f, 0.f);
        g_pPrimitiveRenderer->SetColor(red);
        g_pPrimitiveRenderer->DrawLine(pCommandBuffer, zeroVector, axisPos);
        nn::util::VectorSet(&axisPos, 0.f, 0.f, 5.f);
        g_pPrimitiveRenderer->SetColor(blue);
        g_pPrimitiveRenderer->DrawLine(pCommandBuffer, zeroVector, axisPos);
        nn::util::VectorSet(&axisPos, 0.f, 5.f, 0.f);
        g_pPrimitiveRenderer->SetColor(green);
        g_pPrimitiveRenderer->DrawLine(pCommandBuffer, zeroVector, axisPos);


        nn::util::Vector3fType center = { 0.f, 0.f, 0.f };
        nn::util::Vector3fType size = { 1.f, 1.f, 1.f };

        nn::util::Vector3f vecZero;
        nn::util::VectorZero(&vecZero);

        float rotY = frame * 0.05f;
        nn::util::Vector3f rotValue;
        nn::util::VectorSet(&rotValue, 0.2f, rotY, 0.2f);
        nn::util::MatrixIdentity(&modelMatrix);
        nn::util::MatrixSetRotateXyz(&modelMatrix, rotValue);
        nn::util::MatrixSetAxisW(&modelMatrix, vecZero);

        // Draw QUADS.
        nn::util::VectorSet(&translate, 4.f, 0.f, -2.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetColor(red);
        g_pPrimitiveRenderer->DrawQuad(pCommandBuffer, center, size);

        nn::util::VectorSet(&translate, 8.f, 0.f, -2.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        // Set the multiplicative color to white.
        g_pPrimitiveRenderer->SetColor(white);
        g_pPrimitiveRenderer->DrawQuad(pCommandBuffer, center, size, textureDescriptor, samplerDescriptor);


        // Draw a triangle.
        nn::util::VectorSet(&translate, 2.f, 0.f, -4.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetLineWidth(1.f);
        g_pPrimitiveRenderer->SetColor(blue);
        g_pPrimitiveRenderer->DrawTriangle(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Wired,
            center, size);

        nn::util::VectorSet(&translate, 4.f, 0.f, -4.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawTriangle(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Solid,
            center, size);

        nn::util::VectorSet(&translate, 6.f, 0.f, -4.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawTriangle(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Normal,
            center, size);

        g_pPrimitiveRenderer->SetColor(white);
        nn::util::VectorSet(&translate, 8.f, 0.f, -4.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawTriangle(pCommandBuffer,
            center, size, textureDescriptor, samplerDescriptor);

        // Draw a circle.
        nn::util::VectorSet(&translate, 2.f, 0.f, -6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetColor(green);
        g_pPrimitiveRenderer->DrawCircle(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Wired,
            nns::gfx::PrimitiveRenderer::Subdiv::Subdiv_Normal,
            center, 1.f);

        nn::util::VectorSet(&translate, 4.f, 0.f, -6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetLineWidth(1.f);
        g_pPrimitiveRenderer->DrawCircle(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Solid,
            nns::gfx::PrimitiveRenderer::Subdiv::Subdiv_Normal,
            center, 1.f);

        nn::util::VectorSet(&translate, 6.f, 0.f, -6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetLineWidth(1.f);
        g_pPrimitiveRenderer->DrawCircle(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Normal,
            nns::gfx::PrimitiveRenderer::Subdiv::Subdiv_Normal,
            center, 1.f);

        g_pPrimitiveRenderer->SetColor(white);
        nn::util::VectorSet(&translate, 8.f, 0.f, -6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetLineWidth(1.f);
        g_pPrimitiveRenderer->DrawCircle(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Subdiv::Subdiv_Normal,
            center, 1.f, textureDescriptor, samplerDescriptor);


        // Draw a cube.
        nn::util::VectorSet(&translate, 2.f, 0.f, 9.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetLineWidth(1.f);
        g_pPrimitiveRenderer->SetColor(red);
        g_pPrimitiveRenderer->DrawCube(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Wired,
            center, size);

        nn::util::VectorSet(&translate, 4.f, 0.f, 9.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCube(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Solid,
            center, size);

        nn::util::VectorSet(&translate, 6.f, 0.f, 9.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCube(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Normal,
            center, size);

        g_pPrimitiveRenderer->SetColor(white);
        nn::util::VectorSet(&translate, 8.f, 0.f, 9.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCube(pCommandBuffer,
            center, size, textureDescriptor, samplerDescriptor);

        // Draw a sphere.
        nn::util::VectorSet(&translate, 2.f, 0.f, 3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetLineWidth(1.f);
        g_pPrimitiveRenderer->SetColor(green);
        g_pPrimitiveRenderer->DrawSphere(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Wired,
            nns::gfx::PrimitiveRenderer::Subdiv::Subdiv_Normal,
            center, 1.f);

        nn::util::VectorSet(&translate, 4.f, 0.f, 3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawSphere(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Solid,
            nns::gfx::PrimitiveRenderer::Subdiv::Subdiv_Normal,
            center, 1.f);

        nn::util::VectorSet(&translate, 6.f, 0.f, 3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawSphere(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Normal,
            nns::gfx::PrimitiveRenderer::Subdiv::Subdiv_Normal,
            center, 1.f);

        g_pPrimitiveRenderer->SetColor(white);
        nn::util::VectorSet(&translate, 8.f, 0.f, 3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawSphere(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Subdiv::Subdiv_Normal,
            center, 1.f, textureDescriptor, samplerDescriptor);

        // Draw a cone.
        nn::util::VectorSet(&translate, -2.f, 0.f, 3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetColor(blue);
        g_pPrimitiveRenderer->DrawCone(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Wired,
            center, 0.5, 1.0);

        nn::util::VectorSet(&translate, -4.f, 0.f, 3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCone(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Solid,
            center, 0.5, 1.0);

        nn::util::VectorSet(&translate, -6.f, 0.f, 3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCone(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Normal,
            center, 0.5, 1.0);

        g_pPrimitiveRenderer->SetColor(white);
        nn::util::VectorSet(&translate, -8.f, 0.f, 3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCone(pCommandBuffer,
            center, 0.5, 1.0, textureDescriptor, samplerDescriptor);

        // Draw a capsule.
        nn::util::VectorSet(&translate, 2.f, 0.f, 6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetColor(red);
        g_pPrimitiveRenderer->DrawCapsule(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Wired,
            center, 0.5, 1.f);

        nn::util::VectorSet(&translate, 4.f, 0.f, 6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCapsule(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Solid,
            center, 0.5, 1.f);

        nn::util::VectorSet(&translate, 6.f, 0.f, 6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCapsule(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Normal,
            center, 0.5, 1.f);

        g_pPrimitiveRenderer->SetColor(white);
        nn::util::VectorSet(&translate, 8.f, 0.f, 6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCapsule(pCommandBuffer,
            center, 0.5, 1.f, textureDescriptor, samplerDescriptor);

        // Draw a pipe.
        nn::util::VectorSet(&translate, -2.f, 0.f, 0.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetColor(red);
        g_pPrimitiveRenderer->DrawPipe(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Wired,
            center, 0.5, 1.f);

        nn::util::VectorSet(&translate, -4.f, 0.f, 0.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawPipe(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Solid,
            center, 0.5, 1.f);

        nn::util::VectorSet(&translate, -6.f, 0.f, 0.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawPipe(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Normal,
            center, 0.5, 1.f);

        g_pPrimitiveRenderer->SetColor(white);
        nn::util::VectorSet(&translate, -8.f, 0.f, 0.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawPipe(pCommandBuffer,
            center, 0.5, 1.f, textureDescriptor, samplerDescriptor);

        // Draw a cylinder.
        nn::util::VectorSet(&translate, -2.f, 0.f, -3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetColor(red);
        g_pPrimitiveRenderer->DrawCylinder(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Wired,
            center, 0.5, 1.f);

        nn::util::VectorSet(&translate, -4.f, 0.f, -3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCylinder(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Solid,
            center, 0.5, 1.f);

        nn::util::VectorSet(&translate, -6.f, 0.f, -3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCylinder(pCommandBuffer,
            nns::gfx::PrimitiveRenderer::Surface::Surface_Normal,
            center, 0.5, 1.f);

        g_pPrimitiveRenderer->SetColor(white);
        nn::util::VectorSet(&translate, -8.f, 0.f, -3.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawCylinder(pCommandBuffer,
            center, 0.5, 1.f, textureDescriptor, samplerDescriptor);


        // Rendering Using Proprietary Vertex Buffers
        nns::gfx::PrimitiveRenderer::PrimitiveMesh userMeshPos;        // Vertex.
        nns::gfx::PrimitiveRenderer::PrimitiveMesh userMeshPosUv;      // Vertex - texture coordinates.
        nns::gfx::PrimitiveRenderer::PrimitiveMesh userMeshPosColor;   // Vertex - vertex color.
        nns::gfx::PrimitiveRenderer::PrimitiveMesh userMeshPosUvColor; // Vertex - texture coordinates - vertex color.

        CreateUserMesh(&userMeshPos, false, false, false);
        CreateUserMesh(&userMeshPosUv, false, true, false);
        CreateUserMesh(&userMeshPosColor, true, false, false);
        CreateUserMesh(&userMeshPosUvColor, true, true, false);

        // When not using vertex color.
        nn::util::VectorSet(&translate, -2.f, 0.f, 9.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetColor(green);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawUserMesh(pCommandBuffer,
            nn::gfx::PrimitiveTopology::PrimitiveTopology_TriangleList, &userMeshPos);

        nn::util::VectorSet(&translate, -4.f, 0.f, 9.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetColor(white);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawUserMesh(pCommandBuffer,
            nn::gfx::PrimitiveTopology::PrimitiveTopology_TriangleList, &userMeshPosUv, textureDescriptor, samplerDescriptor);

        // When using vertex color.
        nn::util::VectorSet(&translate, -6.f, 0.f, 9.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetColor(white);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawUserMesh(pCommandBuffer,
            nn::gfx::PrimitiveTopology::PrimitiveTopology_TriangleList, &userMeshPosColor);

        nn::util::VectorSet(&translate, -8.f, 0.f, 9.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->DrawUserMesh(pCommandBuffer,
            nn::gfx::PrimitiveTopology::PrimitiveTopology_TriangleList, &userMeshPosUvColor, textureDescriptor, samplerDescriptor);

        nns::gfx::PrimitiveRenderer::PrimitiveMesh userMeshPosNormal;        // Vertex - normal.
        nns::gfx::PrimitiveRenderer::PrimitiveMesh userMeshPosUvNormal;      // Vertex - texture coordinates - normal.
        nns::gfx::PrimitiveRenderer::PrimitiveMesh userMeshPosColorNormal;   // Vertex - vertex color - normal.
        nns::gfx::PrimitiveRenderer::PrimitiveMesh userMeshPosUvColorNormal; // Vertex - texture coordinates - vertex color - normal.


        CreateUserMesh(&userMeshPosNormal, false, false, true);
        CreateUserMesh(&userMeshPosUvNormal, false, true, true);
        CreateUserMesh(&userMeshPosColorNormal, true, false, true);
        CreateUserMesh(&userMeshPosUvColorNormal, true, true, true);

        // When using the normal.
        nn::util::VectorSet(&translate, -2.f, 0.f, 6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetColor(green);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetUserPixelShader(g_pShadingWithoutTexShader);
        g_pPrimitiveRenderer->DrawUserMesh(pCommandBuffer,
            nn::gfx::PrimitiveTopology::PrimitiveTopology_TriangleList, &userMeshPosNormal);

        // When using texture coordinates and the normal.
        nn::util::VectorSet(&translate, -4.f, 0.f, 6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetColor(white);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetUserPixelShader(g_pShadingWithTexShader);
        g_pPrimitiveRenderer->DrawUserMesh(pCommandBuffer,
            nn::gfx::PrimitiveTopology::PrimitiveTopology_TriangleList, &userMeshPosUvNormal, textureDescriptor, samplerDescriptor);


        // When using vertex color and the normal.
        nn::util::VectorSet(&translate, -6.f, 0.f, 6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetColor(white);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetUserPixelShader(g_pShadingWithoutTexShader);
        g_pPrimitiveRenderer->DrawUserMesh(pCommandBuffer,
            nn::gfx::PrimitiveTopology::PrimitiveTopology_TriangleList, &userMeshPosColorNormal);

        // When using texture coordinates and vertex color and the normal.
        nn::util::VectorSet(&translate, -8.f, 0.f, 6.f);
        nn::util::MatrixSetAxisW(&modelMatrix, translate);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);
        g_pPrimitiveRenderer->SetUserPixelShader(g_pShadingWithTexShader);
        g_pPrimitiveRenderer->DrawUserMesh(pCommandBuffer,
            nn::gfx::PrimitiveTopology::PrimitiveTopology_TriangleList, &userMeshPosUvColorNormal, textureDescriptor, samplerDescriptor);

        g_pPrimitiveRenderer->SetUserPixelShader(nullptr);

        // 2D Rendering
        const float rectX = static_cast<float>(g_RenderWidth) / 32.f;
        const float rectY = rectX;
        const float rectWidth = static_cast<float>(g_RenderWidth) / 2.f;
        const float rectHeight = static_cast<float>(g_RenderWidth) / 2.f /
            g_pResTextureFile->GetResTexture(g_IndexText)->GetTextureInfo()->GetWidth() * g_pResTextureFile->GetResTexture(g_IndexText)->GetTextureInfo()->GetHeight();

        nn::gfx::DescriptorSlot rectDescriptor;
        g_TextureDescriptorPool.GetDescriptorSlot(&rectDescriptor, g_TextureDescriptorBaseIndex + 1);

        nn::util::MatrixIdentity(&viewMatrix);
        nn::util::MatrixIdentity(&projectionMatrix);
        nn::util::MatrixIdentity(&modelMatrix);
        g_pPrimitiveRenderer->SetViewMatrix(&viewMatrix);
        g_pPrimitiveRenderer->SetProjectionMatrix(&projectionMatrix);
        g_pPrimitiveRenderer->SetModelMatrix(&modelMatrix);

        // Depth disable.
        g_pPrimitiveRenderer->SetDepthStencilState(pCommandBuffer, nns::gfx::PrimitiveRenderer::DepthStencilType::DepthStencilType_DepthNoWriteTest);

        // Set the multiplicative color to white.
        g_pPrimitiveRenderer->SetColor(white);

        // 2D rectangle rendering.
        g_pPrimitiveRenderer->Draw2DRect(pCommandBuffer,
            rectX, rectY,
            rectWidth, rectHeight,
            rectDescriptor, samplerDescriptor);

        // Use the pixel shader entered by the user.
        // Configure the constant buffer defined by the user.
        nn::gfx::GpuAddress gpuAddress;
        const float textureWidth = static_cast<float>(g_pResTextureFile->GetResTexture(g_IndexText)->GetTextureInfo()->GetWidth());
        const float textureHeight = static_cast<float>(g_pResTextureFile->GetResTexture(g_IndexText)->GetTextureInfo()->GetHeight());
        float rate = 8 * sin(frame / 500.f);
        PixelSize pixelSize;
        pixelSize.u_pixel_size.x = 1.f / (static_cast<float>(g_RenderWidth) / rate);
        pixelSize.u_pixel_size.y = 1.f / (static_cast<float>(g_RenderWidth) / rate / textureWidth * textureHeight);

        g_pPrimitiveRenderer->SetUserConstantBuffer(&gpuAddress, &pixelSize, sizeof(PixelSize));
        pCommandBuffer->SetConstantBuffer(g_SlotPixelSize, nn::gfx::ShaderStage::ShaderStage_Pixel, gpuAddress, sizeof(PixelSize));

        // Set the pixel shader for mosaic processing.
        g_pPrimitiveRenderer->SetUserPixelShader(g_pMosaicShader);
        g_pPrimitiveRenderer->Draw2DRect(pCommandBuffer,
            rectX, rectY + rectHeight + 8,
            rectWidth, rectHeight,
            rectDescriptor, samplerDescriptor);

        // Draw a line on the screen.
        g_pPrimitiveRenderer->SetUserPixelShader(NULL);
        g_pPrimitiveRenderer->SetLineWidth(1.f);
        g_pPrimitiveRenderer->SetColor(green);
        g_pPrimitiveRenderer->Draw2DLine(pCommandBuffer, rectX, rectY, rectX + rectWidth, rectY);
        g_pPrimitiveRenderer->Draw2DLine(pCommandBuffer, rectX + rectWidth, rectY, rectX + rectWidth, rectY + 2.f * rectHeight + 8);
        g_pPrimitiveRenderer->Draw2DLine(pCommandBuffer, rectX + rectWidth, rectY + 2.f * rectHeight + 8, rectX, rectY + 2.f * rectHeight + 8);
        g_pPrimitiveRenderer->Draw2DLine(pCommandBuffer, rectX, rectY + 2.f * rectHeight + 8, rectX, rectY);

        //Draw the load meter.
        // If NN_PERF_IS_ENABLED() is false, do not render the meter.
        if (NN_STATIC_CONDITION(NN_PERF_IS_ENABLED()))
        {
            nn::perf::CpuMeter* pFrameMeter = NN_PERF_GET_FRAME_METER();
            nn::util::Float2 pos = NN_UTIL_FLOAT_2_INITIALIZER(32.f, g_RenderHeight - g_MeterDrawer.GetHeight(pFrameMeter) - g_MeterDrawer.GetBarHeight());
            g_MeterDrawer.SetDebugFontTextWriter(&g_Writer);
            g_MeterDrawer.SetPosition(pos);
            g_MeterDrawer.SetWidth(g_RenderWidth - 64.f);
            g_MeterDrawer.Draw(pCommandBuffer, g_pPrimitiveRenderer, pFrameMeter);
        }

        // Draw text.
        g_Writer.Draw(pCommandBuffer);

        // Use PrimitiveRenderer to copy to the scan buffer.
        pTarget = g_pScanBufferViews[bufferIndex];
        pCommandBuffer->SetRenderTargets(1, &pTarget, nullptr);

        // Flush the cache because the color buffer will be used as a texture.
        pCommandBuffer->FlushMemory(nn::gfx::GpuAccess_ColorBuffer);

        pCommandBuffer->InvalidateMemory(nn::gfx::GpuAccess_Texture);

        // Overwrite Settings
        pCommandBuffer->SetBlendState(g_pPrimitiveRenderer->GetBlendState(nns::gfx::PrimitiveRenderer::BlendType::BlendType_Opacity));

        nn::gfx::DescriptorSlot colorDescriptor;
        g_TextureDescriptorPool.GetDescriptorSlot(&colorDescriptor, g_TextureDescriptorBaseIndex + 2);
        g_pPrimitiveRenderer->SetColor(white);

#if NN_GFX_IS_TARGET_VK
        pCommandBuffer->SetTextureStateTransition(
            g_pScanBufferTextures[bufferIndex], nullptr,
            nn::gfx::TextureState_Undefined, 0,
            nn::gfx::TextureState_ColorTarget, nn::gfx::PipelineStageBit_RenderTarget);

        g_pPrimitiveRenderer->DrawScreenQuad(pCommandBuffer, colorDescriptor, samplerDescriptor);

        pCommandBuffer->SetTextureStateTransition(
            g_pScanBufferTextures[bufferIndex], nullptr,
            nn::gfx::TextureState_ColorTarget, nn::gfx::PipelineStageBit_RenderTarget,
            nn::gfx::TextureState_Present, 0);
#else
        g_pPrimitiveRenderer->DrawScreenQuadYFlip(pCommandBuffer, colorDescriptor, samplerDescriptor);
#endif
    }

    NN_PERF_END_MEASURE_GPU(pCommandBuffer);
    pCommandBuffer->FlushMemory(nn::gfx::GpuAccess_QueryBuffer);
    pCommandBuffer->End();

    return true;
} //NOLINT(impl/function_size)

//---------------------------------------------------------------
// Frame Processing
//---------------------------------------------------------------
bool ProcessFrame(int frame)
{
    // FrameworkMode: DeferredSubmission

    static int currentBufferIndex = 0;
    int previousBufferIndex = 1 - currentBufferIndex;

    if (frame > 0)
    {
        // ExecuteCommand(previousBufferIndex)
        NN_PERF_SET_COLOR(nn::util::Color4u8::Blue());
        NN_PERF_BEGIN_MEASURE_NAME("ExecuteCommand");
        {
            g_Queue.ExecuteCommand(&g_CommandBuffer[previousBufferIndex], &g_GpuFence[previousBufferIndex]);
        }
        NN_PERF_END_MEASURE();

        // QueuePresentTexture
        NN_PERF_SET_COLOR(nn::util::Color4u8::Yellow());
        NN_PERF_BEGIN_MEASURE_NAME("PresentTexture");
        {
            g_Queue.Present(&g_SwapChain, 1);
        }
        NN_PERF_END_MEASURE();
    }

    // AcquireTexture(currentBufferIndex)
    NN_PERF_SET_COLOR(nn::util::Color4u8::Blue());
    NN_PERF_BEGIN_MEASURE_NAME("AcquireTexture");
    {
        g_NextScanBufferIndex = -1;
        nn::gfx::AcquireScanBufferResult acquireResult = g_SwapChain.AcquireNextScanBufferIndex(
            &g_NextScanBufferIndex, &g_DisplaySemaphore[currentBufferIndex], &g_DisplayFence[currentBufferIndex]);
        NN_SDK_ASSERT(acquireResult == nn::gfx::AcquireScanBufferResult_Success);
        NN_UNUSED(acquireResult);
    }
    NN_PERF_END_MEASURE();

#if NN_GFX_IS_TARGET_VK
    g_Queue.SyncSemaphore(&g_DisplaySemaphore[currentBufferIndex]);
    g_Queue.Flush();
#endif

    // MakeCommand(currentBufferIndex)
    NN_PERF_SET_COLOR(nn::util::Color4u8::Green());
    NN_PERF_BEGIN_MEASURE_NAME("MakeCommand");
    if (MakeCommand(frame, currentBufferIndex) == false)
    {
        return false;
    }
    NN_PERF_END_MEASURE();

    // WaitDisplaySync(currentBufferIndex)
    g_DisplayFence[currentBufferIndex].Sync(nn::TimeSpan::FromSeconds(1));

    if (frame > 0)
    {
        // WaitGpuSync(previousBufferIndex)
        g_GpuFence[previousBufferIndex].Sync(nn::TimeSpan::FromSeconds(1));
    }

    currentBufferIndex = 1 - currentBufferIndex;
    return true;
}

// HID
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
    map.buttonA      = nn::hid::KeyboardKey::A::Index;
    map.buttonB      = nn::hid::KeyboardKey::B::Index;
    map.buttonX      = nn::hid::KeyboardKey::X::Index;
    map.buttonY      = nn::hid::KeyboardKey::Y::Index;
    map.buttonL      = nn::hid::KeyboardKey::L::Index;
    map.buttonR      = nn::hid::KeyboardKey::R::Index;
    map.buttonLeft   = nn::hid::KeyboardKey::LeftArrow::Index;
    map.buttonRight  = nn::hid::KeyboardKey::RightArrow::Index;
    map.buttonUp     = nn::hid::KeyboardKey::UpArrow::Index;
    map.buttonDown   = nn::hid::KeyboardKey::DownArrow::Index;
    map.buttonStart  = nn::hid::KeyboardKey::Space::Index;
    map.buttonSelect = nn::hid::KeyboardKey::Minus::Index;
    nn::settings::SetDebugPadKeyboardMap(map);
}

//---------------------------------------------------------------
//  Main Function
//  The main function.
//---------------------------------------------------------------
extern "C" void nnMain()
{
    /////////////////////////////////////////////////////////////////////////////
    // Files I used
    // 
    // HidNpadSimple
    // HidVibrationBasic
    // GfxPrimitiveRenderer
    // AudioEffect
    // 
    /////////////////////////////////////////////////////////////////////////////

    Circle cirA(0, 0, 1);
    Circle cirB(0, 2, 1);
    Circle cirC(0, 3, 1);

    Init();
    InitializeFs();
    InitializeResources();

    // gfxprimitive
    g_TextureDescriptorPool.BeginUpdate();
    {
        g_TextureDescriptorPool.SetTextureView(g_TextureDescriptorBaseIndex,
            g_pResTextureFile->GetResTexture(g_IndexBg)->GetTextureView());
        g_TextureDescriptorPool.SetTextureView(g_TextureDescriptorBaseIndex + 1,
            g_pResTextureFile->GetResTexture(g_IndexText)->GetTextureView());
        g_TextureDescriptorPool.SetTextureView(g_TextureDescriptorBaseIndex + 2,
            &g_ColorTextureView);
    }
    g_TextureDescriptorPool.EndUpdate();

    g_SamplerDescriptorPool.BeginUpdate();
    {
        g_SamplerDescriptorPool.SetSampler(g_SamplerDescriptorBaseIndex, &g_Sampler);
    }
    g_SamplerDescriptorPool.EndUpdate();

    // Initialize the primitive renderer.
    InitializePrimitiveRenderer();

    // Initialize the debug font.
    InitializeDebugFont();

    // Initialize the processing meter.
    InitializeLoadMeter();

    // Assign the current main thread to core 0.
    nn::os::SetThreadCoreMask(nn::os::GetCurrentThread(), 0, 1);

    ///////////////////////////////////////////////
    // AudioEffect
    ///////////////////////////////////////////////
    nn::mem::StandardAllocator allocator(g_HeapBuffer, sizeof(g_HeapBuffer));
    nn::audio::AudioOut audioOut;

    // Open audio output with the specified sampling rate and number of channels.
    // If the specified sampling rate is not supported and the process fails, the default sampling rate is used.
    nn::os::SystemEvent systemEvent;
    nn::audio::AudioOutParameter parameter;
    nn::audio::InitializeAudioOutParameter(&parameter);
    parameter.sampleRate = 48000;
    parameter.channelCount = 2;
    // parameter.channelCount = 6;  // For 5.1ch output, specify 6 for the number of channels.
    if (nn::audio::OpenDefaultAudioOut(&audioOut, &systemEvent, parameter).IsFailure())
    {
        parameter.sampleRate = 0;
        parameter.channelCount = 0;
        NN_ABORT_UNLESS(
            nn::audio::OpenDefaultAudioOut(&audioOut, &systemEvent, parameter).IsSuccess(),
            "Failed to open AudioOut."
        );
    }

    // Get the audio output properties.
    int channelCount = nn::audio::GetAudioOutChannelCount(&audioOut);
    int sampleRate = nn::audio::GetAudioOutSampleRate(&audioOut);
    nn::audio::SampleFormat sampleFormat = nn::audio::GetAudioOutSampleFormat(&audioOut);
    // This sample assumes that the sample format is 16-bit.
    NN_ASSERT(sampleFormat == nn::audio::SampleFormat_PcmInt16);

    // Prepare parameters for the buffer.
    const int frameRate = 20;                             // 20 fps
    const int frameSampleCount = sampleRate / frameRate;  // 50 milliseconds (in samples)
    const size_t dataSize = frameSampleCount * channelCount * nn::audio::GetSampleByteSize(sampleFormat);
    const size_t bufferSize = nn::util::align_up(dataSize, nn::audio::AudioOutBuffer::SizeGranularity);
    const int bufferCount = 4;
    const int amplitude = std::numeric_limits<int16_t>::max() / 16;

    nn::audio::AudioOutBuffer audioOutBuffer[bufferCount];
    void* outBuffer[bufferCount];
    for (int i = 0; i < bufferCount; ++i)
    {
        outBuffer[i] = allocator.Allocate(bufferSize, nn::audio::AudioOutBuffer::AddressAlignment);
        NN_ASSERT(outBuffer[i]);
        GenerateSquareWave(sampleFormat, outBuffer[i], channelCount, sampleRate, frameSampleCount, amplitude);
        nn::audio::SetAudioOutBufferInfo(&audioOutBuffer[i], outBuffer[i], bufferSize, dataSize);
        nn::audio::AppendAudioOutBuffer(&audioOut, &audioOutBuffer[i]);
    }

    // Start playback.
    NN_ABORT_UNLESS(
        nn::audio::StartAudioOut(&audioOut).IsSuccess(),
        "Failed to start playback."
    );
    // Audio end

    //////////////////////////////////////////////
    // HidVibrationBasic
    //////////////////////////////////////////////
    // Set the Npad devices to use. Comment out the Npad devices that are not used.
    // Set the style of operation to use. Comment out the styles that are not used.
    nn::hid::SetSupportedNpadStyleSet(
#if defined(NN_BUILD_TARGET_PLATFORM_NX)
        nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleHandheld::Mask | nn::hid::NpadStyleJoyDual::Mask |
        nn::hid::NpadStyleJoyLeft::Mask | nn::hid::NpadStyleJoyRight::Mask
#else
        nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleHandheld::Mask
#endif
    );
    const nn::hid::NpadIdType npadIds[] =
    {
        nn::hid::NpadId::No1,
        nn::hid::NpadId::No2,
        nn::hid::NpadId::No3,
        nn::hid::NpadId::No4,
        nn::hid::NpadId::Handheld
    };
    const char* npadMsg[] =
    {
        "No1", "No2", "No3", "No4", "Handheld"
    };

    nn::hid::SetSupportedNpadIdType(npadIds, NN_ARRAY_SIZE(npadIds));

    // Add a controller to use.
    std::vector<nns::hid::NpadController*> controllers;
    for (auto i = 0; i < NN_ARRAY_SIZE(npadIds); i++)
    {
        controllers.push_back(new nns::hid::NpadController(npadIds[i], npadMsg[i]));
    }
    // Exit if there are no controllers.
    NN_ASSERT(controllers.size() > 0);

    // Initialize controllers.
    for (auto it = controllers.begin();
        it != controllers.end();
        ++it)
    {
        (*it)->Initialize();
    }
    // Vibe End;


    // Draw each frame.
    for (int frame = 0; frame < 600000; ++frame)
    {
        for (int i = 0; i < NpadIdCountMax; i++)
        {
            controllers[i]->Update();
            ////////////////////////////////
            // HidNpadSimple
            // /////////////////////////////
            // A button was pressed.
            if (currentNpadJoyDualState[i].buttons.Test<nn::hid::NpadButton::A>())
            {
                if(frame > 2)
                    frame-= 2;
                // Display the Npad input state.
            }

            if (currentNpadJoyDualState[i].buttons.Test<nn::hid::NpadButton::B>())
            {
                ///////////////////////////////
                // AUDIO Update
                ///////////////////////////////
                // Get the buffer that completed playback.
                nn::audio::AudioOutBuffer* pAudioOutBuffer = nullptr;

                pAudioOutBuffer = nn::audio::GetReleasedAudioOutBuffer(&audioOut);
                while (pAudioOutBuffer)
                {
                    // Create square waveform data and register it again.
                    void* pOutBuffer = nn::audio::GetAudioOutBufferDataPointer(pAudioOutBuffer);
                    NN_ASSERT(nn::audio::GetAudioOutBufferDataSize(pAudioOutBuffer) == frameSampleCount * channelCount * nn::audio::GetSampleByteSize(sampleFormat));
                    GenerateSquareWave(sampleFormat, pOutBuffer, channelCount, sampleRate, frameSampleCount, amplitude);
                    nn::audio::AppendAudioOutBuffer(&audioOut, pAudioOutBuffer);

                    pAudioOutBuffer = nn::audio::GetReleasedAudioOutBuffer(&audioOut);
                }

            }
            if (currentNpadJoyDualState[i].buttons.Test<nn::hid::NpadButton::X>())
            {
                NN_LOG("Circle 1 and Circle 2");
                if(cirA.Collide(cirB))
                    NN_LOG("Collide!!!");
                else
                    NN_LOG("Not Collide");

                NN_LOG("Circle 2 and Circle 3");
                if (cirB.Collide(cirC))
                    NN_LOG("Collide!!!");
                else
                    NN_LOG("Not Collide");


                NN_LOG("Circle 1 and Circle 3");
                if (cirA.Collide(cirC))
                    NN_LOG("Collide!!!");
                else
                    NN_LOG("Not Collide");

            }
        }
        // HID Update
        Update();
        //GFX UPDATE
        NN_PERF_BEGIN_FRAME();
        {
            ProcessFrame(frame);
        }
        NN_PERF_END_FRAME();

        // Replacement for Vsync.
        nn::os::SleepThread(nn::TimeSpan::FromMilliSeconds(16));

    }
    g_Queue.Sync();

    // Free the processing meter.
    FinalizeLoadMeter();
    // Free the Debug Font
    FinalizeDebugFont();
    // Free the Primitive Renderer
    FinalizePrimitiveRenderer();
    FinalizeResources();
    FinalizeFs();

    // Audio
    NNS_LOG("Stop audio playback\n");

    // Stop playback.
    nn::audio::StopAudioOut(&audioOut);
    NNS_LOG("AudioOut is closed\n  State: %s\n", GetAudioOutStateName(nn::audio::GetAudioOutState(&audioOut)));

    // Close audio output.
    nn::audio::CloseAudioOut(&audioOut);
    nn::os::DestroySystemEvent(systemEvent.GetBase());

    // Free memory.
    for (int i = 0; i < bufferCount; ++i)
    {
        allocator.Free(outBuffer[i]);
    }
    // Audio end
}
