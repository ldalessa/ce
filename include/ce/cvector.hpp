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
// This is not an implementation of std::vector for constexpr context, but
// simply an implementation of a constexpr vector with a fixed capacity. The
// only thing special about it is that it is intended to properly support types
// without trivial ctor/dtor/assign, while preserving the `is_trivially_*` set
// of traits from its storage type and providing contiguous iterators as if it
// were just a vector of `T`.
//
// The vector itself is mostly implemented in `cvector_impl`, but should be
// accessed in user code via the `cvector` strong typedef, which includes the
// P0848 workarounds.
//
// Hopefully the API is fairly straightforward. The cvector supports CTAD much
// like the std::vector, but does not provide any other sophisticated
// initialization. Its push and pop operations will return references and values
// for your convenience.
//
// constexpr auto foo()
// {
//   const char* hello = "hello world\n";
//   cvector<char, 24> v;
//   for (char c : hello) {
//     v.push_back(c);
//   }
//   return v;
// }
//
// ******** NOTE ABOUT STYLE ********
// I try to assert preconditions on the same line as the function
// declaration. This produces useful text output from constexpr errors, as both
// the function declaration and assertion will be printed. I also like to do
// some manual horizontal alignment where I feel like it looks more consistent.
// **********************************
#pragma once

#include "P0848.hpp"
#include <algorithm>
#include <cassert>
#include <concepts>
#include <memory>

namespace ce {
// The core vector implementation.
//
// This class template contains the definition of the vector's data and all of
// its API. It provides on_dtor, on_copy_ctor, and on_move_ctor, which allow the
// P0848 machinery to work.
template <typename T, int N>
struct cvector_impl
{
  using   value_type = T;
  using storage_type = P0848::storage<T>;

  // The storage array and current size.
  storage_type storage[N] = {};
  int n = 0;

  constexpr cvector_impl()                    = default;
  constexpr cvector_impl(const cvector_impl&) = default;
  constexpr cvector_impl(cvector_impl&&)      = default;

  constexpr cvector_impl(int n) : n(n) { assert(0 <= n && n <= N);
    assert(std::is_default_constructible_v<T> || n == 0);
    for (int i = 0; i < n; ++i) {
      construct(storage[i]);
    }
  }

  template <std::convertible_to<T>... Ts>
  constexpr cvector_impl(std::in_place_t, Ts&&... ts) {
    static_assert(sizeof...(ts) <= N);
    (emplace_back(std::forward<Ts>(ts)), ...);
  }

  template <std::convertible_to<T>... Ts>
  constexpr cvector_impl(std::in_place_type_t<T>, Ts&&... ts)
      : cvector_impl(std::in_place, std::forward<Ts>(ts)...)
  {}

  // If the underlying type is trivially copy assignable, then the default copy
  // operator is fine, otherwise we manually manage the underlying storage `t`.
  constexpr static bool triv_copy = std::is_trivially_copy_assignable_v<T>;
  constexpr cvector_impl& operator=(const cvector_impl&) requires(triv_copy) = default;
  constexpr cvector_impl& operator=(const cvector_impl& b) requires(!triv_copy) {
    int i = 0;
    for (; i < std::min(n, b.n); ++i) {
      storage[i].t = b.storage[i].t;
    }
    for (; i < b.n; ++i) {
      construct(storage[i], b.storage[i].t);
    }
    for (; i < n; ++i) {
      destroy(storage[i]);
    }
    n = b.n;
    return *this;
  }

  // If the underlying type is trivially move assignable, then the default move
  // operator is fine, otherwise we manually manage the underlying storage `t`.
  constexpr static bool triv_move = std::is_trivially_move_assignable_v<T>;
  constexpr cvector_impl& operator=(cvector_impl&&) requires(triv_move) = default;
  constexpr cvector_impl& operator=(cvector_impl&& b) requires(!triv_move) {
    int i = 0;
    for (; i < std::min(n, b.n); ++i) {
      storage[i].t = std::move(b.storage[i]).t;
    }
    for (; i < b.n; ++i) {
      construct(storage[i], std::move(b.storage[i]).t);
    }
    for (; i < n; ++i) {
      destroy(storage[i]);
    }
    n = std::exchange(b.n, 0);
    return *this;
  }

