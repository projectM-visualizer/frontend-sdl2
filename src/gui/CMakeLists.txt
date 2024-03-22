# Font embedding
if (CMAKE_CROSSCOMPILING)
    find_program(BINARY_TO_COMPRESSED_EXECUTABLE binary_to_compressed_c)
    if (NOT BINARY_TO_COMPRESSED_EXECUTABLE)
        message(FATAL_ERROR "Could not find host-executable \"binary_to_compressed_c\" tool. Add its location to CMAKE_PREFIX_PATH.")
    endif ()
else ()
    set(BINARY_TO_COMPRESSED_EXECUTABLE "$<TARGET_FILE:ImGuiBinaryToCompressedC>")
endif ()

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        COMMAND ${BINARY_TO_COMPRESSED_EXECUTABLE} "${CMAKE_SOURCE_DIR}/src/resources/AnonymousPro-Regular.ttf" AnonymousPro > "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        MAIN_DEPENDENCY "${CMAKE_SOURCE_DIR}/src/resources/AnonymousPro-Regular.ttf"
        )

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/LiberationSansFont.h"
        COMMAND ${BINARY_TO_COMPRESSED_EXECUTABLE} "${CMAKE_SOURCE_DIR}/src/resources/LiberationSans-Regular.ttf" LiberationSans > "${CMAKE_CURRENT_BINARY_DIR}/LiberationSansFont.h"
        MAIN_DEPENDENCY "${CMAKE_SOURCE_DIR}/src/resources/LiberationSans-Regular.ttf"
        )

add_library(ProjectMSDL-GUI STATIC
        "${CMAKE_CURRENT_BINARY_DIR}/AnonymousProFont.h"
        "${CMAKE_CURRENT_BINARY_DIR}/LiberationSansFont.h"
        AboutWindow.cpp
        AboutWindow.h
        FileChooser.cpp
        FileChooser.h
        HelpWindow.cpp
        HelpWindow.h
        MainMenu.cpp
        MainMenu.h
        PresetSelection.cpp
        PresetSelection.h
        ProjectMGUI.cpp
        ProjectMGUI.h
        SettingsWindow.cpp
        SettingsWindow.h
        SystemBrowser.cpp
        SystemBrowser.h
        ToastMessage.cpp
        ToastMessage.h
        )

target_compile_definitions(ProjectMSDL-GUI
        PRIVATE
        PROJECTMSDL_VERSION="${PROJECT_VERSION}"
        )

target_include_directories(ProjectMSDL-GUI
        PUBLIC
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_BINARY_DIR}"
        )

target_link_libraries(ProjectMSDL-GUI
        PUBLIC
        Poco::Util
        ImGui
        libprojectM::projectM
        )

