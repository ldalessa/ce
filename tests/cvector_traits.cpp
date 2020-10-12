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

// Just check to make sure we correctly propagate traits of T into cvector of
// T.

template <int dtor,
          int copy_ctor,
          int move_ctor,
          int copy_assign,
          int move_assign>
struct Foo;

template <typename T>
struct check_traits
{
  using vector = ce::cvector<T, 8>;
  static_assert(not (std::is_trivially_destructible_v<T> ^ std::is_trivially_destructible_v<vector>));
  static_assert(not (std::is_trivially_copy_constructible_v<T> ^ std::is_trivially_copy_constructible_v<vector>));
  static_assert(not (std::is_trivially_move_constructible_v<T> ^ std::is_trivially_move_constructible_v<vector>));
  static_assert(not (std::is_trivially_copy_assignable_v<T> ^ std::is_trivially_copy_assignable_v<vector>));
  static_assert(not (std::is_trivially_move_assignable_v<T> ^ std::is_trivially_move_assignable_v<vector>));
};

#define DTOR_0
#define COPY_CTOR_0
#define MOVE_CTOR_0
#define COPY_0
#define MOVE_0

#define DTOR_1      constexpr ~Foo() {}
#define COPY_CTOR_1 constexpr  Foo(const Foo&) {}
#define MOVE_CTOR_1 constexpr  Foo(Foo&&) {}
#define COPY_1      constexpr  Foo& operator=(const Foo&) { return *this; }
#define MOVE_1      constexpr  Foo& operator=(Foo&&)      { return *this; }

#define MAKE_FOO(dtor, copy_ctor, move_ctor, copy, move)    \
  template <>                                               \
  struct Foo<dtor, copy_ctor, move_ctor, copy, move> {      \
    DTOR_##dtor;                                            \
    COPY_CTOR_##copy_ctor;                                  \
    MOVE_CTOR_##move_ctor;                                  \
    COPY_##copy;                                            \
    MOVE_##move;                                            \
  }


#define WRAP(x) x

#define MAKE_CHECK(dtor, copy_ctor, move_ctor, copy, move)              \
  MAKE_FOO(dtor, copy_ctor, move_ctor, copy, move);                     \
  constexpr check_traits<Foo<dtor, copy_ctor, move_ctor, copy, move>> check##dtor##copy_ctor##move_ctor##copy##move;

MAKE_CHECK(0, 0, 0, 0, 0);
MAKE_CHECK(0, 0, 0, 0, 1);
MAKE_CHECK(0, 0, 0, 1, 0);
MAKE_CHECK(0, 0, 0, 1, 1);
MAKE_CHECK(0, 0, 1, 0, 0);
MAKE_CHECK(0, 0, 1, 0, 1);
MAKE_CHECK(0, 0, 1, 1, 0);
MAKE_CHECK(0, 0, 1, 1, 1);
MAKE_CHECK(0, 1, 0, 0, 0);
MAKE_CHECK(0, 1, 0, 0, 1);
MAKE_CHECK(0, 1, 0, 1, 0);
MAKE_CHECK(0, 1, 0, 1, 1);
MAKE_CHECK(0, 1, 1, 0, 0);
MAKE_CHECK(0, 1, 1, 0, 1);
MAKE_CHECK(0, 1, 1, 1, 0);
MAKE_CHECK(0, 1, 1, 1, 1);
MAKE_CHECK(1, 0, 0, 0, 0);
MAKE_CHECK(1, 0, 0, 0, 1);
MAKE_CHECK(1, 0, 0, 1, 0);
MAKE_CHECK(1, 0, 0, 1, 1);
MAKE_CHECK(1, 0, 1, 0, 0);
MAKE_CHECK(1, 0, 1, 0, 1);
MAKE_CHECK(1, 0, 1, 1, 0);
MAKE_CHECK(1, 0, 1, 1, 1);
MAKE_CHECK(1, 1, 0, 0, 0);
MAKE_CHECK(1, 1, 0, 0, 1);
MAKE_CHECK(1, 1, 0, 1, 0);
MAKE_CHECK(1, 1, 0, 1, 1);
MAKE_CHECK(1, 1, 1, 0, 0);
MAKE_CHECK(1, 1, 1, 0, 1);
MAKE_CHECK(1, 1, 1, 1, 0);
MAKE_CHECK(1, 1, 1, 1, 1);
