include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
    SYSTEM
    EXCLUDE_FROM_ALL
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(avernal-assets-render-tests avmesh_io_test.cpp)
target_link_libraries(avernal-assets-render-tests PRIVATE avernal::assets_render GTest::gtest_main)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(avernal-assets-render-tests PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(avernal-assets-render-tests PRIVATE /W4)
endif()

include(GoogleTest)
gtest_discover_tests(avernal-assets-render-tests DISCOVERY_MODE PRE_TEST)
