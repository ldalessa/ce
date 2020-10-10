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
