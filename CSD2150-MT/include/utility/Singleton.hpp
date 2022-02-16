/*!*****************************************************************************
 * @file    Singleton.hpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    22 SEP 2021
 * @brief   This file provides the instance handling for the template class
 *
 * Copyright (C) 2021 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef SINGLETON_IMPLEMENTATION_H
#define SINGLETON_IMPLEMENTATION_H

#include <new>
#include <utility/Singleton.h>

template<typename T>
std::unique_ptr<T> Singleton<T>::instance{ nullptr };
// should be fine since header guards and all, plus only instantiated as needed.

template <typename T>
template <typename... constructionArgs>
T* Singleton<T>::createInstance(constructionArgs&&... cArgs)
{
    instance.reset(::new T{ std::forward<constructionArgs>(cArgs)... });
    return instance.get();
}

template<typename T>
T* Singleton<T>::getPInstance()
{
    return instance.get();
}

template<typename T>
void Singleton<T>::destroyInstance()
{   // deleting instance will call destructor, setting instance to nullptr
    instance.reset();
}

template<typename T>
Singleton<T>::~Singleton()
{
    
}

template<typename T>
Singleton<T>::Singleton()
{

}

#endif//SINGLETON_IMPLEMENTATION_H
