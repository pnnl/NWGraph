/**
 * @file scrap.cpp
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

class const_outer_iterator {
  std::vector<index_t>::const_iterator                     indices_;
  typename struct_of_arrays<Attributes...>::const_iterator indexed_;
  index_t                                                  i_;

public:
  using difference_type   = std::make_signed_t<index_t>;
  using value_type        = const_sub_view;
  using reference         = const_sub_view;    // const value_type&;
  using pointer           = const value_type*;
  using iterator_category = std::random_access_iterator_tag;

  const_outer_iterator() = default;

  const_outer_iterator(std::vector<index_t>::const_iterator indices, typename struct_of_arrays<Attributes...>::const_iterator indexed,
                       index_t i)
      : indices_(indices), indexed_(indexed), i_(i) {}

  const_outer_iterator& operator++() {
    ++i_;
    return *this;
  }

  const_outer_iterator operator++(int) const {
    const_outer_iterator tmp(*this);
    ++i_;
    return tmp;
    ;
  }

  const_outer_iterator& operator--() {
    --i_;
    return *this;
  }

  const_outer_iterator operator--(int) const {
    const_outer_iterator tmp(*this);
    --i_;
    return tmp;
    ;
  }

  const_outer_iterator& operator+=(difference_type n) {
    i_ += n;
    return *this;
  }

  const_outer_iterator& operator-=(difference_type n) {
    i_ -= n;
    return *this;
  }

  const_outer_iterator operator+(difference_type n) const { return {indices_, indexed_, i_ + n}; }
  const_outer_iterator operator-(difference_type n) const { return {indices_, indexed_, i_ - n}; }

  difference_type operator-(const const_outer_iterator& b) const { return i_ - b.i_; }

  bool operator==(const const_outer_iterator& b) const { return i_ == b.i_; }
  bool operator!=(const const_outer_iterator& b) const { return i_ != b.i_; }
  bool operator<(const const_outer_iterator& b) const { return i_ < b.i_; }
  bool operator>(const const_outer_iterator& b) const { return i_ > b.i_; }
  bool operator<=(const const_outer_iterator& b) const { return i_ <= b.i_; }
  bool operator>=(const const_outer_iterator& b) const { return i_ >= b.i_; }

  reference operator*() { return {indexed_ + indices_[i_], indexed_ + indices_[i_ + 1]}; }
  reference operator*() const { return {indexed_ + indices_[i_], indexed_ + indices_[i_ + 1]}; }

  pointer operator->() { return nullptr; }
  pointer operator->() const { return nullptr; }

  reference operator[](index_t n) { return {indexed_ + indices_[i_ + n], indexed_ + indices_[i_ + n + 1]}; }
  reference operator[](index_t n) const { return {indexed_ + indices_[i_ + n], indexed_ + indices_[i_ + n + 1]}; }
};
