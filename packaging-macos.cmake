# projectMSDL Default Packaging Configuration for Linux

# General packaging variables
set(CPACK_PACKAGE_NAME "projectM")
set(CPACK_PACKAGE_VENDOR "The projectM Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/package-description.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A standalone, Milkdrop-like audio visualization application")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://projectm-visualizer.org/")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/gpl-3.0.txt")
set(CPACK_STRIP_FILES TRUE)

# Package generator defaults. Override using "cpack -G [generator]"
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)

include(CPack)
