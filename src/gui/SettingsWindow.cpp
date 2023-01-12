#include "SettingsWindow.h"

#include "ProjectMGUI.h"

#include "imgui.h"

#include <Poco/Util/Application.h>

SettingsWindow::SettingsWindow(ProjectMGUI& gui)
    : _gui(gui)
{
}

void SettingsWindow::Show()
{
    _visible = true;
}

void SettingsWindow::Draw()
{
    if (!_visible)
    {
        return;
    }

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
    constexpr ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_None;
    constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_None;

    if (ImGui::Begin("Settings", &_visible, windowFlags))
    {
        if (ImGui::BeginTabBar("projectM Settings", tabBarFlags))
        {

            if (ImGui::BeginTabItem("projectM"))
            {
                if (ImGui::BeginTable("projectM", 3, tableFlags))
                {
                    ImGui::TableSetupColumn("##desc", ImGuiTableColumnFlags_WidthFixed, .0f);
                    ImGui::TableSetupColumn("##setting", ImGuiTableColumnFlags_WidthStretch, .0f);
                    ImGui::TableSetupColumn("##reset", ImGuiTableColumnFlags_WidthFixed, 100.0f);

                    ImGui::TableNextRow();
                    LabelWithTooltip("Preset Path", "Path to search for preset files if no playlist is loaded.");
                    PathSetting("projectM.presetPath");

                    ImGui::TableNextRow();
                    LabelWithTooltip("Texture Path", "Path to search for texture/image files requested by presets.");
                    PathSetting("projectM.texturePath");

                    ImGui::TableNextRow();
                    LabelWithTooltip("Display projectM Logo Preset", "If enabled, the projectM logo preset is shown on startup.");
                    BooleanSetting("projectM.enableSplash", false);

                    ImGui::TableNextRow();
                    LabelWithTooltip("Lock Preset", "If enabled, the current preset will be shown until switched manually.");
                    BooleanSetting("projectM.presetLocked", false);

                    ImGui::TableNextRow();
                    LabelWithTooltip("Shuffle Presets", "Selects presets randomly from the current playlist.");
                    BooleanSetting("projectM.shuffleEnabled", true);

                    ImGui::TableNextRow();
                    LabelWithTooltip("Preset Display Duration", "Time in seconds a preset will be displayed before it's switched.");
                    DoubleSetting("projectM.displayDuration", 30.0, 1.0, 240.0);

                    ImGui::TableNextRow();
                    LabelWithTooltip("Enable Hard Cuts", "Enables beat-driven, fast preset changes.\nSensitivity and earliest switch time can be configured separately.");
                    BooleanSetting("projectM.hardCutsEnabled", false);

                    ImGui::TableNextRow();
                    LabelWithTooltip("  Hard Cut Duration", "Time in seconds before a preset will be switched at\nthe earliest on hard cuts. If larger than display duration,\nhard cuts won't happen at all.");
                    DoubleSetting("projectM.hardCutDuration", 20.0, 1.0, 240.0);

                    ImGui::TableNextRow();
                    LabelWithTooltip("  Hard Cut Threshold", "Volume difference between measurements required to trigger a hard cut.\nHigher values mean fewer hard cuts.");
                    DoubleSetting("projectM.hardCutSensitivity", 1.0, 0.0, 10.0);

                    ImGui::TableNextRow();
                    LabelWithTooltip("Aspect Correction", "Enables aspect ration correction in presets.\nOnly affects presets using the aspect ratio variables.");
                    BooleanSetting("projectM.aspectCorrectionEnabled", true);

                    ImGui::TableNextRow();
                    LabelWithTooltip("Per-Point Mesh Size X/Y", "Size of the per-point transformation grid.\nHigher values produce better quality, but require more CPU time to calculate.");
                    IntegerSettingVec("projectM.meshX", "projectM.meshY", 200, 125, 8, 250);

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Window"))
            {
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Audio"))
            {
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Help"))
            {
                Poco::Path userConfigurationDir = Poco::Path::configHome();
                userConfigurationDir.makeDirectory().append("projectM/");

                ImGui::TextUnformatted("General:");
                ImGui::Bullet();
                ImGui::TextWrapped("%s", "Hover over the setting name to see a short description.");
                ImGui::Bullet();
                ImGui::TextWrapped("%s", "Settings overridden by command-line parameters cannot be changed.");
                ImGui::Bullet();
                ImGui::TextWrapped("All values changed/set in this window are stored in a separate configuration file, projectMSDL_UI.properties, in the user configuration directory \"%s\"",
                                   userConfigurationDir.toString().c_str());

                ImGui::Separator();

                ImGui::TextUnformatted("Changing values:");
                ImGui::Bullet();
                ImGui::TextWrapped("%s", "Hold down control/command key when clicking on sliders to enter a custom value. The manually entered value can be outside the slider's value range.");
                ImGui::Bullet();
                ImGui::TextWrapped("%s", "Click on \"Reset\" to unset the value and use the value from the user or application default configuration file (projectMSDL.properties).");

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::Separator();

    }
    ImGui::End();
}

void SettingsWindow::LabelWithTooltip(const std::string& label, const std::string& tooltipText)
{
    ImGui::TableSetColumnIndex(0);

    ImGui::TextUnformatted(label.c_str());
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltipText.c_str());
        ImGui::EndTooltip();
    }
}

void SettingsWindow::PathSetting(const std::string& property)
{
    ImGui::TableSetColumnIndex(1);

    auto path = Poco::Util::Application::instance().config().getString(property, "");
    char pathBuffer[2048]{};
    strncpy(pathBuffer, path.c_str(), std::min<size_t>(2047, path.size()));

    bool isOverridden{false};
    if (_gui.CommandLineConfiguration()->has(property))
    {
        isOverridden = true;
        ImGui::BeginDisabled();
    }

    if (ImGui::InputText(std::string("##path_" + property).c_str(), &pathBuffer[0], IM_ARRAYSIZE(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        _gui.UIConfiguration()->setString(property, std::string(pathBuffer));
    }

    ImGui::SameLine();

    ImGui::PushID(std::string(property + "_ChoosePathButton").c_str());
    if (ImGui::Button("..."))
    {
    }
    ImGui::PopID();

    if (isOverridden)
    {
        ImGui::EndDisabled();
        OverriddenSettingMarker();
    }
    else
    {
        ResetButton(property);
    }
}


void SettingsWindow::BooleanSetting(const std::string& property, bool defaultValue)
{
    ImGui::TableSetColumnIndex(1);

    auto value = Poco::Util::Application::instance().config().getBool(property, defaultValue);

    bool isOverridden{false};
    if (_gui.CommandLineConfiguration()->has(property))
    {
        isOverridden = true;
        ImGui::BeginDisabled();
    }

    if (ImGui::Checkbox(std::string("##boolean_" + property).c_str(), &value))
    {
        _gui.UIConfiguration()->setBool(property, value);
    }

    if (isOverridden)
    {
        ImGui::EndDisabled();
        OverriddenSettingMarker();
    }
    else
    {
        ResetButton(property);
    }
}

void SettingsWindow::IntegerSetting(const std::string& property, int defaultValue, int min, int max)
{
    ImGui::TableSetColumnIndex(1);

    auto value = Poco::Util::Application::instance().config().getInt(property, defaultValue);

    bool isOverridden{false};
    if (_gui.CommandLineConfiguration()->has(property))
    {
        isOverridden = true;
        ImGui::BeginDisabled();
    }

    if (ImGui::SliderInt(std::string("##integer_" + property).c_str(), &value, min, max))
    {
        _gui.UIConfiguration()->setInt(property, value);
    }

    if (isOverridden)
    {
        ImGui::EndDisabled();
        OverriddenSettingMarker();
    }
    else
    {
        ResetButton(property);
    }
}

void SettingsWindow::IntegerSettingVec(const std::string& property1, const std::string& property2, int defaultValue1, int defaultValue2, int min, int max)
{
    ImGui::TableSetColumnIndex(1);

    int values[2] = {
        Poco::Util::Application::instance().config().getInt(property1, defaultValue1),
        Poco::Util::Application::instance().config().getInt(property2, defaultValue2)};

    bool isOverridden{false};
    if (_gui.CommandLineConfiguration()->has(property1) || _gui.CommandLineConfiguration()->has(property2))
    {
        isOverridden = true;
        ImGui::BeginDisabled();
    }

    if (ImGui::SliderInt2(std::string("##integer_" + property1 + property2).c_str(), values, min, max))
    {
        _gui.UIConfiguration()->setInt(property1, values[0]);
        _gui.UIConfiguration()->setInt(property2, values[1]);
    }

    if (isOverridden)
    {
        ImGui::EndDisabled();
        OverriddenSettingMarker();
    }
    else
    {
        ResetButton(property1, property2);
    }
}

void SettingsWindow::DoubleSetting(const std::string& property, double defaultValue, double min, double max)
{
    ImGui::TableSetColumnIndex(1);

    auto value = static_cast<float>(Poco::Util::Application::instance().config().getDouble(property, defaultValue));

    bool isOverridden{false};
    if (_gui.CommandLineConfiguration()->has(property))
    {
        isOverridden = true;
        ImGui::BeginDisabled();
    }

    if (ImGui::SliderFloat(std::string("##double_" + property).c_str(), &value, static_cast<float>(min), static_cast<float>(max)))
    {
        _gui.UIConfiguration()->setDouble(property, value);
    }

    if (isOverridden)
    {
        ImGui::EndDisabled();
        OverriddenSettingMarker();
    }
    else
    {
        ResetButton(property);
    }
}

void SettingsWindow::ResetButton(const std::string& property1, const std::string& property2)
{
    if (!_gui.UIConfiguration()->has(property1) && (property2.empty() || !_gui.UIConfiguration()->has(property2)))
    {
        return;
    }

    ImGui::TableSetColumnIndex(2);

    ImGui::PushID(std::string(property1 + property2 + "_ResetButton").c_str());
    if (ImGui::Button("Reset"))
    {
        _gui.UIConfiguration()->remove(property1);
        if (!property2.empty())
        {
            _gui.UIConfiguration()->remove(property2);
        }
    }
    ImGui::PopID();
}

void SettingsWindow::OverriddenSettingMarker()
{
    ImGui::TableSetColumnIndex(2);

    ImGui::TextColored(ImVec4(1, 0, 0, 1), "[Locked]");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Value set via command line argument.");
        ImGui::TextUnformatted("Can only be changed if not overridden.");
        ImGui::EndTooltip();
    }
}
