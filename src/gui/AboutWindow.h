#pragma once

#include <string>

class ProjectMGUI;
class ProjectMSDLApplication;
class ProjectMWrapper;

class AboutWindow
{
public:
    explicit AboutWindow(ProjectMGUI& gui);

    ~AboutWindow() = default;

    /**
     * @brief Displays the about window.
     */
    void Show();

    /**
     * @brief Draws the about window.
     */
    void Draw();

private:
    ProjectMGUI& _gui; //!< Reference to the projectM GUI instance
    ProjectMSDLApplication& _application;
    ProjectMWrapper& _projectMWrapper;

    bool _visible{false};
};
