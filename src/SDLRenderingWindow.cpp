#include "SDLRenderingWindow.h"

#include <Poco/Util/Application.h>

#include <SDL2/SDL_opengl.h>

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

void SDLRenderingWindow::GetDrawableSize(int& width, int& height) const
{
    SDL_GL_GetDrawableSize(_renderingWindow, &width, &height);
}

void SDLRenderingWindow::SetTitle(const std::string& title) const
{
    SDL_SetWindowTitle(_renderingWindow, title.c_str());
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
        if (_logger.debug())
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
        if (_logger.debug())
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
        SDL_ShowCursor(true);

        poco_debug_f2(_logger, "Entered windowed mode with size %dx%d",
                      _lastWindowWidth, _lastWindowHeight);
    }

    _fullscreen = false;
}

void SDLRenderingWindow::NextDisplay()
{
    auto numDisplays = SDL_GetNumVideoDisplays();

    poco_debug_f1(_logger, "Displays available: %?d.", numDisplays);

    if (numDisplays < 2)
    {
        poco_debug(_logger, "Cannot move the window anywhere.");
        return;
    }

    bool wasFullscreen{ _fullscreen };
    if (_fullscreen)
    {
        poco_debug(_logger, "Leaving fullscreen before moving.");
        Windowed();
    }

    int left;
    int top;

    SDL_GetWindowPosition(_renderingWindow, &left, &top);

    poco_debug_f2(_logger, "Window position is X=%?d Y=%?d", left, top);

    // Find current display
    int currentDisplay{ -1 };
    for (int display = 0; display < numDisplays; display++)
    {
        SDL_Rect bounds;
        SDL_GetDisplayBounds(display, &bounds);

        poco_debug_f1(_logger, "Bounds for display %?d:", display);
        poco_debug_f4(_logger, "    X=%?d Y=%?d W=%?d H=%?d", bounds.x, bounds.y, bounds.w, bounds.h);

        // Need to add 1 to left and top, as some window managers will screw up here.
        if (left + 1 >= bounds.x && left + 1 < bounds.x + bounds.w
            && top + 1 >= bounds.y && top + 1 < bounds.y + bounds.h)
        {
            poco_debug_f1(_logger, "Window is currently on display %?d.", display);
            currentDisplay = display;
            break;
        }
    }

    int newDisplay = (currentDisplay + 1) % numDisplays;

    poco_debug_f1(_logger, "Moving window to display %?d.", newDisplay);

    if (newDisplay != currentDisplay)
    {

        SDL_Rect oldBounds{};
        SDL_Rect newBounds;

        int leftOffset{ 0 };
        int topOffset{ 0 };

        SDL_GetDisplayBounds(newDisplay, &newBounds);
        if (currentDisplay >= 0)
        {
            SDL_GetDisplayBounds(currentDisplay, &oldBounds);
            leftOffset = left - oldBounds.x;
            topOffset = top - oldBounds.y;
        }

        int newLeft = newBounds.x + leftOffset;
        int newTop = newBounds.y + topOffset;

        SDL_SetWindowPosition(_renderingWindow, newLeft, newTop);

        poco_debug_f4(_logger, "Old position: X=%?d Y=%?d, new position: X=%?d Y=%?d.",
                     left, top, newLeft, newTop);
    }

    if (wasFullscreen)
    {
        poco_debug(_logger, "Was in fullscreen before moving.");
        Fullscreen();
    }
}

void SDLRenderingWindow::CreateSDLWindow()
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    int width{ _config->getInt("width", 800) };
    int height{ _config->getInt("height", 600) };
    int left{ _config->getInt("left", SDL_WINDOWPOS_UNDEFINED) };
    int top{ _config->getInt("top", SDL_WINDOWPOS_UNDEFINED) };

    auto display = _config->getInt("monitor", 0);
    if (display > 0)
    {
        poco_debug_f1(_logger, "User requested to place window on monitor %?d.", display);
        auto numDisplays = SDL_GetNumVideoDisplays();
        if (display > numDisplays)
        {
            display = numDisplays;
        }

        SDL_Rect bounds;
        SDL_GetDisplayBounds(display - 1, &bounds);
        left = bounds.x;
        top = bounds.y;

        poco_debug_f3(_logger, "Creating window on monitor %?d at X=%?d Y=%?d.", display, left, top);
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
    poco_debug(_logger, "Closing rendering window and destroying OpenGL context.");

    SDL_GL_DeleteContext(_glContext);
    _glContext = nullptr;

    SDL_DestroyWindow(_renderingWindow);
    _renderingWindow = nullptr;

    SDL_QuitSubSystem(SDL_INIT_VIDEO);

}

void SDLRenderingWindow::DumpOpenGLInfo()
{
    poco_debug_f1(_logger, "- GL_VERSION: %s", std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
    poco_debug_f1(_logger, "- GL_SHADING_LANGUAGE_VERSION: %s",
                  std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
    poco_debug_f1(_logger, "- GL_VENDOR: %s", std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
}
