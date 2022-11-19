/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

#include <nn/nn_Assert.h>
#include <nn/nn_Log.h>

#include "NpadController.h"

namespace nns { namespace hid {

namespace
{
    // Buttons on the left side of the controller.
    const auto LeftSideButtonMask =
        nn::hid::NpadButton::Up::Mask | nn::hid::NpadButton::Right::Mask | nn::hid::NpadButton::Down::Mask |
        nn::hid::NpadButton::Left::Mask | nn::hid::NpadButton::L::Mask | nn::hid::NpadButton::ZL::Mask;

    // Buttons on the right side of the controller.
    const auto RightSideButtonMask =
        nn::hid::NpadButton::A::Mask | nn::hid::NpadButton::B::Mask | nn::hid::NpadButton::X::Mask |
        nn::hid::NpadButton::Y::Mask | nn::hid::NpadButton::R::Mask | nn::hid::NpadButton::ZR::Mask;

    // Quit when both the + Button and - Button are pressed at the same time.
    const auto QuitButtonMask =
        nn::hid::NpadButton::Plus::Mask | nn::hid::NpadButton::Minus::Mask;
}

void NpadController::Initialize() NN_NOEXCEPT
{
    // Initialize VibrationState.
    for (auto i = 0; i < VibrationDeviceCountMax; i++)
    {
        auto& v = m_VibrationStateArray[i];
        v.deviceHandle = {};
        v.vibrationPatternId = 0;
        v.deviceInfo = {};
        v.currentVibrationValue = nn::hid::VibrationValue::Make();
        v.actualVibrationValue = nn::hid::VibrationValue::Make();
    }

    // A Style must be set before getting the handle.
    NN_ASSERT(m_Style.IsAnyOn());

    // Get the vibration motor's handle.
    nn::hid::VibrationDeviceHandle handles[VibrationDeviceCountMax];
    m_VibrationDeviceCount = nn::hid::GetVibrationDeviceHandles(
        handles, VibrationDeviceCountMax, m_NpadId, m_Style);

    for (auto i = 0; i < m_VibrationDeviceCount; i++)
    {
        auto& v = m_VibrationStateArray[i];
        v.deviceHandle = handles[i];

        // Initialize the vibration motor.
        nn::hid::InitializeVibrationDevice(v.deviceHandle);

        // Get vibration motor information.
        nn::hid::GetVibrationDeviceInfo(&v.deviceInfo, v.deviceHandle);

        NN_LOG("InitializeVibrationDevice %d/%d DeviceType=%s Position=%s\n",
            i, m_VibrationDeviceCount,
            (v.deviceInfo.deviceType == nn::hid::VibrationDeviceType_LinearResonantActuator) ? "LinearResonantActuator" : "Unknown",
            (v.deviceInfo.position == nn::hid::VibrationDevicePosition_Left) ? "Left" :
            (v.deviceInfo.position == nn::hid::VibrationDevicePosition_Right) ? "Right" : "Unknown");
    }
}

void NpadController::Update() NN_NOEXCEPT
{
    UpdateNpadState();
    UpdateVibrationPattern();
    UpdateVibrationValue();
    m_LoopCount++;
}

bool NpadController::IsQuitRequired() const NN_NOEXCEPT
{
    return ((m_Buttons & QuitButtonMask) == QuitButtonMask);
}

const char* GetStyle(const nn::hid::NpadStyleSet& style) NN_NOEXCEPT
{
    if (style.Test<nn::hid::NpadStyleFullKey>())
    {
        return "NpadFullKeyState";
    }
    else if (style.Test<nn::hid::NpadStyleHandheld>())
    {
        return "NpadHandheldState";
    }
#if defined(NN_BUILD_TARGET_PLATFORM_NX)
    else if (style.Test<nn::hid::NpadStyleJoyDual>())
    {
        return "NpadJoyDualState";
    }
    else if (style.Test<nn::hid::NpadStyleJoyLeft>())
    {
        return "NpadStyleJoyLeft";
    }
    else if (style.Test<nn::hid::NpadStyleJoyRight>())
    {
        return "NpadStyleJoyRight";
    }
#endif
    else
    {
        return "Unknown";
    }
}

const char* NpadController::GetCurrentStyleName() const NN_NOEXCEPT
{
    return GetStyle(m_Style);
}

void NpadController::UpdateNpadState() NN_NOEXCEPT
{
    const auto& style = GetStyleSet();

    if (style.IsAllOff())
    {
        // Controller disconnected.
        if (m_IsConnected)
        {
            NN_LOG("%s is disconnected.\n", GetName());
            ResetButton();
            m_IsConnected = false;
        }
    }
    else
    {
        // Controller connected.
        if (m_IsConnected == false)
        {
            NN_LOG("%s(%s) is connected.\n", GetName(), GetStyle(style));
            m_IsConnected = true;
        }

        // If the connection style of the controller changes, the vibration handle is initialized again.
        if (m_Style != style)
        {
            NN_LOG("%s NpadStyleSet is changed from %s to %s.\n", GetName(), GetCurrentStyleName(), GetStyle(style));
            // Save the connection style.
            m_Style = style;
            Initialize();
        }

        if (style.Test<nn::hid::NpadStyleFullKey>())
        {
            nn::hid::NpadFullKeyState state;
            nn::hid::GetNpadState(&state, m_NpadId);
            UpdateButton(state.buttons);
        }
        else if (style.Test<nn::hid::NpadStyleHandheld>())
        {
            nn::hid::NpadHandheldState state;
            nn::hid::GetNpadState(&state, m_NpadId);
            UpdateButton(state.buttons);
        }
#if defined(NN_BUILD_TARGET_PLATFORM_NX)
        else if (style.Test<nn::hid::NpadStyleJoyDual>())
        {
            nn::hid::NpadJoyDualState state;
            nn::hid::GetNpadState(&state, m_NpadId);
            UpdateButton(state.buttons);
        }
        else if (style.Test<nn::hid::NpadStyleJoyLeft>())
        {
            nn::hid::NpadJoyLeftState state;
            nn::hid::GetNpadState(&state, m_NpadId);
            UpdateButton(state.buttons);
        }
        else if (style.Test<nn::hid::NpadStyleJoyRight>())
        {
            nn::hid::NpadJoyRightState state;
            nn::hid::GetNpadState(&state, m_NpadId);
            UpdateButton(state.buttons);
        }
#endif
        else
        {
            NN_ASSERT("Unknown Style");
        }
    }
}

void NpadController::UpdateButton(const nn::hid::NpadButtonSet& buttons) NN_NOEXCEPT
{
    m_Buttons = buttons;
}

void NpadController::ResetButton() NN_NOEXCEPT
{
    m_Buttons.Reset();
}

void NpadController::UpdateVibrationPattern() NN_NOEXCEPT
{
    // Update the vibration pattern along with the number of buttons pressed.
    for (auto i = 0; i < m_VibrationDeviceCount; i++)
    {
        auto& v = m_VibrationStateArray[i];
        switch (v.deviceInfo.position)
        {
        case nn::hid::VibrationDevicePosition_Left:
            v.vibrationPatternId = (m_Buttons & LeftSideButtonMask).CountPopulation();
            break;
        case nn::hid::VibrationDevicePosition_Right:
            v.vibrationPatternId = (m_Buttons & RightSideButtonMask).CountPopulation();
            break;
        default:
            v.vibrationPatternId = 0;
            break;
        }
    }
}

void NpadController::StopVibration() NN_NOEXCEPT
{
    // Send the default vibration value to stop the vibration.
    for (auto i = 0; i < m_VibrationDeviceCount; i++)
    {
        const auto& v = m_VibrationStateArray[i];
        const auto& vib = nn::hid::VibrationValue::Make();
        nn::hid::SendVibrationValue(v.deviceHandle, vib);
    }
}

void NpadController::UpdateVibrationValue() NN_NOEXCEPT
{
    for (int i = 0; i < m_VibrationDeviceCount; i++)
    {
        auto& v = m_VibrationStateArray[i];

        // Generate a vibration value that corresponds to the vibration pattern.
        auto vib = nn::hid::VibrationValue::Make();
        switch (v.vibrationPatternId)
        {
        case 0:
            // No vibrations are triggered.
            break;
        case 1:
            // Quickly enable and disable the vibration.
            vib.amplitudeLow = (m_LoopCount % 3 < 1) ? 0.6f : 0.0f;
            vib.frequencyLow = 180.0f;
            break;
        case 2:
            // Generate a low-frequency vibration with a changing amplitude.
            vib.amplitudeLow = (m_LoopCount % 15 < 10) ? 0.3f : 0.05f;
            vib.frequencyLow = 160.0f;
            break;
        case 3:
            // Generate a high-frequency vibration with a slowly changing amplitude.
            vib.amplitudeLow = (m_LoopCount % 30 < 25) ? 0.4f : 0.05f;
            vib.frequencyLow = 300.0f;
            break;
        case 4:
            // Generate a buzz using two different frequencies.
            vib.amplitudeLow = 0.50f;
            vib.frequencyLow = 150.0f;
            vib.amplitudeHigh = 0.50f;
            vib.frequencyHigh = 158.0f;
            break;
        default:
            // Generate a high-frequency vibration where the amplitude attenuates.
            break;
        }

        // Send the vibration value. No error even if the controller is not connected.
        nn::hid::SendVibrationValue(v.deviceHandle, vib);

        // Save the vibration command value.
        v.currentVibrationValue = vib;

        // Get the current vibration. No error even if the controller is not connected.
        nn::hid::GetActualVibrationValue(&v.actualVibrationValue, v.deviceHandle);
    }
}

}}
