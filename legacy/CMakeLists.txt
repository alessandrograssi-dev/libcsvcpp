add_library(libcsv STATIC
    libcsv.c
)

target_include_directories(libcsv
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(libcsv
    PUBLIC c_std_99
)

# Optional but nice
set_target_properties(libcsv PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Build original C test suite for behavioral verification
add_executable(test_csv test_csv.c)
target_link_libraries(test_csv PRIVATE libcsv)

# Register as CTest for CI/CD
enable_testing()
add_test(NAME legacy_c_tests COMMAND test_csv)
