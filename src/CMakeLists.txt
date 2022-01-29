
configure_file(resources/projectMSDL.properties.in ${CMAKE_CURRENT_BINARY_DIR}/projectMSDL.properties @ONLY)

add_executable(projectMSDL
        AudioCapture.cpp
        AudioCapture.h
        AudioCaptureImpl_SDL.cpp
        AudioCaptureImpl_SDL.h
        AudioCaptureImpl_WASAPI.cpp
        AudioCaptureImpl_WASAPI.h
        FPSLimiter.cpp
        FPSLimiter.h
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
            AUDIO_IMPL_INCLUDE="AudioCaptureImpl_WASAPI.h"
            )
else()
    target_sources(projectMSDL
            PRIVATE
            AudioCaptureImpl_SDL.h
            AudioCaptureImpl_SDL.cpp
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            AUDIO_IMPL_INCLUDE="AudioCaptureImpl_SDL.h"
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
        Poco::Util
        SDL2::SDL2$<$<STREQUAL:"${SDL2_LINKAGE}","static">:"-static">
        )
