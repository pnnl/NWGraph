/**
 * @file vector_bool_eg.cpp
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


#include "common/test_header.hpp"


int main() {
  size_t n_vtx = 5;

  edge_list<directedness::directed, bool> A_list {
    {0,  1, true },
    { 1, 2, true },
    { 2, 3, false},
    { 3, 4, true }
  };

  adjacency<0, bool> A(A_list);
  A.stream_indices();
}
