#pragma once

#include <SDL2/SDL.h>

#include <Poco/Logger.h>

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
     * @brief Sets the window title.
     * @param title The new window title.
     */
    void SetTitle(const std::string& title) const;

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
     * @brief Moves the current window to the next display.
     *
     * Internally, this is done by finding the current display using the X/Y coords, then move it to the next
     * index (or the first if the window was on the last display). If only one display is available, no changes
     * are made.
     */
    void NextDisplay();

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

    Poco::AutoPtr<Poco::Util::AbstractConfiguration> _config; //!< View of the "window" configuration subkey.

    SDL_Window* _renderingWindow{ nullptr }; //!< Pointer to the SDL window used for rendering.
    SDL_GLContext _glContext{ nullptr }; //!< Pointer to the OpenGL context associated with the window.

    Poco::Logger& _logger{ Poco::Logger::get("SDLRenderingWindow") }; //!< The class logger.

    int _lastWindowWidth{ 0 };
    int _lastWindowHeight{ 0 };

    bool _fullscreen{ false };

};


