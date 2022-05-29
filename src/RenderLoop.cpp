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

    projectm_set_preset_switched_event_callback(_projectMHandle, &RenderLoop::PresetSwitchedEvent,
                                                static_cast<void*>(this));

    // Update title bar with initial preset.
    unsigned int currentIndex;
    if (projectm_get_selected_preset_index(_projectMHandle, &currentIndex))
    {
        PresetSwitchedEvent(true, currentIndex, this);
    }

    while (!_wantsToQuit)
    {
        limiter.StartFrame();
        PollEvents();
        _projectMWrapper.RenderFrame();
        _sdlRenderingWindow.Swap();
        limiter.EndFrame();
    }

    projectm_set_preset_switched_event_callback(_projectMHandle, nullptr, nullptr);
}

void RenderLoop::PollEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
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

                        if (renderWidth != _renderWidth || renderHeight != _renderHeight)
                        {
                            projectm_set_window_size(_projectMHandle, renderWidth, renderHeight);
                            _renderWidth = renderWidth;
                            _renderHeight = renderHeight;

                            poco_debug_f2(_logger, "Resized rendering canvas to %?dx%?d.", renderWidth, renderHeight);
                        }
                    }
                        break;

                    default:
                        break;
                }
                break;

            case SDL_MOUSEWHEEL:
                ScrollEvent(event.wheel);
                break;

            case SDL_KEYDOWN:
                if (projectm_is_text_input_active(_projectMHandle, true))
                {
                    SearchKeyEvent(event.key);
                }
                else
                {
                    KeyEvent(event.key, true);
                }
                break;

            case SDL_KEYUP:
                if (!projectm_is_text_input_active(_projectMHandle, true))
                {
                    KeyEvent(event.key, false);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                MouseDownEvent(event.button);
                break;

            case SDL_MOUSEBUTTONUP:
                MouseUpEvent(event.button);
                break;

            case SDL_TEXTINPUT:
                if (projectm_is_text_input_active(_projectMHandle, true))
                {
                    projectm_set_search_text(_projectMHandle, event.text.text);
                    projectm_populate_preset_menu(_projectMHandle);
                }
                break;

            case SDL_QUIT:
                _wantsToQuit = true;
                break;
        }
    }
}

void RenderLoop::KeyEvent(const SDL_KeyboardEvent& event, bool down)
{
    auto keyModifier{ static_cast<SDL_Keymod>(event.keysym.mod) };
    auto keyCode{ event.keysym.sym };
    bool modifierPressed{ false };

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
        case SDLK_a:
        {
            bool aspectCorrectionEnabled = !projectm_get_aspect_correction(_projectMHandle);
            projectm_set_aspect_correction(_projectMHandle, aspectCorrectionEnabled);
            projectm_set_toast_message(_projectMHandle, aspectCorrectionEnabled ? "Aspect Correction Enabled"
                                                                                : "Aspect Correction Disabled");
        }
            break;

#ifdef _DEBUG
        case SDLK_d:
            // Write next rendered frame to file
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_d, PROJECTM_KMOD_NONE);
            projectm_set_toast_message(_projectMHandle, "Main Texture Captured");
            break;
