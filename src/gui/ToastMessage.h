#pragma once

#include <string>

class ProjectMGUI;

class ToastMessage
{
public:
    ToastMessage() = delete;

    explicit ToastMessage(std::string toastText, float displayTime);

    /**
     * @brief Draws the toast message and returns if it still should be kept.
     * @param lastFrameTime The time in seconds since the last frame.
     * @return True if the toast message should still be displayed, false if not.
     */
    bool Draw(float lastFrameTime);

private:

    ProjectMGUI& _gui; //!< Reference to the projectM GUI instance

    std::string _toastText; //!< The toast message text to be displayed.
    float _displayTimeLeft{3.0f}; //!< Remaining toast message display time in seconds.

};
