/*!*****************************************************************************
 * @file    lockableObject.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 FEB 2022
 * @brief   This file contains the template and necessary includes for 
 *          lockable objects, following the xgpu examples.
 *
 * @par Copyright (C) 2022 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef UTILITY_LOCKABLE_OBJECT_HEADER_SOURCE
#define UTILITY_LOCKABLE_OBJECT_HEADER_SOURCE

#include <cassert>
#include <mutex>

// allow debugging proper flow control before turning it off entirely in release
#if defined(DEBUG) || defined(_DEBUG)
#define LOCKABLE_DEBUG_ONLY(A) A
#else
#define LOCKABLE_DEBUG_ONLY(A)
#endif//DEBUG

// although default assert only runs if NDEBUG is false, I will wrap it with 
// LOCKABLE_DEBUG_ONLY in case it gets changed with other kinds of assert later.

template <typename T>
class lockableObject
{
public:

    template <typename... constructArgs>
    lockableObject(constructArgs&&... cArgs);

    T&          get()       noexcept;
    T const&    get()       const noexcept;
    void        lock()      noexcept;
    void        unlock()    noexcept;
    bool        try_lock()  noexcept;
    
private:

    std::mutex  m_Mutex{};
    T           m_Value;    // unlike xgpu, this must be constructed by user
    LOCKABLE_DEBUG_ONLY(bool m_Locked{ false });

};

template <typename T>
template <typename... constructArgs>
lockableObject<T>::lockableObject(constructArgs&&... cArgs) : 
    m_Value{ std::forward<constructArgs>(cArgs)... }
{
    /* Empty constructor body */
}

template <typename T>
T& lockableObject<T>::get() noexcept
{
    LOCKABLE_DEBUG_ONLY(assert(m_Locked));// assumes user will lock before get
    return m_Value;
}

template <typename T>
T const& lockableObject<T>::get() const noexcept
{
    LOCKABLE_DEBUG_ONLY(assert(m_Locked));// assumes user will lock before get
    return m_Value;
}

template <typename T>
void lockableObject<T>::lock() noexcept
{
    m_Mutex.lock();
    LOCKABLE_DEBUG_ONLY(assert(!m_Locked));
    LOCKABLE_DEBUG_ONLY(m_Locked = true);
}

template <typename T>
void lockableObject<T>::unlock() noexcept
{
    LOCKABLE_DEBUG_ONLY(assert(m_Locked));
    LOCKABLE_DEBUG_ONLY(m_Locked = false);
#pragma warning (suppress : 26110)
    m_Mutex.unlock();   // weird warning C26110. Sounds like a false positive to me from std::scoped_lock but I can't be sure.
}

template <typename T>
bool lockableObject<T>::try_lock() noexcept
{
    if (m_Mutex.try_lock())
    {
        LOCKABLE_DEBUG_ONLY(assert(!m_Locked));
        LOCKABLE_DEBUG_ONLY(m_Locked = true);
        return true;
    }
    return false;
}

#endif//UTILITY_LOCKABLE_OBJECT_HEADER_SOURCE
