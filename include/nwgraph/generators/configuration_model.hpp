/**
 * @file configuration_model.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_CONFIGURATION_MODEL_HPP
#define NW_GRAPH_CONFIGURATION_MODEL_HPP

#include <edge_list.hpp>
#include <execution>
#include <vector>
#include <random>
#include <math.h>

namespace nw {
namespace graph {

/* 
* dist_min - Minimum value of distribution.  
* dist_max - Maximum value of distribution.
*/
std::size_t power_gen (double dist_min, double dist_max) {
  /* Inverse square.  */
  double exponent = -2;

  double min_pow = pow (dist_min, exponent + 1);
  double max_pow = pow (dist_max, exponent + 1);

  return (std::size_t) pow((max_pow - min_pow) * drand48 ()
                      + min_pow, 1 / (exponent + 1));
}
static double standard_exponential_unlikely(bitgen_t *bitgen_state,
                                                uint8_t idx, double x) {
  if (idx == 0) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    return ziggurat_exp_r - log(1.0 - next_double(bitgen_state));
  } else if ((fe_double[idx - 1] - fe_double[idx]) * next_double(bitgen_state) +
                 fe_double[idx] <
             exp(-x)) {
    return x;
  } else {
    return random_standard_exponential(bitgen_state);
  }
}
double random_standard_exponential(bitgen_t *bitgen_state) {
  uint64_t ri;
  uint8_t idx;
  double x;
  ri = next_uint64(bitgen_state);
  ri >>= 3;
  idx = ri & 0xFF;
  ri >>= 8;
  x = ri * we_double[idx];
  if (ri < ke_double[idx]) {
    return x; /* 98.9% of the time we return here 1st try */
  }
  return standard_exponential_unlikely(bitgen_state, idx, x);
}

double random_power(bitgen_t *bitgen_state, double a) {
  return pow(1 - exp(-random_standard_exponential(bitgen_state)), 1. / a);
}

std::vector<std::size_t> power_law_gen(double std::size_t samples)
/*
* Return a random undirected bipartite graph (in edge_list) from two given degree sequences.
* The bipartite graph is composed of two partitions. 
* By default, contiguous_id_space is false, set A has nodes 0 to
* (deg_seqa.size() - 1) and set B has nodes 0 to (deg_seqb.size() - 1).
* When contiguous_id_space is true, set A has nodes 0 to (deg_seqa.size() - 1)
* and set B has nodes deg_seqa.size() to (deg_seqa.size() + deg_seqb.size() - 1).
* Nodes from set A are connected to nodes in set B by choosing
* randomly from the possible free stubs, one in A and one in B.
* Note directed graph is not supported in configuration model.
* 
*/
template<class T>
nw::graph::edge_list<nw::graph::undirected> 
configuration_model(std::vector<T>& deg_seqa, std::vector<T>& deg_seqb, bool contiguous_id_space = false) {
    //validate degree sequences such that their summation are equivalent
    nw::graph::edge_list<nw::graph::undirected> el;
    T suma = std::reduce(std::execution::par_unseq, deg_seqa.cbegin(), deg_seqa.cend());
    T sumb = std::reduce(std::execution::par_unseq, deg_seqb.cbegin(), deg_seqb.cend());
    if (suma != sumb) {
        std::cout << "Invalid degree sequences, sum(deg_seqa)!=sum(deg_seqb)" << std::endl;
        return el;
    }
    auto populate_stubs = [&](auto& degree_sequence) -> std::vector<T> {
        std::vector<T> stubs(suma);
        std::size_t cursor = 0;
        for (std::size_t v = 0, e = degree_sequence.size(); v < e; ++v) {
            auto degree_v = degree_sequence[v];
            auto begin = stubs.begin() + cursor;
            cursor += degree_v;
            auto end = begin + cursor;
            std::fill(std::execution::par_unseq, begin, end, v);
        }
        return stubs;
    };
    auto astubs = populate_stubs(deg_seqa);
    auto bstubs = populate_stubs(deg_seqb);

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(astubs.begin(), astubs.end(), g);
    std::shuffle(bstubs.begin(), bstubs.end(), g);

    std::copy(astubs.begin(), astubs.end(), std::ostream_iterator<int>(std::cout, " "));


    el.open_for_push_back();
    for(std::size_t i = 0, e = astubs.size(); i < e; ++i) {
        el.push_back(astubs[i], bstubs[i]);
    }
    el.close_for_push_back();
    return el;
}

}//namespace graph
}//namespace nw

#endif // NW_GRAPH_CONFIGURATION_MODEL_HPP
