/*!*****************************************************************************
 * @file    Timer.cpp
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 SEP 2021
 * @brief   This file contains the interface for the perf class
 *
 * Copyright (C) 2021 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#include <utility/Timer.h>

MTU::Timer::Timer() :
  tpStart{ /* default is epoch? */ },
  elapsed{ 0 }
{

}

MTU::Timer::Timer(MTU::Timer::tp const& _forcedStartTime) :
  tpStart{ _forcedStartTime },
  elapsed{ 0 }
{

}

void MTU::Timer::start()
{
  tpStart = clocktype::now();
  elapsed = tp::duration(0);
}

void MTU::Timer::resume()
{
  tpStart = clocktype::now();
}

void MTU::Timer::stop()
{
  tp prev{ std::move(tpStart) }; // move previous tp into a temp var
  tpStart = clocktype::now();
  elapsed += tpStart - prev;
}

MTU::Timer::tp MTU::Timer::getCurrentTP()
{
  return clocktype::now();
}

MTU::Timer::tp const& MTU::Timer::getLastUpdatedTimePoint()
{
  return tpStart;
}

uint64_t MTU::Timer::getElapsedCount()
{
  return elapsed.count();
}
