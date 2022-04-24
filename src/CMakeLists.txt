
set(PROJECTM_CONFIGURATION_FILE "${CMAKE_CURRENT_BINARY_DIR}/projectMSDL.properties")
set(PROJECTM_CONFIGURATION_FILE "${PROJECTM_CONFIGURATION_FILE}" PARENT_SCOPE)
configure_file(resources/projectMSDL.properties.in "${PROJECTM_CONFIGURATION_FILE}" @ONLY)

# Font embedding
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        COMMAND $<TARGET_FILE:ImGuiBinaryToCompressedC> "${CMAKE_CURRENT_SOURCE_DIR}/resources/AnonymousPro-Regular.ttf" AnonymousPro > "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/resources/AnonymousPro-Regular.ttf"
        )

add_executable(projectMSDL WIN32
        "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        AudioCapture.cpp
        AudioCapture.h
        FPSLimiter.cpp
        FPSLimiter.h
        GuiFileChooserWindow.cpp
        GuiFileChooserWindow.h
        PresetSelectionGui.cpp
        PresetSelectionGui.h
        ProjectMSDLApplication.cpp
        ProjectMSDLApplication.h
        ProjectMWrapper.cpp
        ProjectMWrapper.h
        RenderLoop.cpp
        RenderLoop.h
        SDLRenderingWindow.cpp
        SDLRenderingWindow.h
        main.cpp
        )

target_include_directories(projectMSDL
        PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
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
        )

target_link_libraries(projectMSDL
        PRIVATE
        ImGui
        libprojectM::playlist
        Poco::Util
        SDL2::SDL2$<$<STREQUAL:${SDL2_LINKAGE},static>:-static>
        SDL2::SDL2main
        )

if(MSVC)
    set_target_properties(projectMSDL
            PROPERTIES
            VS_DPI_AWARE "PerMonitor"
            )
endif()
