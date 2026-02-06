add_executable(test_parity test_csv.cpp)
target_link_libraries(test_parity csvcpp)
target_include_directories(test_parity PRIVATE ${CMAKE_SOURCE_DIR}/csvcpp/include)

# Allow legacy-style C test code to compile under clang++
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(test_parity PRIVATE -fpermissive -Wno-writable-strings)
endif()

# Enable testing
enable_testing()
add_test(NAME test_parity COMMAND test_parity)
