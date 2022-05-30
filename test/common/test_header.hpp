/**
 * @file test_header.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   liux238
 *
 */


// This must be included only in the CPP file, and only once.

#ifndef SGL_TEST_HEADER
#define SGL_TEST_HEADER
#else
#error "test_header is included multiple times. test header must be included only once"
#endif

#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/io/mmio.hpp"

#include <catch2/catch.hpp>
