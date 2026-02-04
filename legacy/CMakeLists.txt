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
