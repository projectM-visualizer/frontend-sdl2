#include "AboutWindow.h"

#include "ProjectMGUI.h"
#include "SystemBrowser.h"

#include "ProjectMSDLApplication.h"
#include "ProjectMWrapper.h"

#include <imgui.h>

#include <Poco/Util/Application.h>

AboutWindow::AboutWindow(ProjectMGUI& gui)
    : _gui(gui)
    , _application(ProjectMSDLApplication::instance())
    , _projectMWrapper(Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>())
{
}

void AboutWindow::Show()
{
    _visible = true;
}

void AboutWindow::Draw()
{
    if (!_visible)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(750, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("About the projectM SDL Frontend", &_visible, ImGuiWindowFlags_NoCollapse))
    {
        _gui.PushToastFont();
        ImGui::TextUnformatted("projectM SDL Frontend");
        _gui.PopFont();
        ImGui::Dummy({.0f, 10.0f});
        ImGui::Text("Version: %s", PROJECTMSDL_VERSION);
        ImGui::Text("libprojectM: %s (built with %s)", _projectMWrapper.ProjectMRuntimeVersion().c_str(), _projectMWrapper.ProjectMBuildVersion().c_str());
        ImGui::Dummy({.0f, 20.0f});
        ImGui::TextUnformatted("Brought to you by the projectM Team and contributors!");
        ImGui::Dummy({.0f, 10.0f});
        ImGui::Separator();
        ImGui::Dummy({.0f, 10.0f});
        ImGui::TextWrapped("The projectM SDL frontend is open-source software licensed under the GNU General Public License, version 3.");
        ImGui::Dummy({.0f, 10.0f});
        ImGui::TextWrapped("Get the source code on GitHub or report an issue with the SDL frontend:");
        if (ImGui::SmallButton("https://github.com/projectM-visualizer/frontend-sdl2"))
        {
            SystemBrowser::OpenURL("https://github.com/projectM-visualizer/frontend-sdl2");
        }
        ImGui::Dummy({.0f, 10.0f});
        if (ImGui::CollapsingHeader("Open-Source Software Used in this Application"))
        {
            ImGui::TextUnformatted("Used in projectM SDL:");
            ImGui::BulletText("libprojectM by The projectM Team (GNU LGPL v2.1)");
            ImGui::BulletText("Simple DirectMedia Layer 2 (SDL) (zlib License)");
            ImGui::BulletText("Dear ImGui by Omar Cornut and contributors (MIT)");
            ImGui::BulletText("The POCO C++ Framework by Applied Informatics GmbH (MIT)");
            ImGui::BulletText("FreeType 2 (FreeType License / GNU GPL v2)");

            ImGui::Dummy({.0f, 10.0f});
            ImGui::TextUnformatted("Via libprojectM:");
            ImGui::BulletText("projectm-eval by The projectM Team (MIT)");
            ImGui::BulletText("SOIL2 by Martín Lucas Golini (MIT-0)");
            ImGui::BulletText("hlslparser by Unknown Worlds Entertainment, Inc. (MIT)");
            ImGui::BulletText("OpenGL Mathematics (GLM) by G-Truc Creation (The Happy Bunny License)");
        }
    }
    ImGui::End();
}
