# ToDo: Make directory structure configurable
install(TARGETS projectMSDL
        RUNTIME DESTINATION .
        )

install(FILES ${PROJECTM_CONFIGURATION_FILE}
        DESTINATION .
        )
