#include "SettingsWindow.h"

#include "ProjectMSDLApplication.h"

#include "ProjectMGUI.h"

#include "notifications/DisplayToastNotification.h"

#include <imgui.h>

#include <Poco/NotificationCenter.h>

SettingsWindow::SettingsWindow(ProjectMGUI& gui)
    : _gui(gui)
    , _userConfiguration(ProjectMSDLApplication::instance().UserConfiguration())
    , _commandLineConfiguration(ProjectMSDLApplication::instance().CommandLineConfiguration())
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

    std::string windowId = "Settings";
    if (_changed)
    {
        windowId.append(" [CHANGED - NOT SAVED]");
    }
    windowId.append("###Settings");

    if (ImGui::Begin(windowId.c_str(), &_visible, windowFlags))
    {
        if (ImGui::BeginTabBar("projectM Settings", tabBarFlags))
        {

            if (ImGui::BeginTabItem("projectM"))
            {
                if (ImGui::BeginTable("projectM", 5, tableFlags))
                {
                    ImGui::TableSetupColumn("##desc", ImGuiTableColumnFlags_WidthFixed, .0f);
                    ImGui::TableSetupColumn("##setting", ImGuiTableColumnFlags_WidthStretch, .0f);
                    ImGui::TableSetupColumn("##choose", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                    ImGui::TableSetupColumn("##reset", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableSetupColumn("##override", ImGuiTableColumnFlags_WidthFixed, .0f);

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
                    LabelWithTooltip("Lock Preset", "If enabled, presets will not be switched automatically.");
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
                    LabelWithTooltip("Per-Point Mesh Size X/Y", "Size of the per-point transformation grid.\nHigher values produce better quality, but require exponentially more CPU time to calculate.\nMilkdrop's default is 48x32.");
                    IntegerSettingVec("projectM.meshX", "projectM.meshY", 64, 48, 8, 300);

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

        if (ImGui::Button("Save Settings"))
        {
            try
            {
                auto configFile = _commandLineConfiguration->getString("app.UserConfigurationFile", "");
                if (!configFile.empty())
                {
                    _userConfiguration->save(configFile);
                    Poco::NotificationCenter::defaultCenter().postNotification(new DisplayToastNotification("Settings saved!"));
                }
                else
                {
                    Poco::NotificationCenter::defaultCenter().postNotification(new DisplayToastNotification("Error saving settings"));
                }
            }
            catch (...)
            {
                Poco::NotificationCenter::defaultCenter().postNotification(new DisplayToastNotification("Error saving settings"));
            }

            _changed = false;
        }
    }
    ImGui::End();

    if (_pathChooser.Draw())
    {
        auto& selectedDirectory = _pathChooser.SelectedFiles();
        if (!selectedDirectory.empty())
        {
            _userConfiguration->setString(_pathChooser.Context(), Poco::Path(selectedDirectory.at(0).path()).makeDirectory().toString());
            _changed = true;
        }
    }
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

    auto path = _userConfiguration->getString(property, "");
    char pathBuffer[2048]{};
    strncpy(pathBuffer, path.c_str(), std::min<size_t>(2047, path.size()));

    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText(std::string("##path_" + property).c_str(), &pathBuffer[0], IM_ARRAYSIZE(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        _userConfiguration->setString(property, std::string(pathBuffer));
        _changed = true;
    }

    ImGui::TableSetColumnIndex(2);

    ImGui::PushID(std::string(property + "_ChoosePathButton").c_str());
    if (ImGui::Button("..."))
    {
        _pathChooser.CurrentDirectory(path);
        _pathChooser.Title("Select directory");
        _pathChooser.Context(property);
        _pathChooser.Show();
    }
    ImGui::PopID();

    ResetButton(property);

    if (_commandLineConfiguration->has(property))
    {
        OverriddenSettingMarker();
    }
}


void SettingsWindow::BooleanSetting(const std::string& property, bool defaultValue)
{
    ImGui::TableSetColumnIndex(1);

    auto value = _userConfiguration->getBool(property, defaultValue);

    if (ImGui::Checkbox(std::string("##boolean_" + property).c_str(), &value))
    {
        _userConfiguration->setBool(property, value);
        _changed = true;
    }

    ResetButton(property);

    if (_commandLineConfiguration->has(property))
    {
        OverriddenSettingMarker();
    }
}

void SettingsWindow::IntegerSetting(const std::string& property, int defaultValue, int min, int max)
{
    ImGui::TableSetColumnIndex(1);

    auto value = _userConfiguration->getInt(property, defaultValue);

    if (ImGui::SliderInt(std::string("##integer_" + property).c_str(), &value, min, max))
    {
        _userConfiguration->setInt(property, value);
        _changed = true;
    }

    ResetButton(property);

    if (_commandLineConfiguration->has(property))
    {
        OverriddenSettingMarker();
    }
}

void SettingsWindow::IntegerSettingVec(const std::string& property1, const std::string& property2, int defaultValue1, int defaultValue2, int min, int max)
{
    ImGui::TableSetColumnIndex(1);

    int values[2] = {
        _userConfiguration->getInt(property1, defaultValue1),
        _userConfiguration->getInt(property2, defaultValue2)};

    if (ImGui::SliderInt2(std::string("##integer_" + property1 + property2).c_str(), values, min, max))
    {
        _userConfiguration->setInt(property1, values[0]);
        _userConfiguration->setInt(property2, values[1]);
        _changed = true;
    }

    ResetButton(property1, property2);

    if (_commandLineConfiguration->has(property1) || _commandLineConfiguration->has(property2))
    {
        OverriddenSettingMarker();
    }
}

void SettingsWindow::DoubleSetting(const std::string& property, double defaultValue, double min, double max)
{
    ImGui::TableSetColumnIndex(1);

    auto value = static_cast<float>(_userConfiguration->getDouble(property, defaultValue));

    if (ImGui::SliderFloat(std::string("##double_" + property).c_str(), &value, static_cast<float>(min), static_cast<float>(max)))
    {
        _userConfiguration->setDouble(property, value);
        _changed = true;
    }

    ResetButton(property);

    if (_commandLineConfiguration->has(property))
    {
        OverriddenSettingMarker();
    }
}

void SettingsWindow::ResetButton(const std::string& property1, const std::string& property2)
{
    if (!_userConfiguration->has(property1) && (property2.empty() || !_userConfiguration->has(property2)))
    {
        return;
    }

    ImGui::TableSetColumnIndex(3);

    ImGui::PushID(std::string(property1 + property2 + "_ResetButton").c_str());
    if (ImGui::Button("Reset"))
    {
        _userConfiguration->remove(property1);
        if (!property2.empty())
        {
            _userConfiguration->remove(property2);
        }
        _changed = true;
    }
    ImGui::PopID();
}

void SettingsWindow::OverriddenSettingMarker()
{
    ImGui::TableSetColumnIndex(4);

    ImGui::TextColored(ImVec4(1, 0, 0, 1), "[!]");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Value set via command line argument.");
        ImGui::TextUnformatted("It will only be used if not overridden.");
        ImGui::EndTooltip();
    }
}
