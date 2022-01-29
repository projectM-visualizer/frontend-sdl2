#pragma once

#include <SDL2/SDL.h>

#include <Poco/Logger.h>

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/AbstractConfiguration.h>

class SDLRenderingWindow : public Poco::Util::Subsystem
{
public:
    const char* name() const override;

protected:
    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;

    void CreateSDLWindow();

    bool _fullscreen{ false };

    Poco::Util::AbstractConfiguration::Ptr _config;

    SDL_Window* _renderingWindow{ nullptr };
    SDL_GLContext _glContext{ nullptr };

    Poco::Logger& _logger{ Poco::Logger::get("SDLRenderingWindow") };

    void DestroySDLWindow();

    void DumpOpenGLInfo();
};


