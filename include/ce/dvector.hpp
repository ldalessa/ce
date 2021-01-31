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
#pragma once

// ----------------------------------------------------------------------------
// A C++20 constexpr vector-ish implementation.
// ----------------------------------------------------------------------------

//#include <bit>
#include <cassert>
//#include <concepts>
#include <iterator>
#include <utility>

namespace ce {
template <typename T>
struct dvector
{
  using value_type = T;

 private:
  std::allocator<T> alloc_ = {};
  int capacity_ = 0;
  int size_ = 0;
  T *data_ = nullptr;

 public:
  constexpr ~dvector()
  {
    clear();
    deallocate(data_, capacity_);
    capacity_ = 0;
  }

  constexpr dvector() = default;

  constexpr dvector(int n)
      : capacity_(n)
      ,     size_(n)
      ,     data_(allocate(capacity_))
  {
    assert(n >= 0);
    assert(std::is_default_constructible_v<T> || n == 0);
    for (int i = 0; i < size_; ++i) {
      construct(i);
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
      ,     data_(allocate(capacity_))
  {
    // copy construct the storage array elements
    for (int i = 0; i < size_; ++i) {
      construct(i, b[i]);
    }
  }

  constexpr dvector(dvector&& b)
      : capacity_(std::exchange(b.capacity_, 0))
      ,     size_(std::exchange(b.size_, 0))
      ,     data_(std::exchange(b.data_, nullptr))
  {
  }

  constexpr dvector& operator=(const dvector& b)
  {
    // if `b` has a larger capacity, then clear and reallocate our storage array
    if (capacity_ < b.capacity_) {
      clear();
      deallocate(data_, capacity_);
      data_ = allocate(b.capacity_);
      capacity_ = b.capacity_;
    }

    // copy assign or construct as necessary
    int i = 0;
    for (; i < std::min(size_, b.size_); ++i) {
      data_[i] = b[i];
    }
    for (; i < b.size_; ++i) {
      construct(i, b[i]);
    }
    for (; i < size_; ++i) {
      destroy(i);
    }
    size_ = b.size_;
    return *this;
  }

  constexpr dvector& operator=(dvector&& b)
  {
    // destroy our active storage array
    clear();
    deallocate(data_, capacity_);

    // and take all of b's data
    capacity_ = std::exchange(b.capacity_, 0);
    size_     = std::exchange(b.size_, 0);
    data_     = std::exchange(b.data_, nullptr);

    return *this;
  }

  // Element access.
  constexpr const T& operator[](int i) const { assert(0 <= i && i < size_);
    return data_[i];
  }

  constexpr T& operator[](int i) { assert(0 <= i && i < size_);
    return data_[i];
  }

  constexpr const T& front() const { assert(size_ > 0);
    return data_[0];
  }

  constexpr T& front() { assert(size_ > 0);
    return data_[0];
  }

  constexpr const T& back() const { assert(size_ > 0);
    return data_[size_ - 1];
  }

  constexpr T& back() { assert(size_ > 0);
    return data_[size_ - 1];
  }

  const T* data() const {
    return data_;
  }

  T* data() {
    return data_;
  }

  constexpr auto begin() const { return data_; }
  constexpr auto begin()       { return data_; }
  constexpr auto   end() const { return data_ + size_; }
  constexpr auto   end()       { return data_ + size_; }

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
      reallocate(n);
    }
  }

  constexpr int capacity() {
    return capacity_;
  }

  constexpr void shrink_to_fit() {
    if (size_ < capacity_) {
      reallocate(size_);
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
    return construct(size_++, std::forward<Ts>(ts)...);
  }

  constexpr T& push_back(const T& t) {
    if (size_ == capacity_) {
      reserve(std::max(2 * capacity_, 1));
    }
    return construct(size_++, t);
  }

  constexpr T& push_back(T&& t) {
    if (size_ == capacity_) {
      reserve(std::max(2 * capacity_, 1));
    }
    return construct(size_++, std::move(t));
  }

  constexpr T pop_back() { assert(size_ > 0);
    return std::move(data_[--size_]);
  }

  constexpr void resize(int n) {
    // reserve enough space for n
    if (capacity_ < n) {
      reserve(n);
    }

    // if n was smaller than the size, destroy the excess elements
    for (int i = n; i < size_; ++i) {
      destroy(i);
    }

    // if n was larger than the size, then default construct the excess
    for (int i = size_; i < n; ++i) {
      assert(std::is_default_constructible_v<T>);
      construct(i);
    }

    size_ = n;
  }

  constexpr void clear()
  {
    // destroy the active elements
    for (int i = 0; i < size_; ++i) {
      destroy(i);
    }
    size_ = 0;
  }

 private:
  constexpr T* allocate(int n) {
    return alloc_.allocate(n);
  }

  constexpr void deallocate(T *ptr, int n) {
    if (ptr) {
      alloc_.deallocate(ptr, n);
    }
  }

  template <class... Ts>
  constexpr T& construct(int i, Ts&&... ts) {
    return *std::construct_at(data_ + i, std::forward<Ts>(ts)...);
  }

  constexpr void destroy(int i) {
    std::destroy_at(data_ + i);
  }

  constexpr void reallocate(int n) {
    T *old = std::exchange(data_, allocate(n));
    for (int i = 0; i < size_; ++i) {
      std::construct_at(&data_[i], std::move(old[i]));
    }
    alloc_.deallocate(old, capacity_);
    capacity_ = n;
  }
};

template <typename T, std::convertible_to<T>... Ts>
dvector(std::in_place_t, T, Ts...) -> dvector<T>;

template <typename T, std::convertible_to<T>... Ts>
dvector(std::in_place_type_t<T>, Ts...) -> dvector<T>;
}
