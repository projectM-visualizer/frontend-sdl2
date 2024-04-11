# projectMSDL Default Packaging Configuration for Linux

# General packaging variables
set(CPACK_PACKAGE_NAME "projectMSDL")
set(CPACK_PACKAGE_VENDOR "The projectM Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/package-description.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A standalone, Milkdrop-like audio visualization application")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://projectm-visualizer.org/")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/icons/icon_32x32.png")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/gpl-3.0.txt")
set(CPACK_STRIP_FILES TRUE)

# Package generator defaults. Override using "cpack -G [generator]"
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)

# DEB generator
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "\"The projectM Development Team\" <projectm@example.com>")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "A standalone, Milkdrop-like audio visualization application")
set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
# Require SDL2 2.0.16 or higher, lower versions can't use monitor devices
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0 (>=2.0.16), libgl1, libfreetype6")
set(CPACK_DEBIAN_ARCHIVE_TYPE "gnutar")
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "standard")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/projectM-visualizer/frontend-sdl2/")

# RPM generator
set(CPACK_RPM_PACKAGE_LICENSE "GPL")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
set(CPACK_RPM_PACKAGE_URL "https://github.com/projectM-visualizer/frontend-sdl2/")

include(CPack)
