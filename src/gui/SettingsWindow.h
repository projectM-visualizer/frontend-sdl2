#pragma once

#include "FileChooser.h"

#include <Poco/Util/MapConfiguration.h>
#include <Poco/Util/PropertyFileConfiguration.h>

#include <string>

class AudioCapture;
class ProjectMGUI;

class SettingsWindow
{
public:
    SettingsWindow() = delete;

    explicit SettingsWindow(ProjectMGUI& gui);

    /**
     * @brief Displays the settings window.
     */
    void Show();

    /**
     * @brief Draws the settings window.
     */
    void Draw();

private:
    /**
     * @brief Displays a setting name laben and shows a tooltip if hovered.
     * @param label The label text.
     * @param tooltipText The tooltip displayed when hovering the setting name.
     */
    void LabelWithTooltip(const std::string& label, const std::string& tooltipText);

    /**
     * @brief Displays an editable path field, with a directory chooser button.
     * @param property The property name in the config.
     */
    void PathSetting(const std::string& property);

    /**
     * @brief Displays a checkbox.
     * @param property The property name in the config.
     * @param defaultValue Default value for the property if not set.
     */
    void BooleanSetting(const std::string& property, bool defaultValue);

    /**
     * @brief Displays a slider to set an integer min/max value.
     * @param property The property name in the config.
     * @param defaultValue Default value for the property if not set.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     */
    void IntegerSetting(const std::string& property, int defaultValue, int min, int max);

    /**
     * @brief Displays a slider to set two integer min/max values.
     * @param property1 The first property name in the config.
     * @param property2 The second property name in the config.
     * @param defaultValue1 Default value for the first property if not set.
     * @param defaultValue2 Default value for the second property if not set.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     */
    void IntegerSettingVec(const std::string& property1, const std::string& property2,
                           int defaultValue1, int defaultValue2,
                           int min, int max);

    /**
     * @brief Displays a slider to set a double min/max value.
     * @param property The property name in the config.
     * @param defaultValue Default value for the property if not set.
     * @param min Minimum slider value.
     * @param max Maximum slider value.
     */
    void DoubleSetting(const std::string& property, double defaultValue, double min, double max);

    /**
     * @brief Displays a checkbox to override the window startup position, and if this is selected, displays two sliders.
     */
    void WindowPositionSetting();

    /**
     * @brief Displays a combobox with available audio devices.
     */
    void AudioDeviceSetting();

    /**
     * @brief Displays a reset button and removes the property from the UI map if clicked.
     * @param property1 First property to reset.
     * @param property2 Optional second property to reset.
     */
    void ResetButton(const std::string& property1, const std::string& property2 = "");

    /**
     * @brief Displays a red note and a tooltip when hovered explaining the setting can't be changed now.
     */
    void OverriddenSettingMarker();

    ProjectMGUI& _gui; //!< The GUI subsystem.
    AudioCapture& _audioCapture; //!< The audio capture subsystem.

    bool _visible{false}; //!< Window visibility flag.
    bool _changed{false}; //!< true if the user changed any setting since the last save.

    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> _userConfiguration;
    Poco::AutoPtr<Poco::Util::MapConfiguration> _commandLineConfiguration;

    FileChooser _pathChooser{FileChooser::Mode::Directory}; //!< The file chooser dialog to select preset and texture paths.
};
