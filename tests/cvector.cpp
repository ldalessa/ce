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

#include "ce/cvector.hpp"
#include <fmt/core.h>
#include <functional>
#include <utility>

using ce::cvector;

struct Foo {
  int a;
  mutable int b;

  // unit tests want this to work
  constexpr bool operator==(const Foo& rhs) const {
    return a == rhs.a;
  }

  constexpr Foo() : a(-1), b(0) {
    if (!std::is_constant_evaluated()) {
      fmt::print(" Foo({}) (default ctor)\n", a);
    }
  }

  constexpr Foo(int a) : a(a), b(0) {
    if (!std::is_constant_evaluated()) {
      fmt::print(" Foo({}) (ctor)\n", a);
    }
  }

  constexpr Foo(const Foo& rhs) : a(rhs.a) {
    if (!std::is_constant_evaluated()) {
      fmt::print(" Foo(const Foo&({})) (copy ctor)\n", a);
    }
    rhs.b = -4;
  }

  constexpr Foo(Foo&& rhs) : a(rhs.a) {
    if (!std::is_constant_evaluated()) {
      fmt::print(" Foo(Foo&&({})) (move ctor)\n", a);
    }
    rhs.b = -3;
  }

  constexpr Foo& operator=(const Foo& rhs) {
    a = rhs.a;
    rhs.b = -5;
    if (!std::is_constant_evaluated()) {
      fmt::print(" Foo = const Foo&({}) (copy)\n", a);
    }
    return *this;
  }

  constexpr Foo& operator=(Foo&& rhs) {
    a = rhs.a;
    //assert(rhs.b == 0);
    rhs.b = -2;
    if (!std::is_constant_evaluated()) {
      fmt::print(" Foo = Foo&&({}) (move)\n", a);
    }
    return *this;
  }

  constexpr ~Foo() {
    if (!std::is_constant_evaluated()) {
      switch (b) {
       case -1: fmt::print("~Foo({}) (uninitialized)\n", a);         return;
       case -2: fmt::print("~Foo({}) (move assigned from)\n", a);    return;
       case -3: fmt::print("~Foo({}) (move constructed from)\n", a); return;
       case -4: fmt::print("~Foo({}) (copy constructed from)\n", a); return;
       case -5: fmt::print("~Foo({}) (copied from)\n", a);           return;
       default: fmt::print("~Foo({})\n", a); return;
      }
    }
  }
};

template <typename T>
constexpr bool ctor() {
  cvector<T, 3> a;
  assert(size(a) == 0);
  cvector<T, 3> b(3);
  assert(size(b) == 3);
  return true;
}

template <typename T>
constexpr bool variadic_ctor() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  assert(size(a) == 2);
  return true;
}

template <typename T>
constexpr bool read() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  assert(size(a) == 2);
  assert(a[0] == T(1));
  assert(a[1] == T(2));
  return true;
}

template <typename T>
constexpr bool ctad() {
  cvector a = { std::in_place, T(1), T(2) };
    assert(size(a) == 2);
    assert(a[0] == T(1));
    assert(a[1] == T(2));

    // T-relative someday?
    cvector b = { std::in_place, 1u, 2.0, -1 };
    assert(size(b) == 3);
    assert(b[0] == 1);
    assert(b[1] == 2);
    assert(b[2] == unsigned(-1));

    cvector c = { std::in_place_type<double>, 1u, 2.5, -1 };
    assert(size(c) == 3);
    assert(c[0] ==  1.0);
    assert(c[1] ==  2.5);
    assert(c[2] == -1.0);
    return true;
}

template <typename T>
constexpr bool resize() {
    cvector<T, 3> a;
    a.resize(1);
    assert(size(a) == 1);
    return true;
}

template <typename T>
constexpr bool assign() {
  cvector<Foo, 3> a;
  a.resize(1);
  assert(size(a) == 1);
  assert((a[0] = 1) == T(1));
  return true;
}

template <typename T>
constexpr bool pop() {
  cvector<T, 3> a = { std::in_place, 1, 2};
  assert(size(a) == 2);
  assert(a.pop_back() == T(2));
  assert(a.pop_back() == T(1));
  assert(size(a) == 0);
  return true;
}

