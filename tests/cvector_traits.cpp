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

// This test file just instantiates a class template with the truth table of
// trivial traits in order to make sure that a cvector of <T> has the same
// traits as T itself, for all possible combinations.
//
// The cvector cannot be trivially constructible so that's not one that we'll
// need to test.

// Class template to instantiate.
template <int dtor,
          int copy_ctor,
          int move_ctor,
          int copy_assign,
          int move_assign>
struct T;

#define DTOR_0      constexpr ~T() = default;
#define DTOR_1      constexpr ~T() {}

#define COPY_CTOR_0 constexpr T(const T&) = default;
#define COPY_CTOR_1 constexpr T(const T&) {}

#define MOVE_CTOR_0 constexpr T(T&&) = default;
#define MOVE_CTOR_1 constexpr T(T&&) {}

#define COPY_0      constexpr T& operator=(const T&) = default;
#define COPY_1      constexpr T& operator=(const T&) { return *this; }

#define MOVE_0      constexpr T& operator=(T&&) = default;
#define MOVE_1      constexpr T& operator=(T&&) { return *this; }

#define MAKE_T(dtor, copy_ctor, move_ctor, copy, move)      \
  template <>                                               \
  struct T<dtor, copy_ctor, move_ctor, copy, move> {        \
    constexpr T() = default;                                \
    DTOR_##dtor;                                            \
    COPY_CTOR_##copy_ctor;                                  \
    MOVE_CTOR_##move_ctor;                                  \
    COPY_##copy;                                            \
    MOVE_##move;                                            \
  }

MAKE_T(0, 0, 0, 0, 0);
MAKE_T(0, 0, 0, 0, 1);
MAKE_T(0, 0, 0, 1, 0);
MAKE_T(0, 0, 0, 1, 1);
MAKE_T(0, 0, 1, 0, 0);
MAKE_T(0, 0, 1, 0, 1);
MAKE_T(0, 0, 1, 1, 0);
MAKE_T(0, 0, 1, 1, 1);
MAKE_T(0, 1, 0, 0, 0);
MAKE_T(0, 1, 0, 0, 1);
MAKE_T(0, 1, 0, 1, 0);
MAKE_T(0, 1, 0, 1, 1);
MAKE_T(0, 1, 1, 0, 0);
MAKE_T(0, 1, 1, 0, 1);
MAKE_T(0, 1, 1, 1, 0);
MAKE_T(0, 1, 1, 1, 1);
MAKE_T(1, 0, 0, 0, 0);
MAKE_T(1, 0, 0, 0, 1);
MAKE_T(1, 0, 0, 1, 0);
MAKE_T(1, 0, 0, 1, 1);
MAKE_T(1, 0, 1, 0, 0);
MAKE_T(1, 0, 1, 0, 1);
MAKE_T(1, 0, 1, 1, 0);
MAKE_T(1, 0, 1, 1, 1);
MAKE_T(1, 1, 0, 0, 0);
MAKE_T(1, 1, 0, 0, 1);
MAKE_T(1, 1, 0, 1, 0);
MAKE_T(1, 1, 0, 1, 1);
MAKE_T(1, 1, 1, 0, 0);
MAKE_T(1, 1, 1, 0, 1);
MAKE_T(1, 1, 1, 1, 0);
MAKE_T(1, 1, 1, 1, 1);

TEMPLATE_TEST_CASE_SIG("Check trait propagation", "[cvector][traits]",
                       ((int dtor, int copy_ctor, int move_ctor, int copy, int move), dtor, copy_ctor, move_ctor, copy, move),
                       (0, 0, 0, 0, 0),
                       (0, 0, 0, 0, 1),
                       (0, 0, 0, 1, 0),
                       (0, 0, 0, 1, 1),
                       (0, 0, 1, 0, 0),
                       (0, 0, 1, 0, 1),
                       (0, 0, 1, 1, 0),
                       (0, 0, 1, 1, 1),

                       (0, 1, 0, 0, 0),
                       (0, 1, 0, 0, 1),
                       (0, 1, 0, 1, 0),
                       (0, 1, 0, 1, 1),
                       (0, 1, 1, 0, 0),
                       (0, 1, 1, 0, 1),
                       (0, 1, 1, 1, 0),
                       (0, 1, 1, 1, 1),

                       (1, 0, 0, 0, 0),
                       (1, 0, 0, 0, 1),
                       (1, 0, 0, 1, 0),
                       (1, 0, 0, 1, 1),
                       (1, 0, 1, 0, 0),
                       (1, 0, 1, 0, 1),
                       (1, 0, 1, 1, 0),
                       (1, 0, 1, 1, 1),

                       (1, 1, 0, 0, 0),
                       (1, 1, 0, 0, 1),
                       (1, 1, 0, 1, 0),
                       (1, 1, 0, 1, 1),
                       (1, 1, 1, 0, 0),
                       (1, 1, 1, 0, 1),
                       (1, 1, 1, 1, 0),
                       (1, 1, 1, 1, 1))
{
  using U = T<dtor, copy_ctor, move_ctor, copy, move>;
  using V = ce::cvector<U, 8>;
  static_assert(not (std::is_trivially_destructible_v<U> ^ std::is_trivially_destructible_v<V>));
  static_assert(not (std::is_trivially_copy_constructible_v<U> ^ std::is_trivially_copy_constructible_v<V>));
  static_assert(not (std::is_trivially_move_constructible_v<U> ^ std::is_trivially_move_constructible_v<V>));
  static_assert(not (std::is_trivially_copy_assignable_v<U> ^ std::is_trivially_copy_assignable_v<V>));
  static_assert(not (std::is_trivially_move_assignable_v<U> ^ std::is_trivially_move_assignable_v<V>));
  constexpr V decl;
  constexpr V ctor{};
}
