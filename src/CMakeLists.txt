add_subdirectory(notifications)
add_subdirectory(gui)

set(PROJECTM_CONFIGURATION_FILE "${CMAKE_CURRENT_BINARY_DIR}/projectMSDL.properties")
set(PROJECTM_CONFIGURATION_FILE "${PROJECTM_CONFIGURATION_FILE}" PARENT_SCOPE)
configure_file(resources/projectMSDL.properties.in "${PROJECTM_CONFIGURATION_FILE}" @ONLY)

add_executable(projectMSDL WIN32 MACOSX_BUNDLE
        AudioCapture.cpp
        AudioCapture.h
        FPSLimiter.cpp
        FPSLimiter.h
        ProjectMSDLApplication.cpp
        ProjectMSDLApplication.h
        ProjectMWrapper.cpp
        ProjectMWrapper.h
        RenderLoop.cpp
        RenderLoop.h
        SDLRenderingWindow.cpp
        SDLRenderingWindow.h
        main.cpp
        projectMSDL.rc
        )

string(TIMESTAMP BUILD_YEAR "%Y" UTC)
set_target_properties(projectMSDL PROPERTIES
        OUTPUT_NAME "$<IF:$<PLATFORM_ID:Darwin>,projectM,projectMSDL>" # On macOS, we name the app bundle just "projectM"
        MACOSX_BUNDLE_BUNDLE_NAME "projectM"
        MACOSX_BUNDLE_BUNDLE_VERSION "${projectMSDL_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © ${BUILD_YEAR} The projectM team and contributors"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.projectm.frontend.sdl2"
        MACOSX_BUNDLE_ICON_FILE "projectMSDL.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${projectMSDL_VERSION}"
        )

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(projectMSDL
            PRIVATE
            AudioCaptureImpl_WASAPI.h
            AudioCaptureImpl_WASAPI.cpp
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            AUDIO_IMPL_HEADER="AudioCaptureImpl_WASAPI.h"
            )
else ()
    target_sources(projectMSDL
            PRIVATE
            AudioCaptureImpl_SDL.h
            AudioCaptureImpl_SDL.cpp
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            AUDIO_IMPL_HEADER="AudioCaptureImpl_SDL.h"
            )
endif ()

# GLEW needs to be initialized if libprojectM depends on it.
if (TARGET GLEW::glew OR TARGET GLEW::glew_s)
    target_compile_definitions(projectMSDL
            PRIVATE
            USE_GLEW
            )
endif ()

set_source_files_properties(ProjectMSDLApplication.cpp PROPERTIES
        COMPILE_DEFINITIONS PROJECTMSDL_CONFIG_LOCATION=\"${DEFAULT_CONFIG_PATH}\"
        )

target_compile_definitions(projectMSDL
        PRIVATE
        PROJECTMSDL_VERSION="${PROJECT_VERSION}"
        )

target_link_libraries(projectMSDL
        PRIVATE
        ProjectMSDL-GUI
        ProjectMSDL-Notifications
        libprojectM::playlist
        Poco::Util
        SDL2::SDL2$<$<STREQUAL:${SDL2_LINKAGE},static>:-static>
        SDL2::SDL2main
        )

if (MSVC)
    set_target_properties(projectMSDL
            PROPERTIES
            VS_DPI_AWARE "PerMonitor"
            )
endif ()
