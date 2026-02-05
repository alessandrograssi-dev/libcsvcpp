add_executable(csvtest csvtest.cpp)
target_include_directories(csvtest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../csvcpp/include)
target_link_libraries(csvtest PRIVATE csvcpp)

add_executable(csvinfo csvinfo.cpp)
target_include_directories(csvinfo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../csvcpp/include)
target_link_libraries(csvinfo PRIVATE csvcpp)

add_executable(csvfix csvfix.cpp)
target_include_directories(csvfix PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../csvcpp/include)
target_link_libraries(csvfix PRIVATE csvcpp)

add_executable(csvvalid csvvalid.cpp)
target_include_directories(csvvalid PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../csvcpp/include)
target_link_libraries(csvvalid PRIVATE csvcpp)
