#include "HelpWindow.h"

#include "imgui.h"

#include <Poco/Util/Application.h>

void HelpWindow::Show()
{
    if (_commandLineArguments.empty())
    {
        FillCommandLineArgumentTable();
    }

    if (_shortcuts.empty())
    {
        FillKeyboardShortcutsTable();
    }

    _visible = true;
    ImGui::SetWindowFocus("Help");
}

void HelpWindow::Draw()
{
    if (!_visible)
    {
        return;
    }

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    constexpr ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_None;
    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Quick Help###Help", &_visible, windowFlags))
    {
        if (ImGui::BeginTabBar("Help Topics", tabBarFlags))
        {
            if (ImGui::BeginTabItem("Keyboard Shortcuts"))
            {
                if (ImGui::BeginTable("Keyboard Shortcuts", 2, tableFlags))
                {
                    ImGui::TableSetupColumn("Function", ImGuiTableColumnFlags_WidthFixed, .0f);
                    ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthFixed, .0f);
                    ImGui::TableHeadersRow();

                    for (const auto& shortcut : _shortcuts)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(shortcut.first.c_str());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(shortcut.second.c_str());
                    }

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Command Line Arguments"))
            {
                if (ImGui::BeginTable("Command Line Arguments", 3, tableFlags))
                {
                    ImGui::TableSetupColumn("Short", ImGuiTableColumnFlags_WidthFixed, .0f);
                    ImGui::TableSetupColumn("Argument", ImGuiTableColumnFlags_WidthFixed, .0f);
                    ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch, .0f);
                    ImGui::TableHeadersRow();

                    for (const auto& argument : _commandLineArguments)
                    {
                        int columnIndex = 0;
                        ImGui::TableNextRow();
#ifdef POCO_OS_FAMILY_UNIX
                        ImGui::TableSetColumnIndex(columnIndex++);
                        ImGui::TextUnformatted(std::get<0>(argument).c_str());
#endif
                        ImGui::TableSetColumnIndex(columnIndex++);
                        ImGui::TextUnformatted(std::get<1>(argument).c_str());
                        ImGui::TableSetColumnIndex(columnIndex++);
                        ImGui::TextWrapped("%s", std::get<2>(argument).c_str());
                    }

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void HelpWindow::FillCommandLineArgumentTable()
{
    auto& app = Poco::Util::Application::instance();
    const auto& options = app.options();

    for (const auto& option : options)
    {
        std::string shortArgument = option.shortName();
        std::string longArgument = option.fullName();
        std::string description = option.description();

        if (option.takesArgument())
        {
            if (!shortArgument.empty())
            {
#ifdef POCO_OS_FAMILY_UNIX
                shortArgument = "-" + shortArgument;
#else
                shortArgument = "/" + shortArgument;
#endif
                if (!option.argumentRequired())
                {
                    shortArgument += "[";
                }
                shortArgument += option.argumentName();
                if (!option.argumentRequired())
                {
                    shortArgument += "]";
                }
            }

            if (!longArgument.empty())
            {
#ifdef POCO_OS_FAMILY_UNIX
                longArgument = "--" + longArgument;
#else
                longArgument = "/" + longArgument;
#endif
                if (!option.argumentRequired())
                {
                    longArgument += "[";
                }
                longArgument += "=" + option.argumentName();
                if (!option.argumentRequired())
                {
                    longArgument += "]";
                }
            }
        }

        _commandLineArguments.emplace_back(shortArgument, longArgument, description);
    }
}

void HelpWindow::FillKeyboardShortcutsTable()
{
    _shortcuts = {
        {"Quit projectM", "Ctrl+q"},
        {"Toggle Menu/UI", "Escape"},
        {"Next Preset in Playlist (immediate)", "n"},
        {"Next Preset in Playlist (smooth)", "Shift+n"},
        {"Previous Preset in Playlist (immediate)", "p"},
        {"Previous Preset in Playlist (smooth)", "Shift+p"},
        {"Last Played Preset (Back, immediate)", "Backspace"},
        {"Last Played Preset (Back, smooth)", "Shift+Backspace"},
        {"Random Preset (immediate)", "r"},
        {"Random Preset (smooth)", "Shift+r"},
        {"Lock Current Preset", "Spacebar"},
        {"Toggle Shuffle", "y"},
        {"Toggle Fullscreen", "Ctrl-f, Right Mouse"},
        {"Toggle Aspect Correction", "Ctrl-a"},
        {"Next Audio Input Device", "Ctrl-i"},
        {"Move to Next Monitor", "Ctrl-m"},
        {"Increase Beat Sensitivity by 1%", "Cursor Up"},
        {"Decrease Beat Sensitivity by 1%", "Cursor Down"},
        {"Add Random Waveform at Mouse Pointer", "Shift+Left Mouse"},
        {"Clear Random Waveforms", "Middle Mouse"}};
}
