install(TARGETS projectMSDL
        RUNTIME DESTINATION ${PROJECTMSDL_BIN_DIR}
        COMPONENT projectMSDL
        )

install(FILES ${PROJECTM_CONFIGURATION_FILE}
        DESTINATION ${PROJECTMSDL_DATA_DIR}
        COMPONENT projectMSDL
        )

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT ENABLE_FLAT_PACKAGE)
    if(ENABLE_DESKTOP_ICON)
        install(FILES src/resources/projectMSDL.desktop
                DESTINATION ${PROJECTMSDL_DESKTOP_DIR}
                COMPONENT projectMSDL
                )

        macro(INSTALL_ICON size)
            install(FILES src/resources/icons/icon_${size}x${size}.png
                    DESTINATION ${PROJECTMSDL_ICONS_DIR}/${size}x${size}/apps
                    RENAME projectMSDL.png
                    COMPONENT projectMSDL
                    )
        endmacro()

        foreach(size 16 32 48 64 72 96 128 256)
            install_icon(${size})
        endforeach()

    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND NOT ENABLE_FLAT_PACKAGE)
    set(ICNS_FILE ${CMAKE_BINARY_DIR}/projectMSDL.icns)
    execute_process(COMMAND iconutil -c icns -o "${ICNS_FILE}" "${CMAKE_SOURCE_DIR}/src/resources/icons")

    install(FILES ${ICNS_FILE}
            DESTINATION ${PROJECTMSDL_DATA_DIR}
            COMPONENT projectMSDL
            )
endif()

# Install optional presets
foreach(preset_dir ${PRESET_DIRS})
    install(DIRECTORY ${preset_dir}
            DESTINATION "${PROJECTMSDL_PRESETS_DIR}"
            COMPONENT projectMSDL
            PATTERN *.md EXCLUDE
            )
endforeach()

# Install optional textures
foreach(texture_dir ${TEXTURE_DIRS})
    install(DIRECTORY ${texture_dir}
            DESTINATION "${PROJECTMSDL_TEXTURES_DIR}"
            COMPONENT projectMSDL
            PATTERN *.md EXCLUDE
            )
endforeach()
