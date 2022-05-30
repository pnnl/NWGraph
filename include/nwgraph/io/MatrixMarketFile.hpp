/**
 * @file MatrixMarketFile.hpp
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

#ifndef NW_GRAPH_MATRIXMARKETFILE_HPP
#define NW_GRAPH_MATRIXMARKETFILE_HPP

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

extern "C" {
#include "mmio_nist.h"
}

namespace mmio {
template <typename It>
class Range {
  It begin_;
  It end_;

public:
  Range(It begin, It end) : begin_(begin), end_(end) {}
  It begin() const { return begin_; }
  It end() const { return end_; }
};

class MatrixMarketFile final {
  int fd_  = -1;    // .mtx file descriptor
  int n_   = 0;     // number of rows
  int m_   = 0;     // number of columns
  int nnz_ = 0;     // number of edges

  char* base_ = nullptr;    // base pointer to mmap-ed file
  long  i_    = 0;          // byte offset of the first edge
  long  e_    = 0;          // bytes in the mmap-ed file

  MM_typecode type_;

public:
  explicit MatrixMarketFile(std::filesystem::path path) : fd_(open(path.c_str(), O_RDONLY)) {
    if (fd_ < 0) {
      fprintf(stderr, "open failed, %d: %s\n", errno, strerror(errno));
      std::terminate();
    }

    FILE* f = fdopen(fd_, "r");
    if (f == nullptr) {
      fprintf(stderr, "fdopen failed, %d: %s\n", errno, strerror(errno));
      goto error;
    }

    switch (mm_read_banner(f, &type_)) {
      case MM_PREMATURE_EOF:       // if all items are not present on first line of file.
      case MM_NO_HEADER:           // if the file does not begin with "%%MatrixMarket".
      case MM_UNSUPPORTED_TYPE:    // if not recongizable description.
        goto error;
    }

    if (!mm_is_coordinate(type_)) {
      goto error;
    }

    switch (mm_read_mtx_crd_size(f, &n_, &m_, &nnz_)) {
      case MM_PREMATURE_EOF:    // if an end-of-file is encountered before processing these three values.
        goto error;
    }

    i_ = ftell(f);
    fseek(f, 0L, SEEK_END);
    e_ = ftell(f);

    base_ = static_cast<char*>(mmap(nullptr, e_, PROT_READ, MAP_PRIVATE, fd_, 0));
    if (base_ == MAP_FAILED) {
      fprintf(stderr, "mmap failed, %d: %s\n", errno, strerror(errno));
      goto error;
    }

    return;

  error:
    close(fd_);
    std::terminate();
  };

  ~MatrixMarketFile() { release(); };

  /// Release the memory mapping and file descriptor early.
  void release() {
    if (base_ && munmap(base_, e_)) {
      fprintf(stderr, "munmap failed, %d: %s\n", errno, strerror(errno));
    }
    base_ = nullptr;

    if (fd_ != -1 && close(fd_)) {
      fprintf(stderr, "close(fd) failed, %d: %s\n", errno, strerror(errno));
    }
    fd_ = -1;
  };

  /// ADL
  friend void release(MatrixMarketFile& mm) { mm.release(); }

  int getNRows() const { return n_; }

  int getNCols() const { return m_; }

  int getNEdges() const { return nnz_; }

  bool isPattern() const { return mm_is_pattern(type_); }

  bool isSymmetric() const { return mm_is_symmetric(type_); }

  // Iterator over edges in the file.
  template <typename... Vs>
  class iterator {
    const char* i_;

    /// Read the next token in the stream as a U, and update the pointer.
    ///
    /// This horrible code is required because normal stream processing is very
    /// slow, while this tokenized version is just sort of slow.
    template <typename U>
    static constexpr U get(const char*(&i)) {
      U     v;
      char* e;
      if constexpr (std::is_same_v<U, int>) {
        v = std::strtol(i, &e, 10);
      } else {
        v = std::strtod(i, &e);
      }
      i = e;
      return v;
    }

  public:
    iterator(const char* i) : i_(i) {}

    std::tuple<int, int, Vs...> operator*() const {
      const char* i = i_;
      int         u = get<int>(i) - 1;
      int         v = get<int>(i) - 1;
      return std::tuple(u, v, get<Vs>(i)...);
    }

    iterator& operator++() {
      ++(i_ = std::strchr(i_, '\n'));
      return *this;
    }

    bool operator!=(const iterator& b) const { return i_ != b.i_; }
  };

  template <typename... Vs>
  iterator<Vs...> begin() const {
    return {base_ + i_};
  }

  template <typename... Vs>
  iterator<Vs...> end() const {
    return {base_ + e_};
  }

  template <typename... Vs>
  iterator<Vs...> at(long edge) const {
    if (edge == 0) return begin<Vs...>();
    if (edge == nnz_) return end<Vs...>();

    // Use the edge id to come up with an approximation of the byte to start
    // searching at, and then search backward to find the byte offset of the
    // nearest edge.
    long approx = (double(edge) / double(nnz_)) * (e_ - i_) + i_;
    while (i_ <= approx && base_[approx] != '\n') {
      --approx;
    }
    ++approx;
    return {base_ + approx};
  }
};

template <typename... Vs>
auto edges(const MatrixMarketFile& mm, int j, int k) {
  return Range(mm.template at<Vs...>(j), mm.template at<Vs...>(k));
}
}    // namespace mmio

#endif // NW_GRAPH_MATRIXMARKETFILE_HPP
