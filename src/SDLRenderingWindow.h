#pragma once

#include "notifications/UpdateWindowTitleNotification.h"

#include <SDL2/SDL.h>

#include <Poco/Logger.h>
#include <Poco/NObserver.h>

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/AbstractConfiguration.h>

struct projectm;

class SDLRenderingWindow : public Poco::Util::Subsystem
{
public:
    const char* name() const override;

    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;

    /**
     * @brief Retrieves the current drawable size of the rendering window.
     *
     * This is the actual canvas size for use with OpenGL. Might be less than the actual window size,
     * as the OS might apply DPI scaling and subtract window decorations etc. from the window dimensions.
     *
     * @param width[out] A reference to a variable that will receive the canvas width.
     * @param height[out] A reference to a variable that will receive the canvas height.
     */
    void GetDrawableSize(int& width, int& height) const;

    /**
     * Swaps the OpenGL front- and back buffers.
     */
    void Swap() const;

    /**
     * @brief Toggles the window's fullscreen mode.
     *
     * There is a caveat on macOS: If the user toggles fullscreen mode via the green system button in the title
     * bar, SDL will not notice it and keep the last known state. Thus, the user would have to toggle it twice
     * via the hotkey to see an effect.
     */
    void ToggleFullscreen();

    /**
     * @brief Enters fullscreen mode.
     */
    void Fullscreen();

    /**
     * @brief Leaves fullscreen mode and returns to windows mode.
     */
    void Windowed();

    /**
     * @brief Sets the visibility of the mouse cursor
     * @param visible true if the cursor should be displayed, false otherwise.
     */
    void ShowCursor(bool visible);

    /**
     * @brief Moves the current window to the next display.
     *
     * Internally, this is done by finding the current display using the X/Y coords, then move it to the next
     * index (or the first if the window was on the last display). If only one display is available, no changes
     * are made.
     */
    void NextDisplay();

    /**
     * @brief Returns the ID of the current display the window is shown on.
     * @return
     */
    int GetCurrentDisplay();

    /**
     * @brief Returns the dimensions of the window.
     * @param [out] width The width of the window.
     * @param [out] height The height of the window.
     */
    void GetWindowSize(int& width, int& height);

    /**
     * @brief Returns the position of the window.
     * @param [out] left The left position of the window.
     * @param [out] top Top top position of the window.
     * @param [in] relative If true, returns the position relative to the current display.
     */
    void GetWindowPosition(int& left, int& top, bool relative = false);

    SDL_Window* GetRenderingWindow() const;

    SDL_GLContext GetGlContext() const;

    /**
     * @brief Returns the ID of the first game controller found
     * @return SDL_GameController * Returns a gamecontroller identifier or NULL
     */
    SDL_GameController* FindController();

    /**
     * @brief Handles SDL game controller add events (plugin in a new controller) events.
     * @param id The added controller id
     */
    void ControllerAdd(const int id );

    /**
     * @brief Handles SDL game controller remove events.
     * @param id The removed controller id
     */
    void ControllerRemove(const int id );

    /**
     * @brief Returns true if the given controller is initialized and the one we currently use.
     * @param id The removed controller id
     */
    bool ControllerIsOurs(const int id );

protected:

    /**
     * Creates the SDL rendering window and an OpenGL rendering context.
     */
    void CreateSDLWindow();

    /**
     * Closes and destroys the rendering window and the associated rendering context.
     */
    void DestroySDLWindow();

    /**
     * Prints OpenGL debug information.
     */
    void DumpOpenGLInfo();

    /**
     * @brief Receives notifications requesting an update of the window title.
     * @param notification The update notification.
     */
    void UpdateWindowTitleNotificationHandler(const Poco::AutoPtr<UpdateWindowTitleNotification>& notification);

    /**
     * @brief Updates the window title.
     */
    void UpdateWindowTitle();

    /**
     * @brief Updates the swap interval from the user settings.
     */
    void UpdateSwapInterval();

    /**
     * @brief Event callback if a configuration value has changed.
     * @param property The key and value that has been changed.
     */
    void OnConfigurationPropertyChanged(const Poco::Util::AbstractConfiguration::KeyValue& property);

    /**
     * @brief Event callback if a configuration value has been removed.
     * @param key The key of the removed property.
     */
    void OnConfigurationPropertyRemoved(const std::string& key);

    Poco::AutoPtr<Poco::Util::AbstractConfiguration> _userConfig; //!< View of the "projectM" configuration subkey in the "user" configuration.
    Poco::AutoPtr<Poco::Util::AbstractConfiguration> _config; //!< View of the "window" configuration subkey.

    SDL_Window* _renderingWindow{ nullptr }; //!< Pointer to the SDL window used for rendering.
    SDL_GLContext _glContext{ nullptr }; //!< Pointer to the OpenGL context associated with the window.

    Poco::NObserver<SDLRenderingWindow, UpdateWindowTitleNotification> _updateWindowTitleObserver{*this, &SDLRenderingWindow::UpdateWindowTitleNotificationHandler}; //!< the observer for title update notifications

    Poco::Logger& _logger{ Poco::Logger::get("SDLRenderingWindow") }; //!< The class logger.

    int _lastWindowWidth{ 0 };
    int _lastWindowHeight{ 0 };

    bool _fullscreen{ false };

    SDL_GameController *_controller;
};


