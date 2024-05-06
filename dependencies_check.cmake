if(projectM4_VERSION VERSION_LESS 4.0.0)
    message(FATAL_ERROR "libprojectM version 4.0.0 or higher is required. Version found: ${projectM4_VERSION}.")
endif()

if(SDL2_VERSION VERSION_LESS 2.0.5)
    message(FATAL_ERROR "libSDL version 2.0.5 or higher is required. Version found: ${SDL2_VERSION}.")
endif()

if(Poco_VERSION VERSION_LESS 1.11.2 AND Poco_VERSION VERSION_GREATER_EQUAL 1.10.0)
    message(FATAL_ERROR "Your Poco library contains a serious bug which will cause crashes. Your version is ${Poco_VERSION}.
Affected versions are 1.10.0 to 1.11.1, including. Please upgrade Poco to at least 1.11.2 or downgrade to 1.9.x.
projectMSDL will NOT work with the affected versions.
See https://github.com/pocoproject/poco/issues/3507 for details on this particular issue.
")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND SDL2_VERSION VERSION_LESS 2.0.16)
    message(AUTHOR_WARNING
            "NOTE: libSDL 2.0.15 and lower do not support capture from PulseAudio \"monitor\" devices.\n"
            "It is highly recommended to use at least version 2.0.16!"
            )
endif()

if(Poco_VERSION VERSION_GREATER_EQUAL 1.10.0 AND Poco_VERSION VERSION_LESS_EQUAL 1.10.1)
    message(AUTHOR_WARNING "Poco versions 1.10.0 and 1.10.1 have a known issue with subsystem uninitialization order.\n"
            "It is HIGHLY recommended to use at least version 1.11.0, otherwise it can lead to crashes on application shutdown.")
endif()
