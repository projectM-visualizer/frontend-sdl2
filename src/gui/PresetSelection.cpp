#include "PresetSelection.h"

#include "ProjectMWrapper.h"
#include "SDLRenderingWindow.h"
#include "AnonymousProFont.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

#include <Poco/Util/Application.h>

#include <cmath>

const char* PresetSelection::name() const
{
    return "Preset Selection GUI";
}

void PresetSelection::initialize(Poco::Util::Application& app)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();

    auto& renderingWindow = Poco::Util::Application::instance().getSubsystem<SDLRenderingWindow>();
    auto& projectMWrapper = Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>();

    _sdlRenderingWindow = &renderingWindow;
    _projectMWrapper = &projectMWrapper;

    _renderingWindow = renderingWindow.GetRenderingWindow();
    _glContext = renderingWindow.GetGlContext();

    ImGui_ImplSDL2_InitForOpenGL(_renderingWindow, _glContext);
    ImGui_ImplOpenGL3_Init("#version 130");

    UpdateFontSize();
}

void PresetSelection::uninitialize()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    _sdlRenderingWindow = nullptr;
    _projectMWrapper = nullptr;
    _renderingWindow = nullptr;
    _glContext = nullptr;
}

void PresetSelection::UpdateFontSize()
{
    ImGuiIO& io = ImGui::GetIO();

    float dpi{96.0f};// Use default value of 96 DPI if SDL_GetDisplayDPI doesn't return a value!
    auto displayIndex = SDL_GetWindowDisplayIndex(_renderingWindow);
    if (displayIndex < 0)
    {
        poco_debug_f1(_logger, "Could not get display index for application window: %s", std::string(SDL_GetError()));
        return;
    }

    auto result = SDL_GetDisplayDPI(displayIndex, &dpi, nullptr, nullptr);
    if (result != 0)
    {
        poco_debug_f2(_logger, "Could not get DPI info for display %?d: %s", displayIndex, std::string(SDL_GetError()));
    }

    // Only interested in changes of > 1 DPI, really.
    if (static_cast<uint32_t>(dpi) == static_cast<uint32_t>(_dpi))
    {
        return;
    }

    poco_debug_f3(_logger, "DPI change for display %?d: %hf -> %hf", displayIndex, _dpi, dpi);

    _dpi = dpi;

    _font = io.Fonts->AddFontFromMemoryCompressedTTF(&AnonymousPro_compressed_data, AnonymousPro_compressed_size, floor(12.0f * (_dpi / 96.0f)));
    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();

    ImGui::GetStyle().ScaleAllSizes(1.0);
}

void PresetSelection::ProcessInput(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
    unsigned int position;
    projectm_get_selected_preset_index(_projectMWrapper->ProjectM(), &position);
    _playlistPosition = static_cast<int>(position);
    _playlistSize = projectm_get_playlist_size(_projectMWrapper->ProjectM());
}

void PresetSelection::Draw()
{
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    if (_settingsVisible)
    {
        DrawSettingsWindow();

    }

    try {

        if (_fileChooser.Draw())
        {
            // Preset selected, load & switch.
            std::string presetName = Poco::Path(_fileChooser.SelectedFile().path()).getFileName();

            int ratingList[2]{};
            projectm_add_preset_url(_projectMWrapper->ProjectM(),
                                    _fileChooser.SelectedFile().path().c_str(),
                                    presetName.c_str(),
                                    &ratingList[0], 2);
            projectm_select_preset(_projectMWrapper->ProjectM(), projectm_get_playlist_size(_projectMWrapper->ProjectM()) - 1, true);
        }

    }
    catch(Poco::Exception& ex)
    {
        poco_error_f1(_logger, "Exception in file chooser: %s", ex.message());
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool PresetSelection::WantsKeyboardInput()
{
    auto& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

bool PresetSelection::WantsMouseInput()
{
    auto& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

void PresetSelection::DrawSettingsWindow()
{
    if (ImGui::Begin("projectM Settings", &_settingsVisible))
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Hint:");
        ImGui::SameLine();
        ImGui::Text("Press <TAB> after clicking on a slider to enter a custom value.");

        if (ImGui::SliderFloat("Preset Display Duration", &_displayDuration, 1.0f, 60.0f))
        {
            projectm_set_preset_duration(_projectMWrapper->ProjectM(), _displayDuration);
        }

        if (ImGui::SliderInt("Playlist Position", &_playlistPosition, 0, _playlistSize - 1))
        {
            projectm_select_preset(_projectMWrapper->ProjectM(), _playlistPosition, true);
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::Button("Toggle Fullscreen"))
        {
            _sdlRenderingWindow->ToggleFullscreen();
        }

        ImGui::SameLine();

        if (ImGui::Button("Random Preset"))
        {
            projectm_select_random_preset(_projectMWrapper->ProjectM(), true);
        }

        ImGui::SameLine();

        if (ImGui::Button("Load Preset..."))
        {
            _fileChooser.Show();
        }

    }
    ImGui::End();
}
