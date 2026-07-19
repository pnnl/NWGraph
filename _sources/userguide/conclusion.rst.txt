.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

.. _`sec:conclusion`:

Conclusion
==========

In this paper we presented the design and rationale for a modern generic
C++ library of graph algorithms and data structures, NWGraph. Based on a
careful analysis of the graph problem domain, the fundamental interface
abstraction underlying NWGraph is that of a random access range of
forward ranges. Intentionally minimal, this interface admits composition
with any types that meet its requirements. The library implementation
includes selected concreted containers and a rich selection of common
graph algorithms. Though the library is implemented with standard
library components using idiomatic C++, experimental results showed that
the interfaces present no abstraction penalty and that the NWGraph
implementation has performance on par with the highest performing
competition. We intend to propose the design to the C++ standardization
committee for consideration as a standard C++ graph library.
