#pragma once

#include "AudioCapture.h"
#include "ProjectMWrapper.h"
#include "SDLRenderingWindow.h"

#include "notifications/QuitNotification.h"

#include <Poco/Logger.h>
#include <Poco/NObserver.h>
#include <Poco/Notification.h>

class ProjectMGUI;

class RenderLoop
{
public:
    RenderLoop();

    void Run();

protected:
    struct ModifierKeyStates {
        bool _shiftPressed{false}; //!< L/R shift keys
        bool _ctrlPressed{false}; //!< L/R control keys
        bool _altPressed{false}; //!< L/R alt keys
        bool _metaPressed{false}; //!< Logo/meta/command key
    };

    /**
     * @brief Polls all SDL events in the queue and takes action if required.
     */
    void PollEvents();

    /**
     * @brief Checks if the GL viewport size has changed and if so, reconfigured projectM accordingly.
     */
    void CheckViewportSize();

    /**
     * @brief Handles SDL key press events.
     * @param event The key event.
     */
    void KeyEvent(const SDL_KeyboardEvent& event, bool down);

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

    /**
     * @brief Handler for quit notifications.
     * @param notification The received notification.
     */
    void QuitNotificationHandler(const Poco::AutoPtr<QuitNotification>& notification);

    AudioCapture& _audioCapture;
    ProjectMWrapper& _projectMWrapper;
    SDLRenderingWindow& _sdlRenderingWindow;

    projectm_handle _projectMHandle{nullptr};
    projectm_playlist_handle _playlistHandle{nullptr};

    ProjectMGUI& _projectMGui;

    Poco::NObserver<RenderLoop, QuitNotification> _quitNotificationObserver{*this, &RenderLoop::QuitNotificationHandler}; //!< The observer for quit notifications.

    bool _wantsToQuit{false};

    bool _mouseDown{false}; //!< Left mouse button is pressed

    int _renderWidth{0};
    int _renderHeight{0};

    ModifierKeyStates _keyStates; //!< Current "pressed" states of modifier keys

    Poco::Logger& _logger{Poco::Logger::get("RenderLoop")}; //!< The class logger.
};
