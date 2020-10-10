include(FetchContent)

FetchContent_Declare(fmt
  GIT_REPOSITORY     https://github.com/fmtlib/fmt.git
  GIT_TAG            master)
FetchContent_MakeAvailable(fmt)

#TODO use Catch2 instead of raw main()
add_executable(cvector cvector.cpp cvector_traits.cpp)
target_link_libraries(cvector PRIVATE ce::ce fmt::fmt)
add_test(NAME cvector_tests COMMAND cvector)
