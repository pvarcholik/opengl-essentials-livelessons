cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME Library)

find_package(glm CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)

file(GLOB TARGET_SOURCE "../Library.Shared/*.h" "../Library.Shared/*.cpp")

add_library (${TARGET_NAME} STATIC ${TARGET_SOURCE})

target_link_libraries(${TARGET_NAME} PRIVATE Microsoft.GSL::GSL)

set(LIBRARY_CONTENT "${CMAKE_CURRENT_SOURCE_DIR}/Content" CACHE STRING "")
