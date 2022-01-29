#include "SDLRenderingWindow.h"

#include <Poco/Util/Application.h>

#include <GL/gl.h>

const char* SDLRenderingWindow::name() const
{
    return "SDL2 Rendering Window";
}

void SDLRenderingWindow::initialize(Poco::Util::Application& app)
{
    _config = app.config().createView("window");

    if (!_renderingWindow)
    {
        CreateSDLWindow();
    }
}

void SDLRenderingWindow::uninitialize()
{
    if (_renderingWindow)
    {
        DestroySDLWindow();
    }
}

void SDLRenderingWindow::CreateSDLWindow()
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    int width = _config->getInt("width", 800);
    int height = _config->getInt("height", 600);

#if USE_GLES
    // use GLES 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    // projectM Requires at least Core Profile 3.30 for samplers etc.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    _renderingWindow = SDL_CreateWindow("projectM", 0, 0, width, height,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    _glContext = SDL_GL_CreateContext(_renderingWindow);

    if(_logger.debug())
    {
        DumpOpenGLInfo();
    }

}

void SDLRenderingWindow::DestroySDLWindow()
{
    SDL_DestroyWindow(_renderingWindow);
    SDL_GL_DeleteContext(_glContext);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    _renderingWindow = nullptr;
    _glContext = nullptr;
}

void SDLRenderingWindow::DumpOpenGLInfo()
{
    poco_debug_f1(_logger, "- GL_VERSION: %s", std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
    poco_debug_f1(_logger, "- GL_SHADING_LANGUAGE_VERSION: %s", std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
    poco_debug_f1(_logger, "- GL_VENDOR: %s", std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
}
