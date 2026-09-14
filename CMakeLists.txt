cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_POSITION_INDEPENDENT_CODE YES)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(projectMSDL
        LANGUAGES C CXX
        VERSION 2.0.0
        )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(ENABLE_FLAT_PACKAGE "Creates a \"flat\" install layout with the executable, configuration file(s) and preset/texture dirs directly in the install prefix." OFF)
option(ENABLE_INSTALL_BDEPS "Installs all shared libraries projectMSDL requires to run. On some platforms, CMake 3.31 or higher is required for this to work!" OFF)
option(ENABLE_GLES "Build the application to use OpenGL ES instead of Core GL. May not work on all platforms and requires CMake 3.27 or higher." OFF)
option(ENABLE_CONTROL_SOCKET "Build the UNIX-domain control socket for external application control. UNIX platforms only." OFF)

set(PROJECTMSDL_PROPERTIES_FILENAME "projectMSDL.properties")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT ENABLE_FLAT_PACKAGE)
    include(GNUInstallDirs)

    set(PROJECTMSDL_BIN_DIR "${CMAKE_INSTALL_BINDIR}" CACHE STRING "Directory to install executables in, relative to the install prefix.")
    set(PROJECTMSDL_LIB_DIR "${CMAKE_INSTALL_LIBDIR}" CACHE STRING "Directory to install additional libraries in, relative to the install prefix.")
    set(PROJECTMSDL_DATA_DIR "${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}" CACHE STRING "Directory to install the config file, presets and texture, relative to the install prefix.")
    set(PROJECTMSDL_PRESETS_DIR "${PROJECTMSDL_DATA_DIR}/presets" CACHE STRING "Directory to install optional preset files, relative to the install prefix.")
    set(PROJECTMSDL_TEXTURES_DIR "${PROJECTMSDL_DATA_DIR}/textures" CACHE STRING "Directory to install optional texture files, relative to the install prefix.")

    # Additional options for desktop integration
    option(ENABLE_DESKTOP_ICON "Install a .desktop file and icons" ON)
    set(PROJECTMSDL_DESKTOP_DIR "${CMAKE_INSTALL_DATAROOTDIR}/applications" CACHE STRING "Directory to install the .desktop file in, relative to the install prefix.")
    set(PROJECTMSDL_ICONS_DIR "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor" CACHE STRING "Directory to install the icons in, relative to the install prefix.")

    GNUInstallDirs_get_absolute_install_dir(_config_dir_abs_init PROJECTMSDL_CONFIG_DIR DATAROOTDIR)
    set(DEFAULT_CONFIG_PATH "${_config_dir_abs_init}" CACHE STRING "Optional path to look for the configuration file in addition to PROJECTMSDL_BIN_DIR. May include POCO variables like \${application.dir}.")
    set(DEFAULT_PRESETS_PATH "${_config_dir_abs_init}/presets" CACHE STRING "Default presets path in the configuration file. May include POCO variables like \${application.dir}.")
    set(DEFAULT_TEXTURES_PATH "${_config_dir_abs_init}/textures" CACHE STRING "Default textures path in the configuration file. May include POCO variables like \${application.dir}.")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND NOT ENABLE_FLAT_PACKAGE)
    set(DEFAULT_PACKAGING_CONFIG "packaging-macos.cmake")

    # Package as .app bundle on macOS
    set(BUNDLE_BASE_DIR "projectM.app/Contents")
    set(PROJECTMSDL_BIN_DIR "${BUNDLE_BASE_DIR}/MacOS" CACHE STRING "Directory to install executables in, relative to the install prefix.")
    set(PROJECTMSDL_LIB_DIR "${BUNDLE_BASE_DIR}/PlugIns" CACHE STRING "Directory to install additional libraries in, relative to the install prefix.")
    set(PROJECTMSDL_DATA_DIR "${BUNDLE_BASE_DIR}/Resources" CACHE STRING "Directory to install the config file, presets and texture, relative to the install prefix.")
    set(PROJECTMSDL_PRESETS_DIR "${PROJECTMSDL_DATA_DIR}/Presets" CACHE STRING "Directory to install optional preset files, relative to the install prefix.")
    set(PROJECTMSDL_TEXTURES_DIR "${PROJECTMSDL_DATA_DIR}/Textures" CACHE STRING "Directory to install optional texture files, relative to the install prefix.")

    set(DEFAULT_CONFIG_PATH "\${application.dir}/../Resources" CACHE STRING "Optional path to look for the configuration file in addition to PROJECTMSDL_BIN_DIR.")
    set(DEFAULT_PRESETS_PATH "\${application.dir}/../Resources/Presets" CACHE STRING "Default presets path in the configuration file.")
    set(DEFAULT_TEXTURES_PATH "\${application.dir}/../Resources/Textures" CACHE STRING "Default textures path in the configuration file.")

    set(PROJECTMSDL_PROPERTIES_FILENAME "projectM.properties")

    set(CMAKE_INSTALL_RPATH "@loader_path/../PlugIns")
