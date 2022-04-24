#pragma once

#include <SDL2/SDL.h>

#include <Poco/Logger.h>

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/AbstractConfiguration.h>

struct ImFont;
class ProjectMWrapper;
class SDLRenderingWindow;

class PresetSelectionGui : public Poco::Util::Subsystem
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

    SDL_Window* _renderingWindow{ nullptr }; //!< Pointer to the SDL window used for rendering.
    SDL_GLContext _glContext{ nullptr }; //!< Pointer to the OpenGL context associated with the window.
    ImFont* _font{ nullptr }; //!< Currently loaded UI font.

    float _dpi{ 0.0f }; //!< Last DPI value.

    Poco::Logger& _logger{ Poco::Logger::get("PresetSelectionGui") }; //!< The class logger.


};
