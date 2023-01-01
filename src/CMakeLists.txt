
set(PROJECTM_CONFIGURATION_FILE "${CMAKE_CURRENT_BINARY_DIR}/projectMSDL.properties" PARENT_SCOPE)
configure_file(resources/projectMSDL.properties.in ${PROJECTM_CONFIGURATION_FILE} @ONLY)

add_executable(projectMSDL WIN32
        AudioCapture.cpp
        AudioCapture.h
        FPSLimiter.cpp
        FPSLimiter.h
        main.cpp
        ProjectMSDLApplication.cpp
        ProjectMSDLApplication.h
        ProjectMWrapper.cpp
        ProjectMWrapper.h
        RenderLoop.cpp
        RenderLoop.h
        SDLRenderingWindow.h
        SDLRenderingWindow.cpp
        )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(projectMSDL
            PRIVATE
            AudioCaptureImpl_WASAPI.h
            AudioCaptureImpl_WASAPI.cpp
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            AUDIO_IMPL_HEADER="AudioCaptureImpl_WASAPI.h"
            )
else()
    target_sources(projectMSDL
            PRIVATE
            AudioCaptureImpl_SDL.h
            AudioCaptureImpl_SDL.cpp
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            AUDIO_IMPL_HEADER="AudioCaptureImpl_SDL.h"
            )
endif()

target_compile_definitions(projectMSDL
        PRIVATE
        PROJECTMSDL_VERSION="${PROJECT_VERSION}"
        LIBPROJECTM_BUILD_VERSION="${libprojectM_VERSION}"
        )

target_link_libraries(projectMSDL
        PRIVATE
        libprojectM::${PROJECTM_LINKAGE}
        libprojectM::playlist
        Poco::Util
        SDL2::SDL2$<$<STREQUAL:${SDL2_LINKAGE},static>:-static>
        SDL2::SDL2main
        )

if(MSVC)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
        set_target_properties(projectMSDL
                PROPERTIES
                VS_DPI_AWARE "PerMonitor"
                )
    else()
        message(AUTHOR_WARNING
                "You're using a CMake version less than 3.16 with Visual Studio.\n"
                "The resulting projectMSDL executable will not be DPI-aware and possibly render at a "
                "lower-than-expected resolution on high-DPI displays.")
    endif()
endif()
