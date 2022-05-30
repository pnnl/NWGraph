/**
 * @file disjoint_set.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *   Kevin Deweese	
 *
 */

#ifndef NW_GRAPH_DISJOINT_SET_HPP
#define NW_GRAPH_DISJOINT_SET_HPP

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string.h>
#include <vector>

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/execution>
#else
#include <execution>
#endif

#include "nwgraph/util/defaults.hpp"

namespace nw {
namespace graph {

using vertex_id_type = default_vertex_id_type;

vertex_id_type disjoint_find(std::vector<std::pair<vertex_id_type, size_t>>& subsets, vertex_id_type vtx) {
  vertex_id_type parent = subsets[vtx].first;
  while (parent != subsets[parent].first) {
    parent = subsets[parent].first;
  }
  while (vtx != parent) {
    vtx                = subsets[vtx].first;
    subsets[vtx].first = parent;
  }

  return parent;
}

void disjoint_union(std::vector<std::pair<vertex_id_type, size_t>>& subsets, vertex_id_type u, vertex_id_type v) {
  vertex_id_type u_root = disjoint_find(subsets, u);
  vertex_id_type v_root = disjoint_find(subsets, v);

  if (subsets[u_root].second < subsets[v_root].second)
    subsets[u_root].first = v_root;

  else if (subsets[u_root].second > subsets[v_root].second)
    subsets[v_root].first = u_root;

  else {
    subsets[v_root].first = u_root;
    subsets[u_root].second++;
  }
}

bool disjoint_union_find(std::vector<std::pair<vertex_id_type, size_t>>& subsets, vertex_id_type u, vertex_id_type v) {
  vertex_id_type u_root = disjoint_find(subsets, u);
  vertex_id_type v_root = disjoint_find(subsets, v);

  if (u_root != v_root) {

    if (subsets[u_root].second < subsets[v_root].second)
      subsets[u_root].first = v_root;

    else if (subsets[u_root].second > subsets[v_root].second)
      subsets[v_root].first = u_root;

    else {
      subsets[v_root].first = u_root;
      subsets[u_root].second++;
    }

    return true;
  }

  return false;
}

/*
 * UnionFind data structure for disjoint sets (i.e., sets that don’t overlap).
 * Implemenation is based Union-Find data structure.
 * Implemented union-by-size and union-by height.
 * Implemented both simple find and path compression find.
 * Union-by-height or Union-by-size with simple find has a
 * worse case runtime O(mlogn) for m operations.
 * Union-by-height or Union-by-size with path compression find
 * has a worse case runtime O(mlog*n) for m operations.
 */
template <class T>
class disjoint_set {
public:
  disjoint_set() : nsets_(0), maxid_(0), sets_(nullptr) {
    if (std::is_unsigned<T>::value) {
      std::cerr << "only support signed number" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  /**
		 * Create a new set representation with not more the @max_element elements.
		 * Initially every element is in its own set (-1).
		 * @param max_element maximum number of elements
		 */
  explicit disjoint_set(T max_element) : nsets_(max_element), maxid_(max_element) {
    if (std::is_unsigned<T>::value) {
      std::cerr << "only support signed number" << std::endl;
      exit(EXIT_FAILURE);
    }
    this->sets_ = new T[max_element];
    memset(this->sets_, -1, maxid_ * sizeof(T));
  }
  /**
		 * Copy constructor.
		 * @param[in] other, the other set
		 */
  explicit disjoint_set(const disjoint_set<T>& other) : nsets_(other.nsets_), maxid_(other.maxid_) {
    if (std::is_unsigned<T>::value) {
      std::cerr << "only support signed number" << std::endl;
      exit(EXIT_FAILURE);
    }
    delete[] this->sets_;
    this->sets_ = new T[other.maxid_];
    memcpy(this->sets_, other.sets_, sizeof(T) * other.maxid_);
  }

  ~disjoint_set() { delete[] this->sets_; }

  /**
		 * Assigns every element to -1.
		 * Set id is equal to element id.
		 */
  void reset() { memset(this->sets_, -1, maxid_ * sizeof(T)); }
  /**
		 * Assigns every element to -1.
		 * Set id to element id.
		 */
  void reset(const T index) {
    //element must be between 0 to size()
    if (index > this->maxid_ || index < 0) {
      throw std::range_error(std::to_string(index) + "is out of range\n");
    }
    this->sets_[index] = index;
  }
  /**
		 * Set id is equal to element id.
		 */
  template <class ExecutionPolicy = std::execution::parallel_unsequenced_policy>
  void allToSingletons(ExecutionPolicy&& policy = {}) {
    size_t i = 0;
    std::for_each(policy, this->sets_, this->sets_ + this->maxid_, [&i, this]() {
      this->sets[i] = i;
      ++i;
    });
  }
  template <typename L>
  static void allToSingletons(L* arr, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      arr[i] = i;
  }
  template <typename L>
  static void allToSingletons(std::vector<L>& arr, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      arr[i] = i;
  }
  /**
		 * find_with_pathcompression is find with path compression.
		 * @param u element
		 * @return representative of set containing @u
		 */
  virtual T find_with_pathcompression(T index) {
    //element must be between 0 to size()
    if (index > this->maxid_ || index < 0) {
      throw std::range_error(std::to_string(index) + "is out of range\n");
    }
    //negative means index is a root
    if (0 > this->sets_[index]) {
      return index;
    } else {
      //positive means index is not a root.
      // recursion and path compression
      //return this->sets_[u] = find_with_pathcompression(this->sets_[u]);

      //non-recurisve find with path compression
      std::stack<T> s;
      //we iterate through the sets_[index] until a negative is found
      //a negative means index is a root.
      while (0 <= this->sets_[index]) {
        s.push(index);
        index = this->sets_[index];
      }
      //Path compression: assign all the accessed sets_[index] to root
      while (!s.empty()) {
        this->sets_[s.top()] = index;
        s.pop();
      }
      return index;
    }
  }
  static T find_with_pathcompression(disjoint_set<T>& s, T index) { return s.find_with_pathcompression(index); }
  /**
		 * find is a simple find for @u.
		 * @param[in] index
		 * @return representative of set containing @u
		 */
  virtual T find(T index) const {
    //element must be between 0 to size()
    if (index > this->maxid_ || index < 0) {
      throw std::range_error(std::to_string(index) + "is out of range\n");
    }
    //recursion and simple find
    //return find(this->sets_[u]);

    //non-recursive simple find
    //negative means root
    volatile T t = 0;
    while (0 <= this->sets_[t]) {
      //if (t > this->maxid_ || t < 0) {
      t = this->sets_[t];
    }
    return t;
  }
  /**
		 * Static wrapper for find.
		 * @param[in] s, disjoint_set
		 * @param[in] index
		 * @return representative of set containing @u
		 */
  static T find(disjoint_set<T>& s, T index) { return s.find(index); }
  /**
		 * directFind is a direct find for element at 'index'.
		 * @param[in] index
		 * @return the element at index
		 */
  virtual T directFind(const T index) const {
    //element must be between 0 to maxid_
    //element must be between 0 to size()
    if (index > this->maxid_ || index < 0) {
      throw std::range_error(std::to_string(index) + "is out of range\n");
    }

    return this->sets_[index];
  }

  /**
		 *	Element v changes its membership to the set of element u, v->setOf(u).
		 *  @param u element u
		 *  @param v element v
		 */
  virtual bool join(T const& u, T const& v) {
    if (u == v) return true;
    T rootu = find(u);
    T rootv = find(v);
    if (rootu != rootv) {
      //if u and v are not in the same set
      if (-1 > this->sets_[rootv]) {
        //if v is not a singleton set, it belongs to setOf(rootv)
        if (v != rootv) {
          //if v leaves setRootV to setRootU
          //decrement the size of setOf(rootv)
          ++this->sets_[rootv];
          //increment the size of setOf(rootu)
          --this->sets_[rootu];
          //change the root of v to root of u
          this->sets_[v] = rootu;
        } else {
          //if v is a root, it will join all its set to setOf(rootU)
          //combine the size of setOf(rootu) and setOf(rootv)
          this->sets_[rootu] += this->sets_[rootv];
          //change the the root of v to root of u
          this->sets_[v] = rootu;
        }
      } else {
        // v is a singleton set, we just change its membership
        //change the the root of v to root of u
        this->sets_[v] = rootu;
        //increment the size of setOf(rootu)
        --this->sets_[rootu];
        //the size of all the set decrement 1
        --this->nsets_;
      }
      return true;
    }
    //if u v have the same root
    return true;
  }
  /**
		 *	Element v changes its value to u, set[v] = u.
		 *  @param u element u
		 *  @param v element v
		 */
  virtual bool directJoin(T const& u, T const& v) {
    //element must be between 0 to size()
    if (v > this->maxid_ || v < 0) {
      throw std::range_error(std::to_string(v) + "is out of range\n");
    }
    this->sets_[v] = u;
    return true;
  }
  /**
		 *  Union the two disjoint sets @u and @v by the size of them.
		 *  Depends how you want to join, you can have the combination
		 *  of UnionByheight, UnionBySize with find, find_with_pathcompression.
		 *  Path compression is perfectly compatible with union-by-size.
		 *  Path compression is not entirely compatible with
		 *  union-by-height, because path compression can change the
		 *  heights of the trees.
		 *  Currently, the smaller set will join the larger set. If the size of two sets are equal, the latter will join the former one.
		 *  @param u element u
		 *  @param v element v
		 */
  virtual void unionBySize(T const& u, T const& v) {
    T rootu = find(u);
    T rootv = find(v);
    if (rootu != rootv) {
      //if u, v does not belong to the same set
      combineBySize(rootu, rootv);
      --this->nsets_;
    }
  }
  static void  unionBySize(disjoint_set<T>& s, T const& u, T const& v) { s.unionBySize(u, v); }
  virtual void unionByHeight(T const& u, T const& v) {
    T rootu = find(u);
    T rootv = find(v);
    if (rootu != rootv) {
      //if u, v does not belong to the same set
      combineByHeight(rootu, rootv);
      --this->nsets_;
    }
  }

  /**
		 *  Union the two disjoint sets by size @u and @v, the set with
		 *  smaller size will be included in the set with larger size.
		 *  If two sets have the same size, the latter will join the former.
		 *  @param u element u
		 *  @param v element v
		 */
  void combineBySize(T const& u, T const& v) {
    if (0 < this->sets_[u]) std::cerr << u << "u:" << this->sets_[u] << std::endl;
    if (0 < this->sets_[v]) {
      ;
      std::cerr << v << "v:" << this->sets_[v] << std::endl;
    }
    assert(0 >= this->sets_[u]);
    assert(0 >= this->sets_[v]);
    if (this->sets_[u] > this->sets_[v]) {
      //remember both are negative numbers
      //merge the smaller tree u into larger tree v
      //combine the size of two merged trees
      this->sets_[v] += this->sets_[u];
      //set the new root of u to v
      this->sets_[u] = v;
    } else {
      this->sets_[u] += this->sets_[v];
      this->sets_[v] = u;
    }
  }
  //TODO NOT fully tested.
  //u is always larger than v
  void unionBySizeAtomic(T u, T v) {
    T rootu, rootv;
    T valueofu, valueofv;

    rootu = u;
    rootv = find_with_pathcompression(v);
    if (rootu == rootv) return;
    valueofu = this->sets_[rootu];
    valueofv = this->sets_[rootv];
    //if (0 < this->sets_[rootu]) { continue; }
    //if (0 < this->sets_[rootv]) { continue; }
    if (valueofu > valueofv) {
      //remember both are negative numbers
      //merge the smaller tree u into larger tree v
      //combine the size of two merged trees
      //this->sets_[v] += this->sets_[u];
      //set the new root of u to v
      //this->sets_[u] = v;
      writeAdd(this->sets_[rootv], valueofu);
      assert(this->sets_[rootv] < std::numeric_limits<T>::max());
      //writeMin(this->sets_[rootu], rootv, std::greater<T>());
      //assert(this->sets_[rootu] < std::numeric_limits<T>::max());
      // large id joins small id's set
    } else {
      //this->sets_[u] += this->sets_[v];
      //this->sets_[v] = u;
      writeAdd(this->sets_[rootu], valueofv);
      assert(this->sets_[rootu] < std::numeric_limits<T>::max());
      //writeMin(this->sets_[rootv], rootu, std::greater<T>());
      //assert(this->sets_[rootv] < std::numeric_limits<T>::max());
      // large id joins small id's set
    }
    if (u < v)
      this->sets_[rootu] = rootv;
    else
      this->sets_[rootv] = rootu;
    std::cout << u << "v" << v << ":" << rootu << "v" << rootv << ":" << valueofu << "v" << valueofv << ":" << this->sets_[rootu] << "v"
              << this->sets_[rootv] << std::endl;
  }    //unionBySizeAtomic

  /**
		 *  Union the two disjoint sets by height @u and @v.
		 *  @param u element u
		 *  @param v element v
		 */
  void combineByHeight(T const& v, T const& u) {
    assert(0 >= this->sets_[u]);
    assert(0 >= this->sets_[v]);
    if (this->sets_[v] > this->sets_[u]) {
      //remember both are negative numbers
      //merge the shorter tree v into taller tree u
      this->sets_[v] = u;    //make v the new root
    } else {
      //merge the shorter tree u into taller tree v
      if (this->sets_[u] == this->sets_[v]) --this->sets_[v];    //update height if same
      this->sets_[u] = v;                                        //make u the new root
    }
  }
  void UnionBySplicing(T v, T u) {
    T z;    //temp variable to store element
    while (this->sets_[v] != this->sets_[u]) {
      //a lower numbered node points to a higher numbered node or itself(root)
      if (this->sets_[v] < this->sets_[u]) {

        if (v == this->sets_[v]) {
          //when v is the root of current set, set v's parent as u
          this->sets_[v] = this->sets_[u];
          break;
        }
        z              = v;
        this->sets_[v] = this->sets_[u];
        v              = this->sets_[z];
      } else {
        if (u == this->sets_[u]) {
          //when u the root of current set, set u's parent as u
          this->sets_[u] = this->sets_[v];
          break;
        }
        z              = u;
        this->sets_[u] = this->sets_[v];
        u              = this->sets_[z];
      }
    }    //while
  }      //UnionBySplicing

  std::vector<T> getRawSubsets() const {
    std::vector<T> sets(this->maxid_, 0);
    for (T i = 0; i < maxid_; ++i) {
      //for each entry belongs the same subset, insert the entry index into the sets[root]
      sets[i] = this->sets_[i];
    }
    return sets;
  }
  /**
		 *  Get the subsets.
		 *  @return subsets in disjoint_set.
		 */
  std::vector<std::set<T>> getSubsets() const {
    std::vector<std::set<T>> sets(this->maxid_ + 1);
    for (T i = 0; i < maxid_; ++i) {
      //for each entry belongs the same subset, insert the entry index into the sets[root]
      sets[find(i)].insert(i);
    }
    std::vector<std::set<T>> subsets;
    for (std::set<T> s : sets) {
      if (0 < s.size()) {
        subsets.push_back(s);
      }
    }
    return subsets;
  }
  /**
		 * Get the compact version of communities in vector form for verification.
		 * This version is for disjoint sets communities.
		 * @return a vector of elements labeled under different set label.
		 */
  std::vector<T> getCompactDisjSets(size_t& nsets) const {
    std::vector<T> compactsets(this->maxid_, -1);
    T              j = 0;
    for (T i = 0; i < maxid_; ++i) {
      //std::cout << this->sets_[i] << " ";
      //find the root of the set and give it an artifitial label
      if (0 > sets_[i]) {
        compactsets[i] = ++j;
      }
    }
    nsets = j;
    for (T i = 0; i < maxid_; ++i) {
      //for every non-root element in the set, change its label to the artifitial label
      if (0 <= sets_[i]) {
        compactsets[i] = compactsets[find(i)];
      }
    }
    return compactsets;
  }

  std::vector<T> getCompactDisjSets() const {
    T              j = 1;
    std::vector<T> compactsets(this->maxid_);
    std::map<T, T> compactmap;
    for (T i = 0; i < maxid_; ++i) {
      auto result = compactmap.insert(std::make_pair(sets_[i], j));
      if (result.second) ++j;
    }

    //#pragma omp parallel for
    for (T i = 0; i < maxid_; ++i) {
      compactsets[i] = compactmap[sets_[i]];
      //std::cout << i << "(" << sets_[i] << ")"<< compactsets[i] << " ";
    }
    return compactsets;
  }
  /**
		 *  Get the number of disjoint sets.
		 *  @return the number of disjoint sets.
		 */
  size_t getSetNumber() const { return this->nsets_; }
  /**
		 *  Given an element in a set, get the size of disjoint set containing this element.
		 *  @ param[in] u, an element of the set to query
		 *  @return the size of disjoint set.
		 */
  size_t getSetSize(const T u) const { return -find(u); }
  /**
		 * Overload operator<< for format output.
		 * @param os, outstream pointer
		 * @param e, the edge to print
		 * @return os, outstream pointer
		 */
  friend inline std::ostream& operator<<(std::ostream& os, disjoint_set<T> const& s) {
    s.printDisjSets(os);
    return os;
  }
  /**
		 *  Print the whole disjoint sets.
		 */
  void printDisjSets(std::ostream& os) const {
    for (T i = 0; i < this->maxid_; ++i) {
      os << i << " ";
    }
    os << std::endl;
    for (T i = 0; i < this->maxid_; ++i) {
      os << this->sets_[i] << " ";
    }
    os << std::endl;
  }
  /**
		 *  Get the max id in this disjoint sets.
		 *  @return max id
		 */
  T getMaxId() const { return this->maxid_; }

private:
  inline bool compare_and_swap(T& x, T old_val, T new_val) { return __sync_bool_compare_and_swap(&x, *(&old_val), *(&new_val)); }

  inline bool writeMin(T& a, T b) {
    T    c;
    bool r = false;
    do
      c = a;
    while (c > b && !(r = compare_and_swap(a, c, b)));
    return r;
  }

  template <typename F>
  inline bool writeMin(T& addr, T new_val, F less) {
    T    old_val;
    bool r = false;
    do
      old_val = addr;
    while (less(new_val, old_val) && !(r = compare_and_swap(addr, old_val, new_val)));
    return r;
  }

  inline void writeAdd(T& a, T b) {
    volatile T newv, oldv;
    do {
      oldv = a;
      newv = oldv + b;
    } while (!compare_and_swap(a, oldv, newv));
  }

public:
  template <typename L>
  inline void forEach(L handle) const {
    for (T i = 0; i < this->maxid_; ++i)
      handle(this->sets_[i]);
  }
  template <typename L>
  inline void forEachElement(L handle) const {
    for (T i = 0; i < this->maxid_; ++i)
      handle(i);
  }
  template <typename L>
  inline void forEachRoot(L handle) const {
    for (T i = 0; i < this->maxid_; ++i)
      //by definition, root is the element of negative height/size in disjoint_set
      if (0 > this->sets_[i] || i == this->sets_[i]) handle(this->sets_[i]);
  }
  template <typename L>
  inline void forEachRootIndex(L handle) const {
    for (T i = 0; i < this->maxid_; ++i)
      //by definition, root is the element of negative height/size in disjoint_set
      if (0 > this->sets_[i] || i == this->sets_[i]) handle(i);
  }

protected:
  /**
		 * Number of disjoint sets.
		 */
  size_t nsets_;    //not used in parallel
  /**
		 * Max element appears in the set.
		 */
  T  maxid_;
  T* sets_;
};    //class disjoint_set

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_DISJOINT_SET_HPP
