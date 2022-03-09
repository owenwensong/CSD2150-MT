/*!*****************************************************************************
 * @file    Timer.h
 * @author  Owen Huang Wensong  (w.huang@digipen.edu)
 * @date    15 SEP 2021
 * @brief   This file contains the interface for the timer class
 *
 * Copyright (C) 2021 DigiPen Institute of Technology. All rights reserved.
*******************************************************************************/

#ifndef OKP_TIMER_H
#define OKP_TIMER_H

#include <chrono>

namespace MTU
{
  class Timer
  {
  public:
    // decided to use system clock since steady clock resolution 
    // didn't matter if it kept updating in multiples of 100
    using clocktype = std::chrono::system_clock;
    using tp = clocktype::time_point;

    Timer();

    Timer(tp const& _forcedStartTime);

    /// @brief sets the timer start point and clears any existing elapsed
    void start();

    /// @brief sets the timer start point but keeps any existing elapsed
    void resume();

    /// @brief technically doesn't stop, sets the timer start point and 
    /// updates elapsed. For continuous timing, keep calling stop(), 
    /// this will make the function act more like lapping
    void stop();

    /// @brief frequency of the clock used. reciprocal gives Hz.
    static constexpr uint64_t clockFrequency{ clocktype::period::den };

    /// @brief return the time point of the time this function is called
    /// @return current time according to the clock
    static tp getCurrentTP();

    /// @brief gets the time point from the last time any of the three 
    /// functions (start, resume, stop) were called
    /// @return the time point in question
    tp const& getLastUpdatedTimePoint();

    /// @brief get the elapsed time in timer clock cycles
    /// @return timer clocks elapsed
    uint64_t getElapsedCount();

  private:

    tp tpStart;
    tp::duration elapsed;
  };
}

#endif//OKP_TIMER_H
