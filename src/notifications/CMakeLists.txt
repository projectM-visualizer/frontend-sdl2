add_library(ProjectMSDL-Notifications STATIC
        DisplayToastNotification.cpp
        DisplayToastNotification.h
        QuitNotification.cpp QuitNotification.h PlaybackControlNotification.cpp PlaybackControlNotification.h UpdateWindowTitleNotification.cpp UpdateWindowTitleNotification.h)

target_include_directories(ProjectMSDL-Notifications
        PRIVATE
        "${CMAKE_SOURCE_DIR}/src/"
        )

target_link_libraries(ProjectMSDL-Notifications
        PUBLIC
        Poco::Foundation
        )
