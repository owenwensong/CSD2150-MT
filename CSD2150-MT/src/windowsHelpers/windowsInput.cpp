/*!*****************************************************************************
 * @file    inputHandler_windows.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    10 FEB 2022
 * @brief   This is the implementation of the windows Input handler
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <windowsHelpers/windowsInput.h>

windowsInput::windowsInput() : 
    keystatesTriggeredA{ /* default initialized */ },
    keystatesTriggeredB{ /* default initialized */ },
    keystatesPressed{ /* default initialized */ },
    keystatesReleasedA{ /* default initialized */ },
    keystatesReleasedB{ /* default initialized */ },
    pTriggeredAccumulate{ &keystatesTriggeredA },
    pTriggeredCurrent{ &keystatesTriggeredB },
    pReleasedAccumulate{ &keystatesReleasedA },
    pReleasedCurrent{ &keystatesReleasedB },
    scrollAmountA{ 0 },
    scrollAmountB{ 0 },
    pScrollAccumulate{ &scrollAmountA },
    pScrollCurrent{ &scrollAmountB },
    cursorXAccumulate{ 0 },
    cursorYAccumulate{ 0 },
    cursorXCurrent{ 0 },
    cursorYCurrent{ 0 }
{
    keystatesReleasedA.set();
    keystatesReleasedB.set();
}

void windowsInput::initialize() noexcept
{
    // Key State Reset
    keystatesTriggeredA.reset();
    keystatesTriggeredB.reset();
    keystatesPressed.reset();
    keystatesReleasedA.set();
    keystatesReleasedB.set();

    // Mouse State Reset
    scrollAmountA = scrollAmountB = 0;
}

void windowsInput::update() noexcept
{
    // Key State updates
    std::swap(pTriggeredAccumulate, pTriggeredCurrent);
    std::swap(pReleasedAccumulate, pReleasedCurrent);
    pTriggeredAccumulate->reset();
    pReleasedAccumulate->set();     // reversed bit representation
    keystatesPressed |= *pTriggeredCurrent; // OR to set pressed
    keystatesPressed &= *pReleasedCurrent;  // AND to unset (0 means released)

    // Mouse State updates
    std::swap(pScrollAccumulate, pScrollCurrent);
    *pScrollAccumulate = 0;
    cursorXCurrent = cursorXAccumulate;
    cursorYCurrent = cursorYAccumulate;

}

// *****************************************************************************
// ***************************************************************** CHECKS ****

bool windowsInput::isTriggered(keyIdx_T vkCode) noexcept
{
    return pTriggeredCurrent->test(vkCode);
}

bool windowsInput::isPressed(keyIdx_T vkCode) noexcept
{
    return keystatesPressed.test(vkCode);
}

bool windowsInput::isReleased(keyIdx_T vkCode) noexcept
{
    return !pReleasedCurrent->test(vkCode); // bit 0 means released
}

bool windowsInput::anyTriggered() noexcept
{
    return pTriggeredAccumulate->any();
}

bool windowsInput::anyPressed() noexcept
{
    return keystatesPressed.any();
}

bool windowsInput::anyReleased() noexcept
{
    return pReleasedAccumulate->any();
}

int windowsInput::getScrollSteps() noexcept
{
    return *pScrollCurrent / WHEEL_DELTA;
}

float windowsInput::getScrollFine() noexcept
{
    return static_cast<float>(*pScrollCurrent) / WHEEL_DELTA;
}

// *****************************************************************************
// ****************************************************************** DEBUG ****

