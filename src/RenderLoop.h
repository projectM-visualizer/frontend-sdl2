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
    void KeyEvent(const SDL_Event& event);

    AudioCapture& _audioCapture;
    ProjectMWrapper& _projectMWrapper;
    SDLRenderingWindow& _sdlRenderingWindow;

    projectm* _projectMHandle{ nullptr };

    bool _wantsToQuit{ false };

    Poco::Logger& _logger{ Poco::Logger::get("RenderLoop") }; //!< The class logger.
};


