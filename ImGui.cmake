
add_library(ImGui STATIC
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui.h
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_tables.cpp
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/backends/imgui_impl_sdl.cpp
        vendor/imgui/backends/imgui_impl_sdl.h
        vendor/imgui/backends/imgui_impl_opengl3.cpp
        vendor/imgui/backends/imgui_impl_opengl3.h
        vendor/imgui/backends/imgui_impl_opengl3_loader.h
        )

target_include_directories(ImGui
        PUBLIC
        ${CMAKE_SOURCE_DIR}/vendor/imgui
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
        ${SDL2_INCLUDE_DIRS}
        )

# Add SDL2/OpenGL 3 Dear ImGui example application target for testing
add_executable(ImGuiDemo EXCLUDE_FROM_ALL
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/examples/example_sdl_opengl3/main.cpp
        )

target_link_libraries(ImGuiDemo
        PRIVATE
        ImGui
        SDL2::SDL2
        SDL2::SDL2main
        OpenGL::GL
        )
