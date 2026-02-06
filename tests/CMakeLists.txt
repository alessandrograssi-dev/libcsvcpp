add_executable(test_csv test_csv.cpp)
target_link_libraries(test_csv csvcpp)
target_include_directories(test_csv PRIVATE ${CMAKE_SOURCE_DIR}/csvcpp/include)

# Enable testing
enable_testing()
add_test(NAME test_csv COMMAND test_csv)
