cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

if(CMAKE_VERSION VERSION_LESS 3.19 AND CMAKE_GENERATOR STREQUAL "Xcode")
    message(AUTHOR_WARNING "Using a CMake version before 3.19 with a recent Xcode SDK and the Xcode generator "
            "will likely result in CMake failing to find the AppleClang compiler. Either upgrade CMake to at least "
            "version 3.19 or use a different generator, e.g. \"Unix Makefiles\" or \"Ninja\".")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_POSITION_INDEPENDENT_CODE YES)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(projectM-SDL
        LANGUAGES C CXX
        VERSION 2.0.0
        )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(PROJECTM_LINKAGE "shared" CACHE STRING "Set to either shared or static to specify how libprojectM should be linked. Defaults to shared.")
set(SDL2_LINKAGE "shared" CACHE STRING "Set to either shared or static to specify how libSDL2 should be linked. Defaults to shared.")

if(NOT PROJECTM_LINKAGE STREQUAL "shared" AND NOT PROJECTM_LINKAGE STREQUAL "static")
    message(FATAL_ERROR "Invalid libprojectM linkage provided in PROJECTM_LINKAGE: \"${PROJECTM_LINKAGE}\".\n"
                        "Please specify either \"shared\" or \"static\"."
            )
endif()

if(NOT SDL2_LINKAGE STREQUAL "shared" AND NOT SDL2_LINKAGE STREQUAL "static")
    message(FATAL_ERROR "Invalid libSDL2 linkage provided in SDL2_LINKAGE: \"${SDL2_LINKAGE}\".\n"
            "Please specify either \"shared\" or \"static\"."
            )
endif()

find_package(libprojectM REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Poco REQUIRED COMPONENTS Util)

include(SDL2Target)
include(dependencies_check.cmake)

add_subdirectory(src)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

include(install.cmake)
include(packaging.cmake)

message(STATUS "SDL version: ${SDL2_VERSION}")
message(STATUS "Poco version: ${Poco_VERSION}")
message(STATUS "projectM version: ${libprojectM_VERSION}")