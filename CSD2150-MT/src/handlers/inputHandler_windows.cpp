/*!*****************************************************************************
 * @file    inputHandler_windows.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    10 FEB 2022
 * @brief   This is the implementation for the inputHandler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <handlers/inputHandler_windows.h>
#include <bitset>

namespace inputHandler
{
    // various unused/reserved/niche keys but whatever
    static constexpr size_t NUM_VK_KEYS{ 0xFF };
    
    using KSContainer = std::bitset<NUM_VK_KEYS>;   // Key State Container

    static KSContainer keystatesTriggeredA;
    static KSContainer keystatesTriggeredB;
    static KSContainer keystatesPressed;
    static KSContainer keystatesReleasedA;
    static KSContainer keystatesReleasedB;

    static KSContainer* pTriggeredAccumulate{ &keystatesTriggeredA };
    static KSContainer* pTriggeredCurrent{ &keystatesTriggeredB };
    static KSContainer* pReleasedAccumulate{ &keystatesReleasedA };
    static KSContainer* pReleasedCurrent{ &keystatesReleasedB };

}

// *****************************************************************************
// ******************************************************** STATE FUNCTIONS ****

void inputHandler::initialize() noexcept
{
    // Key State Reset
    keystatesTriggeredA.reset();
    keystatesTriggeredB.reset();
    keystatesPressed.reset();
    keystatesReleasedA.reset();
    keystatesReleasedB.reset();
}

void inputHandler::update() noexcept
{
    // Key State updates
    std::swap(pTriggeredAccumulate, pTriggeredCurrent);
    std::swap(pReleasedAccumulate, pReleasedCurrent);
    pTriggeredAccumulate->reset();
    pReleasedAccumulate->reset();
    keystatesPressed |= *pTriggeredCurrent; // OR to set pressed
    keystatesPressed ^= *pReleasedCurrent;  // XOR to unset (experimental)


}

// *****************************************************************************
// ***************************************************************** CHECKS ****

bool inputHandler::isTriggered(keyIdx_T vkCode) noexcept
{
    return pTriggeredCurrent->test(vkCode);
}

bool inputHandler::isPressed(keyIdx_T vkCode) noexcept
{
    return keystatesPressed.test(vkCode);
}

bool inputHandler::isReleased(keyIdx_T vkCode) noexcept
{
    return pReleasedCurrent->test(vkCode);
}

bool inputHandler::anyTriggered() noexcept
{
    return pTriggeredAccumulate->any();
}

bool inputHandler::anyPressed() noexcept
{
    return keystatesPressed.any();
}

bool inputHandler::anyReleased() noexcept
{
    return pReleasedAccumulate->any();
}

// *****************************************************************************
// ****************************************************************** DEBUG ****

void inputHandler::debugPrint(size_t flagRPT) noexcept
{
// DO NOT PUT ANYTHING AFTER THE BACKSLASH!!! IMPORTANT PREPROCESSOR STUFF
#define MY_IH_TMP_DEBUG_MACRO(vkc) \
if (flagRPT & 0x001 && inputHandler::isTriggered(vkc))printf_s("%s TRIGGERED\n", #vkc);\
if (flagRPT & 0x010 && inputHandler::isPressed(vkc))printf_s("%s PRESSED\n", #vkc);\
if (flagRPT & 0x100 && inputHandler::isReleased(vkc))printf_s("%s RELEASED\n", #vkc)
    MY_IH_TMP_DEBUG_MACRO(VK_LBUTTON);
    MY_IH_TMP_DEBUG_MACRO(VK_RBUTTON);
    MY_IH_TMP_DEBUG_MACRO(VK_MBUTTON);
    MY_IH_TMP_DEBUG_MACRO(VK_XBUTTON1);
    MY_IH_TMP_DEBUG_MACRO(VK_XBUTTON2);

    MY_IH_TMP_DEBUG_MACRO(VK_A);
    MY_IH_TMP_DEBUG_MACRO(VK_B);
    MY_IH_TMP_DEBUG_MACRO(VK_C);
    MY_IH_TMP_DEBUG_MACRO(VK_D);
    MY_IH_TMP_DEBUG_MACRO(VK_E);
    MY_IH_TMP_DEBUG_MACRO(VK_F);
    MY_IH_TMP_DEBUG_MACRO(VK_G);
    MY_IH_TMP_DEBUG_MACRO(VK_H);
    MY_IH_TMP_DEBUG_MACRO(VK_I);
    MY_IH_TMP_DEBUG_MACRO(VK_J);
    MY_IH_TMP_DEBUG_MACRO(VK_K);
    MY_IH_TMP_DEBUG_MACRO(VK_L);
    MY_IH_TMP_DEBUG_MACRO(VK_M);
    MY_IH_TMP_DEBUG_MACRO(VK_N);
    MY_IH_TMP_DEBUG_MACRO(VK_O);
    MY_IH_TMP_DEBUG_MACRO(VK_P);
    MY_IH_TMP_DEBUG_MACRO(VK_Q);
    MY_IH_TMP_DEBUG_MACRO(VK_R);
    MY_IH_TMP_DEBUG_MACRO(VK_S);
    MY_IH_TMP_DEBUG_MACRO(VK_T);
    MY_IH_TMP_DEBUG_MACRO(VK_U);
    MY_IH_TMP_DEBUG_MACRO(VK_V);
    MY_IH_TMP_DEBUG_MACRO(VK_W);
    MY_IH_TMP_DEBUG_MACRO(VK_X);
    MY_IH_TMP_DEBUG_MACRO(VK_Y);
    MY_IH_TMP_DEBUG_MACRO(VK_Z);

    MY_IH_TMP_DEBUG_MACRO(VK_0);
    MY_IH_TMP_DEBUG_MACRO(VK_1);
    MY_IH_TMP_DEBUG_MACRO(VK_2);
    MY_IH_TMP_DEBUG_MACRO(VK_3);
    MY_IH_TMP_DEBUG_MACRO(VK_4);
    MY_IH_TMP_DEBUG_MACRO(VK_5);
    MY_IH_TMP_DEBUG_MACRO(VK_6);
    MY_IH_TMP_DEBUG_MACRO(VK_7);
    MY_IH_TMP_DEBUG_MACRO(VK_8);
    MY_IH_TMP_DEBUG_MACRO(VK_9);
#undef MY_IH_TMP_DEBUG_MACRO
}

// *****************************************************************************
// ******************************************************************* SETS ****

void inputHandler::setVKTrigger(keyIdx_T vkCode) noexcept
{
    pTriggeredAccumulate->set(vkCode);
}

void inputHandler::setVKRelease(keyIdx_T vkCode) noexcept
{
    pReleasedAccumulate->set(vkCode);
}

// *****************************************************************************
