#pragma once

#include "AudioCapture.h"
#include "ProjectMWrapper.h"
#include "SDLRenderingWindow.h"

#include <Poco/Logger.h>

class RenderLoop
{
public:
    RenderLoop();

    void Run();

protected:
    /**
     * @brief Polls all SDL events in the queue and takes action if required.
     */
    void PollEvents();

    /**
     * @brief Handles SDL key press events.
     * @param event The key event.
     */
    void KeyEvent(const SDL_KeyboardEvent& event);

    /**
     * @brief Handles SDL key press events when inside preset search mode.
     * @param event The key event.
     */
    void SearchKeyEvent(const SDL_KeyboardEvent& event);

    /**
     * @brief Handles SDL mouse wheel events.
     * @param event The mouse wheel event
     */
    void ScrollEvent(const SDL_MouseWheelEvent& event);

    /**
     * @brief Handles SDL mouse button down events.
     * @param event The mouse button event
     */
    void MouseDownEvent(const SDL_MouseButtonEvent& event);

    /**
     * @brief Handles SDL mouse button up events.
     * @param event The mouse button event
     */
    void MouseUpEvent(const SDL_MouseButtonEvent& event);

    AudioCapture& _audioCapture;
    ProjectMWrapper& _projectMWrapper;
    SDLRenderingWindow& _sdlRenderingWindow;

    projectm* _projectMHandle{ nullptr };

    bool _wantsToQuit{ false };

    bool _mouseDown{ false }; //!< Left mouse button is pressed

    Poco::Logger& _logger{ Poco::Logger::get("RenderLoop") }; //!< The class logger.
};


