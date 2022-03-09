/*!*****************************************************************************
 * @file    inputHandler_windows.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    10 FEB 2022
 * @brief   This is the interface of the windows Input handler
 *
 *          modified version of my previously authored simpleInput system...
 *          simple input 2.1?
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef WINDOWS_INPUT_HELPER_HEADER
#define WINDOWS_INPUT_HELPER_HEADER

#include <utility/windowsInclude.h>
#include <bitset>

class windowsInput
{
public:

    using keyIdx_T = uint8_t;// making assumption that 0xFF is num VK keys

    windowsInput();

    /// @brief reset the input system, zeroing out all keystates
    void initialize() noexcept;

    /// @brief update the input system, setting the current frame keystates
    void update() noexcept;

    /// @brief check if a certain key was triggered this frame
    /// @param vkCode virtual key code to check
    /// @return true if the checked key was triggered
    bool isTriggered(keyIdx_T vkCode) noexcept;

    /// @brief check if a certain key was pressed this frame
    /// @param vkCode virtual key code to check
    /// @return true if the checked key was pressed
    bool isPressed(keyIdx_T vkCode) noexcept;

    /// @brief check if a certain key was released this frame
    /// @param vkCode virtual key code to check
    /// @return true if the checked key was released
    bool isReleased(keyIdx_T vkCode) noexcept;


    /// @brief check if any key was triggered this frame
    /// @return true if something was triggered
    bool anyTriggered() noexcept;

    /// @brief check if any key was released this frame
    /// @return true if something was pressed
    bool anyPressed() noexcept;

    /// @brief check if any key was released this frame
    /// @return true if something was released
    bool anyReleased() noexcept;

    /// @brief get the number of full scrolls
    /// @return how many multiples of WHEEL_DELTA the scroll wheel has travelled
    int getScrollSteps() noexcept;

    /// @brief get the the number of scroll steps with any extra partial steps
    /// @return how many multiples of WHEEL_DELTA the scroll wheel has travelled
    float getScrollFine() noexcept;


    /// @brief debug print (most keys, not all)
    /// @param flagRTP 0b0001 Triggered, 
    ///                0b0010 Pressed, 
    ///                0b0100 Released,
    ///                0b1000 CursorPos
    void debugPrint(size_t flagRPT) noexcept;

    /// @brief set virtual key trigger state to true.
    ///        making assumption that 0xFF is num VK keys
    /// @param vkCode code to set trigger to true
    void setVKTrigger(keyIdx_T vkCode) noexcept;

    /// @brief set virtual key release state to true.
    ///        making assumption that 0xFF is num VK keys
    /// @param vkCode code to set release to true
    void setVKRelease(keyIdx_T vkCode) noexcept;

    /// @brief Add scroll amount to accumulate.
    ///        The amount added is usually in multiples of WHEEL_DELTA
    /// @param scrollAmt scroll distance, +ve is forwards, -ve is backwards
    void addMouseScroll(short scrollAmt) noexcept;

    /// @brief update the cursor position from windows message
    /// @param cX X position relative to upper-left corner of the client area
    /// @param cY Y position relative to upper-left corner of the client area
    void updateCursorPos(int cX, int cY) noexcept;

    /// @brief get the current frame cursor position
    /// @param cX output for X
    /// @param cY output for Y
    void getCursorPos(int& cX, int& cY) noexcept;

private:

    // various unused/reserved/niche keys but whatever
    static constexpr size_t NUM_VK_KEYS{ 0xFF };

    using KSContainer = std::bitset<NUM_VK_KEYS>;   // Key State Container

    KSContainer keystatesTriggeredA; // 1 for triggered
    KSContainer keystatesTriggeredB; // 0 otherwise
    KSContainer keystatesPressed;
    KSContainer keystatesReleasedA;  // 0 for released
    KSContainer keystatesReleasedB;  // 1 otherwise

    KSContainer* pTriggeredAccumulate{ &keystatesTriggeredA };
    KSContainer* pTriggeredCurrent{ &keystatesTriggeredB };
    KSContainer* pReleasedAccumulate{ &keystatesReleasedA };
    KSContainer* pReleasedCurrent{ &keystatesReleasedB };

    int scrollAmountA;
    int scrollAmountB;

    int* pScrollAccumulate{ &scrollAmountA };
    int* pScrollCurrent{ &scrollAmountB };

    int cursorXAccumulate;
    int cursorYAccumulate;
    int cursorXCurrent;
    int cursorYCurrent;

};

// define more VKs to keep calling convention the same (VK prefix)

/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x3A - 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */

 // assume VK_0 to VK_9 are not defined if VK_0 is not
#ifndef VK_0
#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#endif // !VK_0

// assume VK_A to VK_Z are not defined if VK_A is not
#ifndef VK_A
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A
#endif // !VK_A

#endif//WINDOWS_INPUT_HELPER_HEADER
