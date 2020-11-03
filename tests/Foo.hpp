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

namespace ce::tests
{
// Class template to instantiate.
template <int dtor,
          int copy_ctor,
          int move_ctor,
          int copy_assign,
          int move_assign>
struct Foo;
}

#define CE_FOO_DTOR_0      constexpr ~Foo() = default;
#define CE_FOO_DTOR_1      constexpr ~Foo() {}

#define CE_FOO_COPY_CTOR_0 constexpr Foo(const Foo&) = default;
#define CE_FOO_COPY_CTOR_1 constexpr Foo(const Foo& b) : n(b.n) {}

#define CE_FOO_MOVE_CTOR_0 constexpr Foo(Foo&&) = default;
#define CE_FOO_MOVE_CTOR_1 constexpr Foo(Foo&& b) : n(b.n) {}

#define CE_FOO_COPY_0      constexpr Foo& operator=(const Foo&) = default;
#define CE_FOO_COPY_1      constexpr Foo& operator=(const Foo& b) { \
    return (n = b.n, *this);                                        \
  }

#define CE_FOO_MOVE_0      constexpr Foo& operator=(Foo&&) = default;
#define CE_FOO_MOVE_1      constexpr Foo& operator=(Foo&& b) {  \
    return (n = b.n, *this);                                    \
  }

#define CE_MAKE_FOO(dtor, copy_ctor, move_ctor, copy, move) \
  namespace ce::tests {                                     \
  template <>                                               \
  struct Foo<dtor, copy_ctor, move_ctor, copy, move> {      \
    int n = 0;                                              \
    constexpr Foo() = default;                              \
    constexpr Foo(int n) : n(n) {}                          \
    CE_FOO_DTOR_##dtor;                                     \
    CE_FOO_COPY_CTOR_##copy_ctor;                           \
    CE_FOO_MOVE_CTOR_##move_ctor;                           \
    CE_FOO_COPY_##copy;                                     \
    CE_FOO_MOVE_##move;                                     \
    constexpr bool operator==(const Foo& b) const {         \
      return n == b.n;                                      \
    }                                                       \
    constexpr auto operator<=>(const Foo& b) const {        \
      return n <=> b.n;                                     \
    }                                                       \
    constexpr bool operator==(int b) const {                \
      return n == b;                                        \
    }                                                       \
    constexpr auto operator<=>(int b) const {               \
      return n <=> b;                                       \
    }                                                       \
  };                                                        \
  }

#define CE_EXPAND(...) CE_MAKE_FOO(__VA_ARGS__)
#include "common.decl"
#undef CE_EXPAND
