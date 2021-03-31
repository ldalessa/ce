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

#include "ce/dvector.hpp"
#include "common.hpp"
#include "Foo.hpp"
#include <functional>

using namespace ce::tests;

template <typename T>
struct tests {
  constexpr static void
  check_structure(auto& v, int capacity, int size, source_location&& src = source_location::current())
  {
    CE_CHECK(v.capacity() == capacity, std::move(src));
    CE_CHECK(v.size() == size, std::move(src));
    if (!std::is_constant_evaluated()) {
      CE_CHECK(v.data() != nullptr || v.size() == 0, std::move(src));
    }
  }

  constexpr static void ctor_default() {
    ce::dvector<T> a;
  }

  constexpr static void ctor_n() {
    ce::dvector<T> a(10);
    check_structure(a, 10, 10);
  }

  constexpr static void read() {
    ce::dvector<T> a(2);
    check_structure(a, 2, 2);
    CE_CHECK(a[0] == 0);
    CE_CHECK(a[1] == 0);
  }

  constexpr static void ctor_inplace() {
    ce::dvector<T> a = { std::in_place, 1 };
    check_structure(a, 1, 1);
    CE_CHECK(a[0] == 1);
  }

  constexpr static void ctor_inplace_type() {
    ce::dvector<T> a = { std::in_place_type<T>, 1 };
    check_structure(a, 1, 1);
    CE_CHECK(a[0] == 1);
  }

  constexpr static void ctad_inplace() {
    ce::dvector a = { std::in_place, T(1) };
    static_assert(std::same_as<typename decltype(a)::value_type, T>);
    check_structure(a, 1, 1);
    CE_CHECK(a[0] == 1);
  }

  constexpr static void ctad_inplace_type() {
    ce::dvector a = { std::in_place_type<T>, 1 };
    static_assert(std::same_as<typename decltype(a)::value_type, T>);
    check_structure(a, 1, 1);
    CE_CHECK(a[0] == 1);
  }

