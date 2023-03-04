#include "RenderLoop.h"

#include "FPSLimiter.h"

#include <Poco/Util/Application.h>

#include <SDL2/SDL.h>

RenderLoop::RenderLoop()
    : _audioCapture(Poco::Util::Application::instance().getSubsystem<AudioCapture>())
    , _projectMWrapper(Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>())
    , _sdlRenderingWindow(Poco::Util::Application::instance().getSubsystem<SDLRenderingWindow>())
    , _projectMHandle(_projectMWrapper.ProjectM())
    , _playlistHandle(_projectMWrapper.Playlist())
{
}

void RenderLoop::Run()
{
    FPSLimiter limiter;
    limiter.TargetFPS(_projectMWrapper.TargetFPS());

    projectm_playlist_set_preset_switched_event_callback(_playlistHandle, &RenderLoop::PresetSwitchedEvent, static_cast<void*>(this));

    _projectMWrapper.DisplayInitialPreset();

    while (!_wantsToQuit)
    {
        limiter.StartFrame();
        PollEvents();
        CheckViewportSize();
        _audioCapture.FillBuffer();
        _projectMWrapper.RenderFrame();
        _sdlRenderingWindow.Swap();
        limiter.EndFrame();
    }

    projectm_playlist_set_preset_switched_event_callback(_playlistHandle, nullptr, nullptr);
}

void RenderLoop::PollEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_MOUSEWHEEL:
                ScrollEvent(event.wheel);
                break;

            case SDL_KEYDOWN:
                KeyEvent(event.key, true);
                break;

            case SDL_KEYUP:
                KeyEvent(event.key, false);
                break;

            case SDL_MOUSEBUTTONDOWN:
                MouseDownEvent(event.button);
                break;

            case SDL_MOUSEBUTTONUP:
                MouseUpEvent(event.button);
                break;

            case SDL_QUIT:
                _wantsToQuit = true;
                break;
        }
    }
}

void RenderLoop::CheckViewportSize()
{
    int renderWidth;
    int renderHeight;
    _sdlRenderingWindow.GetDrawableSize(renderWidth, renderHeight);

    if (renderWidth != _renderWidth || renderHeight != _renderHeight)
    {
        projectm_set_window_size(_projectMHandle, renderWidth, renderHeight);
        _renderWidth = renderWidth;
        _renderHeight = renderHeight;

        poco_debug_f2(_logger, "Resized rendering canvas to %?dx%?d.", renderWidth, renderHeight);
    }
}

void RenderLoop::KeyEvent(const SDL_KeyboardEvent& event, bool down)
{
    auto keyModifier{static_cast<SDL_Keymod>(event.keysym.mod)};
    auto keyCode{event.keysym.sym};
    bool modifierPressed{false};

    if (keyModifier & KMOD_LGUI || keyModifier & KMOD_RGUI || keyModifier & KMOD_LCTRL)
    {
        modifierPressed = true;
    }

    // Handle modifier keys and save state for use in other methods, e.g. mouse events
    switch (keyCode)
    {
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            _keyStates._ctrlPressed = down;
            break;

        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            _keyStates._shiftPressed = down;
            break;

        case SDLK_LALT:
        case SDLK_RALT:
            _keyStates._altPressed = down;
            break;

        case SDLK_LGUI:
        case SDLK_RGUI:
            _keyStates._metaPressed = down;
            break;

        default:
            break;
    }

    if (!down)
    {
        return;
    }

    // Currently mapping all SDL keycodes manually to projectM, as the key handler API will be gone before the
    // 4.0 release, being replaced by API methods reflecting the action instead of requiring knowledge about
    // projectM's internal hotkey bindings.
    switch (keyCode)
    {
        case SDLK_a: {
            bool aspectCorrectionEnabled = !projectm_get_aspect_correction(_projectMHandle);
            projectm_set_aspect_correction(_projectMHandle, aspectCorrectionEnabled);
        }
        break;

#ifdef _DEBUG
        case SDLK_d:
            // Write next rendered frame to file
            projectm_write_debug_image_on_next_frame(_projectMHandle, nullptr);
            break;
#endif

        case SDLK_f:
            if (modifierPressed)
            {
                _sdlRenderingWindow.ToggleFullscreen();
            }
            break;

        case SDLK_i:
            if (modifierPressed)
            {
                _audioCapture.NextAudioDevice();
            }
            break;

        case SDLK_m:
            if (modifierPressed)
            {
                _sdlRenderingWindow.NextDisplay();
                break;
            }
            break;

        case SDLK_n:
            projectm_playlist_play_next(_playlistHandle, true);
            break;

        case SDLK_p:
            projectm_playlist_play_previous(_playlistHandle, true);
            break;

        case SDLK_r: {
            bool shuffleEnabled = projectm_playlist_get_shuffle(_playlistHandle);
            projectm_playlist_set_shuffle(_playlistHandle, true);
            projectm_playlist_play_next(_playlistHandle, true);
            projectm_playlist_set_shuffle(_playlistHandle, shuffleEnabled);
            break;
        }

        case SDLK_q:
            if (modifierPressed)
            {
                _wantsToQuit = true;
            }
            break;

        case SDLK_y: {
            projectm_playlist_set_shuffle(_playlistHandle, !projectm_playlist_get_shuffle(_playlistHandle));
        }
        break;

        case SDLK_BACKSPACE:
            projectm_playlist_play_last(_playlistHandle, true);
            break;

        case SDLK_SPACE:
            projectm_set_preset_locked(_projectMHandle, !projectm_get_preset_locked(_projectMHandle));
            UpdateWindowTitle();
            break;

        case SDLK_UP:
            // Increase beat sensitivity
            projectm_set_beat_sensitivity(_projectMHandle, projectm_get_beat_sensitivity(_projectMHandle) + 0.01f);
            break;

        case SDLK_DOWN:
            // Decrease beat sensitivity
            projectm_set_beat_sensitivity(_projectMHandle, projectm_get_beat_sensitivity(_projectMHandle) - 0.01f);
            break;
    }
}

