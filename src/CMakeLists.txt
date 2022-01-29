add_executable(projectMSDL
        AudioCapture.cpp
        AudioCapture.h
        AudioCaptureImpl_SDL.cpp
        AudioCaptureImpl_SDL.h
        AudioCaptureImpl_WASAPI.cpp
        AudioCaptureImpl_WASAPI.h
        ProjectMSDLApplication.cpp
        ProjectMSDLApplication.h
        ProjectMWrapper.cpp
        ProjectMWrapper.h
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


target_link_libraries(projectMSDL
        PRIVATE
        libprojectM::${PROJECTM_LINKAGE}
        Poco::Util
        SDL2::SDL2$<$<STREQUAL:"${SDL2_LINKAGE}","static">:"-static">
        )
