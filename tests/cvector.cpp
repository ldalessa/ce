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
#include <catch2/catch_all.hpp>
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

TEMPLATE_TEST_CASE("Basic Construction", "[cvector][ctor]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a;
    assert(size(a) == 0);
    cvector<TestType, 3> b(3);
    assert(size(b) == 3);
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Sized Construction", "[cvector][ctor]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a(0);
    assert(size(a) == 0);
    cvector<TestType, 3> b(2);
    assert(size(b) == 2);
    cvector<TestType, 3> c(3);
    assert(size(c) == 3);
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Read Access", "[cvector][read]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a(2);
    assert(size(a) == 2);
    assert(a[0] == TestType());
    assert(a[1] == TestType());
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Write Access", "[cvector][write]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a(2);
    assert(size(a) == 2);
    a[0] = TestType(1);
    a[1] = TestType(2);
    assert(a[0] == TestType(1));
    assert(a[1] == TestType(2));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("CTAD Variadic Construction", "[cvector][ctad]", int, Foo) {
  constexpr auto inplace = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    assert(size(a) == 2);
    assert(a[0] == TestType(1));
    assert(a[1] == TestType(2));
    return true;
  };
  static_assert(inplace());
  inplace();
}

TEMPLATE_TEST_CASE("CTAD Variadic Construction with Conversion", "[cvector][ctad]", int) {
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
}

TEMPLATE_TEST_CASE("Resizing", "[cvector][resize]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a;
    a.resize(1);
    assert(size(a) == 1);
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Resizing with data", "[cvector][resize]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    a.resize(2);
    assert(size(a) == 2);
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Resizing smaller", "[cvector][resize]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    a.resize(1);
    assert(size(a) == 1);
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Resizing larger", "[cvector][resize]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    a.resize(3);
    assert(size(a) == 3);
    assert(a[2] == TestType());
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Pop", "[cvector][pop]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2};
    assert(size(a) == 2);
    assert(a.pop_back() == TestType(2));
    assert(a.pop_back() == TestType(1));
    assert(size(a) == 0);
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Emplace", "[cvector][emplace]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a;
    a.emplace_back(1);
    a.emplace_back(2);
    assert(size(a) == 2);
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Pushing references", "[cvector][push]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a;
    TestType one(1), two(2);
    a.push_back(one);
    a.push_back(two);
    assert(size(a) == 2);
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Pushing r-value references", "[cvector][push]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a;
    TestType one(1), two(2);
    a.push_back(std::move(one));
    a.push_back(std::move(two));
    assert(size(a) == 2);
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Copy contructor", "[cvector][ctor]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    cvector<TestType, 3> b = a;
    assert(size(b) == size(a));
    assert(b[1] == TestType(2));
    assert(b[0] == TestType(1));
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Move contructor", "[cvector][ctor]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    cvector<TestType, 3> b = std::move(a);
    assert(size(b) == 2);
    assert(b[1] == TestType(2));
    assert(b[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Copy assignment", "[cvector][assign]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    cvector<TestType, 3> b;
    b = a;
    assert(size(b) == size(a));
    assert(b[1] == TestType(2));
    assert(b[0] == TestType(1));
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Copy assignment smaller to larger", "[cvector][assign]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2, 3 };
    cvector<TestType, 3> b = { std::in_place, 4, 5 };
    b = a;
    assert(size(b) == size(a));
    assert(b[2] == TestType(3));
    assert(b[1] == TestType(2));
    assert(b[0] == TestType(1));
    assert(a[2] == TestType(3));
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Copy assignment larger to smaller", "[cvector][assign]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    cvector<TestType, 3> b = { std::in_place, 3, 4, 5 };
    b = a;
    assert(size(b) == size(a));
    assert(b[1] == TestType(2));
    assert(b[0] == TestType(1));
    assert(a[1] == TestType(2));
    assert(a[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Move assignment", "[cvector][assign]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    cvector<TestType, 3> b;
    b = std::move(a);
    assert(size(b) == 2);
    assert(b[1] == TestType(2));
    assert(b[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Move assignment larger to smaller", "[cvector][assign]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2, 3 };
    cvector<TestType, 3> b = { std::in_place, 4, 5 };
    b = std::move(a);
    assert(size(b) == 3);
    assert(b[2] == TestType(3));
    assert(b[1] == TestType(2));
    assert(b[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Move assignment smaller to larger", "[cvector][assign]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2 };
    cvector<TestType, 3> b = { std::in_place, 3, 4, 5 };
    b = std::move(a);
    assert(size(b) == 2);
    assert(b[1] == TestType(2));
    assert(b[0] == TestType(1));
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Clearing", "[cvector][clear]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 3> a = { std::in_place, 1, 2, 3 };
    assert(size(a) == 3);
    a.clear();
    assert(size(a) == 0);
    return true;
  };
  static_assert(test());
  test();
}

TEMPLATE_TEST_CASE("Iterators", "[cvector][iteration]", int, Foo) {
  constexpr auto test = [] {
    cvector<TestType, 16> a = { std::in_place, 1, 2, 3, 4 };
    assert(size(a) == 4);
    assert(a.begin() == a.begin());
    assert(a.end() == a.end());
    assert(a.end() - a.begin() == size(a));
    assert(*(a.begin() + 1) == a[1]);
    assert(a.begin()[1] == a[1]);
    assert(*(a.end() - 1) == a[3]);
    assert(a.end()[-1] == a[3]);
    return true;
  };
  static_assert(test());
  test();
}

TEST_CASE("Iteration", "[cvector][iteration]") {
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
}

TEST_CASE("Test with references", "[cvector]") {
  constexpr auto test = [] {
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
  };
  static_assert(test());
  test();
}