void windowsInput::debugPrint(size_t flagRPT) noexcept
{
    // DO NOT PUT ANYTHING AFTER THE BACKSLASH!!! IMPORTANT PREPROCESSOR STUFF
#define MY_IH_TMP_DEBUG_MACRO(vkc) \
if (flagRPT & 0b001 && isTriggered(vkc))printf_s("%s TRIGGERED\n", #vkc);\
if (flagRPT & 0b010 && isPressed(vkc))printf_s("%s PRESSED\n", #vkc);\
if (flagRPT & 0b100 && isReleased(vkc))printf_s("%s RELEASED\n", #vkc)
    MY_IH_TMP_DEBUG_MACRO(VK_LBUTTON);
    MY_IH_TMP_DEBUG_MACRO(VK_RBUTTON);
    MY_IH_TMP_DEBUG_MACRO(VK_MBUTTON);
    MY_IH_TMP_DEBUG_MACRO(VK_XBUTTON1);
    MY_IH_TMP_DEBUG_MACRO(VK_XBUTTON2);

    MY_IH_TMP_DEBUG_MACRO(VK_BACK);
    MY_IH_TMP_DEBUG_MACRO(VK_TAB);

    MY_IH_TMP_DEBUG_MACRO(VK_CLEAR);
    MY_IH_TMP_DEBUG_MACRO(VK_RETURN);

    MY_IH_TMP_DEBUG_MACRO(VK_SHIFT);
    MY_IH_TMP_DEBUG_MACRO(VK_CONTROL);
    MY_IH_TMP_DEBUG_MACRO(VK_MENU);
    MY_IH_TMP_DEBUG_MACRO(VK_PAUSE);
    MY_IH_TMP_DEBUG_MACRO(VK_CAPITAL);

    //MY_IH_TMP_DEBUG_MACRO(VK_KANA   );
    //MY_IH_TMP_DEBUG_MACRO(VK_HANGEUL);
    //MY_IH_TMP_DEBUG_MACRO(VK_HANGUL );
    //MY_IH_TMP_DEBUG_MACRO(VK_IME_ON );
    //MY_IH_TMP_DEBUG_MACRO(VK_JUNJA  );
    //MY_IH_TMP_DEBUG_MACRO(VK_FINAL  );
    //MY_IH_TMP_DEBUG_MACRO(VK_HANJA  );
    //MY_IH_TMP_DEBUG_MACRO(VK_KANJI  );
    //MY_IH_TMP_DEBUG_MACRO(VK_IME_OFF);

    MY_IH_TMP_DEBUG_MACRO(VK_ESCAPE);

    //MY_IH_TMP_DEBUG_MACRO(VK_CONVERT   );
    //MY_IH_TMP_DEBUG_MACRO(VK_NONCONVERT);
    //MY_IH_TMP_DEBUG_MACRO(VK_ACCEPT    );
    //MY_IH_TMP_DEBUG_MACRO(VK_MODECHANGE);

    MY_IH_TMP_DEBUG_MACRO(VK_SPACE);
    MY_IH_TMP_DEBUG_MACRO(VK_PRIOR);
    MY_IH_TMP_DEBUG_MACRO(VK_NEXT);
    MY_IH_TMP_DEBUG_MACRO(VK_END);
    MY_IH_TMP_DEBUG_MACRO(VK_HOME);
    MY_IH_TMP_DEBUG_MACRO(VK_LEFT);
    MY_IH_TMP_DEBUG_MACRO(VK_UP);
    MY_IH_TMP_DEBUG_MACRO(VK_RIGHT);
    MY_IH_TMP_DEBUG_MACRO(VK_DOWN);
    MY_IH_TMP_DEBUG_MACRO(VK_SELECT);
    MY_IH_TMP_DEBUG_MACRO(VK_PRINT);
    MY_IH_TMP_DEBUG_MACRO(VK_EXECUTE);
    MY_IH_TMP_DEBUG_MACRO(VK_SNAPSHOT);
    MY_IH_TMP_DEBUG_MACRO(VK_INSERT);
    MY_IH_TMP_DEBUG_MACRO(VK_DELETE);
    MY_IH_TMP_DEBUG_MACRO(VK_HELP);

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

    MY_IH_TMP_DEBUG_MACRO(VK_LWIN);
    MY_IH_TMP_DEBUG_MACRO(VK_RWIN);
    MY_IH_TMP_DEBUG_MACRO(VK_APPS);

    MY_IH_TMP_DEBUG_MACRO(VK_SLEEP);// LOL

    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD0);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD1);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD2);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD3);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD4);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD5);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD6);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD7);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD8);
    MY_IH_TMP_DEBUG_MACRO(VK_NUMPAD9);
    MY_IH_TMP_DEBUG_MACRO(VK_MULTIPLY);
    MY_IH_TMP_DEBUG_MACRO(VK_ADD);
    MY_IH_TMP_DEBUG_MACRO(VK_SEPARATOR);
    MY_IH_TMP_DEBUG_MACRO(VK_SUBTRACT);
    MY_IH_TMP_DEBUG_MACRO(VK_DECIMAL);
    MY_IH_TMP_DEBUG_MACRO(VK_DIVIDE);
    MY_IH_TMP_DEBUG_MACRO(VK_F1);
    MY_IH_TMP_DEBUG_MACRO(VK_F2);
    MY_IH_TMP_DEBUG_MACRO(VK_F3);
    MY_IH_TMP_DEBUG_MACRO(VK_F4);
    MY_IH_TMP_DEBUG_MACRO(VK_F5);
    MY_IH_TMP_DEBUG_MACRO(VK_F6);
    MY_IH_TMP_DEBUG_MACRO(VK_F7);
    MY_IH_TMP_DEBUG_MACRO(VK_F8);
    MY_IH_TMP_DEBUG_MACRO(VK_F9);
    MY_IH_TMP_DEBUG_MACRO(VK_F10);
    MY_IH_TMP_DEBUG_MACRO(VK_F11);
    MY_IH_TMP_DEBUG_MACRO(VK_F12);
    MY_IH_TMP_DEBUG_MACRO(VK_F13);
    MY_IH_TMP_DEBUG_MACRO(VK_F14);
    MY_IH_TMP_DEBUG_MACRO(VK_F15);
    MY_IH_TMP_DEBUG_MACRO(VK_F16);
    MY_IH_TMP_DEBUG_MACRO(VK_F17);
    MY_IH_TMP_DEBUG_MACRO(VK_F18);
    MY_IH_TMP_DEBUG_MACRO(VK_F19);
    MY_IH_TMP_DEBUG_MACRO(VK_F20);
    MY_IH_TMP_DEBUG_MACRO(VK_F21);
    MY_IH_TMP_DEBUG_MACRO(VK_F22);
    MY_IH_TMP_DEBUG_MACRO(VK_F23);
    MY_IH_TMP_DEBUG_MACRO(VK_F24);

    MY_IH_TMP_DEBUG_MACRO(VK_NAVIGATION_VIEW);
    MY_IH_TMP_DEBUG_MACRO(VK_NAVIGATION_MENU);
    MY_IH_TMP_DEBUG_MACRO(VK_NAVIGATION_UP);
    MY_IH_TMP_DEBUG_MACRO(VK_NAVIGATION_DOWN);
    MY_IH_TMP_DEBUG_MACRO(VK_NAVIGATION_LEFT);
    MY_IH_TMP_DEBUG_MACRO(VK_NAVIGATION_RIGHT);
    MY_IH_TMP_DEBUG_MACRO(VK_NAVIGATION_ACCEPT);
    MY_IH_TMP_DEBUG_MACRO(VK_NAVIGATION_CANCEL);

    MY_IH_TMP_DEBUG_MACRO(VK_NUMLOCK);
    MY_IH_TMP_DEBUG_MACRO(VK_SCROLL);

    MY_IH_TMP_DEBUG_MACRO(VK_OEM_NEC_EQUAL);

    //MY_IH_TMP_DEBUG_MACRO(VK_LSHIFT  );
    //MY_IH_TMP_DEBUG_MACRO(VK_RSHIFT  );
    //MY_IH_TMP_DEBUG_MACRO(VK_LCONTROL);
    //MY_IH_TMP_DEBUG_MACRO(VK_RCONTROL);
    //MY_IH_TMP_DEBUG_MACRO(VK_LMENU   );
    //MY_IH_TMP_DEBUG_MACRO(VK_RMENU   );

    //MY_IH_TMP_DEBUG_MACRO(VK_BROWSER_BACK     );
    //MY_IH_TMP_DEBUG_MACRO(VK_BROWSER_FORWARD  );
    //MY_IH_TMP_DEBUG_MACRO(VK_BROWSER_REFRESH  );
    //MY_IH_TMP_DEBUG_MACRO(VK_BROWSER_STOP     );
    //MY_IH_TMP_DEBUG_MACRO(VK_BROWSER_SEARCH   );
    //MY_IH_TMP_DEBUG_MACRO(VK_BROWSER_FAVORITES);
    //MY_IH_TMP_DEBUG_MACRO(VK_BROWSER_HOME     );

    MY_IH_TMP_DEBUG_MACRO(VK_VOLUME_MUTE);
    MY_IH_TMP_DEBUG_MACRO(VK_VOLUME_DOWN);
    MY_IH_TMP_DEBUG_MACRO(VK_VOLUME_UP);
    MY_IH_TMP_DEBUG_MACRO(VK_MEDIA_NEXT_TRACK);
    MY_IH_TMP_DEBUG_MACRO(VK_MEDIA_PREV_TRACK);
    MY_IH_TMP_DEBUG_MACRO(VK_MEDIA_STOP);
    MY_IH_TMP_DEBUG_MACRO(VK_MEDIA_PLAY_PAUSE);
    MY_IH_TMP_DEBUG_MACRO(VK_LAUNCH_MAIL);
    MY_IH_TMP_DEBUG_MACRO(VK_LAUNCH_MEDIA_SELECT);
    MY_IH_TMP_DEBUG_MACRO(VK_LAUNCH_APP1);
    MY_IH_TMP_DEBUG_MACRO(VK_LAUNCH_APP2);

    MY_IH_TMP_DEBUG_MACRO(VK_OEM_1);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_PLUS);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_COMMA);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_MINUS);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_PERIOD);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_2);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_3);

    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_A);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_B);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_X);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_Y);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_RIGHT_SHOULDER);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_LEFT_SHOULDER);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_LEFT_TRIGGER);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_RIGHT_TRIGGER);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_DPAD_UP);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_DPAD_DOWN);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_DPAD_LEFT);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_DPAD_RIGHT);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_MENU);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_VIEW);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_LEFT_THUMBSTICK_UP);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_RIGHT_THUMBSTICK_UP);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT);
    MY_IH_TMP_DEBUG_MACRO(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT);

    MY_IH_TMP_DEBUG_MACRO(VK_OEM_4);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_5);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_6);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_7);
    MY_IH_TMP_DEBUG_MACRO(VK_OEM_8);

    MY_IH_TMP_DEBUG_MACRO(VK_TAB);
