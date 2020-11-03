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
#include "common.hpp"
#include "Foo.hpp"
#include <functional>
#include <cstdio>
#include <utility>

using namespace ce::tests;

template <typename T>
struct tests {
  constexpr static ce::cvector<T, 1> declare;

  constexpr static bool basic_ctor() {
    ce::cvector<T, 3> a;
    CE_CHECK(size(a) == 0);
    ce::cvector<T, 3> b(3);
    CE_CHECK(size(b) == 3);
    return true;
  }

  constexpr static bool sized_ctor() {
    ce::cvector<T, 3> a(0);
    CE_CHECK(size(a) == 0);
    ce::cvector<T, 3> b(2);
    CE_CHECK(size(b) == 2);
    ce::cvector<T, 3> c(3);
    CE_CHECK(size(c) == 3);
    return true;
  }

  constexpr static bool read() {
    ce::cvector<T, 3> a(2);
    CE_CHECK(size(a) == 2);
    CE_CHECK(a[0] == T());
    CE_CHECK(a[1] == T());
    return true;
  }

  constexpr static bool write() {
    ce::cvector<T, 3> a(2);
    CE_CHECK(size(a) == 2);
    a[0] = 1;
    a[1] = 2;
    CE_CHECK(a[0] == 1);
    CE_CHECK(a[1] == 2);
    return true;
  }

  constexpr static bool front() {
    ce::cvector<T, 3> a(2);
    a[0] = 1;
    CE_CHECK(a.front() == 1);
    a.front() = 2;
    CE_CHECK(a.front() == 2);
    const ce::cvector<T, 3> b = { std::in_place, 1, 2 };
    CE_CHECK(b.front() == 1);
    return true;
  }

  constexpr static bool back() {
    ce::cvector<T, 3> a(1);
    a[0] = 1;
    CE_CHECK(a.back() == 1);
    a.back() = 2;
    CE_CHECK(a.back() == 2);
    const ce::cvector<T, 3> b = { std::in_place, 1, 2 };
    CE_CHECK(b.back() == 2);
    return true;
  }

  constexpr static bool ctad_inplace() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    CE_CHECK(size(a) == 2);
    CE_CHECK(a[0] == 1);
    CE_CHECK(a[1] == 2);
    return true;
  }

  constexpr static bool resize() {
    ce::cvector<T, 3> a;
    a.resize(1);
    CE_CHECK(size(a) == 1);
    return true;
  }

  constexpr static bool resize_data() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    a.resize(2);
    CE_CHECK(size(a) == 2);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool resize_smaller() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    a.resize(1);
    CE_CHECK(size(a) == 1);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool resize_larger() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    a.resize(3);
    CE_CHECK(size(a) == 3);
    CE_CHECK(a[2] == T());
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool pop() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2};
    CE_CHECK(size(a) == 2);
    CE_CHECK(a.pop_back() == 2);
    CE_CHECK(a.pop_back() == 1);
    CE_CHECK(size(a) == 0);
    return true;
  }

  constexpr static bool emplace() {
    ce::cvector<T, 3> a;
    a.emplace_back(1);
    a.emplace_back(2);
    CE_CHECK(size(a) == 2);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool push_ref() {
    ce::cvector<T, 3> a;
    T one(1), two(2);
    a.push_back(one);
    a.push_back(two);
    CE_CHECK(size(a) == 2);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool push_rref() {
    ce::cvector<T, 3> a;
    T one(1), two(2);
    a.push_back(std::move(one));
    a.push_back(std::move(two));
    CE_CHECK(size(a) == 2);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool copy_ctor() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    ce::cvector<T, 3> b = a;
    CE_CHECK(size(b) == size(a));
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[0] == 1);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool move_ctor() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    ce::cvector<T, 3> b = std::move(a);
    CE_CHECK(size(b) == 2);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[0] == 1);
    return true;
  }

  constexpr static bool copy() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    ce::cvector<T, 3> b;
    b = a;
    CE_CHECK(size(b) == size(a));
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[0] == 1);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool copy_smaller() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2, 3 };
    ce::cvector<T, 3> b = { std::in_place, 4, 5 };
    b = a;
    CE_CHECK(size(b) == size(a));
    CE_CHECK(b[2] == 3);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[0] == 1);
    CE_CHECK(a[2] == 3);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool copy_larger() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    ce::cvector<T, 3> b = { std::in_place, 3, 4, 5 };
    b = a;
    CE_CHECK(size(b) == size(a));
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[0] == 1);
    CE_CHECK(a[1] == 2);
    CE_CHECK(a[0] == 1);
    return true;
  }

  constexpr static bool move() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    ce::cvector<T, 3> b;
    b = std::move(a);
    CE_CHECK(size(b) == 2);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[0] == 1);
    return true;
  }

  constexpr static bool move_smaller() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2, 3 };
    ce::cvector<T, 3> b = { std::in_place, 4, 5 };
    b = std::move(a);
    CE_CHECK(size(b) == 3);
    CE_CHECK(b[2] == 3);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[0] == 1);
    return true;
  }

  constexpr static bool move_larger() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2 };
    ce::cvector<T, 3> b = { std::in_place, 3, 4, 5 };
    b = std::move(a);
    CE_CHECK(size(b) == 2);
    CE_CHECK(b[1] == 2);
    CE_CHECK(b[0] == 1);
    return true;
  }

  constexpr static bool clear() {
    ce::cvector<T, 3> a = { std::in_place, 1, 2, 3 };
    CE_CHECK(size(a) == 3);
    a.clear();
    CE_CHECK(size(a) == 0);
    return true;
  }

  constexpr static bool iteration() {
    ce::cvector<T, 16> a = { std::in_place, 1, 2, 3, 4 };
    CE_CHECK(size(a) == 4);
    CE_CHECK(a.begin() == a.begin());
    CE_CHECK(a.end() == a.end());
    CE_CHECK(a.end() - a.begin() == size(a));
    CE_CHECK(*(a.begin() + 1) == a[1]);
    CE_CHECK(a.begin()[1] == a[1]);
    CE_CHECK(*(a.end() - 1) == a[3]);
    CE_CHECK(a.end()[-1] == a[3]);
    return true;
  }

  constexpr static bool all() {
    CE_CHECK(basic_ctor());
    CE_CHECK(sized_ctor());
    CE_CHECK(read());
    CE_CHECK(write());
    CE_CHECK(front());
    CE_CHECK(back());
    CE_CHECK(ctad_inplace());
    CE_CHECK(resize());
    CE_CHECK(resize_data());
    CE_CHECK(resize_smaller());
    CE_CHECK(resize_larger());
    CE_CHECK(pop());
    CE_CHECK(emplace());
    CE_CHECK(push_ref());
    CE_CHECK(push_rref());
    CE_CHECK(copy_ctor());
    CE_CHECK(move_ctor());
    CE_CHECK(copy());
    CE_CHECK(copy_smaller());
    CE_CHECK(copy_larger());
    CE_CHECK(move());
    CE_CHECK(move_smaller());
    CE_CHECK(move_larger());
    CE_CHECK(clear());
    CE_CHECK(iteration());
    return true;
  }
};

