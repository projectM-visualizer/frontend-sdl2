# Font embedding
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        COMMAND $<TARGET_FILE:ImGuiBinaryToCompressedC> "${CMAKE_SOURCE_DIR}/src/resources/AnonymousPro-Regular.ttf" AnonymousPro > "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        MAIN_DEPENDENCY "${CMAKE_SOURCE_DIR}/src/resources/AnonymousPro-Regular.ttf"
        )

add_library(ProjectMSDL-GUI STATIC
        "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        FileChooser.cpp
        FileChooser.h
        PresetSelection.cpp
        PresetSelection.h
        )

target_include_directories(ProjectMSDL-GUI
        PRIVATE
        "${CMAKE_SOURCE_DIR}/src/"
        "${CMAKE_CURRENT_BINARY_DIR}"
        )

target_link_libraries(ProjectMSDL-GUI
        PUBLIC
        ImGui
        libprojectM::${PROJECTM_LINKAGE}
        )