  constexpr static void ctor_copy() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    ce::dvector b = a;
    check_structure(b, 3, 3);
    CE_CHECK(b[0] == 1);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[2] == 3);
  }

  constexpr static void ctor_move() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    ce::dvector b = std::move(a);
    check_structure(a, 0, 0);
    check_structure(b, 3, 3);
    CE_CHECK(b[0] == 1);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[2] == 3);
  }

  constexpr static void copy() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    ce::dvector b = { std::in_place_type<T>, 4, 5, 6 };
    b = a;
    check_structure(b, 3, 3);
    CE_CHECK(b[0] == 1);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[2] == 3);
  }

  constexpr static void copy_larger() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    ce::dvector b = { std::in_place_type<T>, 4 };
    b = a;
    check_structure(b, 3, 3);
    CE_CHECK(b[0] == 1);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[2] == 3);
  }

  constexpr static void copy_smaller() {
    ce::dvector a = { std::in_place_type<T>, 1 };
    ce::dvector b = { std::in_place_type<T>, 2, 3, 4 };
    b = a;
    check_structure(b, 3, 1);
    CE_CHECK(b[0] == 1);
  }

  constexpr static void move() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    ce::dvector b = { std::in_place_type<T>, 4, 5, 6 };
    b = std::move(a);
    check_structure(a, 0, 0);
    check_structure(b, 3, 3);
    CE_CHECK(b[0] == 1);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[2] == 3);
  }

  constexpr static void move_larger() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    ce::dvector b = { std::in_place_type<T>, 4 };
    b = std::move(a);
    check_structure(a, 0, 0);
    check_structure(b, 3, 3);
    CE_CHECK(b[0] == 1);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[2] == 3);
  }

  constexpr static void move_smaller() {
    ce::dvector a = { std::in_place_type<T>, 1 };
    ce::dvector b = { std::in_place_type<T>, 2, 3, 4 };
    b = std::move(a);
    check_structure(a, 0, 0);
    check_structure(b, 1, 1);
    CE_CHECK(b[0] == 1);
  }

  constexpr static void write() {
    ce::dvector<T> a(2);
    check_structure(a, 2, 2);
    a[0] = 1;
    a[1] = 2;
    CE_CHECK(a[0] == 1);
    CE_CHECK(a[1] == 2);
  }

  constexpr static void front() {
    ce::dvector a = { std::in_place_type<T>, 1 };
    CE_CHECK(a.front() == 1);
    a.front() = 2;
    CE_CHECK(a.front() == 2);
  }

  constexpr static void back() {
    ce::dvector a = { std::in_place_type<T>, 1, 2 };
    CE_CHECK(a.back() == 2);
    a.back() = 3;
    CE_CHECK(a.back() == 3);
  }

  constexpr static void empty() {
    ce::dvector a = { std::in_place_type<T>, 1, 2 };
    CE_CHECK(!a.empty());
    ce::dvector<T> b;
    CE_CHECK(b.empty());
  }

  constexpr static void size() {
    ce::dvector a = { std::in_place_type<T>, 1, 2 };
    CE_CHECK(a.size() == 2);
    ce::dvector<T> b;
    CE_CHECK(b.size() == 0);
  }

  constexpr static void capacity() {
    ce::dvector a = { std::in_place_type<T>, 1, 2 };
    CE_CHECK(a.capacity() == 2);
    ce::dvector<T> b;
    CE_CHECK(b.capacity() == 0);
  }

  constexpr static void reserve() {
    ce::dvector<T> a;
    check_structure(a, 0, 0);
    a.reserve(2);
    check_structure(a, 2, 0);
  }

  constexpr static void shrink_to_fit() {
    ce::dvector<T> a;
    check_structure(a, 0, 0);
    a.reserve(2);
    a.shrink_to_fit();
    check_structure(a, 0, 0);
  }

  constexpr static void emplace_back() {
    ce::dvector<T> a;
    check_structure(a, 0, 0);
    CE_CHECK(a.emplace_back(1) == 1);
    check_structure(a, 1, 1);
    CE_CHECK(a.emplace_back(2) == 2);
    check_structure(a, 2, 2);
    CE_CHECK(a.emplace_back(3) == 3);
    check_structure(a, 4, 3);
    CE_CHECK(a[0] == 1);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[2] == 3);
    a.shrink_to_fit();
    check_structure(a, 3, 3);
  }

  constexpr static void push_back_copy() {
    T x = 1, y = 2, z = 3;
    ce::dvector<T> a;
    check_structure(a, 0, 0);
    CE_CHECK(a.push_back(x) == x);
    check_structure(a, 1, 1);
    CE_CHECK(a.push_back(y) == y);
    check_structure(a, 2, 2);
    CE_CHECK(a.push_back(z) == z);
    check_structure(a, 4, 3);
    CE_CHECK(a[0] == x);
    CE_CHECK(a[1] == y);
    CE_CHECK(a[2] == z);
    a.shrink_to_fit();
    check_structure(a, 3, 3);
  }

  constexpr static void push_back_move() {
    T x = 1, y = 2, z = 3;
    ce::dvector<T> a;
    check_structure(a, 0, 0);
    CE_CHECK(a.push_back(std::move(x)) == 1);
    check_structure(a, 1, 1);
    CE_CHECK(a.push_back(std::move(y)) == 2);
    check_structure(a, 2, 2);
    CE_CHECK(a.push_back(std::move(z)) == 3);
    check_structure(a, 4, 3);
    CE_CHECK(a[0] == 1);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[2] == 3);
    a.shrink_to_fit();
    check_structure(a, 3, 3);
  }

  constexpr static void pop_back() {
    ce::dvector<T> a = { std::in_place, 1, 2, 3 };
    CE_CHECK(a.pop_back() == 3);
    CE_CHECK(a.pop_back() == 2);
    CE_CHECK(a.pop_back() == 1);
    check_structure(a, 3, 0);
    a.shrink_to_fit();
    check_structure(a, 0, 0);
  }

  constexpr static void resize() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    a.resize(3);
    check_structure(a, 3, 3);
    CE_CHECK(a[0] == 1);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[2] == 3);
  }

  constexpr static void resize_smaller() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    a.resize(1);
    check_structure(a, 3, 1);
    CE_CHECK(a[0] == 1);
  }

  constexpr static void resize_larger() {
    if constexpr (std::constructible_from<T>) {
      ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
      a.resize(5);
      check_structure(a, 5, 5);
      CE_CHECK(a[0] == 1);
      CE_CHECK(a[1] == 2);
      CE_CHECK(a[2] == 3);
      CE_CHECK(a[3] == T());
      CE_CHECK(a[4] == T());
    }
  }

  constexpr static void clear() {
    ce::dvector a = { std::in_place_type<T>, 1, 2, 3 };
    check_structure(a, 3, 3);
    a.clear();
    check_structure(a, 3, 0);
    a.clear();
    check_structure(a, 3, 0);
  }

  constexpr static void non_default_ctor() {
    T x = 1, y = 2, z = 3;
    ce::dvector<std::reference_wrapper<T>> a;
    a.emplace_back(x);
    a.emplace_back(y);
    a.emplace_back(z);
    ++a[0].get();
    ++a[1].get();
    ++a[2].get();
    CE_CHECK(x == 2);
    CE_CHECK(y == 3);
    CE_CHECK(z == 4);
    CE_CHECK(a.pop_back().get() == 4);
    CE_CHECK(a.pop_back().get() == 3);
    CE_CHECK(a.pop_back().get() == 2);
    CE_CHECK(a.size() == 0);
  }

  constexpr static void iterator() {
    T x = 1, y = 2, z = 3;
    ce::dvector<std::reference_wrapper<T>> a;
    a.emplace_back(x);
    a.emplace_back(y);
    a.emplace_back(z);
    for (auto&& x : a) {
      ++x.get();
    }
    CE_CHECK(x == 2);
    CE_CHECK(y == 3);
    CE_CHECK(z == 4);
  }

  constexpr static bool all() {
    ctor_default();
    ctor_n();
    read();
    ctor_inplace();
    ctor_inplace_type();
    ctad_inplace();
    ctad_inplace_type();
    ctor_copy();
    ctor_move();
    copy();
    copy_larger();
    copy_smaller();
    move();
    move_larger();
    move_smaller();
    write();
    front();
    back();
    empty();
    size();
    capacity();
    reserve();
    shrink_to_fit();
    emplace_back();
    push_back_copy();
    push_back_move();
    pop_back();
    resize();
    resize_smaller();
    resize_larger();
    clear();
    non_default_ctor();
    iterator();
    return true;
  }
};

#define CE_EXPAND(...) static_assert(tests<Foo<__VA_ARGS__>>::all())
#include "common.decl"
#undef CE_EXPAND

static_assert(tests<int>::all());

int main(int, char* const[]) {
  tests<int>::all();
#define CE_EXPAND(...) tests<Foo<__VA_ARGS__>>::all()
#include "common.decl"
#undef CE_EXPAND
  return 0;
}
