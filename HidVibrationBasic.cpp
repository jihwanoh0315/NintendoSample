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
    @examplesource{HidVibrationBasic.cpp,PageSampleHidVibrationBasic}

    @brief
    Sample program to control vibration motors in a simple way.
*/

/**
    @page PageSampleHidVibrationBasic  Simple Vibration Motor Controls Sample
    @tableofcontents

    @brief
    Description of sample program to control vibration motors in a simple way.

    @section PageSampleHidVibrationBasic_SectionBrief  Overview
    Describes a method for controlling the vibration motors in a controller.

    @section PageSampleHidVibrationBasic_SectionFileStructure  File Structure
    This sample program is located in @link Samples/Sources/Applications/HidVibrationBasic @endlink .

    @section PageSampleHidVibrationBasic_SectionNecessaryEnvironment  Required Environment
    When running in a Windows environment, connect the Bluetooth dongle to the computer ahead of time, and pair the controller to the computer.
    When running in an SDEV or EDEV environment, pair the controller to the SDEV or EDEV ahead of time.
    At least one controller must be connected.

    @section PageSampleHidVibrationBasic_SectionHowToOperate  How to Use
    Run the sample program and press any of the controller buttons.
    After the Bluetooth connection is made, the LED on the controller will stop flashing and will remain lit.

    After connecting, a vibration value is sent to the vibration motor while a controller button is pressed, and the controller vibrates.
    The right vibration motor will vibrate when the A, B, X, Y, L, or ZL Button is pressed. The left vibration motor will vibrate when the up, down, left, right, R, or ZR Button is pressed.
    The sent vibration value varies according to the number of buttons simultaneously pressed on each controller.
    Vibration only occurs on controllers where buttons are pressed. No vibration occurs on a controller where no buttons are pressed.
    - Zero buttons: No vibration.
    - One button: Quickly enables and disables the vibration.
    - Two buttons: Generates a low-frequency vibration with a changing amplitude.
    - Three buttons: Generates a high-frequency vibration with the amplitude gradually changing.
    - Four buttons: Generates a buzz using two different frequencies.
    - Five buttons: Generates a low-frequency vibration where the amplitude attenuates.
    - More than five: Generates a high-frequency vibration where the amplitude attenuates.

    The values of the actually generated vibrations are displayed on the screen.
    For more information about cases where the vibration values configured with the <tt>nn::hid::SendVibrationValue</tt> function do not match the values obtained with the <tt>nn::hid::GetActualVibrationValue</tt> function,
    see the description of the <tt>nn::hid::GetActualVibrationValue</tt> function in the NintendoSDK Documents API reference.

    To end the sample program, press the + Button and the - Button at the same time.

    @section PageSampleHidVibrationBasic_SectionPrecaution  Cautions
    Make sure that the controller is sufficiently charged before using it.

    @section PageSampleHidVibrationBasic_SectionHowToExecute  Execution Procedure
    Build the sample program and then run it.

    @section PageSampleHidVibrationBasic_SectionDetail  Description
    This sample program has the following flow.
    - Initialize <tt>NpadID</tt>.
    - Get the vibration motor's handle from <tt>NpadID</tt>.
    - Get the vibration motor information.
    - Send the vibration value to the vibration motor.
    - Get the actual vibration values occurring on the vibration motor.
*/

#include <cstdlib>

#include <nn/nn_Assert.h>
#include <nn/nn_Log.h>
#include <nn/gfx/util/gfx_DebugFontTextWriter.h>

#include <nns/gfx/gfx_GraphicsFramework.h>
#include <nns/gfx/gfx_PrimitiveRenderer.h>

#if defined(NN_BUILD_TARGET_PLATFORM_NX)
#include <nv/nv_MemoryManagement.h>
#endif

#if defined(NN_BUILD_TARGET_PLATFORM_OS_WIN)
#include <nn/nn_Windows.h>
#endif
#include "NpadController.h"

namespace
{
    const auto FrameRate = 60;
    const auto ProgramName = "HidVibrationBasic";


    nn::mem::StandardAllocator* g_pAppAllocator;
    nn::Bit8* g_pAppMemory;

    void InitializeGraphics() NN_NOEXCEPT
    {
        // Memory
        g_pAppAllocator = new nn::mem::StandardAllocator();
        const size_t appMemorySize = 128 * 1024 * 1024;
        g_pAppMemory = new nn::Bit8[appMemorySize];
        g_pAppAllocator->Initialize(g_pAppMemory, appMemorySize);

#if defined(NN_BUILD_TARGET_PLATFORM_NX)
        const size_t graphicsMemorySize = 256 * 1024 * 1024;
        void* pGraphicsMemory = nns::gfx::GraphicsFramework::DefaultAllocateFunction(graphicsMemorySize, 1, nullptr);
        nv::SetGraphicsAllocator(nns::gfx::GraphicsFramework::DefaultAllocateFunction, nns::gfx::GraphicsFramework::DefaultFreeFunction, nns::gfx::GraphicsFramework::DefaultReallocateFunction, nullptr);
        nv::SetGraphicsDevtoolsAllocator(nns::gfx::GraphicsFramework::DefaultAllocateFunction, nns::gfx::GraphicsFramework::DefaultFreeFunction, nns::gfx::GraphicsFramework::DefaultReallocateFunction, nullptr);
        nv::InitializeGraphics(pGraphicsMemory, graphicsMemorySize);
#endif
        // Graphics
    }
} // Anonymous namespace.

extern "C" void ddnnMain()
{
    NN_LOG("%s Start.\n", ProgramName);

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

    auto isQuitRequired = false;

    while (!isQuitRequired)
    {
        for (auto it = controllers.begin();
            it != controllers.end();
            ++it)
        {
            (*it)->Update();

            if ((*it)->IsQuitRequired())
            {
                isQuitRequired = true;
            }
        }

    }

    NN_LOG("%s Done\n", ProgramName);
}
