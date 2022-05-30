/**
 * @file worklist.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro
 *   Kevin Deweese
 *
 */

#ifndef NW_GRAPH_WORKLIST_HPP
#define NW_GRAPH_WORKLIST_HPP

#include "tbb/concurrent_priority_queue.h"
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_vector.h"

#include "nwgraph/util/util.hpp"
#include <cassert>
#include <list>
#include <queue>
#include <vector>

namespace nw {
namespace graph {

//****************************************************************************
//template<typename Graph, typename Workitem = vertex_id_type, typename Queue = tbb::concurrent_queue<Workitem> >
template <typename Graph, typename Workitem = typename Graph::vertex_id_type, typename Queue = std::queue<Workitem>>
class worklist_range {
public:
  worklist_range(Graph& graph) : the_graph_(graph) {}

  worklist_range(const worklist_range&)  = delete;
  worklist_range(const worklist_range&&) = delete;

  bool empty() { return Q_.empty(); }

  class worklist_range_iterator {
  public:
    worklist_range_iterator(worklist_range<Graph, Workitem>& range) : the_range_(range) {}

    worklist_range_iterator& operator++() {
      the_range_.Q_.pop();
      return *this;
    }

    auto operator*() {
      //Workitem T;
      //the_range_.Q_.try_pop(T);
      //return T;
      return the_range_.Q_.front();
    }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }

  private:
    worklist_range<Graph, Workitem>& the_range_;
  };

  typedef worklist_range_iterator iterator;

  auto begin() { return worklist_range_iterator(*this); }
  auto end() { return typename worklist_range_iterator::end_sentinel_type(); }

  void push_back(Workitem work) { Q_.push(work); }

private:
  Graph& the_graph_;
  Queue  Q_;
};

//****************************************************************************
template <typename Graph, typename Workitem = typename Graph::vertex_id_type, typename Queue = tbb::concurrent_queue<Workitem>>
class tbbworklist_range {
public:
  tbbworklist_range(Graph& graph) : the_graph_(graph) {}

  tbbworklist_range(const tbbworklist_range&)  = delete;
  tbbworklist_range(const tbbworklist_range&&) = delete;

  bool empty() { return Q_.unsafe_size() == 0; }

  class tbbworklist_range_iterator {
  public:
    tbbworklist_range_iterator(tbbworklist_range<Graph, Workitem, Queue>& range) : the_range_(range) {
      item = the_range_.dummy_;
      the_range_.Q_.try_pop(item);
    }

    tbbworklist_range_iterator& operator++() {
      item = the_range_.dummy_;
      the_range_.Q_.try_pop(item);
      //the_range_.Q_.pop();
      return *this;
    }

    auto& operator*() {
      /*std::vector<Workitem> buffer;
      buffer.reserve(10);
      Workitem T = the_range_.dummy_;
      for(size_t i = 0; i < 10; ++i) {
        if(the_range_.Q_.try_pop(T))
          buffer.push_back(T);
      }
      return buffer;*/
      return item;
      //return the_range_.Q_.front();
    }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }

  private:
    tbbworklist_range<Graph, Workitem, Queue>& the_range_;
    Workitem                                   item;
  };

  typedef tbbworklist_range_iterator iterator;

  auto begin() { return tbbworklist_range_iterator(*this); }
  auto end() { return typename tbbworklist_range_iterator::end_sentinel_type(); }

  void push_back(Workitem work) { Q_.push(work); }
  void set_dummy(Workitem dummy) { dummy_ = dummy; }

private:
  Graph&   the_graph_;
  Queue    Q_;
  Workitem dummy_;
};

//****************************************************************************
template <typename Graph, typename Workitem = typename Graph::vertex_id_type, typename Queue = tbb::concurrent_queue<Workitem>>
class tbbworklist_range2 {
public:
  tbbworklist_range2(Graph& graph) : the_graph_(graph), buckets_(10000) {}

  tbbworklist_range2(const tbbworklist_range2&)  = delete;
  tbbworklist_range2(const tbbworklist_range2&&) = delete;

  bool empty() {
    for (size_t i = 0; i < buckets_.size(); ++i) {
      /*if(buckets_[i].unsafe_size() > 0) {
        return false;
      }*/
      if (!buckets_[i].empty()) return false;
    }
    return true;
    //return (counter == 0);
  }

  class tbbworklist_range_iterator2 {
  public:
    tbbworklist_range_iterator2(tbbworklist_range2<Graph, Workitem, Queue>& range) : the_range_(range) {
      /*buffer_.reserve(1000);
      Workitem T = the_range_.dummy_;
      for(size_t i = 0; i < the_range_.buckets_.size(); ++i) {
        if(the_range_.buckets_[i].try_pop(T)) {
          buffer_.push_back(T);
          --the_range_.counter;
          if(buffer_.size()>1000)
            break;
        }
      }*/
      workunit = the_range_.dummy_;
      for (size_t i = 0; i < the_range_.buckets_.size(); ++i) {
        if (the_range_.buckets_[i].try_pop(workunit)) break;
      }
    }

    tbbworklist_range_iterator2& operator++() {
      /*buffer_.clear();
      Workitem T = the_range_.dummy_;
      for(size_t i = 0; i < the_range_.buckets_.size(); ++i) {
        if(the_range_.buckets_[i].try_pop(T)) {
          buffer_.push_back(T);
          --the_range_.counter;
          if(buffer_.size()>1000)
            return *this;
        }
      }*/
      workunit = the_range_.dummy_;
      for (size_t i = 0; i < the_range_.buckets_.size(); ++i) {
        if (the_range_.buckets_[i].try_pop(workunit)) return *this;
      }
      //the_range_.Q_.pop();
      return *this;
    }

    auto& operator*() {
      return workunit;
      //return buffer_;
      //return the_range_.Q_.front();
    }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return the_range_.empty(); }
    bool operator!=(const end_sentinel_type&) const { return !the_range_.empty(); }

  private:
    tbbworklist_range2<Graph, Workitem, Queue>& the_range_;
    //tbb::concurrent_vector<Workitem> buffer_;
    Workitem workunit;
  };

  typedef tbbworklist_range_iterator2 iterator;

  auto begin() { return tbbworklist_range_iterator2(*this); }
  auto end() { return typename tbbworklist_range_iterator2::end_sentinel_type(); }

  void push_back(Workitem work, size_t level) {
    if (level + 1 > buckets_.size()) {
      buckets_.grow_to_at_least(level + 1);
    }

    //std::cout << level << " " << buckets_.size() << std::endl;
    buckets_[level].push(work);
    //Q_.push(work);
    //std::cout << "pushed" << std::endl;
    //counter++;
  }
  void set_dummy(Workitem dummy) { dummy_ = dummy; }

private:
  Graph&                        the_graph_;
  tbb::concurrent_vector<Queue> buckets_;
  Workitem                      dummy_;
  //std::atomic<int> counter;
};

}    // namespace graph
}    // namespace nw

#endif    //  NW_GRAPH_WORKLIST_HPP
