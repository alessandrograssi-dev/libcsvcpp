add_library(csvcpp
    src/CsvParser.cpp
)

target_include_directories(csvcpp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(csvcpp
    PRIVATE
        libcsv
)

target_compile_features(csvcpp
    PUBLIC cxx_std_17
)

# Warnings (optional but good)
if (MSVC)
    target_compile_options(csvcpp PRIVATE /W4)
else()
    target_compile_options(csvcpp PRIVATE -Wall -Wextra -Wpedantic)
endif()
