
add_library(ImGui STATIC
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui.h
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_tables.cpp
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/backends/imgui_impl_sdl2.cpp
        vendor/imgui/backends/imgui_impl_sdl2.h
        vendor/imgui/backends/imgui_impl_opengl3.cpp
        vendor/imgui/backends/imgui_impl_opengl3.h
        vendor/imgui/backends/imgui_impl_opengl3_loader.h
        )

target_link_libraries(ImGui
        PUBLIC
        SDL2::SDL2$<$<STREQUAL:${SDL2_LINKAGE},static>:-static>
        )

if(ENABLE_FREETYPE AND Freetype_FOUND)
    target_sources(ImGui
            PRIVATE
            vendor/imgui/misc/freetype/imgui_freetype.cpp
            vendor/imgui/misc/freetype/imgui_freetype.h
            )

    target_compile_definitions(ImGui
            PUBLIC
            IMGUI_ENABLE_FREETYPE
            )

    target_link_libraries(ImGui
            PUBLIC
            Freetype::Freetype
            )
endif()

target_include_directories(ImGui
        PUBLIC
        ${CMAKE_SOURCE_DIR}/vendor/imgui
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
        ${SDL2_INCLUDE_DIRS}
        )

# Build font embedding tool
add_executable(ImGuiBinaryToCompressedC EXCLUDE_FROM_ALL
        vendor/imgui/misc/fonts/binary_to_compressed_c.cpp
        )

# Add SDL2/OpenGL 3 Dear ImGui example application target for testing
add_executable(ImGuiDemo EXCLUDE_FROM_ALL
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/examples/example_sdl2_opengl3/main.cpp
        )

target_link_libraries(ImGuiDemo
        PRIVATE
        ImGui
        SDL2::SDL2
        SDL2::SDL2main
        OpenGL::GL
        )