constexpr static bool ctad_variadic_conversion() {
  // T-relative someday?
  ce::cvector b = { std::in_place, 1u, 2.0, -1 };
  CE_CHECK(size(b) == 3);
  CE_CHECK(b[0] == 1);
  CE_CHECK(b[1] == 2);
  CE_CHECK(b[2] == unsigned(-1));

  ce::cvector c = { std::in_place_type<double>, 1u, 2.5, -1 };
  CE_CHECK(size(c) == 3);
  CE_CHECK(c[0] ==  1.0);
  CE_CHECK(c[1] ==  2.5);
  CE_CHECK(c[2] == -1.0);
  return true;
}

constexpr static bool iteration() {
  ce::cvector<int, 16> a = { std::in_place, 1, 2, 3, 4 };
  CE_CHECK(size(a) == 4);

  int total = 0;
  for (auto&& i : a) {
    total += i;
  }
  CE_CHECK(total == 10);

  total = 0;
  for (int i : a) {
    total += i;
  }
  CE_CHECK(total == 10);
  return true;
}

constexpr static bool references() {
  int i = 1;
  ce::cvector<std::reference_wrapper<int>, 5> a = {
    std::in_place,
    std::ref(i),
    std::ref(i),
    std::ref(i)
  };
  CE_CHECK(size(a) == 3);
  CE_CHECK(a.pop_back() == 1 && i == 1);
  CE_CHECK(a.pop_back() == 1 && i == 1);
  a.back().get() = 2;
  a.push_back(std::ref(i));
  CE_CHECK(a.pop_back() == 2);
  CE_CHECK(a.pop_back() == 2);
  return true;
}

#define CE_EXPAND(...) static_assert(tests<Foo<__VA_ARGS__>>::all())
#include "common.decl"
#undef CE_EXPAND

static_assert(tests<int>::all());
static_assert(ctad_variadic_conversion());
static_assert(iteration());
static_assert(references());

int main() {

#define CE_EXPAND(...) tests<Foo<__VA_ARGS__>>::all()
#include "common.decl"
#undef CE_EXPAND

  tests<int>::all();
  ctad_variadic_conversion();
  iteration();
  references();
  return 0;
}
