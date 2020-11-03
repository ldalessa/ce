// BSD 3-Clause License
//
// Copyright (c) 2020, Trustees of Indiana University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ----------------------------------------------------------------------------
// A C++20 constexpr vector implementation.
//
// This implementation uses dynamic storage allocation and attempts to more
// closely conform to std::vector.
// ----------------------------------------------------------------------------

#include "P0848.hpp"
#include <bit>
#include <cassert>
#include <concepts>
#include <iterator>

namespace ce {
template <typename T>
struct dvector
{
  using value_type = T;

 private:
  using storage_type = P0848::storage_type<T>;
  int capacity_ = 0;
  int size_ = 0;
  storage_type *storage_ = nullptr;

 public:
  constexpr ~dvector()
  {
    clear();
    delete [] storage_;
  }

  constexpr dvector() = default;

  constexpr dvector(int n)
      : capacity_(n)
      ,     size_(n)
      ,  storage_(allocate(capacity_))
  {
    assert(n >= 0);
    assert(std::is_default_constructible_v<T> || n == 0);
    for (int i = 0; i < size_; ++i) {
      construct(storage_[i]);
    }
  }

  template <std::convertible_to<T>... Ts>
  constexpr dvector(std::in_place_t, Ts&&... ts)
  {
    reserve(sizeof...(ts));
    (emplace_back(std::forward<Ts>(ts)), ...);
  }

  template <std::convertible_to<T>... Ts>
  constexpr dvector(std::in_place_type_t<T>, Ts&&... ts)
      : dvector(std::in_place, std::forward<Ts>(ts)...)
  {
  }

  constexpr dvector(const dvector& b)
      : capacity_(b.capacity_)
      ,     size_(b.size_)
      ,  storage_(allocate(capacity_))
  {
    // copy construct the storage array elements
    for (int i = 0; i < size_; ++i) {
      construct(storage_[i], b[i]);
    }
  }

  constexpr dvector(dvector&& b)
      : capacity_(std::exchange(b.capacity_, 0))
      ,     size_(std::exchange(b.size_, 0))
      ,  storage_(std::exchange(b.storage_, nullptr))
  {
  }

  constexpr dvector& operator=(const dvector& b)
  {
    // if `b` has a larger capacity, then clear and reallocate our storage array
    if (capacity_ < b.capacity_) {
      clear();
      delete [] storage_;
      capacity_ = b.capacity_;
      storage_ = allocate(capacity_);
    }

    // copy assign or construct as necessary
    int i = 0;
    for (; i < std::min(size_, b.size_); ++i) {
      storage_[i].t = b[i];
    }
    for (; i < b.size_; ++i) {
      construct(storage_[i], b[i]);
    }
    for (; i < size_; ++i) {
      destroy(storage_[i]);
    }
    size_ = b.size_;
    return *this;
  }

  constexpr dvector& operator=(dvector&& b)
  {
    // destroy our active storage array
    clear();
    delete [] storage_;

    // and take all of b's data
    capacity_ = std::exchange(b.capacity_, 0);
    size_     = std::exchange(b.size_, 0);
    storage_  = std::exchange(b.storage_, nullptr);

    return *this;
  }

  // Element access.
  constexpr const T& operator[](int i) const { assert(0 <= i && i < size_);
    return storage_[i].t;
  }

  constexpr T& operator[](int i) { assert(0 <= i && i < size_);
    return storage_[i].t;
  }

  constexpr const T& front() const { assert(size_ > 0);
    return storage_[0].t;
  }

  constexpr T& front() { assert(size_ > 0);
    return storage_[0].t;
  }

  constexpr const T& back() const { assert(size_ > 0);
    return storage_[size_ - 1].t;
  }

  constexpr T& back() { assert(size_ > 0);
    return storage_[size_ - 1].t;
  }

  const T* data() const {
    return reinterpret_cast<const T*>(storage_);
  }

  T* data() {
    return reinterpret_cast<T*>(storage_);
  }

  // Iterators.
  using iterator = P0848::storage_iterator_type<storage_type>;
  using const_iterator = P0848::storage_iterator_type<const storage_type>;

  constexpr const_iterator begin() const { return {storage_}; }
  constexpr       iterator begin()       { return {storage_}; }
  constexpr const_iterator   end() const { return {storage_ + size_}; }
  constexpr       iterator   end()       { return {storage_ + size_}; }

  constexpr auto rbegin() const { return std::reverse_iterator(end()); }
  constexpr auto rbegin()       { return std::reverse_iterator(end()); }
  constexpr auto   rend() const { return std::reverse_iterator(begin()); }
  constexpr auto   rend()       { return std::reverse_iterator(begin()); }

  // Capacity
  constexpr bool empty() const {
    return size_ == 0;
  }

  constexpr int size() const {
    return size_;
  }

  constexpr static int max_size() {
    return std::numeric_limits<decltype(capacity_)>::max();
  }

  constexpr void reserve(int n) {
    if (capacity_ < n) {
      capacity_ = n;
      storage_type *temp = allocate(capacity_);
      for (int i = 0; i < size_; ++i) {
        construct(temp[i], std::move(storage_[i]));
      }
      delete [] std::exchange(storage_, temp);
    }
  }

  constexpr int capacity() {
    return capacity_;
  }

  constexpr void shrink_to_fit() {
    if (size_ < capacity_) {
      capacity_ = size_;
      storage_type *temp = allocate(capacity_);
      for (int i = 0; i < size_; ++i) {
        construct(temp[i], std::move(storage_[i]));
      }
      delete [] std::exchange(storage_, temp);
    }
  }

  constexpr friend int size(const dvector& v) {
    return v.size_;
  }

  // Modifiers
  template <typename... Ts>
  constexpr T& emplace_back(Ts&&... ts) {
    static_assert(std::is_constructible_v<T, Ts...>);
    if (size_ == capacity_) {
      reserve(std::max(2 * capacity_, 1));
    }
    return construct(storage_[size_++], std::forward<Ts>(ts)...);
  }

  constexpr T& push_back(const T& t) {
    if (size_ == capacity_) {
      reserve(std::max(2 * capacity_, 1));
    }
    return construct(storage_[size_++], t);
  }

  constexpr T& push_back(T&& t) {
    if (size_ == capacity_) {
      reserve(std::max(2 * capacity_, 1));
    }
    return construct(storage_[size_++], std::move(t));
  }

  constexpr T pop_back() { assert(size_ > 0);
    return std::move(storage_[--size_].t);
  }

  constexpr void resize(int n) {
    // reserve enough space for n
    if (capacity_ < n) {
      reserve(n);
    }

    // if n was smaller than the size, destroy the excess elements
    for (int i = n; i < size_; ++i) {
      destroy(storage_[i]);
    }

    // if n was larger than the size, then default construct the excess
    for (int i = size_; i < n; ++i) {
      assert(std::is_default_constructible_v<T>);
      construct(storage_[i]);
    }
    size_ = n;
  }

  constexpr void clear()
  {
    // destroy the active elements
    for (int i = 0; i < size_; ++i) {
      destroy(storage_[i]);
    }
    size_ = 0;
  }

 private:
  constexpr static storage_type* allocate(int n) {
    return (n) ? new storage_type[n] : nullptr;
  }
};

template <typename T, std::convertible_to<T>... Ts>
dvector(std::in_place_t, T, Ts...) -> dvector<T>;

template <typename T, std::convertible_to<T>... Ts>
dvector(std::in_place_type_t<T>, Ts...) -> dvector<T>;
}
