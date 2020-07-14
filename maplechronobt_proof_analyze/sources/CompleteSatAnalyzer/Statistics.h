/*
 * Statistics.h
 *
 *  Created on: 12.12.2019
 *      Author: hartung
 */

#ifndef CORE_STATISTICS_H_
#define CORE_STATISTICS_H_

#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>
#include <sstream>

template <typename T1, typename T2>
double average(const T1 a, const T2 b)
{
   return static_cast<double>(a) / static_cast<double>(b);
}

template <typename T1, typename T2>
double percent(const T1 a, const T2 b)
{
   return static_cast<double>(a) / static_cast<double>(b);
}

struct UIntStat
{
   static bool const printOnlyAverage = false;
   uint64_t n;
   uint64_t vmin;
   uint64_t vmax;
   uint64_t sum;
   double qsum;

   UIntStat()
         : n(0), vmin(std::numeric_limits<uint64_t>::max()), vmax(0), sum(0), qsum(0)
   {
   }

   template <typename T>
   void add(T const & v)
   {
      ++n;
      sum += v;
      qsum += v * v;
      if (vmax < v)
         vmax = v;
      if (vmin > v)
         vmin = v;
   }

   void add(UIntStat const & s)
   {
      n += s.n;
      sum += s.sum;
      qsum += s.qsum;
      if (vmax < s.vmax)
         vmax = s.vmax;
      if (vmin > s.vmin)
         vmin = s.vmin;
   }

   uint64_t min() const
   {
      return ((n == 0) ? 0 : vmin);
   }
   uint64_t max() const
   {
      return ((n == 0) ? 0 : vmax);
   }

   std::string toString() const
   {
      std::stringstream ss;
      if (printOnlyAverage)
         ss << ((n == 0) ? 0 : average());
      else
         ss << ((n == 0) ? 0 : average()) << " (" << min() << "," << max() << "," << deviation()
            << ")";
      return ss.str();
   }

   std::string toCsv(std::string const & sep = ",") const
   {
      if (printOnlyAverage)
         return std::to_string(average());
      else
      {
         std::stringstream ss;
         ss << average() << sep << min() << sep << max() << sep << deviation();
         return ss.str();
      }

   }
   static std::string csvHeader(std::string const & name, std::string const & sep = ",")
   {
      if (printOnlyAverage)
         return name;
      else
         return name + "_avg" + sep + name + "_min" + sep + name + "_max" + sep + name + "_dev";
   }

   double average() const
   {
      return static_cast<double>(sum) / n;
   }

   double deviation() const
   {
      if (n == 0)
         return 0;
      else
         return sqrt(
               (static_cast<double>(qsum)
                     - (static_cast<double>(sum) * static_cast<double>(sum)) / n) / n);
   }
};

#endif /* CORE_STATISTICS_H_ */
