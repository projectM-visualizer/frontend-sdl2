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
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist.in"
        )

if (ENABLE_GLES)
    target_compile_definitions(projectMSDL
            PUBLIC
            USE_GLES
            )
endif ()

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
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # macOS selects its audio backend at runtime (see AudioCaptureImpl_macOS): the
    # native Core Audio process tap on macOS 14.4+ (captures system output directly,
    # no virtual loopback driver needed), falling back to the shared SDL microphone
    # capture on older systems.
    target_sources(projectMSDL
            PRIVATE
            AudioCaptureImpl_macOS.h
            AudioCaptureImpl_macOS.mm
            AudioCaptureImpl_CoreAudioTap.h
            AudioCaptureImpl_CoreAudioTap.mm
            AudioCaptureImpl_SDL.h
            AudioCaptureImpl_SDL.cpp
            )
    # Compile the shared SDL backend a second time under a distinct class name so it
    # can coexist with the Core Audio tap in the same binary. Leaves the SDL sources
    # themselves untouched for the Linux build.
    set_source_files_properties(AudioCaptureImpl_SDL.cpp PROPERTIES
            COMPILE_DEFINITIONS "AudioCaptureImpl=SdlAudioCapture"
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            AUDIO_IMPL_HEADER="AudioCaptureImpl_macOS.h"
            )
    target_link_libraries(projectMSDL
            PRIVATE
            "-framework CoreAudio"
            "-framework AudioToolbox"
            "-framework Foundation"
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

# GLEW needs to be initialized if libprojectM depends on it (<v4.2).
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

if (ENABLE_CONTROL_SOCKET)
    target_sources(projectMSDL
            PRIVATE
            ControlSocket.cpp
            ControlSocket.h
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            ENABLE_CONTROL_SOCKET
            )
    target_link_libraries(projectMSDL
            PRIVATE
            Poco::JSON
            )
endif ()

target_link_libraries(projectMSDL
        PRIVATE
        ProjectMSDL-GUI
        ProjectMSDL-Notifications
        libprojectM::playlist
        Poco::Util
        SDL2::SDL2$<$<STREQUAL:${SDL2_LINKAGE},static>:-static>
        SDL2::SDL2main
        OpenGL::$<IF:$<BOOL:${ENABLE_GLES}>,GLES3,GL>
        )

if (MSVC)
    set_target_properties(projectMSDL
            PROPERTIES
            VS_DPI_AWARE "PerMonitor"
            )
endif ()
