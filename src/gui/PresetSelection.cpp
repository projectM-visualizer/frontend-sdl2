#include "PresetSelection.h"

#include "ProjectMWrapper.h"
#include "SDLRenderingWindow.h"

#include "imgui.h"

#include <Poco/Util/Application.h>

void PresetSelection::Show()
{
    _visible = true;
}

void PresetSelection::Draw()
{
    if (ImGui::Begin("projectM Settings", &_visible))
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Hint:");
        ImGui::SameLine();
        ImGui::Text("Press <TAB> after clicking on a slider to enter a custom value.");

        if (ImGui::SliderFloat("Preset Display Duration", &_displayDuration, 1.0f, 60.0f))
        {
            auto& projectMWrapper = Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>();
            projectm_set_preset_duration(projectMWrapper.ProjectM(), _displayDuration);
        }

        if (ImGui::SliderInt("Playlist Position", reinterpret_cast<int*>(&_playlistPosition), 0, _playlistSize - 1))
        {
            auto& projectMWrapper = Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>();
            projectm_playlist_set_position(projectMWrapper.Playlist(), _playlistPosition, true);
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::Button("Toggle Fullscreen"))
        {
            auto& renderingWindow = Poco::Util::Application::instance().getSubsystem<SDLRenderingWindow>();
            renderingWindow.ToggleFullscreen();
        }

        ImGui::SameLine();

        if (ImGui::Button("Random Preset"))
        {
            auto& projectMWrapper = Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>();
            bool shuffleEnabled = projectm_playlist_get_shuffle(projectMWrapper.Playlist());
            projectm_playlist_set_shuffle(projectMWrapper.Playlist(), true);
            projectm_playlist_play_next(projectMWrapper.Playlist(), true);
            projectm_playlist_set_shuffle(projectMWrapper.Playlist(), shuffleEnabled);
        }

        ImGui::SameLine();

        if (ImGui::Button("Lock Preset"))
        {
            auto& projectMWrapper = Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>();
            projectm_set_preset_locked(projectMWrapper.ProjectM(), !projectm_get_preset_locked(projectMWrapper.ProjectM()));
        }

        ImGui::SameLine();

        if (ImGui::Button("Load Preset..."))
        {
            _fileChooser.Show();
        }
    }
    ImGui::End();
}