  // Element access.
  constexpr const T& operator[](int i) const { assert(0 <= i && i < n);
    return storage[i].t;
  }

  constexpr T& operator[](int i) { assert(0 <= i && i < n);
    return storage[i].t;
  }

  constexpr const T& front() const {
    return storage[0].t;
  }

  constexpr T& front() {
    return storage[0].t;
  }

  constexpr const T& back() const {
    return storage[n].t;
  }

  constexpr T& back() {
    return storage[n].t;
  }

  const T* data() const {
    return reinterpret_cast<const T*>(&storage);
  }

  T* data() {
    return reinterpret_cast<T*>(&storage);
  }

  // Iterators.
  using iterator = P0848::storage_iterator<storage_type>;
  using const_iterator = P0848::storage_iterator<const storage_type>;

  constexpr const_iterator begin() const { return {storage}; }
  constexpr       iterator begin()       { return {storage}; }
  constexpr const_iterator   end() const { return {storage + n}; }
  constexpr       iterator   end()       { return {storage + n}; }

  constexpr auto rbegin() const { return std::reverse_iterator(end()); }
  constexpr auto rbegin()       { return std::reverse_iterator(end()); }
  constexpr auto   rend() const { return std::reverse_iterator(begin()); }
  constexpr auto   rend()       { return std::reverse_iterator(begin()); }

  // Capacity
  constexpr int empty() const {
    return n == 0;
  }

  constexpr int size() const {
    return n;
  }

  constexpr static int max_size() {
    return N;
  }

  constexpr static void reserve(int n) { assert(n < N);
  }

  constexpr static int capacity() {
    return N;
  }

  constexpr static void shrink_to_fit(int n) {
  }

  constexpr friend int size(const cvector_impl& v) {
    return v.n;
  }

  // Modifiers
  template <typename... Ts>
  constexpr T& emplace_back(Ts&&... ts) { assert(n < N);
    static_assert(std::is_constructible_v<T, Ts...>);
    return construct(storage[n++], std::forward<Ts>(ts)...);
  }

  constexpr T& push_back(const T& t) { assert(n < N);
    return construct(storage[n++], t);
  }

  constexpr T& push_back(T&& t) { assert(n < N);
    return construct(storage[n++], std::move(t));
  }

  constexpr T pop_back() { assert(n > 0);
    return std::move(storage[--n].t);
  }

  constexpr void resize(int i) { assert(0 <= i && i <= N);
    for (int j = i; j < n; ++j) {
      destroy(storage[j]);                      // shrinking
    }
    for (int j = n; j < i; ++j) {
      if constexpr (std::is_default_constructible_v<T>) {
        construct(storage[j]);                  // growing
      }
      else {
        // Can't increase size without default constructor, using the trait
        // leads to a reasonably useful error. Could just uniformly prohibit
        // resizing for non-default_constructible, but I decided not to. I
        // mostly care about `constexpr` use anyway, and this is adequate for
        // that.
        assert(std::is_default_constructible_v<T>);
      }
    }
    n = i;
  }

  constexpr void clear() {
    resize(0);
  }

  // Helpers to manage the lifetime of storage elements.
 private:

  // Protected handlers are called by our subclasses as needed in order to
  // clear, copy construct, and/or move construct the array properly.
 protected:
  constexpr void on_dtor() {
    clear();
  }

  constexpr void on_copy_ctor(const cvector_impl& b) {
    for (n = 0; n < b.n; ++n) {
      construct(storage[n], b.storage[n].t);
    }
  }

  constexpr void on_move_ctor(cvector_impl&& b) {
    for (n = 0; n < b.n; ++n) {
      construct(storage[n], std::move(b.storage[n]).t);
    }
    b.n = 0;
  }
};

// The externally-visible cvector class.
template <typename T, int N>
struct cvector : P0848::ops<cvector_impl<T, N>>
{
  using P0848::ops<cvector_impl<T, N>>::ops;
};

template <typename T, std::convertible_to<T>... Ts>
cvector(std::in_place_t, T&&, Ts&&...) -> cvector<T, 1 + sizeof...(Ts)>;

template <typename T, std::convertible_to<T>... Ts>
cvector(std::in_place_type_t<T>, Ts&&...) -> cvector<T, sizeof...(Ts)>;
}
