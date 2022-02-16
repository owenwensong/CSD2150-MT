/*!*****************************************************************************
 * @file    Singleton.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    22 SEP 2021
 * @brief   This file provides a base class template Singleton to which other 
 *          classes can derive from it's own templated version of this class
 * 
 *          This is a modified version of my GAM implementation, implementing 
 *          the ostrich algorithm. Since I will be the one using this class, 
 *          I will forego the usual foolproofing. 
 * 
 *          Removed SBPtrClass in favour of unique_ptr
 * 
 * Copyright (C) 2021 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>   // unique_ptr

/// @brief A class designed to make creation of singleton systems easier. 
/// IMPORTANT TO NOTE WHEN INHERITING FROM THIS BASE CLASS:
/// private default constructor must exist, and
/// add the following to the class/struct's private section
/// private:
///     friend class Singleton;
///     ClassName& operator=(ClassName const&) = delete;
///     ClassName(ClassName const&) = delete;
/// @tparam T inheriting class that is to become a singleton type object
template<typename T>
class Singleton
{
public:

    /// @brief create new instance of derived object with required arguments
    /// @tparam ...constructionArgs construction arguments passed to constructor
    /// @return pointer to created instance
    template<typename... constructionArgs>
    static T* createInstance(constructionArgs&&...);

    /// @brief get pointer to existing derived instance
    /// @return pointer to derived instance, nullptr if not created.
    static T* getPInstance();

    /// @brief destroy the instance
    static void destroyInstance();

    virtual ~Singleton();
    // destructor works as expected without the virtual, but this makes sure 
    // the pointer calls it's derived class's destructor on the off chance
    // someone does a Singleton* delete

protected:

    Singleton();

private:

    static std::unique_ptr<T> instance;

};

#include <utility/Singleton.hpp>

#endif//SINGLETON_H
