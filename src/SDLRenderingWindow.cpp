#include "SDLRenderingWindow.h"

#include <libprojectM/projectM.h>

#include <Poco/Util/Application.h>

#include <GL/gl.h>

const char* SDLRenderingWindow::name() const
{
    return "SDL2 Rendering Window";
}

void SDLRenderingWindow::GetDrawableSize(int& width, int& height) const
{
    SDL_GL_GetDrawableSize(_renderingWindow, &width, &height);
}

void SDLRenderingWindow::Swap() const
{
    SDL_GL_SwapWindow(_renderingWindow);
}

void SDLRenderingWindow::ToggleFullscreen()
{
    if (_fullscreen)
    {
        Windowed();
    }
    else
    {
        Fullscreen();
    }
}

void SDLRenderingWindow::Fullscreen()
{
    SDL_GetWindowSize(_renderingWindow, &_lastWindowWidth, &_lastWindowHeight);
    SDL_ShowCursor(false);
    if (_config->getBool("fullscreen.exclusiveMode", false))
    {
        int fullscreenWidth = _config->getInt("fullscreen.width", 0);
        int fullscreenHeight = _config->getInt("fullscreen.height", 0);
        if (fullscreenWidth > 0 && fullscreenHeight > 0)
        {
            SDL_RestoreWindow(_renderingWindow);
            SDL_SetWindowSize(_renderingWindow, fullscreenWidth, fullscreenHeight);
        }
        SDL_SetWindowFullscreen(_renderingWindow, SDL_WINDOW_FULLSCREEN);
        if(_logger.debug())
        {
            int width{ 0 };
            int height{ 0 };
            SDL_GetWindowSize(_renderingWindow, &width, &height);
            poco_debug_f2(_logger, "Entered exclusive fullscreen mode with actual resolution %dx%d",
                          width, height);
        }
    }
    else
    {
        SDL_SetWindowFullscreen(_renderingWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        if(_logger.debug())
        {
            int width{ 0 };
            int height{ 0 };
            SDL_GetWindowSize(_renderingWindow, &width, &height);
            poco_debug_f2(_logger, "Entered fake fullscreen mode with resolution %dx%d",
                          width, height);
        }
    }

    _fullscreen = true;
}

void SDLRenderingWindow::Windowed()
{
    SDL_SetWindowFullscreen(_renderingWindow, 0);
    if (_lastWindowWidth > 0 && _lastWindowHeight > 0)
    {
        SDL_SetWindowSize(_renderingWindow, _lastWindowWidth, _lastWindowHeight);
    }
    SDL_ShowCursor(true);

    _fullscreen = false;
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

    int width{ _config->getInt("width", 800) };
    int height{ _config->getInt("height", 600) };
    int left{ _config->getInt("left", SDL_WINDOWPOS_UNDEFINED) };
    int top{ _config->getInt("top", SDL_WINDOWPOS_UNDEFINED) };

    auto monitor = _config->getInt("monitor", 0);
    if (monitor > 0)
    {
        auto numDisplays = SDL_GetNumVideoDisplays();
        if (monitor > numDisplays)
        {
            monitor = numDisplays;
        }

        SDL_Rect bounds;
        SDL_GetDisplayBounds(monitor - 1, &bounds);
        left = bounds.x;
        top = bounds.y;
    }

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

    _renderingWindow = SDL_CreateWindow("projectM", left, top, width, height,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!_renderingWindow)
    {
        auto errorMessage = "Could not create SDL rendering window. Error: " + std::string(SDL_GetError());
        poco_fatal(_logger, errorMessage);
        throw Poco::Exception(errorMessage);
    }

    _glContext = SDL_GL_CreateContext(_renderingWindow);
    if (!_glContext)
    {
        auto errorMessage = "Could not create OpenGL rendering context. Error: " + std::string(SDL_GetError());
        poco_fatal(_logger, errorMessage);
        throw Poco::Exception(errorMessage);
    }

    SDL_SetWindowTitle(_renderingWindow, "projectM");
    SDL_GL_MakeCurrent(_renderingWindow, _glContext);
    SDL_GL_SetSwapInterval(_config->getBool("waitForVerticalSync", true) ? 1 : 0);

    if (_config->getBool("fullscreen", false))
    {
        Fullscreen();
    }
    else
    {
        Windowed();
    }

    if (_logger.debug())
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
    poco_debug_f1(_logger, "- GL_SHADING_LANGUAGE_VERSION: %s",
                  std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
    poco_debug_f1(_logger, "- GL_VENDOR: %s", std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
}
