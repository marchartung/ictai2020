/*
 * Timer.hpp
 *
 *  Created on: 13.07.2020
 *      Author: hartung
 */

#ifndef SOURCES_PTB_TIMER_HPP_
#define SOURCES_PTB_TIMER_HPP_


#include <chrono>
#include <cstdint>

namespace PTB
{

class Timer
{
 public:
   Timer()
         : startT(getTime())
   {

   }

   Timer(const double & timerInterval)
         : startT(getTime())
   {

   }

   void reset()
   {
      startT = getTime();
   }

   double getSeconds() const
   {
      return getTime() - startT;
   }


 private:
   double startT;

   double getTime() const
   {
      int64_t milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      return ((double) milli) / 1000.0;
   }
};

}


#endif /* SOURCES_PTB_TIMER_HPP_ */