else()
    set(DEFAULT_PACKAGING_CONFIG "packaging-windows.cmake")

    # Windows and others: use flat layout.
    set(PROJECTMSDL_BIN_DIR "." CACHE STRING "Directory to install executables in, relative to the install prefix.")
    set(PROJECTMSDL_LIB_DIR "." CACHE STRING "Directory to install additional libraries in, relative to the install prefix.")
    set(PROJECTMSDL_DATA_DIR "." CACHE STRING "Directory to install the config file, presets and texture, relative to the install prefix.")
    set(PROJECTMSDL_PRESETS_DIR "${PROJECTMSDL_DATA_DIR}/presets" CACHE STRING "Directory to install optional preset files, relative to the install prefix.")
    set(PROJECTMSDL_TEXTURES_DIR "${PROJECTMSDL_DATA_DIR}/textures" CACHE STRING "Directory to install optional texture files, relative to the install prefix.")

    set(DEFAULT_CONFIG_PATH "" CACHE STRING "Optional path to look for the configuration file in addition to PROJECTMSDL_BIN_DIR.")
    set(DEFAULT_PRESETS_PATH "\${application.dir}/presets" CACHE STRING "Default presets path in the configuration file.")
    set(DEFAULT_TEXTURES_PATH "\${application.dir}/textures" CACHE STRING "Default textures path in the configuration file.")

endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DEFAULT_PACKAGING_CONFIG "packaging-linux.cmake")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DEFAULT_PACKAGING_CONFIG "packaging-macos.cmake")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(DEFAULT_PACKAGING_CONFIG "packaging-windows.cmake")
else()
    unset(DEFAULT_PACKAGING_CONFIG)
endif()

set(PACKAGING_CONFIG_FILE "${DEFAULT_PACKAGING_CONFIG}" CACHE FILEPATH "CPack configuration file to use for packaging. This file must \"include(CPack)\" at the end to work properly.")

set(SDL2_LINKAGE "shared" CACHE STRING "Set to either shared or static to specify how libSDL2 should be linked. Defaults to shared.")
option(ENABLE_FREETYPE "Use the Freetype font rendering library instead of the built-in stb_truetype if available" ON)


set(PRESET_DIRS "" CACHE STRING "List of paths with presets. Will be installed in \"presets\" ")
set(TEXTURE_DIRS "" CACHE STRING "List of paths with presets.")

if(NOT SDL2_LINKAGE STREQUAL "shared" AND NOT SDL2_LINKAGE STREQUAL "static")
    message(FATAL_ERROR "Invalid libSDL2 linkage provided in SDL2_LINKAGE: \"${SDL2_LINKAGE}\".\n"
            "Please specify either \"shared\" or \"static\"."
            )
endif()

find_package(projectM4 REQUIRED COMPONENTS Playlist)
find_package(SDL2 REQUIRED)
if (NOT ENABLE_GLES)
    find_package(OpenGL REQUIRED)
else ()
    if (CMAKE_VERSION VERSION_LESS 3.27)
        message(FATAL_ERROR "CMake 3.27 or higher is required to build with OpenGL ES 3!")
    endif ()
    find_package(OpenGL REQUIRED COMPONENTS GLES3)
endif()
find_package(Poco REQUIRED COMPONENTS JSON XML Util Foundation)

if(Poco_VERSION VERSION_GREATER_EQUAL 1.14.0)
    # POCO 1.14 requires at least C++17
    set(CMAKE_CXX_STANDARD 17)
endif()

if(ENABLE_FREETYPE)
    find_package(Freetype)
endif()

include(SDL2Target)
include(dependencies_check.cmake)
include(ImGui.cmake)

add_subdirectory(src)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

include(install.cmake)

if(NOT PACKAGING_CONFIG_FILE STREQUAL "")
    include(${PACKAGING_CONFIG_FILE})
endif()

message(STATUS "Control socket: ${ENABLE_CONTROL_SOCKET}")
message(STATUS "SDL version: ${SDL2_VERSION}")
message(STATUS "Poco version: ${Poco_VERSION}")
message(STATUS "projectM version: ${projectM4_VERSION}")
if(Freetype_FOUND)
    message(STATUS "Freetype version: ${FREETYPE_VERSION_STRING}")
endif()

message(STATUS "The projectMSDL binary will look for the following hard-coded paths if not otherwise set:")
message(STATUS "    Configuration file:  ${DEFAULT_CONFIG_PATH}")
message(STATUS "    Presets:             ${DEFAULT_PRESETS_PATH}")
message(STATUS "    Textures:            ${DEFAULT_TEXTURES_PATH}")
