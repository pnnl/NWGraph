/**
 * @file timer.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *
 */

#ifndef NW_UTIL_TIMER_HPP
#define NW_UTIL_TIMER_HPP

#include <chrono>
#include <iostream>

namespace nw {
namespace util {


template <class D = std::chrono::microseconds>
class timer {
private:
  typedef std::chrono::time_point<std::chrono::system_clock> time_t;

public:
  explicit timer(const std::string& msg = "") : start_time(std::chrono::system_clock::now()), stop_time(start_time), msg_(msg) { }

  time_t start()         { return (start_time = std::chrono::system_clock::now()); }
  time_t stop()          { return (stop_time  = std::chrono::system_clock::now()); }
  double elapsed() const { return std::chrono::duration_cast<D>(stop_time - start_time).count(); }
  double lap()           { stop(); return std::chrono::duration_cast<D>(stop_time - start_time).count(); }


  std::string name() const { return msg_; }

private:
  time_t start_time, stop_time;

protected:
  std::string msg_;
};


using seconds_timer = timer<std::chrono::seconds>;
using ms_timer = timer<std::chrono::milliseconds>;
using us_timer = timer<std::chrono::microseconds>;

class empty_timer {
public:
  empty_timer(const std::string& msg = "") {}
  ~empty_timer() {}
};

class life_timer :  public empty_timer, public ms_timer {

public:
  explicit life_timer(const std::string& msg = "") : ms_timer(msg) {}

  ~life_timer() {
    stop();
    if (ms_timer::msg_ != "") {
      std::cout << "# [ " + msg_ + " ]: ";
      std::cout << elapsed() << " ms" << std::endl;
    }
  }
};

std::ostream& operator<<(std::ostream& os, const seconds_timer& t) {
  std::string name = t.name();
  if (t.name() != "") {
    os << "(" << t.name() << ") ";
  }
  os << t.elapsed() << " sec";
  return os;
}

std::ostream& operator<<(std::ostream& os, const ms_timer& t) {
  std::string name = t.name();
  if (t.name() != "") {
    os << "(" << t.name() << ") ";
  }

  os << t.elapsed() << " ms";
  return os;
}

std::ostream& operator<<(std::ostream& os, const us_timer& t) {
  std::string name = t.name();
  if (t.name() != "") {
    os << "(" << t.name() << ") ";
  }
  os << t.elapsed() << " us";
  return os;
}

}         // namespace util
}         // namespace nw



#endif    // NW_UTIL_TIMER_HPP
