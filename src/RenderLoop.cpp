#include "RenderLoop.h"

#include "FPSLimiter.h"

#include <Poco/Util/Application.h>

#include <SDL2/SDL.h>

RenderLoop::RenderLoop()
    : _audioCapture(Poco::Util::Application::instance().getSubsystem<AudioCapture>())
    , _projectMWrapper(Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>())
    , _sdlRenderingWindow(Poco::Util::Application::instance().getSubsystem<SDLRenderingWindow>())
    , _projectMHandle(_projectMWrapper.ProjectM())
{
}

void RenderLoop::Run()
{
    FPSLimiter limiter;
    limiter.TargetFPS(_projectMWrapper.TargetFPS());

    uint32_t lastFPSDisplayTick = SDL_GetTicks();

    while (!_wantsToQuit)
    {
        limiter.StartFrame();
        PollEvents();
        _projectMWrapper.RenderFrame();
        _sdlRenderingWindow.Swap();
        limiter.EndFrame();

        if (SDL_GetTicks() - lastFPSDisplayTick > 1000)
        {
            poco_debug_f1(_logger, "FPS: %hf", limiter.FPS());
            lastFPSDisplayTick = SDL_GetTicks();
        }
    }
}

void RenderLoop::PollEvents()
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        int renderWidth;
                        int renderHeight;
                        _sdlRenderingWindow.GetDrawableSize(renderWidth, renderHeight);

                        projectm_set_window_size(_projectMHandle, renderWidth, renderHeight);
                    }
                        break;

                    default:
                        break;
                }
                break;

            case SDL_MOUSEWHEEL:
                break;
            case SDL_KEYDOWN:
                KeyEvent(event);
                break;
            case SDL_MOUSEBUTTONDOWN:
                break;
            case SDL_MOUSEBUTTONUP:
                break;
            case SDL_TEXTINPUT:
                break;
            case SDL_QUIT:
                _wantsToQuit = true;
                break;
        }
    }
}

void RenderLoop::KeyEvent(const SDL_Event& event)
{
    auto keyModifier{ static_cast<SDL_Keymod>(event.key.keysym.mod) };
    auto keyCode{ event.key.keysym.sym };
    bool modifierPressed{ false };

    if (keyModifier & KMOD_LGUI || keyModifier & KMOD_RGUI || keyModifier & KMOD_LCTRL)
    {
        modifierPressed = true;
    }

    switch (keyCode)
    {
        case SDLK_q:
            if (modifierPressed)
            {
                _wantsToQuit = true;
            }
            break;

        case SDLK_f:
            _sdlRenderingWindow.ToggleFullscreen();
            break;
    }
}
