#pragma once

#include "FileChooser.h"

#include <SDL2/SDL.h>

#include <Poco/Logger.h>

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/AbstractConfiguration.h>

struct ImFont;
class ProjectMWrapper;
class SDLRenderingWindow;

class PresetSelection : public Poco::Util::Subsystem
{
public:
    const char* name() const override;

    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;

    void UpdateFontSize();

    void ProcessInput(const SDL_Event& event);

    void Draw();

    bool WantsKeyboardInput();

    bool WantsMouseInput();

protected:
    void DrawSettingsWindow();

    ProjectMWrapper* _projectMWrapper{ nullptr };
    SDLRenderingWindow* _sdlRenderingWindow{ nullptr };

    SDL_Window* _renderingWindow{ nullptr }; //!< Pointer to the SDL window used for rendering.
    SDL_GLContext _glContext{ nullptr }; //!< Pointer to the OpenGL context associated with the window.
    ImFont* _font{ nullptr }; //!< Currently loaded UI font.

    float _dpi{ 0.0f }; //!< Last DPI value.

    FileChooser _fileChooser; //!< File chooser dialog.

    bool _settingsVisible{ true }; //!< Flag for settings window visibility.
    float _displayDuration{ 0.0f }; //!< Preset display time
    int _playlistPosition{ 0 }; //!< Playlist position
    int _playlistSize{ 0 }; //!< Playlist size

    Poco::Logger& _logger{ Poco::Logger::get("PresetSelectionGui") }; //!< The class logger.

};