#undef MY_IH_TMP_DEBUG_MACRO

    if (int scrAmt{ getScrollSteps() }; scrAmt)
    {
        printf_s("SCROLL: %d\n", scrAmt);
    }
    if (flagRPT & 0b1000)
    {
        printf_s("CURSORX: %d\nCURSORY: %d\n", cursorXCurrent, cursorYCurrent);
    }
}

// *****************************************************************************
// ******************************************************************* SETS ****

void windowsInput::setVKTrigger(keyIdx_T vkCode) noexcept
{
    pTriggeredAccumulate->set(vkCode);
}

void windowsInput::setVKRelease(keyIdx_T vkCode) noexcept
{
    pReleasedAccumulate->reset(vkCode);// bit 0 means released
}

void windowsInput::addMouseScroll(short scrollAmt) noexcept
{
    *pScrollAccumulate += scrollAmt;
}

void windowsInput::updateCursorPos(int cX, int cY) noexcept
{
    cursorXAccumulate = /*static_cast<decltype(cursorXAccumulate)>(*/cX/*)*/;
    cursorYAccumulate = /*static_cast<decltype(cursorYAccumulate)>(*/cY/*)*/;
}

void windowsInput::getCursorPos(int& cX, int& cY) noexcept
{
  cX = cursorXCurrent;
  cY = cursorYCurrent;
}

// *****************************************************************************