#endif

        case SDLK_f:
            if (modifierPressed)
            {
                _sdlRenderingWindow.ToggleFullscreen();
            }
            break;

        case SDLK_h:
            // Display help (same as F1)
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_h, PROJECTM_KMOD_NONE);
            break;

        case SDLK_i:
            if (modifierPressed)
            {
                _audioCapture.NextAudioDevice();
                projectm_set_toast_message(_projectMHandle, _audioCapture.AudioDeviceName().c_str());
            }
            break;

        case SDLK_m:
            if (modifierPressed)
            {
                _sdlRenderingWindow.NextDisplay();
                break;
            }

            // Show preset selection menu
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_m, PROJECTM_KMOD_NONE);
            break;

        case SDLK_n:
            projectm_select_next_preset(_projectMHandle, true);
            break;

        case SDLK_p:
            projectm_select_previous_preset(_projectMHandle, true);
            break;

        case SDLK_r:
            projectm_select_random_preset(_projectMHandle, true);
            break;

        case SDLK_q:
            if (modifierPressed)
            {
                _wantsToQuit = true;
            }
            break;

        case SDLK_y:
        {
            bool shuffleEnabled = !projectm_get_shuffle_enabled(_projectMHandle);
            projectm_set_shuffle_enabled(_projectMHandle, shuffleEnabled);
            projectm_set_toast_message(_projectMHandle, shuffleEnabled ? "Shuffle Enabled" : "Shuffle Disabled");
        }
            break;

        case SDLK_SPACE:
            projectm_lock_preset(_projectMHandle, !projectm_is_preset_locked(_projectMHandle));
            break;

        case SDLK_RETURN:
            if (keyModifier & KMOD_LALT || keyModifier & KMOD_RALT)
            {
                _sdlRenderingWindow.ToggleFullscreen();
                break;
            }

            SDL_StartTextInput();
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_RETURN, PROJECTM_KMOD_NONE);
            break;

        case SDLK_HOME:
            // First preset in playlist or top of search results
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_HOME, PROJECTM_KMOD_NONE);
            break;

        case SDLK_END:
            // Last preset in playlist or end of search results
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_END, PROJECTM_KMOD_NONE);
            break;

        case SDLK_F1:
            // Display help
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_F1, PROJECTM_KMOD_NONE);
            break;

        case SDLK_F2:
            // Display song title, currently unsupported.
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_F2, PROJECTM_KMOD_NONE);
            break;

        case SDLK_F3:
            // Show preset name
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_F3, PROJECTM_KMOD_NONE);
            break;

        case SDLK_F4:
            // Show rendering statistics
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_F4, PROJECTM_KMOD_NONE);
            break;

        case SDLK_F5:
            // Show FPS
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_F5, PROJECTM_KMOD_NONE);
            break;

        case SDLK_PAGEUP:
            // Select a preset half a search page up in the playlist
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_PAGEUP, PROJECTM_KMOD_NONE);
            break;

        case SDLK_PAGEDOWN:
            // Select a preset half a search page down in the playlist
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_PAGEDOWN, PROJECTM_KMOD_NONE);
            break;

        case SDLK_UP:
            // Increase beat sensitivity
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_UP, PROJECTM_KMOD_NONE);
            break;

        case SDLK_DOWN:
            // Decrease beat sensitivity
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_DOWN, PROJECTM_KMOD_NONE);
            break;

    }
}

void RenderLoop::SearchKeyEvent(const SDL_KeyboardEvent& event)
{
    auto keyModifier{ static_cast<SDL_Keymod>(event.keysym.mod) };
    auto keyCode{ event.keysym.sym };
    bool modifierPressed{ false };

    if (keyModifier & KMOD_LGUI || keyModifier & KMOD_RGUI || keyModifier & KMOD_LCTRL)
    {
        modifierPressed = true;
    }


    switch (keyCode)
    {
        case SDLK_f:
            if (modifierPressed)
            {
                _sdlRenderingWindow.ToggleFullscreen();
            }
            break;

        case SDLK_q:
            if (modifierPressed)
            {
                _wantsToQuit = true;
            }
            break;

        case SDLK_BACKSPACE:
            projectm_delete_search_text(_projectMHandle);
            break;

        case SDLK_RETURN:
        case SDLK_ESCAPE:
            SDL_StopTextInput();
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_ESCAPE, PROJECTM_KMOD_NONE);
            break;

        case SDLK_HOME:
            // Top of search results
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_HOME, PROJECTM_KMOD_NONE);
            break;

        case SDLK_END:
            // End of search results
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_END, PROJECTM_KMOD_NONE);
            break;

        case SDLK_PAGEUP:
            // Half a search page up
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_PAGEUP, PROJECTM_KMOD_NONE);
            break;

        case SDLK_PAGEDOWN:
            // Half a search page down
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_PAGEDOWN, PROJECTM_KMOD_NONE);
            break;

        case SDLK_UP:
            // Previous preset in search result list
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_UP, PROJECTM_KMOD_NONE);
            break;

        case SDLK_DOWN:
            // Next preset in search result list
            projectm_key_handler(_projectMHandle, PROJECTM_KEYDOWN, PROJECTM_K_DOWN, PROJECTM_KMOD_NONE);
            break;
    }
}

void RenderLoop::ScrollEvent(const SDL_MouseWheelEvent& event)
{
    // Wheel up is positive
    if (event.y > 0)
    {
        projectm_select_previous_preset(_projectMHandle, true);
    }
        // Wheel down is negative
    else if (event.y < 0)
    {
        projectm_select_next_preset(_projectMHandle, true);
    }
}

void RenderLoop::MouseDownEvent(const SDL_MouseButtonEvent& event)
{
    switch (event.button)
    {
        case SDL_BUTTON_LEFT:
            if (!_mouseDown)
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
    auto presetName = projectm_get_preset_name(that->_projectMHandle, index);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Displaying preset: %s\n", presetName);

    std::string newTitle = "projectM ➫ " + std::string(presetName);
    projectm_free_string(presetName);

    that->_sdlRenderingWindow.SetTitle(newTitle);
}