void RenderLoop::ScrollEvent(const SDL_MouseWheelEvent& event)
{
    // Wheel up is positive
    if (event.y > 0)
    {
        projectm_playlist_play_next(_playlistHandle, true);
    }
    // Wheel down is negative
    else if (event.y < 0)
    {
        projectm_playlist_play_previous(_playlistHandle, true);
    }
}

void RenderLoop::MouseDownEvent(const SDL_MouseButtonEvent& event)
{
    switch (event.button)
    {
        case SDL_BUTTON_LEFT:
            if (!_mouseDown && _keyStates._shiftPressed)
            {
                // ToDo: Improve this to differentiate between single click (add waveform) and drag (move waveform).
                int x;
                int y;
                int width;
                int height;

                _sdlRenderingWindow.GetDrawableSize(width, height);

                SDL_GetMouseState(&x, &y);

                // Scale those coordinates. libProjectM uses a scale of 0..1 instead of absolute pixel coordinates.
                float scaledX = (static_cast<float>(x) / static_cast<float>(width));
                float scaledY = (static_cast<float>(height - y) / static_cast<float>(height));

                // Add a new waveform.
                projectm_touch(_projectMHandle, scaledX, scaledY, 0, PROJECTM_TOUCH_TYPE_RANDOM);
                poco_debug_f2(_logger, "Added new random waveform at %?d,%?d", x, y);

                _mouseDown = true;
            }
            break;

        case SDL_BUTTON_RIGHT:
            _sdlRenderingWindow.ToggleFullscreen();
            break;

        case SDL_BUTTON_MIDDLE:
            projectm_touch_destroy_all(_projectMHandle);
            poco_debug(_logger, "Cleared all custom waveforms.");
            break;
    }
}

void RenderLoop::MouseUpEvent(const SDL_MouseButtonEvent& event)
{
    if (event.button == SDL_BUTTON_LEFT)
    {
        _mouseDown = false;
    }
}

void RenderLoop::PresetSwitchedEvent(bool isHardCut, unsigned int index, void* context)
{
    auto that = reinterpret_cast<RenderLoop*>(context);
    auto presetName = projectm_playlist_item(that->_playlistHandle, index);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Displaying preset: %s\n", presetName);;
    projectm_playlist_free_string(presetName);

    that->UpdateWindowTitle();
}

void RenderLoop::UpdateWindowTitle()
{
    auto presetName = projectm_playlist_item(_playlistHandle, projectm_playlist_get_position(_playlistHandle));

    Poco::Path presetFile(presetName);
    projectm_playlist_free_string(presetName);

    std::string newTitle = "projectM ➫ " + presetFile.getBaseName();
    if (projectm_get_preset_locked(_projectMHandle))
    {
        newTitle += " [locked]";
    }

    _sdlRenderingWindow.SetTitle(newTitle);
}
