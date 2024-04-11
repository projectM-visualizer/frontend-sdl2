install(TARGETS projectMSDL
        RUNTIME_DEPENDENCY_SET projectMSDLDepends
        RUNTIME DESTINATION ${PROJECTMSDL_BIN_DIR}
        BUNDLE DESTINATION . # .app bundle will reside at the top-level of the install prefix
        COMPONENT projectMSDL
        )

install(FILES ${PROJECTM_CONFIGURATION_FILE}
        DESTINATION ${PROJECTMSDL_DATA_DIR}
        COMPONENT projectMSDL
        )

if(ENABLE_INSTALL_BDEPS)
    install(RUNTIME_DEPENDENCY_SET projectMSDLDepends
            RUNTIME DESTINATION ${PROJECTMSDL_BIN_DIR}
            FRAMEWORK DESTINATION ${PROJECTMSDL_DATA_DIR}
            )
endif()

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

        install(FILES src/resources/icons/icon_scalable.svg
                DESTINATION ${PROJECTMSDL_ICONS_DIR}/scalable/apps
                RENAME projectMSDL.svg
                COMPONENT projectMSDL
                )

    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND NOT ENABLE_FLAT_PACKAGE)
    install(FILES src/resources/icons/icon.icns
            DESTINATION ${PROJECTMSDL_DATA_DIR}
            RENAME projectMSDL.icns
            COMPONENT projectMSDL
            )
    install(FILES src/resources/gpl-3.0.txt
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
