# PC-side unit tests: the real library sources compiled for the PC against a
# fake Arduino API (fake_arduino/) that records pin modes and levels and
# returns preset analogRead() values. Configured from the repo root:
#
#   cmake --workflow --preset host-tests

include(FetchContent)
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.5.3   # same version as phyduino_bsp
    GIT_SHALLOW    TRUE)
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "" FORCE)  # test_main.cpp provides main()
set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(doctest)

add_library(fake_arduino STATIC fake_arduino/fake_arduino.cpp)
target_include_directories(fake_arduino PUBLIC "${CMAKE_CURRENT_LIST_DIR}/fake_arduino")
target_compile_features(fake_arduino PUBLIC cxx_std_17)
target_compile_options(fake_arduino PRIVATE -Wall -Wextra $<$<BOOL:${ANALOG_SHIELD_WERROR}>:-Werror>)

# The library finds the fake Arduino.h through this link
target_link_libraries(analog_shield PUBLIC fake_arduino)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/test_*.cpp")
add_executable(host_tests ${TEST_SOURCES})
target_link_libraries(host_tests PRIVATE analog_shield doctest::doctest)
target_compile_options(host_tests PRIVATE -Wall -Wextra $<$<BOOL:${ANALOG_SHIELD_WERROR}>:-Werror>)

# MinGW: link the C++ runtime statically, so the exe never picks up another
# libstdc++ DLL found first on PATH (fails with 0xc0000139).
if(MINGW)
    target_link_options(host_tests PRIVATE -static)
endif()

add_test(NAME host_tests COMMAND host_tests)