template <typename T>
constexpr bool emplace() {
  cvector<T, 3> a;
  a.emplace_back(1);
  a.emplace_back(2);
  assert(size(a) == 2);
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool push_ref() {
  cvector<T, 3> a;
  T one(1), two(2);
  a.push_back(one);
  a.push_back(two);
  assert(size(a) == 2);
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool push_rref() {
  cvector<T, 3> a;
  T one(1), two(2);
  a.push_back(std::move(one));
  a.push_back(std::move(two));
  assert(size(a) == 2);
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool copy_ctor() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  if (!std::is_constant_evaluated()) {
    fmt::print("constructing cvector<T, 3> b = a (begin)\n");
  }
  cvector<T, 3> b = a;
  if (!std::is_constant_evaluated()) {
    fmt::print("constructing cvector<T, 3> b = a (end)\n");
  }
  assert(size(b) == size(a));
  assert(b[1] == T(2));
  assert(b[0] == T(1));
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool move_ctor() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  cvector<T, 3> b = std::move(a);
  assert(size(b) == 2);
  assert(b[1] == T(2));
  assert(b[0] == T(1));
  return true;
}

template <typename T>
constexpr bool copy() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  cvector<T, 3> b;
  if (!std::is_constant_evaluated()) {
    fmt::print("copying b = a (begin) [{}, {}]\n", b.n, a.n);
  }
  b = a;
  if (!std::is_constant_evaluated()) {
    fmt::print("copying b = a (complete)\n");
  }
  assert(size(b) == size(a));
  assert(b[1] == T(2));
  assert(b[0] == T(1));
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool copy_larger() {
  cvector<T, 3> a = { std::in_place, 1, 2, 3 };
  cvector<T, 3> b = { std::in_place, 4, 5 };
  b = a;
  assert(size(b) == size(a));
  assert(b[2] == T(3));
  assert(b[1] == T(2));
  assert(b[0] == T(1));
  assert(a[2] == T(3));
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool copy_smaller() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  cvector<T, 3> b = { std::in_place, 3, 4, 5 };
  b = a;
  assert(size(b) == size(a));
  assert(b[1] == T(2));
  assert(b[0] == T(1));
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool move() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  cvector<T, 3> b;
  b = std::move(a);
  assert(size(b) == 2);
  assert(b[1] == T(2));
  assert(b[0] == T(1));
  return true;
}

template <typename T>
constexpr bool move_larger() {
  cvector<T, 3> a = { std::in_place, 1, 2, 3 };
  cvector<T, 3> b = { std::in_place, 4, 5 };
  b = std::move(a);
  assert(size(b) == 3);
  assert(b[2] == T(3));
  assert(b[1] == T(2));
  assert(b[0] == T(1));
  return true;
}

template <typename T>
constexpr bool move_smaller() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  cvector<T, 3> b = { std::in_place, 3, 4, 5 };
  b = std::move(a);
  assert(size(b) == 2);
  assert(b[1] == T(2));
  assert(b[0] == T(1));
  return true;
}

template <typename T>
constexpr bool resize_inplace() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  a.resize(2);
  assert(size(a) == 2);
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool resize_smaller() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  a.resize(1);
  assert(size(a) == 1);
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool resize_larger() {
  cvector<T, 3> a = { std::in_place, 1, 2 };
  a.resize(3);
  assert(size(a) == 3);
  assert(a[2] == T());
  assert(a[1] == T(2));
  assert(a[0] == T(1));
  return true;
}

template <typename T>
constexpr bool clear() {
  cvector<Foo, 3> a = { std::in_place, 1, 2, 3 };
  assert(size(a) == 3);
  a.clear();
  assert(size(a) == 0);
  return true;
}

template <typename T>
constexpr bool iterator() {
  cvector<T, 16> a = { std::in_place, 1, 2, 3, 4 };
  assert(size(a) == 4);
  assert(a.begin() == a.begin());
  assert(a.end() == a.end());
  assert(a.end() - a.begin() == size(a));
  assert(*(a.begin() + 1) == a[1]);
  assert(a.begin()[1] == a[1]);
  assert(*(a.end() - 1) == a[3]);
  assert(a.end()[-1] == a[3]);
  return true;
}

constexpr bool iteration() {
  cvector<int, 16> a = { std::in_place, 1, 2, 3, 4 };
  assert(size(a) == 4);

  int total = 0;
  for (auto&& i : a) {
    total += i;
  }
  assert(total == 10);

  total = 0;
  for (int i : a) {
    total += i;
  }
  assert(total == 10);
  return true;
}

constexpr bool ref_wrapper() {
  int i = 1;
  cvector<std::reference_wrapper<int>, 5> a = { std::in_place, std::ref(i), std::ref(i), std::ref(i) };
  assert(size(a) == 3);
  assert(a.pop_back() == 1 && i == 1);
  assert(a.pop_back() == 1 && i == 1);
  a.back().get() = 2;
  a.push_back(std::ref(i));
  assert(a.pop_back() == 2);
  assert(a.pop_back() == 2);
  return true;
}

// these could go anywhere
static_assert(ctor<int>());
static_assert(ctor<Foo>());
static_assert(variadic_ctor<int>());
static_assert(variadic_ctor<Foo>());
static_assert(read<int>());
static_assert(read<Foo>());
static_assert(ctad<int>());
static_assert(ctad<Foo>());
static_assert(resize<int>());
static_assert(resize<Foo>());
static_assert(assign<int>());
static_assert(assign<Foo>());
static_assert(pop<int>());
static_assert(pop<Foo>());
static_assert(emplace<int>());
static_assert(emplace<Foo>());
static_assert(push_ref<int>());
static_assert(push_ref<Foo>());
static_assert(push_rref<int>());
static_assert(push_rref<Foo>());
static_assert(copy_ctor<int>());
static_assert(copy_ctor<Foo>());
static_assert(move_ctor<int>());
static_assert(move_ctor<Foo>());
// static_assert(copy<int>());
// static_assert(copy<Foo>());
// static_assert(copy_larger<int>());
// static_assert(copy_larger<Foo>());
static_assert(copy_smaller<int>());
static_assert(copy_smaller<Foo>());
// static_assert(move<int>());
// static_assert(move<Foo>());
// static_assert(move_larger<int>());
// static_assert(move_larger<Foo>());
static_assert(move_smaller<int>());
static_assert(move_smaller<Foo>());
static_assert(resize_inplace<int>());
static_assert(resize_inplace<Foo>());
static_assert(resize_larger<int>());
static_assert(resize_larger<Foo>());
static_assert(resize_smaller<int>());
static_assert(resize_smaller<Foo>());
static_assert(clear<int>());
static_assert(clear<Foo>());
static_assert(iterator<int>());
static_assert(iterator<Foo>());
static_assert(iteration());
static_assert(ref_wrapper());

int main() {
  fmt::print("checking ctor\n");
  ctor<Foo>();

  fmt::print("checking variadic ctor\n");
  variadic_ctor<Foo>();

  fmt::print("checking read access\n");
  read<Foo>();

  fmt::print("checking ctad\n");
  ctad<Foo>();

  fmt::print("checking resize\n");
  resize<Foo>();

  fmt::print("checking assign\n");
  assign<Foo>();

  fmt::print("checking pop\n");
  pop<Foo>();

  fmt::print("checking emplace\n");
  emplace<Foo>();

  fmt::print("checking push ref\n");
  push_ref<Foo>();

  fmt::print("checking push rvalue ref\n");
  push_rref<Foo>();

  fmt::print("checking copy ctor\n");
  copy_ctor<Foo>();

  fmt::print("checking move ctor\n");
  move_ctor<Foo>();

  fmt::print("checking copy operator\n");
  copy<Foo>();

  fmt::print("checking copy larger operator\n");
  copy_larger<Foo>();

  fmt::print("checking copy smaller operator\n");
  copy_smaller<Foo>();

  fmt::print("checking move operator\n");
  move<Foo>();

  fmt::print("checking move larger operator\n");
  move_larger<Foo>();

  fmt::print("checking move smaller operator\n");
  move_smaller<Foo>();

  fmt::print("checking resize inplace\n");
  resize_inplace<Foo>();

  fmt::print("checking resize smaller\n");
  resize_smaller<Foo>();

  fmt::print("checking resize larger\n");
  resize_larger<Foo>();

  fmt::print("checking clear\n");
  clear<Foo>();

  fmt::print("checking iterator\n");
  iterator<Foo>();

  fmt::print("checking iteration\n");
  iteration();

  fmt::print("checking ref wrapper\n");
  ref_wrapper();

  return 0;
}
